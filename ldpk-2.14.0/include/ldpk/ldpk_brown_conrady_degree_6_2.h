// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file ldpk_brown_conrady_degree_6_2.h
//! @brief A polynomial radially symmetric model of radial degree 6
//! and decentering degree 2.

#include <ldpk/ldpk_generic_distortion_base.h>
#include <iostream>

namespace ldpk
	{
//! @brief The Brown-Conrady with radial degree 6
//! and decentered degree 2. Makes much more sense
//! than degree 4 radial and decentered. And it's
//! linear in its coefficients.
	template <class VEC2,class MAT2>
	class brown_conrady_degree_6_2:public ldpk::generic_distortion_base<VEC2,MAT2,5>
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
				double _c2,_c4,_c6;
				double _t1,_t2;
				};
			double _c[5];
			};
	public:
		brown_conrady_degree_6_2()
			{
			_c2 = _c4 = _c6 = 0.0;
			_t1 = _t2 = 0.0;
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
			double r6 = r4 * r2;
			x_dn	= x * (1.0 + _c2 * r2 + _c4 * r4 + _c6 * r6)
				+ _t1 * (r2 + 2.0 * x2) + 2.0 * _t2 * xy;
			
			y_dn	= y * (1.0 + _c2 * r2 + _c4 * r4 + _c6 * r6)
				+ _t2 * (r2 + 2.0 * y2) + 2.0 * _t1 * xy;

			return vec2_type(x_dn,y_dn);
			}

//! @brief Analytic version of the Jacobi-matrix, about two times faster than
//! the base class version which uses difference quotients.
		mat2_type jacobi(const vec2_type& p_dn) const
			{
			mat2_type m;
// pow 1
			double x = p_dn[0];
			double y = p_dn[1];
// pow 2
			double x2 = x * x;
			double xy = x * y;
			double y2 = y * y;
// pow 3
			double x3 = x2 * x;
			double x2y = xy * x;
			double xy2 = xy * y;
			double y3 = y2 * y;
// pow 4
			double x4 = x2 * x2;
			double x3y = x3 * y;
			double x2y2 = x2 * y2;
			double xy3 = x * y3;
			double y4 = y2 * y2;
// pow 5
			double x5 = x3 * x2;
			double y5 = y3 * y2;
// pow 6
			double x6 = x4 * x2;
			double x5y = x5 * y;
			double x4y2 = x4 * y2;
			double x3y3 = x3 * y3;
			double x2y4 = x2 * y4;
			double xy5 = x * y5;
			double y6 = y4 * y2;

			m[0][0] = 1.0	+ _c2 * (3.0 * x2 + y2)
					+ _c4 * (5.0 * x4 + 6.0 * x2y2 + y4)
					+ _c6 * (7.0 * x6 + 15.0 * x4y2 + 9.0 * x2y4 + y6)
					+ 6.0 * _t1 * x
					+ 2.0 * _t2 * y;

			m[1][1] = 1.0	+ _c2 * (3.0 * y2 + x2)
					+ _c4 * (5.0 * y4 + 6.0 * x2y2 + x4)
					+ _c6 * (7.0 * y6 + 15.0 * x2y4 + 9.0 * x4y2 + x6)
					+ 6.0 * _t2 * y
					+ 2.0 * _t1 * x;

			m[0][1] = 	  2.0 * _c2 * xy
					+ _c4 * (4.0 * x3y + 4.0 * xy3)
					+ _c6 * (6.0 * x5y + 12.0 * x3y3 + 6.0 * xy5)
					+ 2.0 * _t1 * y
					+ 2.0 * _t2 * x;

			m[1][0] = 	  2.0 * _c2 * xy
					+ _c4 * (4.0 * xy3 + 4.0 * x3y)
					+ _c6 * (6.0 * xy5 + 12.0 * x3y3 + 6.0 * x5y)
					+ 2.0 * _t2 * x
					+ 2.0 * _t1 * y;

			return m;
			}
		vec2_type twist(const vec2_type& p_dn) const
			{
			vec2_type v;
// pow 1
			double x = p_dn[0];
			double y = p_dn[1];
// pow 2
			double x2 = x * x;
			double xy = x * y;
			double y2 = y * y;
// pow 3
			double x3 = x2 * x;
			double x2y = xy * x;
			double xy2 = xy * y;
			double y3 = y2 * y;
// pow 4
			double x4 = x2 * x2;
			double x3y = x3 * y;
			double x2y2 = x2 * y2;
			double xy3 = x * y3;
			double y4 = y2 * y2;
// pow 5
			double x5 = x3 * x2;
			double x4y = x4 * y;
			double x3y2 = x3 * y2;
			double x2y3 = x2 * y3;
			double xy4 = x * y4;
			double y5 = y3 * y2;
// 0. Compute like dJ[0][0]/dy or dJ[0][1]/dx.
// 1. Compute like dJ[1][1]/dx or dJ[1][0]/dy.
			v[0] = 		  _c2 * (2.0 * y)
					+ _c4 * (12.0 * x2y + 4.0 * y3)
					+ _c6 * (30.0 * x4y + 36.0 * x2y3 + 6.0 * y5)
					+ 2.0 * _t2;

			v[1] = 		  _c2 * (2.0 * x)
					+ _c4 * (12.0 * xy2 + 4.0 * x3)
					+ _c6 * (30.0 * xy4 + 36.0 * x3y2 + 6.0 * x5)
					+ 2.0 * _t1;

			return v;
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
			double r6 = r4 * r2;

			int k = 0;
// c2
			dg[k++] = x * r2;
			dg[k++] = y * r2;
			if(k == size) return;
// c4
			dg[k++] = x * r4;
			dg[k++] = y * r4;
			if(k == size) return;
// c6
			dg[k++] = x * r6;
			dg[k++] = y * r6;
			if(k == size) return;
// t1
			dg[k++] = r2 + 2.0 * x2;
			dg[k++] = 2.0 * xy;
			if(k == size) return;
// t2
			dg[k++] = 2.0 * xy;
			dg[k++] = r2 + 2.0 * y2;
			if(k == size) return;
// Unreachable
			std::cerr << "brown_conrady_degree_6_2: n_parameters out of range" << std::endl;
			}
		std::ostream& out(std::ostream& cout) const
			{
			int p = int(cout.precision());
			cout.precision(5);
			cout << "c2: " << _c2 << std::endl;
			cout << "c4: " << _c4 << std::endl;
			cout << "c6: " << _c6 << std::endl;
			cout << "t1: " << _t1 << std::endl;
			cout << "t2: " << _t2 << std::endl;
			cout.precision(p);
			return cout;
			}
		};
	}

