// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file ldpk_anamorphic_homomorphic_distortion.h
//! @brief An experimental anamorphic model of degree 2.

#include <ldpk/ldpk_generic_distortion_base.h>
#include <iostream>

namespace ldpk
	{
//! @brief The homomorphic radially symmetric model of degree 2 with decentering.
	template <class VEC2,class MAT2>
	class anamorphic_homomorphic_distortion:public ldpk::generic_distortion_base<VEC2,MAT2,3>
		{
	public:
		typedef generic_distortion_base<VEC2,MAT2,3> base_type;
		typedef VEC2 vec2_type;
		typedef MAT2 mat2_type;
	private:
// union allows to access coefficients by index.
		union
			{
			struct
				{
				double _cx2,_cy2,_b2;
				};
			double _c[3];
			};
		mutable bool _valid;
	public:
		anamorphic_homomorphic_distortion()
			{
			_cx2 = _cy2 = _b2 = 0.0;
			}
//! Get coefficient c[i], 0 <= i < 3
		double get_coeff(int i) const
			{
			base_type::check_range(i);
			return _c[i];
			}
//! Set coefficient c[i], 0 <= i < 3
		void set_coeff(int i,double q)
			{
			base_type::check_range(i);
			_c[i] = q;
			}
// This model function is not polynomial; it has a restricted
// domain. This flag indicates range violations for undistort().
		bool valid() const
			{ return _valid; }
		bool check_domain(const vec2_type& p_dn) const
			{
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = x * x;
			double y2 = y * y;
			double xy = x * y;
			double disc = 1.0 - 2.0 * (_cx2 * x2 + _cy2 * y2 + _b2 * xy);
			if(disc <= 1e-12)
				{ return false; }
			return true;
			}
//! Remove distortion. p_dn is a point in diagonally normalized coordinates.
		vec2_type eval(const vec2_type& p_dn) const
			{
			double x_dn,y_dn;
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = x * x;
			double y2 = y * y;
			double xy = x * y;
			double disc = 1.0 - 2.0 * (_cx2 * x2 + _cy2 * y2 + _b2 * xy);
// Check for domain. The return value is (0,0)
// in case of domain violations.
			if(disc <= 1e-12)
				{
				_valid = false;
				return vec2_type(0,0);
				}
			_valid = true;
			double q = ::sqrt(disc);
			x_dn	= x / q;
			y_dn	= y / q;

			return vec2_type(x_dn,y_dn);
			}
		vec2_type eval_inv(const vec2_type& p_dn) const
			{
			double x_dn,y_dn;
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = x * x;
			double y2 = y * y;
			double xy = x * y;
// The special property of this model: simply change sign.
			double disc = 1.0 + 2.0 * (_cx2 * x2 + _cy2 * y2 + _b2 * xy);
// Check for domain. The return value is (0,0)
// in case of domain violations.
			if(disc <= 1e-12)
				{
				_valid = false;
				return vec2_type(0,0);
				}
			_valid = true;
			double q = ::sqrt(disc);
			x_dn	= x / q;
			y_dn	= y / q;

			return vec2_type(x_dn,y_dn);
			}
//! @brief Analytic version of the Jacobi-matrix.
	mat2_type jacobi(const vec2_type& p_dn) const
			{
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = x * x;
			double y2 = y * y;
			double xy = x * y;
			double q = 1.0 - 2.0 * (_cx2 * x2 + _cy2 * y2 + _b2 * x * y);

			mat2_type m_radial = (mat2_type(q) + 2.0 * mat2_type(_cx2,_b2,_b2,_cy2) * tensq(p_dn)) / (::sqrt(q) * q);
			return m_radial;
			}
//! @brief Derivative wrt distortion coefficients.
//! dg points to an array with 2N Elements
		void derive(double* dg,int n_parameters,const vec2_type& p_dn) const
			{
			int size = 2 * n_parameters;
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = p_dn[0] * p_dn[0];
			double y2 = p_dn[1] * p_dn[1];
			double xy = p_dn[0] * p_dn[1];

			int k = 0;
			double q = 1.0 - 2.0 * (_cx2 * x2 + _cy2 * y2 + _b2 * xy);
// cx2
			dg[k++] = x * (-2.0 * x2) / (::sqrt(q) * q);
			dg[k++] = y * (-2.0 * x2) / (::sqrt(q) * q);
			if(k == size) return;
// cy2
			dg[k++] = x * (-2.0 * y2) / (::sqrt(q) * q);
			dg[k++] = y * (-2.0 * y2) / (::sqrt(q) * q);
			if(k == size) return;
// b2
			dg[k++] = x * (-2.0 * xy) / (::sqrt(q) * q);
			dg[k++] = y * (-2.0 * xy) / (::sqrt(q) * q);
			if(k == size) return;
// Unreachable
			std::cerr << "anamorphic_homomorphic_distortion: n_parameters out of range" << std::endl;
			}
		std::ostream& out(std::ostream& cout) const
			{
			int p = int(cout.precision());
			cout.precision(5);
			cout << "cx2: " << _cx2 << std::endl;
			cout << "cy2: " << _cy2 << std::endl;
			cout << "b2: " << _b2 << std::endl;
			cout.precision(p);
			return cout;
			}
		};
	}

