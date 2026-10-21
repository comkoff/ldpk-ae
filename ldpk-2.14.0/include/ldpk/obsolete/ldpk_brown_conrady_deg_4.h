#pragma once

//! @file ldpk_brown_conrady_deg_4.h
//! @brief A polynomial radially symmetric model of degree 4 with decentering.

#include <ldpk/ldpk_generic_distortion_base.h>
#include <iostream>

namespace ldpk
	{
//! @brief A polynomial radially symmetric model of degree 4 with decentering.
//! This is the distortion model for 3DE4's "Radial, Standard - Degree 4".
	template <class VEC2,class MAT2>
	class brown_conrady_deg_4:public ldpk::generic_distortion_base<VEC2,MAT2,5>
		{
	public:
		typedef generic_distortion_base<VEC2,MAT2,5> base_type;
		typedef VEC2 vec2_type;
		typedef MAT2 mat2_type;
	private:
// union allows to access coefficients by index.
		union
			{
			struct
				{
				double _c2,_c4;
				double _t1,_t2,_t3;
				};
			double _c[5];
			};
	public:
		brown_conrady_deg_4()
			{
			_c2 = _c4 = 0.0;
			_t1 = _t2 = _t3 = 0.0;
			}
//! Get coefficient c[i], 0 <= i < 4
		double get_coeff(int i) const
			{
			base_type::check_range(i);
			return _c[i];
			}
//! Set coefficient c[i], 0 <= i < 4
		void set_coeff(int i,double q)
			{
			base_type::check_range(i);
			_c[i] = q;
			}
//! Remove distortion. p_dn is a point in diagonally normalized coordinates.
		vec2_type operator()(const vec2_type& p_dn) const
			{
			double x_dn,y_dn;
			double x = p_dn[0];
			double y = p_dn[1];
			double x2 = x * x;
			double y2 = y * y;
			double xy = x * y;
			double r2 = x2 + y2;
			double r4 = r2 * r2;
			x_dn	= x						* (1.0 + _c2 * r2 + _c4 * r4)
				+ (_t1 * (r2 + 2.0 * x2) + 2.0 * _t2 * xy)	* (1.0 + _t3 * r2);
			
			y_dn	= y						* (1.0 + _c2 * r2 + _c4 * r4)
				+ (_t2 * (r2 + 2.0 * y2) + 2.0 * _t1 * xy)	* (1.0 + _t3 * r2);

			return vec2_type(x_dn,y_dn);
			}
/*
//! @brief Analytic version of the Jacobi-matrix, about two times faster than
//! the base class version which uses difference quotients.
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

// original implementation as in the document.
//			m[0][0] = 1 + _c2 * (3.0 * x2 + y2) + _c4 * (5.0 * x2 + y2) * r2
//				+ 6.0 * _u2 * x + _u4 * (8.0 * xy2 + 12.0 * x3)
//				+ 2.0 * _v2 * y + _v4 * (2.0 * y3 + 6.0 * x2y);
//			m[1][1] = 1 + _c2 * (x2 + 3.0 * y2) + _c4 * (x2 + 5.0 * y2) * r2
//				+ 6.0 * _v2 * y + _v4 * (8.0 * x2y + 12.0 * y3)
//				+ 2.0 * _u2 * x + _u4 * (2.0 * x3 + 6.0 * xy2);
//
//			m[0][1] = 2.0 * _c2 * xy + 4.0 * _c4 * xy * r2
//				+ 2.0 * _u2 * y + _u4 * (8.0 * x2y + 4.0 * y3)
//				+ 2.0 * _v2 * x + _v4 * (2.0 * x3 + 6.0 * xy2);
//			m[1][0] = 2.0 * _c2 * xy + 4.0 * _c4 * xy * r2
//				+ 2.0 * _u2 * y + _u4 * (6.0 * x2y + 2.0 * y3)
//				+ 2.0 * _v2 * x + _v4 * (4.0 * x3 + 8.0 * xy2);
// slightly condensed implementation
			double u2x = _u2 * x;
			double v2y = _v2 * y;
			double c4r2 = _c4 * r2;
			m[0][0] = 1.0 + _c2 * (3.0 * x2 + y2) + c4r2 * (5.0 * x2 + y2)
				+ 6.0 * u2x + _u4 * (8.0 * xy2 + 12.0 * x3)
				+ 2.0 * v2y + _v4 * (2.0 * y3 + 6.0 * x2y);
			m[1][1] = 1.0 + _c2 * (x2 + 3.0 * y2) + c4r2 * (x2 + 5.0 * y2)
				+ 6.0 * v2y + _v4 * (8.0 * x2y + 12.0 * y3)
				+ 2.0 * u2x + _u4 * (2.0 * x3 + 6.0 * xy2);

			double m_off_diag_common	= 2.0 * _c2 * xy + 4.0 * _c4 * xy * r2
							+ 2.0 * _u2 * y + 2.0 * _v2 * x;
			m[0][1] = m_off_diag_common
				+ _u4 * (8.0 * x2y + 4.0 * y3)
				+ _v4 * (2.0 * x3 + 6.0 * xy2);
			m[1][0] = m_off_diag_common
				+ _u4 * (6.0 * x2y + 2.0 * y3)
				+ _v4 * (4.0 * x3 + 8.0 * xy2);

			return m;
			}
//! @brief Derivative wrt distortion coefficients.
//! dg points to an array with N / 2 Elements
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

			int k = 0;
// c2
			dg[k++] = x * r2;
			dg[k++] = y * r2;
			if(k == size) return;
// u2
			dg[k++] = r2 + 2.0 * x2;
			dg[k++] = 2.0 * xy;
			if(k == size) return;
// v2
			dg[k++] = 2.0 * xy;
			dg[k++] = r2 + 2.0 * y2;
			if(k == size) return;
// c4
			dg[k++] = x * r4;
			dg[k++] = y * r4;
			if(k == size) return;
// u4
			dg[k++] = r2 * (r2 + 2.0 * x2);
			dg[k++] = 2.0 * r2 * xy;
			if(k == size) return;
// v4
			dg[k++] = 2.0 * r2 * xy;
			dg[k++] = r2 * (r2 + 2.0 * y2);
			if(k == size) return;
// Unreachable
			std::cerr << "brown_conrady_deg_4: n_parameters out of range" << std::endl;
			}
*/		std::ostream& out(std::ostream& cout) const
			{
			int p = int(cout.precision());
			cout.precision(5);
			cout << "c2: " << _c2 << std::endl;
			cout << "c4: " << _c4 << std::endl;
			cout << "t1: " << _t1 << std::endl;
			cout << "t2: " << _t2 << std::endl;
			cout << "t3: " << _t3 << std::endl;
			cout.precision(p);
			return cout;
			}
		};
	}

