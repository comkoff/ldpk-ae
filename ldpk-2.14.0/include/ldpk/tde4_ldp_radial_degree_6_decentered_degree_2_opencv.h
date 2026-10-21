// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2022,  Science-D-Visions. Current version: 2.8

#pragma once

#include <ldpk/ldpk_ldp_builtin.h>
#include <ldpk/ldpk_radial_decentered_distortion_opencv.h>

//! @file tde4_ldp_radial_degree_6_decentered_degree_2_opencv.h
//! @brief Plugin class for radial distortion with decentering
//! as defined in OpenCV.

//! @brief Plugin class for radial distortion with decentering
//! Not sure if parameters can be calculated by 3DE's Matrix Tool.
template <class VEC2,class MAT2>
class tde4_ldp_radial_degree_6_decentered_degree_2_opencv:public ldpk::ldp_builtin<VEC2,MAT2>
	{
private:
	typedef VEC2 vec2_type;
	typedef MAT2 mat2_type;
	typedef ldpk::ldp_builtin<VEC2,MAT2> base_type;

	ldpk::radial_decentered_distortion_opencv<vec2_type,mat2_type> _radial;

	static const char* _para[5];

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
//! @brief Implies setting the rho-factor for mapping between
//! dn- and OpenCV-coordinates. rho is based on the following:
//! The OpenCV literature says, the model is based on filmback
//! coordinates, which means the coefficients are not dimensionless.
//! Our dn-coordinates are expressed in filmback coordinates like
//! x_cm = x_dn * r_fb_cm and y_cm = y_dn * r_fb_cm. Our cm-coordinates
//! describe a virtual filmback, where x_cm = x_cm_phys * r_pa and
//! y_cm = y_cm_phys. In OpenCV it seems that virtual and physical
//! Filmback are defined as x_cm = x_cm_phys and y_cm = y_cm_phys / r_pa.
//! Their model is based on their virtual filmback, so that
//! x_cm_ocv = x_cm_sdv / r_pa = x_dn * r_fb_cm / r_pa and
//! y_cm_ocv = y_cm_sdv / r_pa = y_dn * r_fb_cm / r_pa.
//! We define rho by x_cm_ocv = rho * x_dn, and hence
//! rho = r_fb_cm / r_pa.
	bool initializeParameters()
		{
		typedef base_type bt;
		bt::check_builtin_parameters();
		_radial.set_rho(bt::r_fb_cm() / bt::fl_cm());
		return true;
		}
	bool getNumParameters(int& n)
		{
		n = 5;
		return true;
		}
	bool getParameterName(int i,char* identifier)
		{
		strcpy(identifier,_para[i]);
		return true;
		}
//! @brief Model parameters in OpenCV coordinates.
	bool setParameterValue(const char *identifier,double v)
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
		if(i < 5)
			{
			if(_radial.get_coeff(i) != v)
				{ bt::no_longer_uptodate_lut(); }
			_radial.set_coeff(i,v);
			}
		return true;
		}
//	virtual bool undistort(double x0,double y0,double &x1,double &y1)
	virtual bool distort(double x0,double y0,double &x1,double &y1)
		{
		typedef base_type bt;
		vec2_type q =	bt::map_dn_to_unit(
					_radial.eval(
						bt::map_unit_to_dn(vec2_type(x0,y0))));
		x1 = q[0];
		y1 = q[1];
		return true;
		}
//	virtual bool distort(double x0,double y0,double &x1,double &y1)
	virtual bool undistort(double x0,double y0,double &x1,double &y1)
		{
		typedef base_type bt;
// The distort-method without initial values is not constant by semantics,
// since it may cause an update of the lookup-tables. Implementing a Nuke node
// it turned out that we need to prevent threads from trying so simultaneously.
// By the following double check of is_uptodate_lut() we keep the mutex lock
// out of our frequently called distort stuff (for performance reasons) and
// prevent threads from updating without need.
		if(!bt::is_uptodate_lut())
			{
			bt::lock();
			if(!bt::is_uptodate_lut())
				{
				bt::update_lut();
				}
			bt::unlock();
			}

// Get initial value from lookup-table
		vec2_type qs = bt::get_lut().get_initial_value(vec2_type(x0,y0));
// Call version of distort with initial value.
		vec2_type q =	bt::map_dn_to_unit(
					_radial.eval_inv(
						bt::map_unit_to_dn(vec2_type(x0,y0)),
						bt::map_unit_to_dn(qs)));
		x1 = q[0];
		y1 = q[1];
		return true;
		}
/*
	virtual bool distort(double x0,double y0,double x1_start,double y1_start,double &x1,double &y1)
		{
		typedef base_type bt;
		vec2_type q =	bt::map_dn_to_unit(
					_radial.map_inverse(
						bt::map_unit_to_dn(vec2_type(x0,y0)),
						bt::map_unit_to_dn(vec2_type(x1_start,y1_start))));
		x1 = q[0];
		y1 = q[1];
		return true;
		}
*/
public:
// Mutex initialized and destroyed in baseclass.
	tde4_ldp_radial_degree_6_decentered_degree_2_opencv()
		{ }
	~tde4_ldp_radial_degree_6_decentered_degree_2_opencv()
		{ }
	bool getModelName(char *name)
		{
#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
		strcpy(name,"OpenCV Radial Degree 6, Decentered Degree 2 [Plugin]");
#else
		strcpy(name,"OpenCV Radial Degree 6, Decentered Degree 2");
#endif
		return true;
		}
	bool getParameterType(const char* identifier,tde4_ldp_ptype& ptype)
		{
		typedef base_type bt;
		int i;
		if(bt::get_builtin_parameter_type(identifier,ptype)) return true;
		if(!decypher(identifier,i)) return false;
		ptype = TDE4_LDP_ADJUSTABLE_DOUBLE;
		return true;
		}
	bool getParameterDefaultValue(const char* identifier,double& v)
		{
		typedef base_type bt;
		int i;
		if(!decypher(identifier,i)) return false;
		v = 0.0;
		return true;
		}
	bool getParameterRange(const char* identifier,double& a,double& b)
		{
		typedef base_type bt;
		int i;
		if(!decypher(identifier,i)) return false;
		if((i == 0) || (i == 1) || (i == 4))
			{
// Distortion - Degree 2, Distortion - Degree 4, Distortion - Degree 6
			a = -0.5;b = 0.5;
			}
		else if((i == 2) || (i == 3))
			{
// Decentering X/Y
			a = -0.5;b = 0.5;
			}
		return true;
		}
	bool getJacobianMatrix(double x0,double y0,double& m00,double& m01,double& m10,double& m11)
		{
		typedef base_type bt;
		mat2_type m = _radial.jacobi(
					bt::map_unit_to_dn(vec2_type(x0,y0)));
// to myself: Eigentlich w/2,h/2 bei beiden. Kuerzt sich weg.
		mat2_type u2d(bt::w_fb_cm() / bt::r_fb_cm(),0.0,0.0,bt::h_fb_cm() / bt::r_fb_cm());
		mat2_type d2u(bt::r_fb_cm() / bt::w_fb_cm(),0.0,0.0,bt::r_fb_cm() / bt::h_fb_cm());
		m = d2u * m * u2d;
		m00 = m[0][0];m01 = m[0][1];m10 = m[1][0];m11 = m[1][1];
		return true;
		}
	};

template <class VEC2,class MAT2>
const char* tde4_ldp_radial_degree_6_decentered_degree_2_opencv<VEC2,MAT2>::_para[5] = {
	"Radial Distortion - Degree 2 - k1",
	"Radial Distortion - Degree 4 - k2",
	"Decentering Y - Degree 2 - p1",
	"Decentering X - Degree 2 - p2",
	"Radial Distortion - Degree 6 - k3",
	};

