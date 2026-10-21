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
uniform float sq;
uniform float cx;
uniform float cy;
uniform float c4;

float cxx = c2 / sq;
float cxy = (c2 + cx) / sq;
float cyx = c2 + cy;
float cyy = c2;
float cxxx = c4 / sq;
float cxxy = 2.0 * c4 / sq;
float cxyy = c4 / sq;
float cyxx = c4;
float cyyx = 2.0 * c4;
float cyyy = c4;

#include <snp/basics/prg_texture2d_bicubic_catmull_rom.snp>

/***************************************************************
* model-dependent functions                                    *
***************************************************************/
vec2 classic(vec2 p_diag)
	{
// apply undistortion
	float x = p_diag[0];
	float y = p_diag[1];
	float x2 = x * x;
	float y2 = y * y;
	float x4 = x2 * x2;
	float y4 = y2 * y2;
	float x2y2 = x2 * y2;

	vec2 q_diag;
	q_diag[0] = x * (1.0 + cxx * x2 + cxy * y2 + cxxx * x4 + cxxy * x2y2 + cxyy * y4);
	q_diag[1] = y * (1.0 + cyx * x2 + cyy * y2 + cyxx * x4 + cyyx * x2y2 + cyyy * y4);
	return q_diag;
	}
vec2 undistort_diag(vec2 p_diag)
	{
	return classic(p_diag);
	}
mat2 jacobian(vec2 p_diag)
	{
	mat2 m;
	float x = p_diag[0];
	float x2 = x * x;
	float x3 = x2 * x;
	float x4 = x2 * x2;
	float y = p_diag[1];
	float y2 = y * y;
	float y3 = y2 * y;
	float y4 = y2 * y2;
// Note that in glsl [i][j] is the component at column i and row j,
// whereas in the LDPK we use the notation [i][j] like in mathematics
// and physics, so that matrix multiplication is (a b)[i][k] = \sum_j a[i][j] b[j][k]
	m[0][0] = 1.0 + x2 * 3.0 * cxx + y2 * cxy
		+ x4 * 5.0 * cxxx + x2 * y2 * 3.0 * cxxy + y4 * cxyy;
	m[1][1] = 1.0 + x2 * cyx + y2 * 3.0 * cyy
		+ x4 * cyxx + x2 * y2 * 3.0 * cyyx + y4 * 5.0 * cyyy;
	m[1][0] = x * y * 2.0 * cxy + x3 * y * 2.0 * cxxy + x * y3 * 4.0 * cxyy;
	m[0][1] = x * y * 2.0 * cyx + x3 * y * 4.0 * cyxx + x * y3 * 2.0 * cyyx;
	return m;
	}

#include <snp/ldpk/prg_model_independent_2.snp>
