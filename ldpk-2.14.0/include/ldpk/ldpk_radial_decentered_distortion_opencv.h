// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2022,  Science-D-Visions. Current version: 2.8

#pragma once

//! @file ldpk_radial_decentered_distortion_opencv.h
//! @brief A polynomial radially symmetric model of degree 6
//! with decentering degree 2 as defined in OpenCV.

#include <ldpk/ldpk_generic_distortion_base.h>
#include <iostream>

namespace ldpk
	{
//! @brief A polynomial radially symmetric model of degree 6
//! with decentering degree 2 as defined in OpenCV.
	template <class VEC2,class MAT2>
	class radial_decentered_distortion_opencv:public ldpk::generic_distortion_base<VEC2,MAT2,5>
		{
	public:
		typedef generic_distortion_base<VEC2,MAT2,5> base_type;
		typedef VEC2 vec2_type;
		typedef MAT2 mat2_type;
	private:
// union allows to access coefficients by index.
// Original OpenCV coefficients for OpenCV-coordinates.
		union
			{
			struct
				{
// p2 in OpenCV corresponds to our u2,
// p1 in OpenCV corresponds to our v2.
// This is the typical order of coefficients in OpenCV
				double _k1;
				double _k2;
				double _p1;
				double _p2;
				double _k3;
				};
			double _c_ocv[5];
			};
// Coefficients in SDV-coordinates.
		union
			{
			struct
				{
// We reproduce the order of coefficients from OpenCV in our notation.
				double _c2;
				double _c4;
				double _v2;
				double _u2;
				double _c6;
				};
			double _c[5];
			};
// OpenCV uses different coordinates than we do. Internally, we will use
// our own coordinate system, but get_coeff() and set_coeff() will
// work with OpenCV-coordinates. For each of the five coefficients
// there is a factor. In reparametrization we call this rho, and this here
// is quite similar.
		union
			{
			struct
				{
				double _rho_pow_c2,_rho_pow_u2,_rho_pow_v2;
				double _rho_pow_c4;
				double _rho_pow_c6;
				};
			double _rho_pow[5];
			};
		double _rho;
	public:
		radial_decentered_distortion_opencv()
			{
			_c2 = _u2 = _v2 = 0.0;
			_c4 = 0.0;
			_c6 = 0.0;
			_rho_pow_c2 = 1.0;
			_rho_pow_u2 = 1.0;
			_rho_pow_v2 = 1.0;
			_rho_pow_c4 = 1.0;
			_rho_pow_c6 = 1.0;
			_rho = 1.0;
			}
		void set_rho(double rho)
			{
			_rho = rho;
// For c2
			_rho_pow[0] = std::pow(rho,2.0);
// For c4
			_rho_pow[1] = std::pow(rho,4.0);
// For v2 and u2
			_rho_pow[2] = std::pow(rho,1.0);
			_rho_pow[3] = std::pow(rho,1.0);
// For c6
			_rho_pow[4] = std::pow(rho,6.0);

			for(int i = 0;i < 5;++i)
				{ _c[i] = _c_ocv[i] * _rho_pow[i]; }
			}
//! Get coefficient c[i], 0 <= i < 5
		double get_coeff(int i) const
			{
			base_type::check_range(i);
			return _c_ocv[i];
			}
//! Set coefficient c[i], 0 <= i < 6
		void set_coeff(int i,double q)
			{
			base_type::check_range(i);
			_c_ocv[i] = q;
			}
//! @brief Remove distortion. p_dn is a point in diagonally normalized coordinates.
//! We compute in dn-coordinates with dn-coefficients.
		vec2_type eval(const vec2_type& p_dn) const
			{
			double x_dn,y_dn;
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = x * x;
			double y2 = y * y;
			double xy = x * y;
			double r2 = x2 + y2;
			double r4 = r2 * r2;
			double r6 = r2 * r4;
// OpenCV has Y-down-coordinates
			x_dn	= x			* (1.0 + _c2 * r2 + _c4 * r4 + _c6 * r6)
				+ (r2 + 2.0 * x2)	* (_u2)
				+ 2.0 * xy		* (-_v2);
			
			y_dn	= y			* (1.0 + _c2 * r2 + _c4 * r4 + _c6 * r6)
				+ (r2 + 2.0 * y2)	* (-_v2)
				+ 2.0 * xy		* (_u2);
			return vec2_type(x_dn,y_dn);
			}
//! @brief Analytic version of the Jacobi-matrix. By definition,
//! we are working in dn-coordinates here, not OpenCV.
		mat2_type jacobi(const vec2_type& p_dn) const
			{
			mat2_type m;
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = x * x;
			double y2 = y * y;
			double x3 = x2 * x;
			double y3 = y2 * y;
			double xy = x * y;
			double x2y = xy * x;
			double xy2 = xy * y;
			double r2 = x2 + y2;
			double r4 = r2 * r2;

// long implementation for testing.
//			m[0][0] = 1 + _c2 * (3.0 * x2 + y2) + _c4 * (5.0 * x2 + y2) * r2 + _c6 * (7.0 * x2 + y2) * r4
//				+ 6.0 * _u2 * x
//				+ 2.0 * (-_v2) * y;
//			m[1][1] = 1 + _c2 * (x2 + 3.0 * y2) + _c4 * (x2 + 5.0 * y2) * r2 + _c6 * (x2 + 7.0 * y2) * r4
//				+ 6.0 * (-_v2) * y
//				+ 2.0 * _u2 * x;
//
//			m[0][1] = 2.0 * _c2 * xy + 4.0 * _c4 * xy * r2 + _c6 * xy * r4
//				+ 2.0 * _u2 * y
//				+ 2.0 * (-_v2) * x;
//			m[1][0] = 2.0 * _c2 * xy + 4.0 * _c4 * xy * r2 + _c6 * xy * r4
//				+ 2.0 * _u2 * y
//				+ 2.0 * (-_v2) * x;
// slightly condensed implementation
			double u2x = _u2 * x;
			double v2y = (-_v2) * y;
			double c4r2 = _c4 * r2;
			double c6r4 = _c6 * r4;
			m[0][0] = 1.0 + _c2 * (3.0 * x2 + y2) + c4r2 * (5.0 * x2 + y2) + c6r4 * (7.0 * x2 + y2)
				+ 6.0 * u2x
				+ 2.0 * v2y;
			m[1][1] = 1.0 + _c2 * (x2 + 3.0 * y2) + c4r2 * (x2 + 5.0 * y2) + c6r4 * (x2 + 7.0 * y2)
				+ 6.0 * v2y
				+ 2.0 * u2x;

			double m_off_diag_common	= 2.0 * _c2 * xy + 4.0 * _c4 * xy * r2 + 6.0 * _c6 * xy * r4
							+ 2.0 * _u2 * y + 2.0 * (-_v2) * x;
			m[0][1] = m_off_diag_common;
			m[1][0] = m_off_diag_common;

			return m;
			}
//! @brief Derivative wrt distortion coefficients.
//! dg points to an array with N / 2 Elements
//! Not tested!
		void derive(double* dg,int n_parameters,const vec2_type& p_dn) const
			{
			int size = 2 * n_parameters;
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = p_dn[0] * p_dn[0];
			double y2 = p_dn[1] * p_dn[1];
			double xy = p_dn[0] * p_dn[1];
			double r2 = x2 + y2;
			double r4 = r2 * r2;
			double r6 = r2 * r4;
// OpenCV order. Since we have defined c_sdv = c_ocv * rho**j,
// the derivatives d/dc_ocv are d/dc_sdv * rho**j.
			int k = 0;
// c2 (= k1)
			dg[k++] = _rho_pow[0] * (x * r2);
			dg[k++] = _rho_pow[0] * (y * r2);
			if(k == size) return;
// c4 (= k2)
			dg[k++] = _rho_pow[1] * (x * r4);
			dg[k++] = _rho_pow[1] * (y * r4);
			if(k == size) return;
// v2 (= p1), mind the sign due to OpenCV's Y-down-coordinates.
			dg[k++] = -_rho_pow[2] * (2.0 * xy);
			dg[k++] = -_rho_pow[2] * (r2 + 2.0 * y2);
			if(k == size) return;
// u2 (= p2)
			dg[k++] = _rho_pow[3] * (r2 + 2.0 * x2);
			dg[k++] = _rho_pow[3] * (2.0 * xy);
			if(k == size) return;
// c6 (= k3)
			dg[k++] = _rho_pow[4] * (x * r6);
			dg[k++] = _rho_pow[4] * (y * r6);
			if(k == size) return;
// Unreachable
			std::cerr << "radial_decentered_distortion_opencv: n_parameters out of range" << std::endl;
			}
		std::ostream& out(std::ostream& cout) const
			{
			int p = int(cout.precision());
			cout.precision(5);
			cout << "c2: " << _c2 << std::endl;
			cout << "c4: " << _c4 << std::endl;
			cout << "v2: " << _v2 << std::endl;
			cout << "u2: " << _u2 << std::endl;
			cout << "c6: " << _c6 << std::endl;
			cout.precision(p);
			return cout;
			}
		};
	}

