// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/ldpk_generic_anamorphic_distortion.h>
#include <ldpk/ldpk_rotation_extender.h>
#include <ldpk/ldpk_squeeze_extender.h>
#include <ldpk/ldpk_linear_extender.h>
#include <ldpk/ldpk_stabilizer.h>

//! @file tde4_ldp_anamorphic_stabilizer_degree_6.h
//! @brief Degree-6 anamorphic model with anamorphic lens rotation,
//! for rescaled anamorphic footage, with compensation for (planar) stabilizing.

//! @brief Degree-6 anamorphic model with anamorphic lens rotation,
//! for rescaled anamorphic footage, with compensation for (planar) stabilizing.
template <class VEC2,class MAT2>
class tde4_ldp_anamorphic_stabilizer_degree_6:public /* clang crashes: virtual */ ldpk::ldp_builtin<VEC2,MAT2> // ,public ldpk::stabilizer_mixin<VEC2,MAT2>
	{
private:
	typedef VEC2 vec2_type;
	typedef MAT2 mat2_type;
	typedef ldpk::vec3d vec3_type;
	typedef ldpk::mat3d mat3_type;
	typedef ldpk::ldp_builtin<VEC2,MAT2> base_type;

// Anamorphic distortion of degree 6.
	ldpk::generic_anamorphic_distortion<VEC2,MAT2,6> _anamorphic;
// Extenders for lens rotation, suqeeze, pixel aspect ratio and rescale.
	ldpk::rotation_extender<VEC2,MAT2> _rotation;
	ldpk::squeeze_x_extender<VEC2,MAT2> _squeeze_x;
	ldpk::squeeze_y_extender<VEC2,MAT2> _squeeze_y;
	ldpk::squeeze_x_extender<VEC2,MAT2> _pa,_rescale;
// Concatenating extenders for better performance
	ldpk::linear_extender<VEC2,MAT2> _rot_sqx_sqy_rescale_pa;
	ldpk::linear_extender<VEC2,MAT2> _pa_rescale_rot;
// We have 18 parameters for anamorphic degree-6
// plus 1 for lens rotation
// plus 2 for squeeze
// plus 1 for rescale.
// plus 2 for the stabilizer
	static const char* _para[24];

	using base_type::w_fb_cm;
	using base_type::h_fb_cm;
	using base_type::r_fb_cm;
	using base_type::fl_cm;
	using base_type::x_lco_cm;
	using base_type::y_lco_cm;
	ldpk::stabilizer<VEC2,MAT2> _stabilizer;

	void stabilizer_x(double v)
		{ _stabilizer.stabilizer_x(v); }
	void stabilizer_y(double v)
		{ _stabilizer.stabilizer_y(v); }
	double stabilizer_x() const
		{ return _stabilizer.stabilizer_x(); }
	double stabilizer_y() const
		{ return _stabilizer.stabilizer_y(); }
// These overrides are necessary to handle the stabilizer parameters.
	vec2_type map_unit_to_dn(const vec2_type& p_unit) const override
		{ return _stabilizer.map_unit_to_dn(p_unit); }
	vec2_type map_dn_to_unit(const vec2_type& p_dn) const override
		{ return _stabilizer.map_dn_to_unit(p_dn); }

	char const* const* get_parameter_names() override
		{ return _para; }
protected:
	bool decypher(const char* name,int& i)
		{
		typedef base_type bt;
		int n;
		getNumParameters(n);
		for(i = 0;i < n;++i)
			{
			if(0 == strcmp(name,_para[i]))
				{ return true; }
			}
		return false;
		}
	bool initializeParameters() override
		{
		typedef base_type bt;
		bt::check_builtin_parameters();
		_pa.set_sq(bt::pa());
// This method is the last one invoked, before the object can be used,
// therefore we have to prepare the concatenated extenders here.
		_rot_sqx_sqy_rescale_pa.set(_rotation,_squeeze_x,_squeeze_y,_rescale,_pa);
		if(_squeeze_x.get_sq() == 0)
			{ std::cerr << "tde4_ldp_anamorphic_stabilizer_degree_6::initializeParameters, error: Squeeze-X is 0." << std::endl; }
		if(_squeeze_y.get_sq() == 0)
			{ std::cerr << "tde4_ldp_anamorphic_stabilizer_degree_6::initializeParameters, error: Squeeze-Y is 0." << std::endl; }
		_pa_rescale_rot.set(_pa,_rescale,_rotation);
		_anamorphic.prepare();
// Stabilizer
		_stabilizer.init(w_fb_cm(),h_fb_cm(),r_fb_cm(),fl_cm(),x_lco_cm(),y_lco_cm());
		return true;
		}
	bool getNumParameters(int& n) override
		{
		n = 24;
		return true;
		}
	bool setParameterValue(const char *identifier,double v) override
		{
		typedef base_type bt;
		int i;
// Does the base class know the parameter?
		if(bt::set_builtin_parameter_value(identifier,v))
			{ return true; }
		if(!decypher(identifier,i))
			{ return false; }
// If any of the parameters is about to be changed,
// we demand that the lookup table is computed anew.
		if(i < 18)
			{
			if(_anamorphic.get_coeff(i) != v)
				{ bt::no_longer_uptodate_lut(); }
			_anamorphic.set_coeff(i,v);
			}
		else if(i == 18)
			{
			double q = v / 180.0 * M_PI;
			if(_rotation.get_phi() != q)
				{ bt::no_longer_uptodate_lut(); }
			_rotation.set_phi(q);
			}
		else if(i == 19)
			{
			if(_squeeze_x.get_sq() != v)
				{ bt::no_longer_uptodate_lut(); }
			_squeeze_x.set_sq(v);
			}
		else if(i == 20)
			{
			if(_squeeze_y.get_sq() != v)
				{ bt::no_longer_uptodate_lut(); }
			_squeeze_y.set_sq(v);
			}
		else if(i == 21)
			{
			if(_rescale.get_sq() != v)
				{ bt::no_longer_uptodate_lut(); }
			_rescale.set_sq(v);
			}
// Stabilizer
		else if(i == 22)
			{
			if(stabilizer_x() != v)
				{ bt::no_longer_uptodate_lut(); }
			stabilizer_x(v);
			}
		else if(i == 23)
			{
			if(stabilizer_y() != v)
				{ bt::no_longer_uptodate_lut(); }
			stabilizer_y(v);
			}
		return true;
		}
	double getModelDoubleParameterValueByIndex(int i) const override
		{
		if(i < 18)
			{ return _anamorphic.get_coeff(i); }
		else if(i == 18)
			{ return _rotation.get_phi(); }
		else if(i == 19)
			{ return _squeeze_x.get_sq(); }
		else if(i == 20)
			{ return _squeeze_y.get_sq(); }
		else if(i == 21)
			{ return _rescale.get_sq(); }
// Stabilizer
		else if(i == 22)
			{ return stabilizer_x(); }
		else if(i == 23)
			{ return stabilizer_y(); }
		throw std::out_of_range(std::to_string(i));
		}
	bool undistort(double x0,double y0,double &x1,double &y1) override
		{
		vec2_type q =	this->map_dn_to_unit(
					_stabilizer.eval(
						_rot_sqx_sqy_rescale_pa.eval(
							_anamorphic.eval(
								_pa_rescale_rot.eval_inv(
									this->map_unit_to_dn(vec2_type(x0,y0)))))));
		x1 = q[0];
		y1 = q[1];
		return true;
		}
	bool distort(double x0,double y0,double &x1,double &y1) override
		{
// Implementing a Nuke node it turned out that we need to prevent
// threads from trying so simultaneously. By the following double
// check of is_uptodate_lut() we keep the mutex lock out of our
// frequently called distort stuff (for performance reasons) and
// prevent threads from updating without need.
		if(!this->is_uptodate_lut())
			{
			this->lock();
			if(!this->is_uptodate_lut())
				{ this->update_lut(); }
			this->unlock();
			}
// Get initial value from lookup-table
		bool clipped;
		vec2_type q(x0,y0);
		vec2_type ps = this->get_lut().get_initial_value(q,clipped);
// Call inverse evaluation with initial value.
// We're in distort(): stabilizer comes first.
		vec2_type q_ana_dn =
			_rot_sqx_sqy_rescale_pa.eval_inv(
				_stabilizer.eval_inv(
					this->map_unit_to_dn(q)));
// Initial value for p in dn-coordinates. Since we feed the
// initial value from the lookup table into the model function
// we have to unroll the last two function applications.
		vec2_type ps_dn =
			_pa_rescale_rot.eval_inv(
				this->map_unit_to_dn(ps));

		vec2_type p;
		if(clipped)
			{ p = this->map_dn_to_unit(
				_pa_rescale_rot.eval(
					_anamorphic.eval_inv_clipped(q_ana_dn,ps_dn))); }
		else
			{ p = this->map_dn_to_unit(
				_pa_rescale_rot.eval(
					_anamorphic.eval_inv(q_ana_dn,ps_dn))); }
		x1 = p[0];
		y1 = p[1];
		return true;
		}
	bool distort(double x0,double y0,double x1_start,double y1_start,double &x1,double &y1) override
		{
// Input for the inverse polynomial model function.
// We're in distort(): stabilizer comes first.
		vec2_type q(x0,y0);
		vec2_type ps(x1_start,y1_start);
		vec2_type q_ana_dn =
			_rot_sqx_sqy_rescale_pa.eval_inv(
				_stabilizer.eval_inv(
					this->map_unit_to_dn(q)));
// Initial value for p in dn-coordinates. Since we feed the
// initial value from the lookup table into the model function
// we have to unroll the last two function applications.
		vec2_type ps_dn =
			_pa_rescale_rot.eval_inv(
				this->map_unit_to_dn(ps));
// Call inverse polynomial model function with initial value.
		vec2_type p =
			this->map_dn_to_unit(
				_pa_rescale_rot.eval(
					_anamorphic.eval_inv(q_ana_dn,ps_dn)));
		x1 = p[0];
		y1 = p[1];
		return true;
		}
public:
// Mutex initialized and destroyed in baseclass.
	tde4_ldp_anamorphic_stabilizer_degree_6()
		{ }
	~tde4_ldp_anamorphic_stabilizer_degree_6()
		{ }
	bool getModelName(char *name) override
		{
#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
		strcpy(name,"3DE4 Anamorphic - Stabilizer, Degree 6 [Plugin]");
#else
		strcpy(name,"3DE4 Anamorphic - Stabilizer, Degree 6");
#endif
		return true;
		}
	bool getParameterType(const char* identifier,tde4_ldp_ptype& ptype) override
		{
		typedef base_type bt;
		int i;
		if(bt::get_builtin_parameter_type(identifier,ptype)) return true;
		if(!decypher(identifier,i)) return false;
		ptype = TDE4_LDP_ADJUSTABLE_DOUBLE;
		return true;
		}
	bool getParameterDefaultValue(const char* identifier,double& v) override
		{
		typedef base_type bt;
		int i;
		if(!decypher(identifier,i)) return false;
		if(i < 19)
			{
// Distortion parameters and Lens Rotation
			v = 0.0;
			}
		else if((i == 19) || (i == 20))
			{
// Squeeze X/Y
			v = 1.0;
			}
		else if(i == 21)
			{
// Rescale
			v = 1.0;
			}
		else if((i == 22) || (i == 23))
			{
// Stabilizer X/Y
			v = 0.0;
			}
		return true;
		}
	bool getParameterRange(const char* identifier,double& a,double& b) override
		{
		typedef base_type bt;
		int i;
		if(!decypher(identifier,i)) return false;
		if(i < 18)
			{
			a = -0.5;
			b = 0.5;
			}
		else if(i == 18)
			{
// Lens Rotation in degree.
			a = -2.0;
			b = +2.0;
			}
		else if((i == 19) || (i == 20))
			{
// Squeeze X/Y
			a = 0.9;
			b = 1.1;
			}
		else if(i == 21)
			{
// Rescale
			a = 0.25;
			b = 4.0;
			}
		else if((i == 22) || (i == 23))
			{
// Stabilizer X/Y
			a = -0.5;
			b = +0.5;
			}
		else
			{
			std::cerr << "getParameterRange: i out of range" << std::endl;
			}
		return true;
		}
//! Tested against difference quotients.
	bool getJacobianMatrix(double x0,double y0,double& m00,double& m01,double& m10,double& m11) override
		{
		typedef base_type bt;
		vec2_type p_dn = this->map_unit_to_dn(vec2_type(x0,y0));
		mat2_type m =	  _stabilizer.jacobi(
					_rot_sqx_sqy_rescale_pa.eval(
						_anamorphic.eval(
							_pa_rescale_rot.eval_inv(p_dn))))
				* _rot_sqx_sqy_rescale_pa.get_mat()
				* _anamorphic.jacobi(
					_pa_rescale_rot.eval_inv(p_dn))
					* _pa_rescale_rot.get_mat_inv()
					;

		mat2_type u2d(bt::w_fb_cm() / bt::r_fb_cm(),0.0,0.0,bt::h_fb_cm() / bt::r_fb_cm());
		mat2_type d2u(bt::r_fb_cm() / bt::w_fb_cm(),0.0,0.0,bt::r_fb_cm() / bt::h_fb_cm());
		m = d2u * m * u2d;
		m00 = m[0][0];m01 = m[0][1];m10 = m[1][0];m11 = m[1][1];
		return true;
		}
	};

template <class VEC2,class MAT2>
const char* tde4_ldp_anamorphic_stabilizer_degree_6<VEC2,MAT2>::_para[24] = {
	"Cx02 - Degree 2","Cy02 - Degree 2",
	"Cx22 - Degree 2","Cy22 - Degree 2",

	"Cx04 - Degree 4","Cy04 - Degree 4",
	"Cx24 - Degree 4","Cy24 - Degree 4",
	"Cx44 - Degree 4","Cy44 - Degree 4",

	"Cx06 - Degree 6","Cy06 - Degree 6",
	"Cx26 - Degree 6","Cy26 - Degree 6",
	"Cx46 - Degree 6","Cy46 - Degree 6",
	"Cx66 - Degree 6","Cy66 - Degree 6",

	"Lens Rotation","Squeeze-X","Squeeze-Y",
	"Rescale",
// For stabilizer mixin
	"Stabilizer-X","Stabilizer-Y"
	};

