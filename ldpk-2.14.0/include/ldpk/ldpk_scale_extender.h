// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file
//! @brief Scale extender for rescaled anamorphic models.

#include <ldpk/ldpk_extender_base.h>

namespace ldpk
	{
//! @brief The scale-extender scales the optical-axis-perpendicular coordinates
//! of the incident ray towards the optical axis, uniformly.
	template <class VEC2,class MAT2>
	class scale_extender:public extender_base<VEC2,MAT2>
		{
	public:
		typedef VEC2 vec2_type;
		typedef MAT2 mat2_type;
	private:
		double _sc;
		mat2_type _m_sc,_inv_m_sc;

	public:
		scale_extender()
			{
			_sc = 1.0;
			_m_sc = mat2_type(1.0);
			_inv_m_sc = mat2_type(1.0);
			}
// The scale extender has one parameter called sc.
		void set_sc(double sc)
			{
			_sc = sc;
			_m_sc = mat2_type(_sc,0.0,0.0,_sc);
			_inv_m_sc = mat2_type(1.0 / _sc,0.0,0.0,1.0 / _sc);
			}
		double get_sc() const
			{ return _sc; }

//! eval() is per definition removal of lens distortion (undistort).
		vec2_type eval(const vec2_type& p) const
			{ return _m_sc * p; }
//! eval_inv() is applying lens distortion (distort)
		vec2_type eval_inv(const vec2_type& q) const
			{ return _inv_m_sc * q; }
//! Generally (but not here), an initial value is needed for calculating the inverse.
		vec2_type eval_inv(const vec2_type& q,const vec2_type& p_start) const
			{ return _inv_m_sc * q; }
//! The matrix for this extender
		const mat2_type& get_mat() const
			{ return _m_sc; }
//! The inverse matrix for this extender
		const mat2_type& get_mat_inv() const
			{ return _inv_m_sc; }
		};
	}
