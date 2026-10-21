// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file ldpk_generic_distortion_base.h
//! @brief Base class for distortion models

#include <ldpk/ldpk_generic_distortion_api.h>
#include <cmath>
#include <functional>
#include <iostream>
#include <vector>

namespace ldpk
	{
//! @brief Base class for a distortion model with N parameters.
//! You may find it useful to derive your own distortion model class from this one.
//! It contains methods for inverting the distortion model function.
//! We derive this class from the standard unary function class
//! in order to get a well-defined function object.
	template <class VEC2,class MAT2,int N>
	class generic_distortion_base:public generic_distortion_api
		{
	public:
		typedef VEC2 vec2_type;
		typedef MAT2 mat2_type;
	private:
// For the inverse mapping we have termination conditions
		double _epsilon;
// After n_max_iter we give up. This should not happen.
		int _n_max_iter;
// n_post_iter is the number of iterations we perform after
// epsilon is reached.
		int _n_post_iter;

	protected:
//! A derived class may check if the index is valid.
		void check_range(int i) const
			{
			if((i < 0) || (i >= N))
				{ throw error_index_out_of_range(i,0,N - 1); }
			}
	public:
		generic_distortion_base()
			{
			_epsilon = 1e-8;
// 100 instead of the original 20: near a fold of the polynomial the
// Jacobian degenerates and Newton converges slowly. With 20 iterations
// such points did not reach the 1e-8 termination criterion and were
// rejected by the caller's round-trip check, eating chunks out of the
// image border.
			_n_max_iter = 100;
			_n_post_iter = 2;
			}
//! Number of parameters, that is N.
		int get_num_parameters() const
			{ return N; }
//! @brief Configure iterative procedure for eval_inv(). Call this, if you don't agree
//! with the default values.
		void setup_eval_inv(int n_max_iter,int n_post_iter,double epsilon)
			{
			_n_max_iter = n_max_iter;
			_n_post_iter = n_post_iter;
			_epsilon = epsilon;
			}
//! @brief After changing one or more coefficients of the model, call this (future use).
//! The derived class may prepare some data structure for fast/precise evalutation..
		virtual void done()
			{ }
//! @brief User-defined maximum number of iterations applied in eval_inv in order to
//! fulfill the termination condition.
		int get_n_max_iter() const
			{ return _n_max_iter; }
//! @brief User-defined number of additional iterations to be applied when
//! the termination condition is fulfilled (which we call post-iterations).
		int get_n_post_iter() const
			{ return _n_post_iter; }
//! @brief Remove distortion. This method is non-iterative.
		virtual vec2_type eval(const vec2_type& p) const = 0;
//! @brief Jacobi-Matrix. The result is a matrix g_{ij} = d/dp_j f(p)_i, where f represents the undistort-function.
//! We compute this by means of difference quotients. This requires four evaluations. For better performance,
//! you may want to implement the analytic form in your derived distortion class.
		virtual mat2_type jacobi(const vec2_type& p_dn) const
			{
			const double epsilon = 1e-4;
			return	trans(mat2_type((eval(p_dn + vec2_type(epsilon,0)) - eval(p_dn - vec2_type(epsilon,0))) / (2.0 * epsilon),
						(eval(p_dn + vec2_type(0,epsilon)) - eval(p_dn - vec2_type(0,epsilon))) / (2.0 * epsilon)));
			}
//! Not all distortion functions will support this
		virtual void derive(double* dg,int n_parameters,const vec2_type& p_dn) const
			{ std::cout << "ldpk::generic_distortion_base::derive: not implemented" << std::endl; }
//! @brief For given q, we are looking for p so that f(p) = q.
//! p_start is the initial value which is suppost to be not too far from p.
		vec2_type eval_inv(const vec2_type& q,const vec2_type& p_start) const
			{
			vec2_type p = p_start;
			for(int i = 0;i < _n_max_iter;++i)
				{
// A Newton iteration
				mat2_type g = jacobi(p);
				vec2_type q_cmp(eval(p));
				try
					{ p += invert(g) * (q - q_cmp); }
				catch(...)
					{ return vec2_type(); }
				if(norm2(q - q_cmp) < 1e-8)
					{ break; }
				}
			return p;
			}
//! @brief In version 2.9.4 and earlier we used to get initial values
//! from a lookup table only, but this has turned out to be insufficient
//! in rare situations.
		vec2_type eval_inv_clipped(const vec2_type& q_dn,const vec2_type& p_dn_start) const
			{
// Try to find an initial value without lookup-table.
// The situation we are trying to handle here is the following:
// Given some considerable off-identity model function, which
// pushes points far outbound, with lookup tables we run into
// the problem that they are either not big enough or inefficient.
// The lookup table tells us it cannot provide a reliable initial value.
// Then we do the small brute force scan with 5x5 values as shown below.
			const int n_radial = 5;
			const int n_angular = 5;
			const double radius[n_radial] = {1.075,1.000,0.925,0.850,0.775};
// plus minus 4 degree.
			const double angle[n_angular] = {-0.07,-0.035,0.0,0.035,0.07};
			const double cos_angle[n_angular] = {0.9975510002532796   ,0.9993875625234886    ,1.0 ,0.9993875625234886   ,0.9975510002532796};
			const double sin_angle[n_angular] = {-0.06994284733753277 ,-0.034992854604336196 ,0.0 ,0.034992854604336196 ,0.06994284733753277};
			mat2_type m;

			vec2_type p_dn_guess;
			double d_min = 1e6;
			vec2_type p_dn_min;
// Iterate for angle
			for(int j = 0;j < n_angular;++j)
				{
				mat2_type m(cos_angle[j],-sin_angle[j],sin_angle[j],cos_angle[j]);
// Iteratre over radius
				for(int i = 0;i < n_radial;++i)
					{
// Variate incoming initial value w.r.t. angle and rescale with radius factor.
					p_dn_guess = radius[i] * (m * p_dn_start);
					double d = norm2(eval(p_dn_guess) - q_dn);
// Find best pair [radius,angle]
					if(d < d_min)
						{
						d_min = d;
						p_dn_min = p_dn_guess;
						}
					}
				}
			vec2_type p_dn_res = eval_inv(q_dn,p_dn_min);
			return p_dn_res;
			}
//! @brief The derived class implements a method for printing values
//! inside 3DE4's matrix tool dialog. This functionality
//! is currently not supported by tde4_ld_plugin.
		virtual std::ostream& out(std::ostream& cout) const
			{ return cout; }
		};
	}

