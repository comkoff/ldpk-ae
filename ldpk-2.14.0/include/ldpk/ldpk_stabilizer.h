// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file
//! @brief The mathematics for compensating (planar) stabilizer artefacts.

#include <ldpk/ldpk_ldp_builtin.h>
#include <ldpk/ldpk_vec3d.h>

namespace ldpk
	{
//! @brief The mathematics for compensating (planar) stabilizer artefacts.
//! Required by tde4_ldp_radial_stabilizer_degree_6.h
//! and tde4_ldp_anamorphic_stabilizer_degree_6.h.
	template <class VEC2,class MAT2>
	class stabilizer
		{
	public:
		typedef VEC2 vec2_type;
		typedef MAT2 mat2_type;
	private:
		typedef ldpk::vec3d vec3_type;
		typedef ldpk::mat3d mat3_type;
// Copies of the surrounding ld-model parameters.
// Default values have no meaning; the object *must* be initialized
// by means of init() which is called by the model class in initializeParameters().
		double _w_fb_cm = 4.0;
		double _h_fb_cm = 3.0;
		double _r_fb_cm = 2.5;
		double _fl_cm = 5.0;
		double _x_lco_cm = 0.0;
		double _y_lco_cm = 0.0;
// Specific parameters for this class.
		double _stabilizer_x = 0.0,_stabilizer_y = 0.0;
// This matrix rotates the ray from unprojecting the lens center
// including the stabilizers into a ray from unprojecting the lens center
// excluding the stabilizers. _r_restabilize is applied in undistort()
		mat3_type _r_restabilize;
// We can express the entire transform as a homography
		mat3_type _H,_H_inv;

	protected:
	public:
//! @brief This method is invoked only by initializeParameters() of the stabilizer models.
		void init(double w_fb_cm,double h_fb_cm,double r_fb_cm,double fl_cm,double x_lco_cm,double y_lco_cm)
			{
			_w_fb_cm = w_fb_cm;
			_h_fb_cm = h_fb_cm;
			_r_fb_cm = r_fb_cm;
			_fl_cm = fl_cm;
			_x_lco_cm = x_lco_cm;
			_y_lco_cm = y_lco_cm;
// Handle stabilizers. First we build the two rays from image center
// one with stabilizer and one without. In centimeter.
			vec3_type c_with_stb(
				-_stabilizer_x * _w_fb_cm,
				-_stabilizer_y * _h_fb_cm,
				-_fl_cm
				);
			vec3_type c_wout_stb(
				0.0,
				0.0,
				-_fl_cm
				);
// Axis of rotation
			vec3_type axis_stb = wdg(c_with_stb,c_wout_stb);
// Angle that rotates c_with_stb into c_wout_stb.
			double phi_stb = ::atan2(norm2(axis_stb),dot(c_with_stb,c_wout_stb));
			if(::fabs(phi_stb) < 1e-12)
				{ _r_restabilize = mat3_type(1.0); }
			else
				{ _r_restabilize = make_rot3_from_angle_and_axis(phi_stb,axis_stb); }
// We describe the evaluator eval() as a homography of the form H = B * R * A,
// where R is the rotation matrix _r_restabilize divided by its lower right component,
// which turns it into homography. A and B are homographies and hence by
// the group property H is a homography.
			mat3_type A(
				_r_fb_cm,	0.0,		0.0,
				0.0,		_r_fb_cm,	0.0,
				0.0,		0.0,		-_fl_cm);
			mat3_type R = _r_restabilize;
			mat3_type B(
				-_fl_cm,	0.0,		-_stabilizer_x * _w_fb_cm,
				0.0,		-_fl_cm,	-_stabilizer_y * _h_fb_cm,
				0.0,		0.0,		_r_fb_cm);
			_H = B * R * A;
			_H_inv = invert(_H);
// Normalized form
			_H /= _H[2][2];
			_H_inv /= _H_inv[2][2];
			}
//! @brief Setter for x_stb_unit
		void stabilizer_x(double v)
			{ _stabilizer_x = v; }
//! @brief Setter for y_stb_unit
		void stabilizer_y(double v)
			{ _stabilizer_y = v; }
//! @brief Getter for x_stb_unit
		double stabilizer_x() const
			{ return _stabilizer_x; }
//! @brief Getter for y_stb_unit
		double stabilizer_y() const
			{ return _stabilizer_y; }

//! @brief Map unit coordinates into diagonally normalized coordinates.
//! This is usually done by some base class of the distortion model classes
//! but the situation for stabilizer models is slightly different,
//! because stabilizer models are centered around lens center + stabilizer * filmback
		vec2_type map_unit_to_dn(const vec2_type& p_unit) const
			{
			vec2_type p_cm( (p_unit[0] - 1.0/2.0 - stabilizer_x()) * _w_fb_cm - _x_lco_cm,
					(p_unit[1] - 1.0/2.0 - stabilizer_y()) * _h_fb_cm - _y_lco_cm);
			return p_cm / _r_fb_cm;
			}
//! @brief Map diagonally normalized coordinates to unit coordinates.
//! Inverse function of map_unit_to_dn().
		vec2_type map_dn_to_unit(const vec2_type& p_dn) const
			{
			vec2_type p_cm(p_dn * _r_fb_cm);
			p_cm += vec2_type(      _w_fb_cm * (1.0/2.0 + stabilizer_x()) + _x_lco_cm,
						_h_fb_cm * (1.0/2.0 + stabilizer_y()) + _y_lco_cm);
			return vec2_type(	p_cm[0] / _w_fb_cm,
						p_cm[1] / _h_fb_cm);
			}
//! @brief Apply compensation for stabilizer artefacts.
		vec2_type eval(vec2_type const& p_dn_in) const
			{
// Split homography into components
			mat2_type h(_H[0][0],_H[0][1],_H[1][0],_H[1][1]);
			vec2_type v(_H[0][2],_H[1][2]);
			vec2_type q(_H[2][0],_H[2][1]);
			return (h * p_dn_in + v) / (dot(q,p_dn_in) + 1.0);
			}
//! @brief Inverse function of eval().
		vec2_type eval_inv(vec2_type const& p_dn_in) const
			{
// Split homography into components
			mat2_type h(_H_inv[0][0],_H_inv[0][1],_H_inv[1][0],_H_inv[1][1]);
			vec2_type v(_H_inv[0][2],_H_inv[1][2]);
			vec2_type q(_H_inv[2][0],_H_inv[2][1]);
			return (h * p_dn_in + v) / (dot(q,p_dn_in) + 1.0);
			}
//! @brief To be tested. Might be transposed. Nudge me please if you nee clarity about this.
		mat2_type jacobi(vec2_type const& p_dn_in) const
			{
// Split homography into components. This part is confirmed in eval.
			mat2_type h(_H[0][0],_H[0][1],_H[1][0],_H[1][1]);
			vec2_type v(_H[0][2],_H[1][2]);
			vec2_type q(_H[2][0],_H[2][1]);

			double dot_q_p_plus_1 = dot(q,p_dn_in) + 1.0;
			return (h * dot_q_p_plus_1 - ten(h * p_dn_in + v,q)) / (dot_q_p_plus_1 * dot_q_p_plus_1);
			}
		};
	}
