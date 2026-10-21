// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/ldpk_ldp_builtin.h>
#include <ldpk/ldpk_anamorphic_homomorphic_distortion.h>

//! @file tde4_ldp_anamorphic_homomorphic_degree_2.h
//! @brief Plugin class for anamorphic distortion

//! @brief Plugin class for homomorphic anamorphic distortion
//! Not sure if parameters can be calculated by 3DE's Matrix Tool.
template <class VEC2,class MAT2>
class tde4_ldp_anamorphic_homomorphic_degree_2:public ldpk::ldp_builtin<VEC2,MAT2>
	{
private:
	typedef VEC2 vec2_type;
	typedef MAT2 mat2_type;
	typedef ldpk::ldp_builtin<VEC2,MAT2> base_type;

	ldpk::anamorphic_homomorphic_distortion<VEC2,MAT2> _anamorphic;

	static const char* _para[3];

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
				{
				return true;
				}
			}
		return false;
		}
	bool initializeParameters() override
		{
		typedef base_type bt;
		bt::check_builtin_parameters();
		return true;
		}
	bool getNumParameters(int& n) override
		{
		n = 3;
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
		if(i < 3)
			{
			if(_anamorphic.get_coeff(i) != v)
				{ bt::no_longer_uptodate_lut(); }
			_anamorphic.set_coeff(i,v);
			}
		return true;
		}
	double getModelDoubleParameterValueByIndex(int i) const override
		{
		if(i < 3)
			{ return _anamorphic.get_coeff(i); }
		throw std::out_of_range(std::to_string(i));
		}
	bool undistort(double x0,double y0,double &x1,double &y1) override
		{
		typedef base_type bt;
		vec2_type q_dn = _anamorphic.eval(bt::map_unit_to_dn(vec2_type(x0,y0)));
		if(!_anamorphic.valid())
			{
			x1 = 0.0;
			y1 = 0.0;
			return false;
			}
// Clipping to a reasonable area, still n times as large as the image.
		if(norm2(q_dn) > 100.0)
			{ q_dn = 100.0 * unit(q_dn); }
		
		vec2_type q = bt::map_dn_to_unit(q_dn);
		x1 = q[0];
		y1 = q[1];
		return true;
		}
	bool distort(double x0,double y0,double &x1,double &y1) override
		{
		typedef base_type bt;
// Special property of this model: we have a non-iterative method eval_inverse().
		vec2_type q_dn = _anamorphic.eval_inv(bt::map_unit_to_dn(vec2_type(x0,y0)));
		if(!_anamorphic.valid())
			{
			x1 = 0.0;
			y1 = 0.0;
			return false;
			}
// Clipping to a reasonable area, still n times as large as the image.
		if(norm2(q_dn) > 100.0)
			{ q_dn = 100.0 * unit(q_dn); }
		
		vec2_type q = bt::map_dn_to_unit(q_dn);
		x1 = q[0];
		y1 = q[1];
		return true;
		}
	bool distort(double x0,double y0,double x1_start,double y1_start,double &x1,double &y1) override
		{
// Special property of this model: we have a non-iterative method eval_inverse().
		return distort(x0,y0,x1,y1);
		}
public:
// Mutex initialized and destroyed in baseclass.
	tde4_ldp_anamorphic_homomorphic_degree_2()
		{ }
	~tde4_ldp_anamorphic_homomorphic_degree_2()
		{ }
	bool getModelName(char *name) override
		{
#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
		strcpy(name,"3DE4 Anamorphic - Homomorphic, Degree 2 [Plugin]");
#else
		strcpy(name,"3DE4 Anamorphic - Homomorphic, Degree 2");
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
		v = 0.0;
		return true;
		}
	bool getParameterRange(const char* identifier,double& a,double& b) override
		{
		typedef base_type bt;
		int i;
		if(!decypher(identifier,i)) return false;
// Distortion - Degree 2
		a = -0.5;b = 0.5;

		return true;
		}
	bool getJacobianMatrix(double x0,double y0,double& m00,double& m01,double& m10,double& m11) override
		{
		typedef base_type bt;
		mat2_type m = _anamorphic.jacobi(
					bt::map_unit_to_dn(vec2_type(x0,y0)));
		mat2_type u2d(bt::w_fb_cm() / bt::r_fb_cm(),0.0,0.0,bt::h_fb_cm() / bt::r_fb_cm());
		mat2_type d2u(bt::r_fb_cm() / bt::w_fb_cm(),0.0,0.0,bt::r_fb_cm() / bt::h_fb_cm());
		m = d2u * m * u2d;
		m00 = m[0][0];m01 = m[0][1];m10 = m[1][0];m11 = m[1][1];
		return true;
		}
	};

template <class VEC2,class MAT2>
const char* tde4_ldp_anamorphic_homomorphic_degree_2<VEC2,MAT2>::_para[3] = {
	"Cx2 - Degree 2",
	"Cy2 - Degree 2",
	"B2 - Degree 2",
	};

