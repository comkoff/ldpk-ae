// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file ldpk_radial_homomorphic_decentered_distortion.h
//! @brief A radially symmetric model of degree 2 with decentering.

#include <ldpk/ldpk_generic_distortion_base.h>
#include <iostream>

namespace ldpk
	{
//! @brief The homomorphic radially symmetric model of degree 2 with decentering.
	template <class VEC2,class MAT2>
	class radial_homomorphic_decentered_distortion:public ldpk::generic_distortion_base<VEC2,MAT2,3>
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
				double _c2,_u2,_v2;
				};
			double _c[3];
			};
		mutable bool _valid;
	public:
		radial_homomorphic_decentered_distortion()
			{
			_c2 = _u2 = _v2 = 0.0;
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
			double r2 = x2 + y2;
			double disc = 1.0 - 2.0 * _c2 * r2;
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
			double r2 = x2 + y2;
			double disc = 1.0 - 2.0 * _c2 * r2;
// Check for domain. The return value is (0,0)
// in case of domain violations.
			if(disc <= 1e-12)
				{
				_valid = false;
				return vec2_type(0,0);
				}
			_valid = true;
			double q = ::sqrt(disc);
			x_dn	= x			/ q
				+ (r2 + 2.0 * x2)	* _u2
				+ 2.0 * xy		* _v2;
			
			y_dn	= y			/ q
				+ (r2 + 2.0 * y2)	* _v2
				+ 2.0 * xy		* _u2;

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
			double r2 = x2 + y2;

			double u2x = _u2 * x;
			double v2y = _v2 * y;
			double u2y = _u2 * y;
			double v2x = _v2 * x;
			double c2xy = _c2 * xy;

			double q = 1.0 - 2.0 * _c2 * r2;
			mat2_type m_radial = (mat2_type(q) + 2.0 * _c2 * tensq(p_dn)) / (::sqrt(q) * q);
			mat2_type m_decenter(	6.0 * u2x + 2.0 * v2y,
						2.0 * (u2y + v2x),
						2.0 * (u2y + v2x),
						6.0 * v2y + 2.0 * u2x);
			return m_radial + m_decenter;
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
			double r2 = x2 + y2;

			int k = 0;
// c2
			double q = 1.0 - 2.0 * _c2 * r2;
			dg[k++] = x * (-2.0 * r2) / (::sqrt(q) * q);
			dg[k++] = y * (-2.0 * r2) / (::sqrt(q) * q);
			if(k == size) return;
// u2
			dg[k++] = r2 + 2.0 * x2;
			dg[k++] = 2.0 * xy;
			if(k == size) return;
// v2
			dg[k++] = 2.0 * xy;
			dg[k++] = r2 + 2.0 * y2;
			if(k == size) return;
// Unreachable
			std::cerr << "radial_homomorphic_decentered_distortion: n_parameters out of range" << std::endl;
			}
		std::ostream& out(std::ostream& cout) const
			{
			int p = int(cout.precision());
			cout.precision(5);
			cout << "c2: " << _c2 << std::endl;
			cout << "u2: " << _u2 << std::endl;
			cout << "v2: " << _v2 << std::endl;
			cout.precision(p);
			return cout;
			}
		};
	}

