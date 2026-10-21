// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file ldpk_generic_radial_distortion_1d.h
//! @brief A polynomial radially symmetric model of degree N (even)

#include <functional>
#include <ldpk/ldpk_generic_distortion_api.h>
#include <ldpk/ldpk_power_ct.h>
#include <iostream>
#include <cmath>

namespace ldpk
	{
//! @brief A polynomial model with N coefficients for radially symmetric lenses.
//! This class contains the model for normalized coordinates with diagonal radius = 1.
//! Lens center is at (0,0).
	template <int N>
	class generic_radial_distortion_1d:public generic_distortion_api
		{
	public:
	private:
// The coefficients
		double _c[N];
// For the inverse mapping we have termination conditions
		double _epsilon;
// After n_max_iter we give up. This should not happen.
		int _n_max_iter;
// n_post_iter is the number of iterations we perform after epsilon is reached.
		int _n_post_iter;
// For debugging: count iterations
		mutable int _k_iter;
// For debugging: maximum number of iterations occured
		mutable int _k_max_iter;

		void check_range(int i) const
			{
			if((i < 0) || (i >= N))
				{ throw error_index_out_of_range(i,0,N - 1); }
			}
	public:
		generic_radial_distortion_1d()
			{
			_epsilon = 1e-6;
// 100 instead of the original 20: near a fold of the polynomial Newton
// converges slowly, see generic_distortion_base.h.
			_n_max_iter = 100;
			_n_post_iter = 2;
			_k_iter = 0;
			_k_max_iter = 0;

			for(int i = 0;i < N;++i)
				{ _c[i] = 0.0; }
			}
		int get_num_parameters() const
			{ return N; }
//! Get coefficient c[i], 0 <= i < N  (i.e. coefficient power r^(2i))
		double get_coeff(int i) const
			{
			check_range(i);
			return _c[i];
			}
//! Set coefficient c[i], 0 <= i < N.
		void set_coeff(int i,double q)
			{
			check_range(i);
			_c[i] = q;
			}
//! Reset k_max_iter for debugging purposes.
		void reset_k_max_iter()
			{ _k_max_iter = 0; }
//! @brief Number of iterations until epsilon was reached. This value is reset at the beginning
//! of each iterative calculation.
		int get_k_iter() const
			{ return _k_iter; }
//! @brief By this value you can check how much iterations per pixel were required
//! to warp an entire image or sequence. Use reset_k_max_iter() to reset to 0.
		int get_k_max_iter() const
			{ return _k_iter; }
//! @brief User-defined maximum number of iterations applied in eval_inv in order to
//! fulfill the termination condition.
		int get_n_max_iter() const
			{ return _n_max_iter; }
//! @brief User-defined number of additional iterations to be applied when
//! the termination condition is fulfilled (which we call post-iterations).
		int get_n_post_iter() const
			{ return _n_post_iter; }
//! Remove distortion. r_dn is a point (distance from center) in diagonally normalized coordinates.
		double eval(double r_dn) const
			{
// Compute powers of r^2: 1,r^2,r^4,...,r^(2N)
			double r2pow[N + 1];
			double q(1.0),r2 = r_dn * r_dn;
			power_ct<double,N + 1>().build(r2,r2pow);

			for(int i = 0;i < N;++i)
				{ q += _c[i] * r2pow[i + 1]; }
			return r_dn * q;
			}
//! @brief Inverse mapping by solving the fixed point equation without providing initial values.
		double eval_inv(double q) const
			{
			double p = q - ((*this)(q) - q);
// Iterate until |f(p_iter) - q| < epsilon.
// We count the iterations for debugging purposes.
			_k_iter = 0;
			for(int i = 0;i < _n_max_iter;++i)
				{
				double q_iter = eval(p);
				p = p + q - q_iter;
				++_k_iter;
				double diff = ::fabs(q_iter - q);
				if(diff < _epsilon)
					{ break; }
				}
// We improve precision by post-iterations.
			for(int i = 0;i < _n_post_iter;++i)
				{
				double q_iter = eval(p);
				p = p + q - q_iter;
				}
// k_max_iter counts the number of iterations for the worst case.
			if(_k_iter > _k_max_iter)
				{ _k_max_iter = _k_iter; }
			return p;
			}
//! @brief We expect that usually p_start=q is sufficient as starting value,
//! since coefficients are low.
		double eval_inv(double q,double p_start) const
			{
			double p = p_start;
			for(_k_iter = 0;_k_iter < _n_max_iter;++_k_iter)
				{
// A Newton iteration
				double g = jacobi(p);
				double q_cmp = eval(p);
				p += (q - q_cmp) / g;
				if(::fabs(q - q_cmp) < _epsilon)
					{ break; }
				}
// We improve precision by post-iterations.
			for(int i = 0;i < _n_post_iter;++i)
				{
// A Newton iteration
				double g = jacobi(p);
				double q_cmp = eval(p);
				p += (q - q_cmp) / g;
				}
// k_max_iter counts the number of iterations for the worst case.
			if(_k_iter > _k_max_iter)
				{ _k_max_iter = _k_iter; }
			return p;
			}
//! @brief Jacobian. For the pure radial polynomial it's simply the derivative.
		double jacobi(double r_dn) const
			{
// Compute powers of r^2: 1,r^2,r^4,...,r^(2N)
			double r2pow[N + 1];
			double r2 = r_dn * r_dn;
			power_ct<double,N + 1>().build(r2,r2pow);

			double qa(1.0),qb(0.0);
			for(int i = 0;i < N;++i)
				{
				qa += _c[i] * r2pow[i + 1];
				qb += _c[i] * (2 * i + 2) * r2pow[i];
				}
			return qa + r2 * qb;
			}
		std::ostream& out(std::ostream& cout) const
			{
			int p = cout.precision();
			cout.precision(5);
			for(int i = 0;i < N;++i)
				{ cout << "    C(" << 2 * i + 2 << "): " << std::right << std::fixed << _c[i] << "\n"; }
			cout.precision(p);
			return cout;
			}
		};
	}

