/*
# Versions:
# - 1.1.3 [2023-09-10]	work around incomplete glsl-compiler on macOS
# - 1.1.2 [2023-09-04]	building shaders from templates (again)
# - 1.1.1 [2023-08-30]	conforming to new version of sdv_tde4_script
# - 1.1.0 [2023-08-29]	conforming to sdv scripting guideline
# - 1.0.0 [0000-00-00]	initial
# Version: 1.1.3
# Location: sys_data/flame/2019.1
# Source: $LDPK/3de4/python
# Scope: public
# Maintainer: uwe@sci-d-vis.com
*/
#include <snp/num/prg_clenshaw_1_rollout.snp>
#include <snp/ldpk/prg_model_independent_1.snp>

/***************************************************************
* model-dependent parameters                                   *
***************************************************************/
uniform float c2;
uniform float c4;
uniform float c6;
uniform float t1;
uniform float t2;

#include <snp/basics/prg_texture2d_bicubic_catmull_rom.snp>

/***************************************************************
* model-dependent functions                                    *
***************************************************************/
vec2 radial_brown_conrady(vec2 p_diag)
	{
// apply undistortion
	float x = p_diag[0];
	float y = p_diag[1];
	float x2 = x * x;
	float y2 = y * y;
	float xy = x * y;
	float r2 = x2 + y2;
	float r4 = r2 * r2;
	float r6 = r4 * r2;

	vec2 q_diag;
	q_diag[0]	= x * (1.0 + c2 * r2 + c4 * r4 + c6 * r6)
			+ t1 * (r2 + 2.0 * x2) + 2.0 * t2 * xy;

	q_diag[1]	= y * (1.0 + c2 * r2 + c4 * r4 + c6 * r6)
			+ t2 * (r2 + 2.0 * y2) + 2.0 * t1 * xy;
	return q_diag;
	}
vec2 undistort_diag(vec2 p_diag)
	{
	return radial_brown_conrady(p_diag);
	}
mat2 jacobian(vec2 p_diag)
	{
	mat2 m;
// pow 1
	float x = p_diag[0];
	float y = p_diag[1];
// pow 2
	float x2 = x * x;
	float xy = x * y;
	float y2 = y * y;
// pow 3
	float x3 = x2 * x;
	float x2y = xy * x;
	float xy2 = xy * y;
	float y3 = y2 * y;
// pow 4
	float x4 = x2 * x2;
	float x3y = x3 * y;
	float x2y2 = x2 * y2;
	float xy3 = x * y3;
	float y4 = y2 * y2;
// pow 5
	float x5 = x3 * x2;
	float y5 = y3 * y2;
// pow 6
	float x6 = x4 * x2;
	float x5y = x5 * y;
	float x4y2 = x4 * y2;
	float x3y3 = x3 * y3;
	float x2y4 = x2 * y4;
	float xy5 = x * y5;
	float y6 = y4 * y2;

	m[0][0] = 1.0	+ c2 * (3.0 * x2 + y2)
			+ c4 * (5.0 * x4 + 6.0 * x2y2 + y4)
			+ c6 * (7.0 * x6 + 15.0 * x4y2 + 9.0 * x2y4 + y6)
			+ 6.0 * t1 * x
			+ 2.0 * t2 * y;
	m[1][1] = 1.0	+ c2 * (3.0 * y2 + x2)
			+ c4 * (5.0 * y4 + 6.0 * x2y2 + x4)
			+ c6 * (7.0 * y6 + 15.0 * x2y4 + 9.0 * x4y2 + x6)
			+ 6.0 * t2 * y
			+ 2.0 * t1 * x;
	m[0][1] = 	  2.0 * c2 * xy
			+ c4 * (4.0 * x3y + 4.0 * xy3)
			+ c6 * (6.0 * x5y + 12.0 * x3y3 + 6.0 * xy5)
			+ 2.0 * t1 * y
			+ 2.0 * t2 * x;
	m[1][0] = 	  2.0 * c2 * xy
			+ c4 * (4.0 * xy3 + 4.0 * x3y)
			+ c6 * (6.0 * xy5 + 12.0 * x3y3 + 6.0 * x5y)
			+ 2.0 * t2 * x
			+ 2.0 * t1 * y;
	return m;
	}

#include <snp/ldpk/prg_model_independent_2.snp>
