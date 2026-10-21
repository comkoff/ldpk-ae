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
uniform float distortion_degree_2;
uniform float u_degree_2;
uniform float v_degree_2;
uniform float quartic_distortion_degree_4;
uniform float u_degree_4;
uniform float v_degree_4;
uniform float phi_cylindric_direction;
uniform float b_cylindric_bending;

#include <snp/basics/prg_texture2d_bicubic_catmull_rom.snp>

/***************************************************************
* model-dependent functions                                    *
***************************************************************/
mat2 cylindric_mat()
	{
	float q = sqrt(1.0 + b_cylindric_bending);
	float c = cos(radians(phi_cylindric_direction));
	float s = sin(radians(phi_cylindric_direction));
	return mat2(	c * c * q + s * s / q,(q - 1.0 / q) * c * s,
			(q - 1.0 / q) * c * s,c * c / q + s * s * q);
	}
vec2 cylindric(vec2 p_diag)
	{
	mat2 m = cylindric_mat();
	return m * p_diag;
	}
vec2 radial(vec2 p_diag)
	{
// apply undistortion
	float x = p_diag[0];
	float y = p_diag[1];
	float x2 = x * x;
	float y2 = y * y;
	float xy = x * y;
	float r2 = x2 + y2;
	float r4 = r2 * r2;

	float c2 = distortion_degree_2;
	float c4 = quartic_distortion_degree_4;
	float u1 = u_degree_2;
	float v1 = v_degree_2;
	float u3 = u_degree_4;
	float v3 = v_degree_4;

	vec2 q_diag;
	q_diag[0]	= x			* (1.0 + c2 * r2 + c4 * r4)
			+ (r2 + 2.0 * x2)	* (u1 + u3 * r2)
			+ 2.0 * xy		* (v1 + v3 * r2);

	q_diag[1]	= y			* (1.0 + c2 * r2 + c4 * r4)
			+ (r2 + 2.0 * y2)	* (v1 + v3 * r2)
			+ 2.0 * xy		* (u1 + u3 * r2);
	return q_diag;
	}
vec2 undistort_diag(vec2 p_diag)
	{
	return cylindric(radial(p_diag));
	}
mat2 jacobian(vec2 p_diag)
	{
//	float epsilon = 0.001;
//	mat2 j_trans;
//	j_trans[0] = (undistort_diag(p_diag + vec2(epsilon,0.0)) - undistort_diag(p_diag - vec2(epsilon,0.0))) / (2.0 * epsilon);
//	j_trans[1] = (undistort_diag(p_diag + vec2(0.0,epsilon)) - undistort_diag(p_diag - vec2(0.0,epsilon))) / (2.0 * epsilon);
//	return trans(j_trans);
// Since undistort() is the composition of the linear mapping cylindric()
// and a non-trivial mapping radial(), the linear mapping is extracted
// by the chain-rule for derivatives. We therefore have to calculate
// the derivative of radial().
	mat2 m;
	float x = p_diag[0];
	float y = p_diag[1];
	float x2 = x * x;
	float y2 = y * y;
	float x3 = x2 * x;
	float y3 = y2 * y;
	float xy = x * y;
	float x2y = xy * x;
	float xy2 = xy * y;
	float r2 = x2 + y2;
	float c2 = distortion_degree_2;
	float c4 = quartic_distortion_degree_4;
	float u1 = u_degree_2;
	float v1 = v_degree_2;
	float u3 = u_degree_4;
	float v3 = v_degree_4;
	float u1x = u1 * x;
	float v1y = v1 * y;
	float c4r2 = c4 * r2;
	float m_off_diag_common	= 2.0 * c2 * xy + 4.0 * c4 * xy * r2
					+ 2.0 * u1 * y + 2.0 * v1 * x;
// Note that in glsl [i][j] is the component at column i and row j,
// whereas in the LDPK we use the notation [i][j] like in mathematics
// and physics, so that matrix multiplication is (a b)[i][k] = \sum_j a[i][j] b[j][k]
	m[0][0] = 1.0 + c2 * (3.0 * x2 + y2) + c4r2 * (5.0 * x2 + y2)
		+ 6.0 * u1x + u3 * (8.0 * xy2 + 12.0 * x3)
		+ 2.0 * v1y + v3 * (2.0 * y3 + 6.0 * x2y);
	m[1][1] = 1.0 + c2 * (x2 + 3.0 * y2) + c4r2 * (x2 + 5.0 * y2)
		+ 6.0 * v1y + v3 * (8.0 * x2y + 12.0 * y3)
		+ 2.0 * u1x + u3 * (2.0 * x3 + 6.0 * xy2);
	m[1][0] = m_off_diag_common
		+ u3 * (8.0 * x2y + 4.0 * y3)
		+ v3 * (2.0 * x3 + 6.0 * xy2);
	m[0][1] = m_off_diag_common
		+ u3 * (6.0 * x2y + 2.0 * y3)
		+ v3 * (4.0 * x3 + 8.0 * xy2);
	return cylindric_mat() * m;
	}

#include <snp/ldpk/prg_model_independent_2.snp>
