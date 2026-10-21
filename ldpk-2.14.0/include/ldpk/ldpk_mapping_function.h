// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/ldpk_vec2d.h>

//! @file
//! @brief Mapping functions for experimental fisheye models.

// 2022-04-05 version 1.2.1, splitting any2plain into any2angle and angle2plain.
// 2022-04-04 version 1.2, mutable valid- and clipped-status; normalizer, scale.
// 2022-04-03 version 1.1, added equidistant, orthographic, stereographic
// 2022-04-01 version 1.0, equisolid only.

namespace ldpk
	{
//! @brief Base class for all fisheye mapping functions.
//!
//! There are four basic types of mapping functions in
//! fisheye lenses, but they can also occur in particular non-fisheye
//! lenses. The four basic types are
//! * equidistant	(ED)
//! * equisolid angle	(ES)
//! * orthographic	(OG)
//! * stereographic	(SG)
//!
//! The plan is to equip lens distortion classes with one of these
//! classes, so that the mapping function can easily be exchanged
//! without re-writing the distortion class, e.g. by using templates
//! or an abstract base class. In this header file we develop the
//! classes for the various mapping functions.
	template <class VEC2,class MAT2>
	class mapping_function_base
		{
	public:
		typedef VEC2 vec2_type;
		typedef MAT2 mat2_type;
	protected:
// The mapping functions are base on a focal length in diagonally
// normalized coordinates. This does not necessarily coincide with
// the focal length of the entire lens system.
		double _fl_dn;
// Optionally, the plain point can be normalized, e.g.
// to make sure the corners of the filmback remain invariant under
// the mapping function.
		double _normalizer;
// While the normalizer is applied to the plain side,
// the scale is applied to the fisheye side, i.e. there
// is a direct impact on the disc that represents the
// target space of remap_plain2any. This parameter corresponds
// to the scale parameter we have in the distortion models.
// Scale and normalizer of equal value cancel each other
// near the lens center (asymptotically).
		double _scale;

// Since some mapping functions tend to have singular behaviour
// we might be urged to clip (or clamp) the mapping function.
		double _r_plain_dn_clip;
// true if the last computation has been valid, otherwise false.
// The domains of the mapping functions, particularly any2plain
// are restricted to discs of a certan radius.
		mutable bool _valid;
// This flag is set if the result becomes false due to clipping
// but is still more or less usable in graphic representations.
		mutable bool _clipped;
	public:
		mapping_function_base():_fl_dn(1.0),_normalizer(1.0),_scale(1.0),_r_plain_dn_clip(50.0)
			{ }
		double get_fl_dn() const
			{ return _fl_dn; }
		void set_fl_dn(double f)
			{ _fl_dn = f; }
		double get_r_plain_dn_clip() const
			{ return _r_plain_dn_clip; }
		void set_r_plain_dn_clip(double r)
			{ _r_plain_dn_clip = r; }
//! @brief Status of the previous invocation of undistort(), distort()
//! or jacobi().
		bool valid() const
			{ return _valid; }
//! @brief Status of the previous invocation of undistort() or jacobi().
//! If valid is false, clipped is undefined, but we tend to set it to false.
		bool clipped() const
			{ return _clipped; }
//! @brief The current API definition is that after setting
//! the normalizer initialize() does not need to be invoked.
//! Callers rely on this property.
		double get_normalizer() const
			{ return _normalizer; }
		void set_normalizer(double n)
			{ _normalizer = n; }
		double get_scale() const
			{ return _scale; }
		void set_scale(double s)
			{ _scale = s; }
//! @brief Initialize after setting parameters.
//! Derived classes must call this. Currently empty.
		virtual void initialize()
			{ }

//! @brief The domain of undistort. This must be a clear, simple
//! expression, because distortion models use it for normalization.
		virtual double threshold_valid() const = 0;
//! @brief The domain which delivers correct results without clipping.
		double threshold_clipped() const
			{
			double r_any;
			remap_plain2any(_r_plain_dn_clip,r_any);
			return r_any;
			}
//! @brief Map a distance in the filmback to an elongation angle (in radian, of course).
//! This is the mapping function in the strict sense.
		virtual bool map_any2angle(double r_any_dn,double& theta) const = 0;
//! @brief Mapping the elongation angle to a radius on the filmback
//! for the plain (gnomonic) mapping function. Return value is false
//! if theta is greater or equal pi/2. The result r_plain_dn is clipped.
		bool map_angle2plain(double theta,double& r_plain_dn) const
			{
// We can only visualize ]-90deg,+90deg[ in plain images.
			if(theta >= M_PI / 2.0)
				{
				r_plain_dn = 0.0;
				return false;
				}
// Now remap to distance in plain filmback
			r_plain_dn = _fl_dn * ::tan(theta) / _normalizer;
// Optionally clip distance.
			if(r_plain_dn > _r_plain_dn_clip)
				{
				r_plain_dn = _r_plain_dn_clip / _normalizer;
				_clipped = true;
				}
			else
				{ _clipped = false; }
			return true;
			}
//! @brief Inverse function of map_angle2plain().
		double map_plain2angle(double r_plain_dn) const
			{ return atan2(r_plain_dn * _normalizer,_fl_dn); }
//! @brief Map elongation angle to radius in the fisheye domain.
		virtual double map_angle2any(double theta) const = 0;
//! @brief Derivative of map_angle2plain().
		double derive_angle2plain(double theta) const
			{
			double tg = ::tan(theta);
			return _fl_dn * (1.0 + tg * tg) / _normalizer;
			}

//! @brief A return value of false indicates that the result
//! r_plain_dn is not well-defined and cannot be used for anything.
//! These two methods do not apply the normalization.
		bool remap_any2plain(double r_any_dn,double& r_plain_dn) const
			{
			double theta;
			if(!map_any2angle(r_any_dn,theta))
				{
				r_plain_dn = 0.0;
				return false;
				}
			return map_angle2plain(theta,r_plain_dn);
			}
// Revised 2022-04-05.
		bool remap_plain2any(double r_plain_dn,double& r_any_dn) const
			{
// Angular elongation in gnomonic lenses: theta = arctan(r / f)
			double theta = map_plain2angle(r_plain_dn);
// Now remap to radius in fisheye domain
			r_any_dn = map_angle2any(theta);
			return true;
			}
//! @brief The derivative of map_any2angle, which is the inverse mapping function
		virtual bool derive_any2angle(double r_any_dn,double& dr_theta) const = 0;
//! @brief Tested against differential quotient.
		bool derive_any2plain(double r_any_dn,double& dr_plain_dn) const
			{
			double dr_theta;
			if(!derive_any2angle(r_any_dn,dr_theta))
				{ return false; }
			double theta;
			if(!map_any2angle(r_any_dn,theta))
				{ return false; }
// We can only visualize ]-90deg,+90deg[ in plain images.
			if(theta >= M_PI / 2.0)
				{ return false; }
			dr_plain_dn = derive_angle2plain(theta) * dr_theta;
			return true;
			}

//! @brief This method maps an undistorted point in diagonally normalized
//! coordinates into an undistorted point in the same coordinated by mapping
//! from non-planar space to planar space. The normalization factor is applied.
		bool remap_any2plain(const vec2_type& p_any_undist_dn,vec2_type& p_plain_undist_dn)
			{
// Distance from origin in dn-coordinates
			double r_any_undist_dn = norm2(p_any_undist_dn);
// Remap equidistant to gnomonic
			double r_plain_undist_dn;
			if(!remap_any2plain(r_any_undist_dn,r_plain_undist_dn))
				{ return false; }
// Calc undistorted point from undistorted radius. We us the direction
// of the non-planar point, which we can do since the direction is
// not affected by the mapping function.
			if(dotsq(p_any_undist_dn) == 0.0)
				{ p_plain_undist_dn = vec2_type(0,0); }
			else
				{ p_plain_undist_dn = r_plain_undist_dn * unit(p_any_undist_dn); }
			return true;
			}
//! @brief This method maps an undistorted point in diagonally normalized
//! coordinates from planar to non-planar space. The normalization is removed
//! before mapping, so that this method is the inverse of remap_any2plain.
		bool remap_plain2any(const vec2_type& p_plain_undist_dn,vec2_type& p_any_undist_dn)
			{
// Distance from origin in dn-coordinates
			double r_plain_undist_dn = norm2(p_plain_undist_dn);
// Remap gnomonic to equidistant with denormalization prepended.
			double r_any_undist_dn;
			if(!remap_plain2any(r_plain_undist_dn,r_any_undist_dn))
				{ return false; }
// Calc undistorted point from undistorted radius. We us the direction
// of the planar point, which we can do since direction is not affected
// by the mapping function.
			if(dotsq(p_plain_undist_dn) == 0.0)
				{ p_any_undist_dn = vec2_type(0,0); }
			else
				{ p_any_undist_dn = r_any_undist_dn * unit(p_plain_undist_dn); }

			return true;
			}
//! @brief Given a point in diagonally normalized coordinates on a filmback
//! with any of the fisheye models we compute the point position as it would
//! be for a plain mapping function. This is done by mapping the point into
//! an elongation angle (i.e. in 3d space), and then mapping the elongation
//! angle back to the filmback. In this form we do not use the mapping function
//! since lens distortion is not taken into account.
//! After calling, the _valid state indicates whether or not the argument
//! is inside the domain of the mapping function.
		vec2_type undistort(const vec2_type& p_any_dn) const
			{
			vec2_type p_plain_dn;
			_valid = remap_any2plain(p_any_dn,p_plain_dn);
			if(!_valid)
				{ _clipped = false; }
			return p_plain_dn;
			}
//! @brief Inverse function to undistort().
		vec2_type distort(const vec2_type& p_plain_dn) const
			{
			vec2_type p_any_dn;
			_valid = remap_plain2any(p_plain_dn,p_any_dn);
			if(!_valid)
				{ _clipped = false; }
			return p_any_dn;
			}
//! @brief Same as undistort(const vec2_type&), but for the
//! radius instead of a 2d-point. The mapping function is
//! rotation invariant, therefore we don't see any 2d-angular dependency.
		double undistort(double r_any_dn) const
			{
			double r_plain_dn;
			_valid = remap_any2plain(r_any_dn,r_plain_dn);
			if(!_valid)
				{ _clipped = false; }
			return r_plain_dn;
			}
//! @brief Inverse function to undistort().
		double distort(double r_plain_dn) const
			{
			double r_any_dn;
			_valid = remap_plain2any(r_plain_dn,r_any_dn);
			if(!_valid)
				{ _clipped = false; }
			return r_any_dn;
			}
//! @brief Tested along main axes and near origin. This method
//! computes the derivative in radial coordinates and maps it
//! to cartesian coordinates.
		mat2_type jacobi(const vec2_type& p_any) const
			{
			double r_plain,dr_plain;
			double r_any = norm2(p_any);
// Mapping to Cartesian coordinates introduces
// a definition gap, which we solve here.
			if(r_any == 0.0)
				{ return mat2_type(1.0); }
			_valid = remap_any2plain(r_any,r_plain);
			if(!_valid)
				{
				_clipped = false;
				return mat2_type(0.0);
				}
			if(!_clipped)
				{
				_valid = derive_any2plain(r_any,dr_plain);
				}
			else
				{
// If the result has been clipped, we compute the Jacobian
// for the clipped position. Not tested.
				double r_any_clipped;
				remap_plain2any(_r_plain_dn_clip,r_any_clipped);
				_valid = derive_any2plain(r_any_clipped,dr_plain);
				}
			if(!_valid)
				{
				_clipped = false;
				return mat2_type(0.0);
				}
			mat2_type tensq_p_any = tensq(unit(p_any));
			return (mat2_type(1.0) - tensq_p_any) / r_any * r_plain + tensq_p_any * dr_plain;
			}
		};

/***************************************************************
* Equisolid Angle                                              *
***************************************************************/
//! @brief The mapping function for equisolid angle fisheye lenses.
//! Essentially theta -> 2 f sin(theta/2).
	template <class VEC2,class MAT2>
	class mapping_function_equisolid:public mapping_function_base<VEC2,MAT2>
		{
	private:
		typedef mapping_function_base<VEC2,MAT2> base_type;
		using base_type::_fl_dn;
		using base_type::_scale;
	public:
		void initialize()
			{
			base_type::initialize();
			}
//! @brief f * sqrt(2) / scale.
		double threshold_valid() const
			{ return 2.0 * _fl_dn * ::sin(M_PI / 4.0) / _scale; }
		bool map_any2angle(double r_any_dn,double& theta) const
			{
// Angular elongation in equisolid lenses
			double arg = (r_any_dn * _scale) / (2.0 * _fl_dn);
// Equisolid has a maximum elongation of 180 degree.
			if((arg >= 1) || (arg <= -1))
				{
				theta = 0.0;
				return false;
				}
			theta = 2.0 * ::asin(arg);
			return true;
			}
		double map_angle2any(double theta) const
			{
			return (2.0 * _fl_dn * ::sin(theta * (1.0/2.0))) / _scale;
			}
		bool derive_any2angle(double r_any_dn,double& dr_theta) const
			{
// Angular elongation in equisolid lenses
			double arg = (r_any_dn * _scale) / (2.0 * _fl_dn);
// Equisolid has a maximum elongation of 180 degree.
			if((arg >= 1) || (arg <= -1))
				{ return false; }
// sqrt is well-defined since we ruled out |arg| > 1 above.
			dr_theta = _scale / (_fl_dn * ::sqrt(1.0 - arg * arg));
 			return true;
			}
		};
/***************************************************************
* Equidistant                                                  *
***************************************************************/
//! @brief The mapping function for equidistant fisheye lenses.
//! Essentially theta -> f theta.
	template <class VEC2,class MAT2>
	class mapping_function_equidistant:public mapping_function_base<VEC2,MAT2>
		{
	private:
		typedef mapping_function_base<VEC2,MAT2> base_type;
		using base_type::_fl_dn;
		using base_type::_scale;
	public:
		void initialize()
			{
			base_type::initialize();
			}
// pi/2 * f / scale
		double threshold_valid() const
			{ return M_PI / 2.0 * _fl_dn / _scale; }
		bool map_any2angle(double r_any_dn,double& theta) const
			{
// Angular elongation in equidistant lenses
			theta = (r_any_dn * _scale) / _fl_dn;
			return true;
			}
		double map_angle2any(double theta) const
			{
			return (_fl_dn * theta) / _scale;
			}
		bool derive_any2angle(double r_any_dn,double& dr_theta) const
			{
			dr_theta = _scale / _fl_dn;
			return true;
			}
		};
/***************************************************************
* Orthographic                                                 *
***************************************************************/
//! @brief The mapping function for orthographic fisheye lenses.
//! Essentially theta -> f sin(theta).
	template <class VEC2,class MAT2>
	class mapping_function_orthographic:public mapping_function_base<VEC2,MAT2>
		{
	private:
		typedef mapping_function_base<VEC2,MAT2> base_type;
		using base_type::_fl_dn;
		using base_type::_scale;
	public:
		void initialize()
			{
			base_type::initialize();
			}
// f / scale
		double threshold_valid() const
			{ return _fl_dn / _scale; }
		bool map_any2angle(double r_any_dn,double& theta) const
			{
// Angular elongation in orthographic lenses
			double arg = (r_any_dn * _scale) / _fl_dn;
// orthographic has a maximum elongation of 180 degree.
			if((arg >= 1) || (arg <= -1))
				{
				theta = 0.0;
				return false;
				}
			theta = ::asin(arg);
			return true;
			}
		double map_angle2any(double theta) const
			{
			return (_fl_dn * ::sin(theta)) / _scale;
			}
		bool derive_any2angle(double r_any_dn,double& dr_theta) const
			{
// Angular elongation in orthographic lenses
			double arg = (r_any_dn * _scale) / _fl_dn;
// orthographic has a maximum elongation of 180 degree.
			if((arg >= 1) || (arg <= -1))
				{ return false; }
// sqrt is well-defined since we ruled out |arg| > 1 above.
			dr_theta = _scale / (_fl_dn * ::sqrt(1.0 - arg * arg));
			return true;
			}
		};
/***************************************************************
* Stereographic                                                *
***************************************************************/
//! @brief The mapping function for stereographic fisheye lenses.
//! Essentially theta -> 2 f tan(theta/2).
	template <class VEC2,class MAT2>
	class mapping_function_stereographic:public mapping_function_base<VEC2,MAT2>
		{
	private:
		typedef mapping_function_base<VEC2,MAT2> base_type;
		using base_type::_fl_dn;
		using base_type::_scale;
	public:
		void initialize()
			{
			base_type::initialize();
			}
// 2 * f / scale.
		double threshold_valid() const
			{ return 2.0 * _fl_dn / _scale; }
		bool map_any2angle(double r_any_dn,double& theta) const
			{
// Angular elongation in stereographic lenses
			double arg = (r_any_dn * _scale) / (2.0 * _fl_dn);
// stereographic has a maximum elongation of 180 degree.
// Testing <= -1 is redundant due to the definition of the function.
			if((arg >= 1) || (arg <= -1))
				{
				theta = 0.0;
				return false;
				}
// Due to the restriction of arg, theta is in ]-pi/2,+pi/2[.
			theta = 2.0 * ::atan(arg);
			return true;
			}
		double map_angle2any(double theta) const
			{
			return (2.0 * _fl_dn * ::tan(theta * (1.0/2.0))) / _scale;
			}
		bool derive_any2angle(double r_any_dn,double& dr_theta) const
			{
// Angular elongation in stereographic lenses
			double arg = (r_any_dn * _scale) / (2.0 * _fl_dn);
// stereographic has a maximum elongation of 180 degree.
			if((arg >= 1) || (arg <= -1))
				{ return false; }
			dr_theta = _scale / (_fl_dn * (1.0 + arg * arg));
			return true;
			}
		};
	}
