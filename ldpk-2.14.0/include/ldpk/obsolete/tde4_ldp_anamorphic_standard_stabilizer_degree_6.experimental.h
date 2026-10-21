#pragma once

#include <ldpk/ldpk_stabilizer_mixin.h>
#include <ldpk/ldpk_generic_anamorphic_distortion.h>
#include <ldpk/ldpk_rotation_extender.h>
#include <ldpk/ldpk_squeeze_extender.h>
#include <ldpk/ldpk_linear_extender.h>

//! @file tde4_ldp_anamorphic_standard_stabilizer_degree_6.h
//! Degree-4 anamorphic model with anamorphic lens rotation and stabilizer offset

template <class VEC2,class MAT2>
class tde4_ldp_anamorphic_standard_stabilizer_degree_6:public virtual ldpk::ldp_builtin<VEC2,MAT2>,public ldpk::stabilizer_mixin<VEC2,MAT2>
	{
private:
	typedef VEC2 vec2_type;
	typedef MAT2 mat2_type;
	typedef ldpk::ldp_builtin<VEC2,MAT2> base_type;
	using base_type::w_fb_cm;
	using base_type::h_fb_cm;
	using base_type::r_fb_cm;
	using ldpk::stabilizer_mixin<VEC2,MAT2>::stabilizer_x;
	using ldpk::stabilizer_mixin<VEC2,MAT2>::stabilizer_y;

// Anamorphic distortion of degree 6.
	ldpk::generic_anamorphic_distortion<VEC2,MAT2,6> _anamorphic;
// Extenders for lens rotation, suqeeze and pixel aspect ratio.
	ldpk::rotation_extender<VEC2,MAT2> _rotation;
	ldpk::squeeze_x_extender<VEC2,MAT2> _squeeze_x;
	ldpk::squeeze_y_extender<VEC2,MAT2> _squeeze_y;
	ldpk::squeeze_x_extender<VEC2,MAT2> _pa;
// Concatenating extenders for better performance
	ldpk::linear_extender<VEC2,MAT2> _rot_sqx_sqy_pa;
	ldpk::linear_extender<VEC2,MAT2> _pa_rot;
// We have 18 parameters for anamorphic degree-6
// plus 1 for lens rotation plus 2 for squeeze.
// plus 2 for the stabilizer
	static const char* _para[23];
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
		_rot_sqx_sqy_pa.set(_rotation,_squeeze_x,_squeeze_y,_pa);
		if(_squeeze_x.get_sq() == 0)
			{ std::cerr << "tde4_ldp_anamorphic_standard_stabilizer_degree_6::initializeParameters, error: Squeeze-X is 0." << std::endl; }
		if(_squeeze_y.get_sq() == 0)
			{ std::cerr << "tde4_ldp_anamorphic_standard_stabilizer_degree_6::initializeParameters, error: Squeeze-Y is 0." << std::endl; }
		_pa_rot.set(_pa,_rotation);
		_anamorphic.prepare();
		return true;
		}
	bool getNumParameters(int& n) override
		{
		n = 23;
		return true;
		}
	bool getParameterName(int i,char* identifier) override
		{
		strcpy(identifier,_para[i]);
		return true;
		}
	bool setParameterValue(const char *identifier,double v) override
		{
		typedef base_type bt;
		int i;
// Does the base class know the parameter?
		if(bt::set_builtin_parameter_value(identifier,v))
			{
			return true;
			}
		if(!decypher(identifier,i))
			{
			return false;
			}
		if(i < 18)
			{
			_anamorphic.set_coeff(i,v);
			}
		else if(i == 18)
			{
			_rotation.set_phi(v / 180.0 * M_PI);
			}
		else if(i == 19)
			{
			_squeeze_x.set_sq(v);
			}
		else if(i == 20)
			{
			_squeeze_y.set_sq(v);
			}
// Stabilizer
		else if(i == 21)
			{
			stabilizer_x(v);
			}
		else if(i == 22)
			{
			stabilizer_y(v);
			}
		return true;
		}
	bool undistort(double x0,double y0,double &x1,double &y1) override
		{
		typedef base_type bt;
		vec2_type q =	this->map_dn_to_unit(
					_rot_sqx_sqy_pa.eval(
						_anamorphic.eval(
							_pa_rot.eval_inv(
								this->map_unit_to_dn(vec2_type(x0,y0))))));
		x1 = q[0];
		y1 = q[1];
		return true;
		}
	bool distort(double x0,double y0,double &x1,double &y1) override
		{
		typedef base_type bt;
// Implementing a Nuke node it turned out that we need to prevent
// threads from trying so simultaneously. By the following double
// check of is_uptodate_lut() we keep the mutex lock out of our
// frequently called distort stuff (for performance reasons) and
// prevent threads from updating without need.
		if(!bt::is_uptodate_lut())
			{
			bt::lock();
			if(!bt::is_uptodate_lut())
				{ bt::update_lut(); }
			bt::unlock();
			}
// Get initial value from lookup-table
		bool clipped;
		vec2_type qs = bt::get_lut().get_initial_value(vec2_type(x0,y0),clipped);
// Call inverse evaluation with initial value.
		vec2_type q(x0,y0);
		vec2_type q_ana_dn = _rot_sqx_sqy_pa.eval_inv(this->map_unit_to_dn(q));
		vec2_type qs_ana_dn = _rot_sqx_sqy_pa.eval_inv(this->map_unit_to_dn(qs));
		if(clipped)
			{ q = this->map_dn_to_unit(_pa_rot.eval(_anamorphic.eval_inv_clipped(q_ana_dn,qs_ana_dn))); }
		else
			{ q = this->map_dn_to_unit(_pa_rot.eval(_anamorphic.eval_inv(q_ana_dn,qs_ana_dn))); }
		x1 = q[0];
		y1 = q[1];
		return true;
		}
	bool distort(double x0,double y0,double x1_start,double y1_start,double &x1,double &y1) override
		{
		typedef base_type bt;
		vec2_type q =	this->map_dn_to_unit(
					_pa_rot.eval(
						_anamorphic.eval_inv(
							_rot_sqx_sqy_pa.eval_inv(
								this->map_unit_to_dn(vec2_type(x0,y0))),
							_rot_sqx_sqy_pa.eval_inv(
								this->map_unit_to_dn(vec2_type(x1_start,y1_start))))));
		x1 = q[0];
		y1 = q[1];
		return true;
		}
public:
// Mutex initialized and destroyed in baseclass.
	tde4_ldp_anamorphic_standard_stabilizer_degree_6()
		{ }
	~tde4_ldp_anamorphic_standard_stabilizer_degree_6()
		{ }
	bool getModelName(char *name) override
		{
#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
		strcpy(name,"3DE4 Anamorphic - Standard - Stabilizer, Degree 6 [Plugin]");
#else
		strcpy(name,"3DE4 Anamorphic - Standard - Stabilizer, Degree 6");
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
// Lens Rotation
			v = 0.0;
			}
		else if((i == 19) || (i == 20))
			{
// Squeeze X/Y
			v = 1.0;
			}
		else if((i == 21) || (i == 22))
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
		else if((i == 21) || (i == 22))
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
		mat2_type m = _rot_sqx_sqy_pa.get_mat()
					* _anamorphic.jacobi(
						_pa_rot.eval_inv(
							this->map_unit_to_dn(vec2_type(x0,y0))))
						* _pa_rot.get_mat_inv();
		mat2_type u2d(bt::w_fb_cm() / bt::r_fb_cm(),0.0,0.0,bt::h_fb_cm() / bt::r_fb_cm());
		mat2_type d2u(bt::r_fb_cm() / bt::w_fb_cm(),0.0,0.0,bt::r_fb_cm() / bt::h_fb_cm());
		m = d2u * m * u2d;
		m00 = m[0][0];m01 = m[0][1];m10 = m[1][0];m11 = m[1][1];
		return true;
		}
	};

template <class VEC2,class MAT2>
const char* tde4_ldp_anamorphic_standard_stabilizer_degree_6<VEC2,MAT2>::_para[23] = {
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
// For stabilizer mixin
	"Stabilizer-X","Stabilizer-Y"
	};

