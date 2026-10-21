// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/ldpk_error.h>
#include <algorithm>
#include <iostream>
#include <cmath>

//! @file ldpk_vec3d.h
//! @brief Double-valued vector and matrix class.

namespace ldpk
	{
	class vec3d;
	class mat3d;

	double dot(const vec3d& a,const vec3d& b);
	double dotsq(const vec3d& a);
	double norm2(const vec3d& a);
	double norminf(const vec3d& a);
	double det(const mat3d& a);
	double det(const mat3d& a,const mat3d& adj);
	double tr(const mat3d& a);

	vec3d had(const vec3d& a,const vec3d& b);
	vec3d hadsq(const vec3d& a);
	vec3d wdg(const vec3d& a,const vec3d& b);
	vec3d unit(const vec3d& v);
	vec3d unitinf(const vec3d& v);

	mat3d wdg(const vec3d& a);
	mat3d ten(const vec3d& a,const vec3d& b);
	mat3d tensq(const vec3d& a);
	mat3d adjugate(const mat3d& a);
	mat3d trans(const mat3d& a);
	mat3d invert(const mat3d& a);

	mat3d make_rot3_from_angle_and_axis(double phi,const vec3d& n);


//! @brief A class for three-dimensional double-valued vectors
//! We have added this class and ldpk::mat3d in order to
//! deliver a self-consistent kit. You may use this one or
//! your own vector and matrix classes, at your choice.
	class vec3d
		{
	private:
		double _x0,_x1,_x2;
	public:
//! @name Constructors
//@{
//! Default: null vector
		vec3d():_x0(0),_x1(0),_x2(0)
			{ }
//! Copy constructor
		vec3d(const vec3d& a):_x0(a._x0),_x1(a._x1),_x2(a._x2)
			{ }
//! Constructing by components
		vec3d(double x0,double x1,double x2):_x0(x0),_x1(x1),_x2(x2)
			{ }
//@}
//! @name Arithmetic operations
//@{
		vec3d& operator += (const vec3d& a)
			{ _x0 += a._x0;_x1 += a._x1;_x2 += a._x2;return *this; }
		vec3d operator + (const vec3d& a) const
			{ vec3d r(*this);return r += a; }
		vec3d& operator -= (const vec3d& a)
			{ _x0 -= a._x0;_x1 -= a._x1;_x2 -= a._x2;return *this; }
		vec3d operator - (const vec3d& a) const
			{ vec3d r(*this);return r -= a; }
		vec3d& operator *= (double q)
			{ _x0 *= q;_x1 *= q;_x2 *= q;return *this; }
		vec3d operator * (double q) const
			{ vec3d r(*this);return r *= q; }
		friend vec3d operator * (double q,const vec3d& a)
			{ vec3d r(a);return r *= q; }
		vec3d& operator /= (double q)
			{ _x0 /= q;_x1 /= q;_x2 /= q;return *this; }
		vec3d operator / (double q) const
			{ vec3d r(*this);return r /= q; }
		vec3d operator - ()
			{ return vec3d(-_x0,-_x1,-_x2); }
//@}
//! @name Accessing components
//@{
		const double& operator [] (int i) const
			{ return *(&_x0 + i); }
		double& operator [] (int i)
			{ return *(&_x0 + i); }
//@}
//! @name I/O functions
//@{
		friend std::ostream& operator << (std::ostream& cout,const vec3d& a)
			{ return cout << a[0] << " " << a[1] << " " << a[2]; }
		friend std::istream& operator >> (std::istream& cin,vec3d& a)
			{ return cin >> a[0] >> a[1] >> a[2]; }
//@}
		};

//! @brief A class for double-valued 3x3-matrices. The matrix class for ldpk::vec3d.
	class mat3d
		{
	private:
		vec3d _x0,_x1,_x2;
	public:
//! @name Constructors
//@{
//! Default: unit matrix
		mat3d():_x0(1,0,0),_x1(0,1,0),_x2(0,0,1)
			{ }
//! Copy constructor
		mat3d(const mat3d& a):_x0(a[0]),_x1(a[1]),_x2(a[2])
			{ }
//! Scalar matrix
		mat3d(double s):_x0(s,0,0),_x1(0,s,0),_x2(0,0,s)
			{ }
//! Constructing by components (numbers)
		mat3d(	double a00,double a01,double a02,
			double a10,double a11,double a12,
			double a20,double a21,double a22)
		:_x0(a00,a01,a02),_x1(a10,a11,a12),_x2(a20,a21,a22)
			{ }
//! Constructing by components (row vectors)
		mat3d(const vec3d& x0,const vec3d& x1,const vec3d& x2):_x0(x0),_x1(x1),_x2(x2)
			{ }
//@}
//! @name Arithmetic operations
//@{
		mat3d& operator += (const mat3d& a)
			{ _x0 += a._x0;_x1 += a._x1;_x2 += a._x2;return *this; }
		mat3d operator + (const mat3d& a) const
			{ mat3d r(*this);return r += a; }
		mat3d& operator -= (const mat3d& a)
			{ _x0 -= a._x0;_x1 -= a._x1;_x2 -= a._x2;return *this; }
		mat3d operator - (const mat3d& a) const
			{ mat3d r(*this);return r -= a; }
		mat3d& operator *= (double q)
			{ _x0 *= q;_x1 *= q;_x2 *= q;return *this; }
		mat3d operator * (double q) const
			{ mat3d r(*this);return r *= q; }
		friend mat3d operator * (double q,const mat3d& a)
			{ mat3d r(a);return r *= q; }
//! Matrix multiplication
		mat3d operator * (const mat3d& a) const
			{
			const mat3d& t(*this);
			return mat3d(
				t[0][0] * a[0][0] + t[0][1] * a[1][0] + t[0][2] * a[2][0],
				t[0][0] * a[0][1] + t[0][1] * a[1][1] + t[0][2] * a[2][1],
				t[0][0] * a[0][2] + t[0][1] * a[1][2] + t[0][2] * a[2][2],

				t[1][0] * a[0][0] + t[1][1] * a[1][0] + t[1][2] * a[2][0],
				t[1][0] * a[0][1] + t[1][1] * a[1][1] + t[1][2] * a[2][1],
				t[1][0] * a[0][2] + t[1][1] * a[1][2] + t[1][2] * a[2][2],

				t[2][0] * a[0][0] + t[2][1] * a[1][0] + t[2][2] * a[2][0],
				t[2][0] * a[0][1] + t[2][1] * a[1][1] + t[2][2] * a[2][1],
				t[2][0] * a[0][2] + t[2][1] * a[1][2] + t[2][2] * a[2][2]
				);
			}
		mat3d& operator /= (double q)
			{ _x0 /= q;_x1 /= q;_x2 /= q;return *this; }
		mat3d operator / (double q) const
			{ mat3d r(*this);return r /= q; }
		mat3d operator - ()
			{ return mat3d(-_x0,-_x1,-_x2); }
//! Apply matrix to vector
		vec3d operator * (const vec3d& a) const
			{ return vec3d(dot(_x0,a),dot(_x1,a),dot(_x2,a)); }
//@}
//! @name Accessing components (row vectors)
//@{
		const vec3d& operator [] (int i) const
			{ return *(&_x0 + i); }
		vec3d& operator [] (int i)
			{ return *(&_x0 + i); }
//@}
//! @name I/O functions
//@{
		friend std::ostream& operator << (std::ostream& cout,const mat3d& a)
			{ return cout << a[0] << " " << a[1] << " " << a[2]; }
		friend std::istream& operator >> (std::istream& cin,mat3d& a)
			{ return cin >> a[0] >> a[1] >> a[2]; }
//@}
		};

//! @name Special vector functions
//@{
//! Inner product
	inline double dot(const vec3d& a,const vec3d& b)
		{ return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
//! Inner product square
	inline double dotsq(const vec3d& a)
		{ return a[0] * a[0] + a[1] * a[1] + a[2] * a[2]; }
//! Hadamard product
	inline vec3d had(const vec3d& a,const vec3d& b)
		{ return vec3d(a[0] * b[0],a[1] * b[1],a[2] * b[2]); }
//! Hadamard product square
	inline vec3d hadsq(const vec3d& a)
		{ return vec3d(a[0] * a[0],a[1] * a[1],a[2] * a[2]); }
//! anti-symmetric product, vector product, cross product...
	inline vec3d wdg(const vec3d& a,const vec3d& b)
		{
		return vec3d(
			a[1] * b[2] - a[2] * b[1],
			a[2] * b[0] - a[0] * b[2],
			a[0] * b[1] - a[1] * b[0]
			);
		}
//! Euclidian norm
	inline double norm2(const vec3d& a)
		{ return ::sqrt(dotsq(a)); }
//! Maximum norm
	inline double norminf(const vec3d& a)
		{ return std::max(::fabs(a[0]),std::max(::fabs(a[1]),::fabs(a[2]))); }
//! Unit vector
	inline vec3d unit(const vec3d& v)
		{ return v / norm2(v); }
//! Unit vector with respect to maximum norm
	inline vec3d unitinf(const vec3d& v)
		{ return v / norminf(v); }
//@}
//! dual object
	inline mat3d wdg(const vec3d& a)
		{ return mat3d(0,+a[2],-a[1],-a[2],0,+a[0],+a[1],-a[0],0); }
//! @name Tensor products
//@{
//! @brief Tensor (dyadic) product of vectors
	inline mat3d ten(const vec3d& a,const vec3d& b)
		{
		return mat3d(
			a[0] * b[0],a[0] * b[1],a[0] * b[2],
			a[1] * b[0],a[1] * b[1],a[1] * b[2],
			a[2] * b[0],a[2] * b[1],a[2] * b[2]
			);
		}
//! @brief Tensor (dyadic) product square
	inline mat3d tensq(const vec3d& a)
		{
		return mat3d(
			a[0] * a[0],a[0] * a[1],a[0] * a[2],
			a[1] * a[0],a[1] * a[1],a[1] * a[2],
			a[2] * a[0],a[2] * a[1],a[2] * a[2]
			);
		}
//@}
//! @name Special matrix functions
//@{
//! @brief Determinant
	inline double det(const mat3d& a)
		{
		return	  (a[0][0] * a[1][1] - a[0][1] * a[1][0]) * a[2][2]
			- (a[0][0] * a[1][2] - a[0][2] * a[1][0]) * a[2][1]
			+ (a[0][1] * a[1][2] - a[0][2] * a[1][1]) * a[2][0];
		}
//! @brief Determinant with known adjugate
	inline double det(const mat3d& a,const mat3d& adj)
		{
		return adj[0][0] * a[0][0] + adj[0][1] * a[1][0] + adj[0][2] * a[2][0];
		}
//! @brief Adjugate matrix - entries are determinants of co-submatrices of the transposed.
	inline mat3d adjugate(const mat3d& a)
		{
		return mat3d(
			a[1][1] * a[2][2] - a[1][2] * a[2][1],
			a[2][1] * a[0][2] - a[2][2] * a[0][1],
			a[0][1] * a[1][2] - a[0][2] * a[1][1],

			a[1][2] * a[2][0] - a[1][0] * a[2][2],
			a[2][2] * a[0][0] - a[2][0] * a[0][2],
			a[0][2] * a[1][0] - a[0][0] * a[1][2],

			a[1][0] * a[2][1] - a[1][1] * a[2][0],
			a[2][0] * a[0][1] - a[2][1] * a[0][0],
			a[0][0] * a[1][1] - a[0][1] * a[1][0]
			);
		}
//! @brief Trace
	inline double tr(const mat3d& a)
		{ return a[0][0] + a[1][1] + a[2][2]; }
//! @brief Transposed
	inline mat3d trans(const mat3d& a)
		{
		return mat3d(	vec3d(a[0][0],a[1][0],a[2][0]),
				vec3d(a[0][1],a[1][1],a[2][1]),
				vec3d(a[0][2],a[1][2],a[2][2]));
		}
//! @brief Inverse, no error checking! Make sure that `a` is invertible.
	inline mat3d invert(const mat3d& a)
		{
		mat3d adj(adjugate(a));
		double dt = det(a,adj);
		return adj / dt;
		}
//! @brief Given an angle (in radian) and an axis, we compute
//! the rotation matrix around the axis by the angle.
	inline mat3d make_rot3_from_angle_and_axis(double phi,const vec3d& n)
		{
		vec3d unit_n(unit(n));

		double c = ::cos(phi);
		double s = ::sin(phi);

		return mat3d(c) + (1.0 - c) * tensq(unit_n) - wdg(s * unit_n);
		}
//@}
	}
