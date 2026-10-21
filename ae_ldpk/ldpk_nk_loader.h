// LDPK-to-AfterEffects bridge.
// Loads an LD_3DE4 node block from a .nk file and feeds the parameters
// into an LDPK model, honouring the FOV mapping implemented by the
// Nuke plugins (see source/ldpk/nuke_ld_3de4_base.C, engine()).
//
// Software is provided "as is" - no warranties implied.

#pragma once

#include "ldpk_nk_block.h"
#include "ldpk_model_registry.h"

#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

namespace ae_ldpk
	{
//! @brief Nukify a 3DE4 parameter name exactly like the exporter script
//! (export_nuke_LD_3DE4_Lens_Distortion_Node.py::nukify_name):
//! - empty string maps to "_"
//! - leading digit gets an underscore prefix
//! - runs of non-alphanumeric chars (except '/') collapse to a single '_'
	inline std::string nukify_name(const std::string& s)
		{
		if(s.empty())
			{ return "_"; }
		std::string t;
		if(isdigit((unsigned char)s[0]))
			{ t += '_'; }
		bool pending_sep = false;
		for(size_t i = 0;i < s.size();++i)
			{
			unsigned char c = (unsigned char)s[i];
			if(c == '/')
				{
// A slash survives nukification (see exporter regex "[/+,.:; _-]+",
// but '/' is handled separately there); keep it verbatim.
				if(pending_sep)
					{ t += '_';pending_sep = false; }
				t += (char)c;
				continue;
				}
			if(isalnum(c))
				{
				if(pending_sep)
					{ t += '_';pending_sep = false; }
				t += (char)c;
				}
			else
				{ pending_sep = true; }
			}
		if(pending_sep)
			{ t += '_'; }
		return t;
		}

//! @brief Parse an LD_3DE4 node block out of an arbitrary .nk script.
//! The 3DE4 exporter writes a standalone block, but a full
//! Nuke script contains the node among the others; the plain parse()
//! requires the class name to be the very first token. Scan the text for
//! a KNOWN class name followed by '{' and parse from there. Returns false
//! with an error text when no supported node block is found.
	inline bool parse_nk_script(std::istream& in,nk_ld_node& node,std::string& err)
		{
		std::string content((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
		size_t pos = 0;
		for(;;)
			{
			size_t p = content.find("LD_3DE",pos);
			if(p == std::string::npos)
				{ err = "no LD_3DE lens distortion node found";return false; }
// Extract the full identifier around the hit and require a word boundary,
// a '{' after it and a class we actually support.
			size_t e = p;
			while((e < content.size()) && (isalnum((unsigned char)content[e]) || (content[e] == '_')))
				{ ++e; }
			std::string cls = content.substr(p,e - p);
			size_t j = e;
			while((j < content.size()) && isspace((unsigned char)content[j]))
				{ ++j; }
			if((j < content.size()) && (content[j] == '{') && create_model_by_nuke_class(cls))
				{
				std::istringstream ss(content.substr(p));
				return node.parse(ss,err);
				}
			pos = e;
			}
		}

//! @brief A distortion model loaded from a .nk node block, evaluated
//! at a given frame. Handles the FOV remapping exactly like the Nuke node:
//!   p_fov = (p_unit - fov_a) / fov_size
//!   q_fov = model(p_fov)
//!   q_unit = q_fov * fov_size + fov_a
	class distortion_from_nk
		{
		private:
			ldm_type* _ldm;
			nk_ld_node _node;
			double _xa_fov,_ya_fov,_xb_fov,_yb_fov;
			double _xd_fov,_yd_fov;
			bool _undistort_direction;
		public:
//! @brief Direct access to the underlying model (for pushing
//! parameter values from the AE UI into the LDPK model).
			ldm_type* model()
				{ return _ldm; }
			const ldm_type* model() const
				{ return _ldm; }
		private:

			double map_in_fov_x(double x_unit) const
				{ return  (x_unit - _xa_fov) / _xd_fov; }
			double map_in_fov_y(double y_unit) const
				{ return  (y_unit - _ya_fov) / _yd_fov; }
			double map_out_fov_x(double x_fov) const
				{ return x_fov * _xd_fov + _xa_fov; }
			double map_out_fov_y(double y_fov) const
				{ return y_fov * _yd_fov + _ya_fov; }

		public:
			distortion_from_nk():_ldm(0),_xa_fov(0.0),_ya_fov(0.0),_xb_fov(1.0),_yb_fov(1.0),_xd_fov(1.0),_yd_fov(1.0),_undistort_direction(true)
				{ }
			~distortion_from_nk()
				{ if(_ldm) delete _ldm; }

			const nk_ld_node& node() const
				{ return _node; }
			bool is_undistort() const
				{ return _undistort_direction; }
//! @brief Change direction at runtime without reloading the file.
//! true = undistort (default of the 3DE4 exporter), false = distort.
			void set_direction(bool undistort)
				{ _undistort_direction = undistort; }

//! @brief Build model from file. frame is the Nuke frame number for
//! animated parameters. Returns error text (empty means success). The
//! node block may sit anywhere in the file (see parse_nk_script).
			std::string load(const std::string& path,int frame)
				{
				if(_ldm)
					{ delete _ldm;_ldm = 0; }
				std::ifstream in(path.c_str());
				if(!in)
					{ return "cannot open file: " + path; }
				std::string err;
				nk_ld_node node;
				if(!parse_nk_script(in,node,err))
					{ return err; }
				return adopt(node,frame);
				}

//! @brief Build the model from an already parsed node (no file access).
//! Used by the AE plugin render path, where the node comes from the
//! sequence-data snapshot instead of being re-read per render.
			std::string adopt(const nk_ld_node& node,int frame)
				{ return adopt_as(node,node.nuke_class,frame); }

//! @brief Build the model from a parsed node, but instantiate the given
//! class: the Model popup may select a different UI-visible
//! class than the snapshot. FOV and the 7 built-in parameters still come
//! from the node; model parameters not present in the node stay at their
//! LDPK defaults. Returns error text (empty means success).
			std::string adopt_as(const nk_ld_node& node,const std::string& model_class,int frame)
				{
				if(_ldm)
					{ delete _ldm;_ldm = 0; }
				_node = node;
				_ldm = create_model_by_nuke_class(model_class);
				if(!_ldm)
					{ return "unknown node class: " + model_class; }
				_undistort_direction = (_node.direction != "distort");
// 7 built-in parameters first (same order as nuke_ld_3de4_base.C::_validate)
				_ldm->setParameterValue("tde4_focal_length_cm",		_node.get_double("tde4_focal_length_cm",1.0,frame));
				_ldm->setParameterValue("tde4_custom_focus_distance_cm",_node.get_double("tde4_custom_focus_distance_cm",100.0,frame));
				_ldm->setParameterValue("tde4_filmback_width_cm",	_node.get_double("tde4_filmback_width_cm",0.8,frame));
				_ldm->setParameterValue("tde4_filmback_height_cm",	_node.get_double("tde4_filmback_height_cm",0.6,frame));
				_ldm->setParameterValue("tde4_lens_center_offset_x_cm",	_node.get_double("tde4_lens_center_offset_x_cm",0.0,frame));
				_ldm->setParameterValue("tde4_lens_center_offset_y_cm",	_node.get_double("tde4_lens_center_offset_y_cm",0.0,frame));
				_ldm->setParameterValue("tde4_pixel_aspect",		_node.get_double("tde4_pixel_aspect",1.0,frame));
// Model-specific parameters. Static values are applied directly;
// for animated parameters we bake the value at the requested frame.
				char name_par[256];
				int num_par;
				_ldm->getNumParameters(num_par);
				for(int i = 0;i < num_par;++i)
					{
					_ldm->getParameterName(i,name_par);
					std::string key(name_par);
// The exporter writes nukified names ("Distortion - Degree 2" becomes
// "Distortion_Degree_2"), so look the key up nukified. Without this the
// parameters are silently skipped and the model stays at its defaults
// (identity mapping -> "no effect on the image").
					const nk_value* v = _node.find(nukify_name(key));
					if(!v)
						{ continue; }
					double val = v->eval(frame);
					_ldm->setParameterValue(name_par,val);
					}
// FOV knobs, defaults as in nuke_ld_3de4_base.C
				_xa_fov = _node.get_double("field_of_view_xa_unit",0.0,frame);
				_ya_fov = _node.get_double("field_of_view_ya_unit",0.0,frame);
				_xb_fov = _node.get_double("field_of_view_xb_unit",1.0,frame);
				_yb_fov = _node.get_double("field_of_view_yb_unit",1.0,frame);
				_xd_fov = _xb_fov - _xa_fov;
				_yd_fov = _yb_fov - _ya_fov;
				if((_xd_fov == 0.0) || (_yd_fov == 0.0))
					{ return "degenerate field of view"; }
				_ldm->initializeParameters();
				return "";
				}

//! @brief Build the model from a class name only (no .nk file).
//! Used by the AE plugin when the user has picked a model in the popup
//! but has not imported a .nk file: built-in parameters and model
//! parameters stay at their LDPK defaults, FOV = [0..1]^2 (the
//! constructor default). Returns error text (empty means success).
			std::string adopt_class(const std::string& nuke_class)
				{
				nk_ld_node empty;
				empty.nuke_class = nuke_class;
				empty.direction = "undistort";
				return adopt_as(empty,nuke_class,1);
				}

//! @brief Bounding box (unit coordinates) of a unit-space rectangle
//! mapped through the distortion. Mirrors nuke_ld_3de4_base::bounds():
//! undistort direction -> getBoundingBoxUndistort, distort -> Distort.
//! NOTE: not thread-safe against apply() on the SAME instance; the AE
//! plugin builds one distortion_from_nk per render, so this is fine.
			bool bounding_box(bool undistort_dir,double xa_in,double ya_in,double xb_in,double yb_in,
				double& xa_out,double& ya_out,double& xb_out,double& yb_out) const
				{
				if(!_ldm)
					{ return false; }
				double xa_f = map_in_fov_x(xa_in),ya_f = map_in_fov_y(ya_in);
				double xb_f = map_in_fov_x(xb_in),yb_f = map_in_fov_y(yb_in);
				double xa,ya,xb,yb;
				if(undistort_dir)
					{ _ldm->getBoundingBoxUndistort(xa_f,ya_f,xb_f,yb_f,xa,ya,xb,yb,8,8); }
				else
					{ _ldm->getBoundingBoxDistort(xa_f,ya_f,xb_f,yb_f,xa,ya,xb,yb,8,8); }
				xa_out = map_out_fov_x(xa);ya_out = map_out_fov_y(ya);
				xb_out = map_out_fov_x(xb);yb_out = map_out_fov_y(yb);
				return true;
				}

//! @brief Map unit coordinates (as used by the Nuke node) through the
//! distortion. In the Nuke node "undistort" applies model->distort()
//! and "distort" applies model->undistort() because image processing
//! fetches pixels along the inverse mapping.
			bool apply(double x0_unit,double y0_unit,double& x1_unit,double& y1_unit) const
				{
				if(!_ldm)
					{ return false; }
				double x0_fov = map_in_fov_x(x0_unit);
				double y0_fov = map_in_fov_y(y0_unit);
				double x1_fov,y1_fov;
				bool ok;
				if(_undistort_direction)
					{ ok = _ldm->distort(x0_fov,y0_fov,x1_fov,y1_fov); }
				else
					{ ok = _ldm->undistort(x0_fov,y0_fov,x1_fov,y1_fov); }
				if(!ok)
					{ return false; }
				x1_unit = map_out_fov_x(x1_fov);
				y1_unit = map_out_fov_y(y1_fov);
				return true;
				}

//! @brief apply() plus a round-trip residual check.
//! In the undistort direction the inverse mapping is computed by a
//! Newton iteration (generic_distortion_base::eval_inv). For output
//! points outside the image of the model function (e.g. strong negative
//! distortion folds the polynomial) NO solution exists; the iteration
//! then lands on an arbitrary point while still reporting success. Such
//! a bogus point may accidentally fall inside the frame window, where a
//! plain range check cannot catch it -> salt-and-pepper noise.
//! Remedy: push the result through the (non-iterative, exact) forward
//! mapping and require that it reproduces the input point. A converged
//! Newton result has a residual <= 1e-8 (its own termination criterion),
//! a diverged one typically 0.1..1. The threshold 1e-4 (fov units) sits
//! four orders of magnitude away from both. In the distort direction the
//! forward polynomial is evaluated directly, so no check is needed.
//!
//! Additionally the candidate must preserve orientation: the Jacobian
//! determinant of the forward mapping at the candidate must be positive.
//! Near a fold the polynomial has TWO branches; the outer one maps
//! points "inside-out" (negative determinant) and would survive the
//! residual check while producing mirrored garbage. The determinant is
//! available through getJacobianMatrix().
//!
//! If the first attempt (Newton started at the point itself) fails,
//! we retry with starts scaled towards the lens center (0.7 and 0.4
//! of the offset). This rescues near-fold points, without any
//! per-pixel randomness.
//!
//! ALL arithmetic happens in fov coordinates —
//! the model maps fov -> fov, so the Newton start values and the
//! candidate check must be fov-based as well; only the final result is
//! mapped back through map_out_fov.
			bool apply_checked(double x0_unit,double y0_unit,double& x1_unit,double& y1_unit) const
				{
				if(!_undistort_direction)
					{ return apply(x0_unit,y0_unit,x1_unit,y1_unit); }
// Undistort direction. The lookup table is BYPASSED on purpose: its
// cells are built by a Newton iteration which diverges for points
// outside the image of the model function (folded polynomial), and a
// diverged cell poisons every pixel that takes its initial value from
// it (block-shaped holes at the frame border).
//
// Instead we always start Newton from the output point itself
// (p_start = q, in fov coordinates). For any q inside the disc of the
// model image this start lies on the INNER (physical) branch of the
// polynomial, from where Newton converges monotonically to the correct
// root. This is deterministic, per-pixel and independent of any cached
// state.
				double x0_fov = map_in_fov_x(x0_unit);
				double y0_fov = map_in_fov_y(y0_unit);
				double xr_fov,yr_fov;
				if(_ldm->distort(x0_fov,y0_fov,x0_fov,y0_fov,xr_fov,yr_fov)
					&& check_candidate(x0_fov,y0_fov,xr_fov,yr_fov))
					{
					x1_unit = map_out_fov_x(xr_fov);
					y1_unit = map_out_fov_y(yr_fov);
					return true;
					}
// Fallback for points whose start was outside the disc (frame corners
// with strong negative distortion): scale the start towards the lens
// center, where the branch is guaranteed physical. The lens center is
// (0.5, 0.5) in fov coordinates by construction.
				const double cx_fov = 0.5,cy_fov = 0.5;
				const double starts[2] = { 0.7,0.4 };
				for(int i = 0;i < 2;++i)
					{
					const double sx = cx_fov + starts[i] * (x0_fov - cx_fov);
					const double sy = cy_fov + starts[i] * (y0_fov - cy_fov);
					if(_ldm->distort(x0_fov,y0_fov,sx,sy,xr_fov,yr_fov)
						&& check_candidate(x0_fov,y0_fov,xr_fov,yr_fov))
						{
						x1_unit = map_out_fov_x(xr_fov);
						y1_unit = map_out_fov_y(yr_fov);
						return true;
						}
					}
				return false;
				}
		private:
//! @brief Round-trip + orientation check of a candidate in fov coordinates.
			bool check_candidate(double x0_fov,double y0_fov,double x1_fov,double y1_fov) const
				{
				double xr_fov,yr_fov;
				if(!_ldm->undistort(x1_fov,y1_fov,xr_fov,yr_fov))
					{ return false; }
				const double dx = xr_fov - x0_fov;
				const double dy = yr_fov - y0_fov;
				if((dx * dx + dy * dy) >= 1e-8) // (1e-4)^2
					{ return false; }
				double m00,m01,m10,m11;
				if(!_ldm->getJacobianMatrix(x1_fov,y1_fov,m00,m01,m10,m11))
					{ return false; }
				return (m00 * m11 - m01 * m10) > 0.0;
				}
		public:
		};
	}
