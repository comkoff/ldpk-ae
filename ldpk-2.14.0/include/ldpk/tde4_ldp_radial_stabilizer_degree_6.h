// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/ldpk_brown_conrady_degree_6_2.h>
#include <ldpk/ldpk_stabilizer.h>

//! @file tde4_ldp_radial_stabilizer_degree_6.h
//! @brief Plugin class for radial distortion with decentering
//! and compensation for (planar) stabilizing.

//! @brief Plugin class for radial distortion with decentering
//! and compensation for (planar) stabilizing.
template <class VEC2,class MAT2>
class tde4_ldp_radial_stabilizer_degree_6:public ldpk::ldp_builtin<VEC2,MAT2>
	{
private:
	typedef VEC2 vec2_type;
	typedef MAT2 mat2_type;
	typedef ldpk::vec3d vec3_type;
	typedef ldpk::mat3d mat3_type;
	typedef ldpk::ldp_builtin<VEC2,MAT2> base_type;
	ldpk::brown_conrady_degree_6_2<VEC2,MAT2> _radial;

// We have 5 parameters for radial degree-6-2
// plus 2 for the stabilizer
	static const char* _para[7];

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
// Our Brown-Conrady-engine has a different order of parameters.
	static const int i_3de4_to_bc[7];
	bool decypher(const char* name,int& i_bc)
		{
		typedef base_type bt;
		int n;
		getNumParameters(n);
		for(int i_3de4 = 0;i_3de4 < n;++i_3de4)
			{
			if(0 == strcmp(name,_para[i_3de4]))
				{
				i_bc = i_3de4_to_bc[i_3de4];
				return true;
				}
			}
		return false;
		}
	bool initializeParameters() override
		{
		typedef base_type bt;
		bt::check_builtin_parameters();
// Stabilizer
		_stabilizer.init(w_fb_cm(),h_fb_cm(),r_fb_cm(),fl_cm(),x_lco_cm(),y_lco_cm());
		return true;
		}
	bool getNumParameters(int& n) override
		{
		n = 7;
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
		if(i < 5)
			{
			if(_radial.get_coeff(i) != v)
				{ bt::no_longer_uptodate_lut(); }
			_radial.set_coeff(i,v);
			}
// Stabilizer
		else if(i == 5)
			{
			if(stabilizer_x() != v)
				{ bt::no_longer_uptodate_lut(); }
			stabilizer_x(v);
			}
		else if(i == 6)
			{
			if(stabilizer_y() != v)
				{ bt::no_longer_uptodate_lut(); }
			stabilizer_y(v);
			}
		return true;
		}
	double getModelDoubleParameterValueByIndex(int i) const override
		{
		if(i < 5)
			{ return _radial.get_coeff(i); }
// Stabilizer
		else if(i == 5)
			{ return stabilizer_x(); }
		else if(i == 6)
			{ return stabilizer_y(); }
		throw std::out_of_range(std::to_string(i));
		}
	virtual bool undistort(double x0,double y0,double &x1,double &y1) override
		{
		typedef base_type bt;
// Weave in the stabilizer.
		vec2_type q =	this->map_dn_to_unit(
					_stabilizer.eval(
						_radial.eval(
							this->map_unit_to_dn(vec2_type(x0,y0)))));
		x1 = q[0];
		y1 = q[1];
		return true;
		}
	virtual bool distort(double x0,double y0,double &x1,double &y1) override
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
		vec2_type q_rad =
			_stabilizer.eval_inv(this->map_unit_to_dn(q));
// Initial value for p in dn-coordinates. Since we feed the
// initial value from the lookup table into the model function
// we have to unroll the last function application.
		vec2_type ps_dn = this->map_unit_to_dn(ps);
		if(clipped)
			{ q = this->map_dn_to_unit(
				_radial.eval_inv_clipped(q_rad,ps_dn)); }
		else
			{ q = this->map_dn_to_unit(
				_radial.eval_inv(q_rad,ps_dn)); }
		x1 = q[0];
		y1 = q[1];
		return true;
		}
	virtual bool distort(double x0,double y0,double x1_start,double y1_start,double &x1,double &y1) override
		{
		vec2_type q(x0,y0);
		vec2_type ps(x1_start,y1_start);
		vec2_type q_rad =
			_stabilizer.eval_inv(
				this->map_unit_to_dn(q));
// Initial value for p in dn-coordinates. Since we feed the
// initial value from the lookup table into the model function
// we have to unroll the last function application.
		vec2_type ps_dn =
			this->map_unit_to_dn(ps);

		vec2_type p =
			this->map_dn_to_unit(
				_radial.eval_inv(q_rad,ps_dn));
		x1 = p[0];
		y1 = p[1];
		return true;
		}
public:
// Mutex initialized and destroyed in baseclass.
	tde4_ldp_radial_stabilizer_degree_6()
		{ }
	~tde4_ldp_radial_stabilizer_degree_6()
		{ }
	bool getModelName(char *name) override
		{
#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
		strcpy(name,"3DE4 Radial - Stabilizer, Degree 6 [Plugin]");
#else
		strcpy(name,"3DE4 Radial - Stabilizer, Degree 6");
#endif
		return true;
		}
	bool getParameterType(const char* identifier,tde4_ldp_ptype& ptype) override
		{
		typedef base_type bt;
		if(bt::get_builtin_parameter_type(identifier,ptype)) return true;
		int i;
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
// decypher gives us the index according to our Brown-Conrady-engine.
		int i_bc;
		if(!decypher(identifier,i_bc)) return false;
		if((i_bc == 0) || (i_bc == 1) || (i_bc == 2))
			{
// Distortion - Degree 2, 4, 6
			a = -0.5;b = 0.5;
			}
		else if((i_bc == 3) || (i_bc == 4))
			{
// U2,V2.
			a = -0.1;
			b = 0.1;
			}
		else if((i_bc == 5) || (i_bc == 6))
			{
// Stabilizer X/Y
			a = -0.5;
			b = 0.5;
			}
		return true;
		}
	bool getJacobianMatrix(double x0,double y0,double& m00,double& m01,double& m10,double& m11) override
		{
		typedef base_type bt;
// Chain rule: d/dp s o g(p) = d/dg s(g(p)) * d/dp g(p).
// where `s` stands for the stabilizer transform and `g`
// for the radial distortion model function.
		vec2_type p_dn = this->map_unit_to_dn(vec2_type(x0,y0));
		mat2_type m =     _stabilizer.jacobi(
					 _radial.eval(p_dn))
				* _radial.jacobi(
					p_dn);

		mat2_type u2d(bt::w_fb_cm() / bt::r_fb_cm(),0.0,0.0,bt::h_fb_cm() / bt::r_fb_cm());
		mat2_type d2u(bt::r_fb_cm() / bt::w_fb_cm(),0.0,0.0,bt::r_fb_cm() / bt::h_fb_cm());
		m = d2u * m * u2d;
		m00 = m[0][0];m01 = m[0][1];m10 = m[1][0];m11 = m[1][1];
		return true;
		}
	};

template <class VEC2,class MAT2>
const int tde4_ldp_radial_stabilizer_degree_6<VEC2,MAT2>::i_3de4_to_bc[7] = {0,3,4,1,2,5,6};

template <class VEC2,class MAT2>
const char* tde4_ldp_radial_stabilizer_degree_6<VEC2,MAT2>::_para[7] = {
	"Distortion - Degree 2",
	"U - Degree 2",
	"V - Degree 2",
	"Distortion - Degree 4",
	"Distortion - Degree 6",
// For stabilizer mixin
	"Stabilizer-X","Stabilizer-Y"
	};

