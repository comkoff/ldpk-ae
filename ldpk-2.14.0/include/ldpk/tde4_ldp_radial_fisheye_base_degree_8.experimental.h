// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file
//! @brief Base class for radial fisheye distortion.

#include <ldpk/ldpk_ldp_builtin.h>
#include <ldpk/ldpk_generic_radial_distortion_1d.h>
#include <ldpk/ldpk_scale_extender.h>
#include <ldpk/ldpk_mapping_function.h>
#include <ldpk/ldpk_vec2d.h>

//! @brief Base class for radial fisheye distortion.
template <class VEC2,class MAT2,class MAPPING_FUNCTION>
class tde4_ldp_radial_fisheye_base_degree_8:public ldpk::ldp_builtin<VEC2,MAT2>
	{
private:
	typedef VEC2 vec2_type;
	typedef MAT2 mat2_type;
	typedef ldpk::ldp_builtin<VEC2,MAT2> base_type;

	static const int num_parameters = 5;

	ldpk::generic_radial_distortion_1d<num_parameters - 1> _distortion;

	static const char* _para[num_parameters];
	static const int index_begin_distortion = 0;
	static const int index_end_distortion = num_parameters - 1;
// Last parameter: scale. We demand that this parameter only affect
// the size of the fisheye domain, not the shape of the distortion.
	static const int index_scale = num_parameters - 1;
// The mapping function
	MAPPING_FUNCTION _mapfun;
// Set in initializeParameters().
	double _threshold_valid;

	char const* const* get_parameter_names() override
		{ return _para; }
// This function is meant to determine the valid domain of undistort().
// On one hand we will know if the elomgation leaves the domain given
// by the projection, as this is encode in the return value. On the other
// hand we can also study the behaviour of the mapped undistorted elongation.
// If it starts flipping back, we have left the valid domain.
	bool undistort_dn(double r_any_dist_dn,double &r_plain_undist_dn)
		{
// See comment in undistort().
		double theta_dist;
		if(!_mapfun.map_any2angle(r_any_dist_dn,theta_dist))
			{ return false; }
		double theta_undist = _distortion.eval(theta_dist);
// Remap angle to gnomonic
		if(!_mapfun.map_angle2plain(theta_undist,r_plain_undist_dn))
			{ return false; }
		return true;
		}
protected:
	double _r_clip_factor;
// Largest radius for which undistort_dn() is still well-defined.
	double _r_any_dn_domain;
// The domain radius mapped to undistorted. We will try
// to build a simple bounding box from this.
	double _r_plain_dn_domain;

	bool decypher(const char* name,int& i)
		{
		typedef base_type bt;
		int n;
		getNumParameters(n);
		for(i = 0;i < n;++i)
			{
			if(0 == strcmp(name,_para[i]))
				{ return true; }
			}
		return false;
		}
	bool initializeParameters() override
		{
		typedef base_type bt;
		bt::check_builtin_parameters();
// Focal length in dn-coordinates.
		_mapfun.set_fl_dn(bt::fl_cm() / bt::r_fb_cm());
// experimental
		_mapfun.set_r_plain_dn_clip(1000.0);

// We do a simple brute force scan per pass, but refine the search interval
// after each pass. Precision should be around 1e-5 afterwards.
// Not quite sure which value makes sense for radius_b.
// Without any distortion, the result r/f for stereographic is always 2.
// For orthographic we get 1, for equisolid sqrt(2) and for equidistant pi/2.
		double radius_a = 0.0;
// Take threshold from mapping function and add a good bunch.
		double radius_b = 1.5 * _mapfun.threshold_valid();
// At most n samples for each pass.
		int n = 50;
// A few passes
		for(int i_pass = 0;i_pass < 5;++i_pass)
			{
			double r_any_dist_dn_prev = -1.0;
			double r_plain_undist_dn_prev = -1.0;
//std::cout << "radius_a / _mapfun.get_fl_dn(), radius_b / _mapfun.get_fl_dn():" << radius_a / _mapfun.get_fl_dn() << " " << radius_b / _mapfun.get_fl_dn() << std::endl;
// We initialize the domain with the largest radius possible.
// In practice, the loop will terminate prior to reaching n,
// because at some point undistort_dn() will return false,
// so this most likely is not relevant, but we want to make
// sure that _r_any_dn_domain has some well-defined value.
			_r_any_dn_domain = radius_b;
			double r_any_dist_dn;
			for(int i = 0;i <= n;++i)
				{
// Normalized iterator.
				double q = double(i) / n;
// Scan r_any_dist_dn from radius_a to radius_b, inclusive.
				r_any_dist_dn = radius_a * (1.0 - q) + radius_b * q;
				double r_plain_undist_dn;
				if(!undistort_dn(r_any_dist_dn,r_plain_undist_dn))
					{
// undistort fails, so we use domain radius and mapped radius from the previous round.
					_r_any_dn_domain = r_any_dist_dn_prev;
					_r_plain_dn_domain = r_plain_undist_dn_prev;
					break;
					}
// Found a turning point? We are looking for the point where the Jacobian degenerates,
// but this criterions is faster and simpler than evaluating the Jacobian.
				if(r_plain_undist_dn <= r_plain_undist_dn_prev)
					{
// The last known radius for which undistort_dn() was well-defined.
					_r_any_dn_domain = r_any_dist_dn_prev;
					_r_plain_dn_domain = r_plain_undist_dn_prev;
					break;
					}
// r_any_dist_dn has successfully been mapped into r_plain_undist_dn.
// Our current radii become the previous radii for the next round.
				r_any_dist_dn_prev = r_any_dist_dn;
				r_plain_undist_dn_prev = r_plain_undist_dn;
// And we can extend our domain.
				_r_any_dn_domain = r_any_dist_dn;
				_r_plain_dn_domain = r_plain_undist_dn;
				}
// Refine search interval. radius_a is always in the valid domain.
			radius_a = r_any_dist_dn_prev;
			radius_b = r_any_dist_dn;
//std::cout << "r_domain / _mapfun.get_fl_dn(): " << _r_any_dn_domain / _mapfun.get_fl_dn() << std::endl;
			}
// Domain radii as defined in the mapping functions.
		_threshold_valid = _mapfun.threshold_valid();
		return true;
		}
	bool getNumParameters(int& n) override
		{
		n = num_parameters;
		return true;
		}
	bool setParameterValue(const char *identifier,double v) override
		{
		typedef base_type bt;
		int i;
// Does the base class know the parameter?
		if(bt::set_builtin_parameter_value(identifier,v))
			{ return true; }
		if(!decypher(identifier,i))
			{ return false; }
		if((i >= index_begin_distortion) && (i < index_end_distortion))
			{
			if(_distortion.get_coeff(i) != v)
				{ bt::no_longer_uptodate_lut(); }
			_distortion.set_coeff(i,v);
			}
		else if(i == index_scale)
			{
			if(_mapfun.get_scale() != v)
				{ bt::no_longer_uptodate_lut(); }
			_mapfun.set_scale(v);
			}
		return true;
		}
	double getModelDoubleParameterValueByIndex(int i) const override
		{
		if((i >= index_begin_distortion) && (i < index_end_distortion))
			{ return _distortion.get_coeff(i); }
		else if(i == index_scale)
			{ return _mapfun.get_scale(); }
		throw std::out_of_range(std::to_string(i));
		}
	void getBoundingBoxUndistort(double xa_in,double ya_in,double xb_in,double yb_in,double& xa_out,double& ya_out,double& xb_out,double& yb_out,int nx,int ny) override
		{
		typedef base_type bt;
// _r_plain_dn_domain represents the radius of the undistorted image in dn-coordinates.
// We form a square and use its corners as the bounding box for the undistorted image.
//		vec2_type p00_dn = vec2_type(-_r_plain_dn_domain,-_r_plain_dn_domain);
//		vec2_type p11_dn = vec2_type( _r_plain_dn_domain, _r_plain_dn_domain);

//		vec2_type p00_unit = bt::map_dn_to_unit(p00_dn);
//		vec2_type p11_unit = bt::map_dn_to_unit(p11_dn);

//		xa_out = p00_unit[0];
//		ya_out = p00_unit[1];
//		xb_out = p11_unit[0];
//		yb_out = p11_unit[1];

		xa_out = 0.0;
		ya_out = 0.0;
		xb_out = 1.0;
		yb_out = 1.0;

//		std::cout << "getBoundingBoxUndistort: " << xa_out << " " << ya_out << " " << xb_out << " " << yb_out << std::endl;
		}
	void getBoundingBoxDistort(double xa_in,double ya_in,double xb_in,double yb_in,double& xa_out,double& ya_out,double& xb_out,double& yb_out,int nx,int ny) override
		{
		typedef base_type bt;

// _r_any_dn_domain is the radius of the valid domain of undistort() in dn-coordinates.
// We form a square and use its corners as the bounding box for the distorted image.
//		vec2_type p00_dn = vec2_type(-_r_any_dn_domain,-_r_any_dn_domain);
//		vec2_type p11_dn = vec2_type( _r_any_dn_domain, _r_any_dn_domain);

//		vec2_type p00_unit = bt::map_dn_to_unit(p00_dn);
//		vec2_type p11_unit = bt::map_dn_to_unit(p11_dn);

//		xa_out = p00_unit[0];
//		ya_out = p00_unit[1];
//		xb_out = p11_unit[0];
//		yb_out = p11_unit[1];

		xa_out = 0.0;
		ya_out = 0.0;
		xb_out = 1.0;
		yb_out = 1.0;

//		std::cout << "getBoundingBoxDistort: " << xa_out << " " << ya_out << " " << xb_out << " " << yb_out << std::endl;
		}

// Overwriting tde4_ldp_common
	bool undistort(double x0,double y0,double &x1,double &y1) override
		{
		typedef base_type bt;

		vec2_type p_any_dist_dn = bt::map_unit_to_dn(vec2_type(x0,y0));

// Distance from origin in dn-coordinates
		double r_any_dist_dn = norm2(p_any_dist_dn);
// We have determined the domain of undistort() in initializeParameters().
		if(r_any_dist_dn > _r_any_dn_domain)
			{ return false; }
// The literature commonly says, undistorting should be done
// before applying the remapping. Usually it's applied to
// elongation angle theta. This is what we do here: We map
// the radial position to the corresponding polar angle,
// undistort the angle and map it to a radius in planar coordinates.
// This guarantees that scale only affects the domain size, not the shape
// of the distortion.
		double theta_dist;
		if(!_mapfun.map_any2angle(r_any_dist_dn,theta_dist))
			{ return false; }
		double theta_undist = _distortion.eval(theta_dist);
// Remap angle to gnomonic
		double r_plain_undist_dn;
		if(!_mapfun.map_angle2plain(theta_undist,r_plain_undist_dn))
			{ return false; }

// Calc undistorted point from undistorted radius
		vec2_type p_plain_undist_dn;
		if(dotsq(p_any_dist_dn) == 0.0)
			{ p_plain_undist_dn = vec2_type(0,0); }
		else
			{ p_plain_undist_dn = r_plain_undist_dn * unit(p_any_dist_dn); }
// Map back to unit coordinates		
		vec2_type p_plain_undist_unit = bt::map_dn_to_unit(p_plain_undist_dn);
		x1 = p_plain_undist_unit[0];
		y1 = p_plain_undist_unit[1];
		return true;
		}
	bool distort(double x0,double y0,double &x1,double &y1) override
		{
		typedef base_type bt;

		vec2_type p_plain_undist_dn = bt::map_unit_to_dn(vec2_type(x0,y0));
// Distance from origin in dn-coordinates
		double r_plain_undist_dn = norm2(p_plain_undist_dn);
// We have determined the domain of undistort() in initializeParameters().
// Using the undistorted radius, we can formulate a criterion for distort().
		if(r_plain_undist_dn > _r_plain_dn_domain)
			{ return false; }
// Map gnomonic to undistorted angle
		double theta_undist = _mapfun.map_plain2angle(r_plain_undist_dn);
// Apply distortion. We use the point itself as initial value, since coefficients will be small.
		double theta_dist = _distortion.eval_inv(theta_undist,theta_undist);
// Map distorted angle to fisheye domain.
		double r_any_dist_dn = _mapfun.map_angle2any(theta_dist);
// Calc distorted point from distorted radius
		vec2_type p_any_dist_dn;
		if(dotsq(p_plain_undist_dn) == 0.0)
			{ p_any_dist_dn = vec2_type(0,0); }
		else
			{ p_any_dist_dn = r_any_dist_dn * unit(p_plain_undist_dn); }
// Map back to unit coordinates		
		vec2_type p_any_dist_unit = bt::map_dn_to_unit(p_any_dist_dn);
		x1 = p_any_dist_unit[0];
		y1 = p_any_dist_unit[1];
		return true;
		}
public:
// Mutex initialized and destroyed in baseclass.
	tde4_ldp_radial_fisheye_base_degree_8():_r_clip_factor(50.0),_r_any_dn_domain(0.0)
		{ }
	~tde4_ldp_radial_fisheye_base_degree_8()
		{ }
//! @brief Fisheye lenses are not rectilinear.
	bool isRectilinear() const override
		{ return false; }
	double r_clip_factor() const
		{ return _r_clip_factor; }
	void r_clip_factor(double f)
		{ _r_clip_factor = f; }
	virtual bool getModelName(char *name) override = 0;
	bool getParameterType(const char* identifier,tde4_ldp_ptype& ptype) override
		{
		typedef base_type bt;
		int i;
		if(bt::get_builtin_parameter_type(identifier,ptype)) return true;
		if(!decypher(identifier,i)) return false;
		ptype = TDE4_LDP_ADJUSTABLE_DOUBLE;
		return true;
		}
	bool getParameterDefaultValue(const char* identifier,double& v) override
		{
		typedef base_type bt;
		int i;
		if(!decypher(identifier,i)) return false;
		if((i >= index_begin_distortion) && (i < index_end_distortion))
			{ v = 0.0; }
		else if(i == index_scale)
			{ v = 1.0; }
		return true;
		}
	bool getParameterRange(const char* identifier,double& a,double& b) override
		{
		typedef base_type bt;
		int i;
		if(!decypher(identifier,i)) return false;
		if((i >= index_begin_distortion) && (i < index_end_distortion))
			{
			a = -0.5;
			b = 0.5;
			}
		else if(i == index_scale)
			{
			a = 0.25;
			b = 4.0;
			}
		return true;
		}
	bool getJacobianMatrix(double x0, double y0, double &m00, double &m01, double &m10, double &m11) override
		{
		typedef base_type bt;

		vec2_type p_dn = bt::map_unit_to_dn(vec2_type(x0,y0));
		double norm_p_dn = norm2(p_dn);
		vec2_type unit_p_dn = unit(p_dn);
		mat2_type tensq_unit_p_dn = tensq(unit_p_dn);
// According to our document we need several intermediate quantities.
// The distorted angle.
		double F_inv_at_norm_p_dn;
		if(!_mapfun.map_any2angle(norm_p_dn,F_inv_at_norm_p_dn))
			{ return false; }
// The undistorted angle.
		double g_poly_rad_at_theta_dist = _distortion.eval(F_inv_at_norm_p_dn);
// The remapped radius
		double G_at_g_poly_rad_at_theta_dist;
		_mapfun.map_angle2plain(g_poly_rad_at_theta_dist,G_at_g_poly_rad_at_theta_dist);
// The derivative of the inverse mapping function at |p|.
		double dF_inv_by_dr_dn;
		if(!_mapfun.derive_any2angle(norm_p_dn,dF_inv_by_dr_dn))
			{ return false; }
// The derivative of the distortion polynomial at F_inv_at_norm_p_dn.
		double dg_poly_rad_by_theta = _distortion.jacobi(F_inv_at_norm_p_dn);
// The derivative of the remapping function at g_poly_rad_at_theta_dist.
		double dG_by_theta = _mapfun.derive_angle2plain(g_poly_rad_at_theta_dist);
// Apply the Leibniz chain rule
		double dg_any_rad_by_dr = dG_by_theta * dg_poly_rad_by_theta * dF_inv_by_dr_dn;
// Extend to 2-dim space.
		mat2_type m;
		if(norm_p_dn == 0.0)
			{ m = mat2_type(1.0); }
		else
			{ m = 1.0 / norm_p_dn * (mat2_type(1.0) - tensq_unit_p_dn) * G_at_g_poly_rad_at_theta_dist + tensq_unit_p_dn * dg_any_rad_by_dr; }

// unit to diagnorm and vice versa as transforms for the Jacobian.
		mat2_type u2d(bt::w_fb_cm() / bt::r_fb_cm(),0.0,0.0,bt::h_fb_cm() / bt::r_fb_cm());
		mat2_type d2u(bt::r_fb_cm() / bt::w_fb_cm(),0.0,0.0,bt::r_fb_cm() / bt::h_fb_cm());
		m = d2u * m * u2d;
		m00 = m[0][0];m01 = m[0][1];m10 = m[1][0];m11 = m[1][1];
		return true;
		}
	};

template <class VEC2,class MAT2,class MAPPING_FUNCTION>
const char* tde4_ldp_radial_fisheye_base_degree_8<VEC2,MAT2,MAPPING_FUNCTION>::_para[num_parameters] = {
	"C2 - Degree 2",
	"C4 - Degree 4",
	"C6 - Degree 6",
	"C8 - Degree 8",
	"Scale"
	};
