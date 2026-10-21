// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#ifndef tde4_ldp_noop_debugging_sdv
#define tde4_ldp_noop_debugging_sdv

#include <ldpk/ldpk_ldp_builtin.h>

//! @file tde4_ldp_noop_debugging.h
//! @brief Plugin class for debugging

//! @brief Plugin class for debugging
template <class VEC2,class MAT2>
class tde4_ldp_noop_debugging:public ldpk::ldp_builtin<VEC2,MAT2>
	{
private:
	typedef VEC2 vec2_type;
	typedef MAT2 mat2_type;
	typedef ldpk::ldp_builtin<VEC2,MAT2> base_type;

	static const char* _para[0];

	char const* const* get_parameter_names() override
		{ return _para; }
protected:
	bool decypher(const char* name,int& i)
		{
		return false;
		}
	bool initializeParameters()
		{
		typedef base_type bt;
		bt::check_builtin_parameters();
		return true;
		}
	bool getNumParameters(int& n)
		{
		n = 0;
		return true;
		}
	bool setParameterValue(const char *identifier,double v)
		{
		return true;
		}
	virtual bool undistort(double x0,double y0,double &x1,double &y1)
		{
		x1 = x0;
		y1 = y0;
		return true;
		}
	virtual bool distort(double x0,double y0,double &x1,double &y1)
		{
		x1 = x0;
		y1 = y0;
		return true;
		}
	virtual bool distort(double x0,double y0,double x1_start,double y1_start,double &x1,double &y1)
		{
		x1 = x0;
		y1 = y0;
		return true;
		}
public:
// Mutex initialized and destroyed in baseclass.
	tde4_ldp_noop_debugging()
		{ }
	~tde4_ldp_noop_debugging()
		{ }
	bool getModelName(char *name)
		{
#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
		strcpy(name,"3DE4 Noop, Debugging [Plugin]");
#else
		strcpy(name,"3DE4 Noop, Debugging Degree 4");
#endif
		return true;
		}
	bool getParameterType(const char* identifier,tde4_ldp_ptype& ptype)
		{
		ptype = TDE4_LDP_ADJUSTABLE_DOUBLE;
		return true;
		}
	bool getParameterDefaultValue(const char* identifier,double& v)
		{
		v = 0.0;
		return true;
		}
	bool getParameterRange(const char* identifier,double& a,double& b)
		{
		return true;
		}
	bool getJacobianMatrix(double x0,double y0,double& m00,double& m01,double& m10,double& m11)
		{
		m00 = m11 = 1.0;
		m01 = m10 = 0.0;
		return true;
		}
	};

template <class VEC2,class MAT2>
const char* tde4_ldp_noop_debugging<VEC2,MAT2>::_para[0] = {
	};

#endif
