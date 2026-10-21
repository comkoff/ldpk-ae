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
# Maintainer: uwe@sci-d-vis.com
*/
// Es geht um bivariate Chebyshev-Polynome durch einfache
// Produktbildung. Als Indexraum verwenden wir den Raum,
// der in num_index mit index_space_inc_degree bezeichnet wird,
// d.h. beim Durchlaufen der Indizies des Koeffizientenarrays
// steigt der totale Grad des Polynoms von 0 bis n-1.
// So ist c aufgebaut.
// i=6: 27
// i=5: 20  26
// i=4: 14  19  25
// i=3:  9  13  18  24
// i=2:  5   8  12  17  23
// i=1:  2   4   7  11  16  22
// i=0:  0   1   3   6  10  15  21
// Der Koeffizient fuer die 0-te Ordnung in y-Richtung ist also
// Das Clenshaw-Polynom mit den Koeffizienten in 0, 1, 3, 6, 10...
// Die Schreibweise zur Initialisierung ist ziemlich old-school,
// aber dafuer laeuft der Shader auf dem aeltesten GLSL ueberhaupt.

/***************************************************************
* Koeffizienten vec2                                           *
***************************************************************/
vec2 clenshaw_n_eq_1(float x,vec2 c[1])
	{
	return c[0];
	}
vec2 clenshaw_n_eq_2(float x,vec2 c[2])
	{
	float x2 = 2.0 * x;
	vec2 tvv = vec2(0);
	vec2 t,tv = c[1];

	t = c[0] + x2 * tv - tvv;
	tvv = tv;
	tv = t;

	return tv - x * tvv;
	}
vec2 clenshaw_n_eq_3(float x,vec2 c[3])
	{
	float x2 = 2.0 * x;
	vec2 tvv = vec2(0);
	vec2 t,tv = c[2];

	t = c[1] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[0] + x2 * tv - tvv;
	tvv = tv;
	tv = t;

	return tv - x * tvv;
	}
vec2 clenshaw_n_eq_4(float x,vec2 c[4])
	{
	float x2 = 2.0 * x;
	vec2 tvv = vec2(0);
	vec2 t,tv = c[3];

	t = c[2] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[1] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[0] + x2 * tv - tvv;
	tvv = tv;
	tv = t;

	return tv - x * tvv;
	}
vec2 clenshaw_n_eq_5(float x,vec2 c[5])
	{
	float x2 = 2.0 * x;
	vec2 tvv = vec2(0);
	vec2 t,tv = c[4];

	t = c[3] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[2] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[1] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[0] + x2 * tv - tvv;
	tvv = tv;
	tv = t;

	return tv - x * tvv;
	}
vec2 clenshaw_n_eq_6(float x,vec2 c[6])
	{
	float x2 = 2.0 * x;
	vec2 tvv = vec2(0);
	vec2 t,tv = c[5];

	t = c[4] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[3] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[2] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[1] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[0] + x2 * tv - tvv;
	tvv = tv;
	tv = t;

	return tv - x * tvv;
	}
vec2 clenshaw_n_eq_7(float x,vec2 c[7])
	{
	float x2 = 2.0 * x;
	vec2 tvv = vec2(0);
	vec2 t,tv = c[6];

	t = c[5] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[4] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[3] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[2] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[1] + x2 * tv - tvv;
	tvv = tv;
	tv = t;
	t = c[0] + x2 * tv - tvv;
	tvv = tv;
	tv = t;

	return tv - x * tvv;
	}

/***************************************************************
* Koeffizienten vec2                                           *
***************************************************************/
vec2 clenshaw_n_eq_1(vec2 p,vec2 c[1])
	{
	vec2 q[1],d1[1];

	d1[0] = c[0];
	q[0] = clenshaw_n_eq_1(p[0],d1);

	return clenshaw_n_eq_1(p[1],q);
	}
vec2 clenshaw_n_eq_2(vec2 p,vec2 c[3])
	{
	vec2 q[2],d2[2],d1[1];

	d2[0] = c[0];
	d2[1] = c[1];
	q[0] = clenshaw_n_eq_2(p[0],d2);

	d1[0] = c[2];
	q[1] = clenshaw_n_eq_1(p[0],d1);

	return clenshaw_n_eq_2(p[1],q);
	}
vec2 clenshaw_n_eq_3(vec2 p,vec2 c[6])
	{
	vec2 q[3],d3[3],d2[2],d1[1];

	d3[0] = c[0];
	d3[1] = c[1];
	d3[2] = c[3];
	q[0] = clenshaw_n_eq_3(p[0],d3);

	d2[0] = c[2];
	d2[1] = c[4];
	q[1] = clenshaw_n_eq_2(p[0],d2);

	d1[0] = c[5];
	q[2] = clenshaw_n_eq_1(p[0],d1);

	return clenshaw_n_eq_3(p[1],q);
	}
vec2 clenshaw_n_eq_4(vec2 p,vec2 c[10])
	{
	vec2 q[4],d4[4],d3[3],d2[2],d1[1];

	d4[0] = c[0];
	d4[1] = c[1];
	d4[2] = c[3];
	d4[3] = c[6];
	q[0] = clenshaw_n_eq_4(p[0],d4);

	d3[0] = c[2];
	d3[1] = c[4];
	d3[2] = c[7];
	q[1] = clenshaw_n_eq_3(p[0],d3);

	d2[0] = c[5];
	d2[1] = c[8];
	q[2] = clenshaw_n_eq_2(p[0],d2);

	d1[0] = c[9];
	q[3] = clenshaw_n_eq_1(p[0],d1);

	return clenshaw_n_eq_4(p[1],q);
	}
vec2 clenshaw_n_eq_5(vec2 p,vec2 c[15])
	{
	vec2 q[5],d5[5],d4[4],d3[3],d2[2],d1[1];

	d5[0] = c[0];
	d5[1] = c[1];
	d5[2] = c[3];
	d5[3] = c[6];
	d5[4] = c[10];
	q[0] = clenshaw_n_eq_5(p[0],d5);

	d4[0] = c[2];
	d4[1] = c[4];
	d4[2] = c[7];
	d4[3] = c[11];
	q[1] = clenshaw_n_eq_4(p[0],d4);

	d3[0] = c[5];
	d3[1] = c[8];
	d3[2] = c[12];
	q[2] = clenshaw_n_eq_3(p[0],d3);

	d2[0] = c[9];
	d2[1] = c[13];
	q[3] = clenshaw_n_eq_2(p[0],d2);

	d1[0] = c[14];
	q[4] = clenshaw_n_eq_1(p[0],d1);

	return clenshaw_n_eq_5(p[1],q);
	}
vec2 clenshaw_n_eq_6(vec2 p,vec2 c[21])
	{
	vec2 q[6],d6[6],d5[5],d4[4],d3[3],d2[2],d1[1];

	d6[0] = c[0];
	d6[1] = c[1];
	d6[2] = c[3];
	d6[3] = c[6];
	d6[4] = c[10];
	d6[5] = c[15];
	q[0] = clenshaw_n_eq_6(p[0],d6);

	d5[0] = c[2];
	d5[1] = c[4];
	d5[2] = c[7];
	d5[3] = c[11];
	d5[4] = c[16];
	q[1] = clenshaw_n_eq_5(p[0],d5);

	d4[0] = c[5];
	d4[1] = c[8];
	d4[2] = c[12];
	d4[3] = c[17];
	q[2] = clenshaw_n_eq_4(p[0],d4);

	d3[0] = c[9];
	d3[1] = c[13];
	d3[2] = c[18];
	q[3] = clenshaw_n_eq_3(p[0],d3);

	d2[0] = c[14];
	d2[1] = c[19];
	q[4] = clenshaw_n_eq_2(p[0],d2);

	d1[0] = c[20];
	q[5] = clenshaw_n_eq_1(p[0],d1);

	return clenshaw_n_eq_6(p[1],q);
	}
vec2 clenshaw_n_eq_7(vec2 p,vec2 c[28])
	{
	vec2 q[7],d7[7],d6[6],d5[5],d4[4],d3[3],d2[2],d1[1];

	d7[0] = c[0];
	d7[1] = c[1];
	d7[2] = c[3];
	d7[3] = c[6];
	d7[4] = c[10];
	d7[5] = c[15];
	d7[6] = c[21];
	q[0] = clenshaw_n_eq_7(p[0],d7);

	d6[0] = c[2];
	d6[1] = c[4];
	d6[2] = c[7];
	d6[3] = c[11];
	d6[4] = c[16];
	d6[5] = c[22];
	q[1] = clenshaw_n_eq_6(p[0],d6);

	d5[0] = c[5];
	d5[1] = c[8];
	d5[2] = c[12];
	d5[3] = c[17];
	d5[4] = c[23];
	q[2] = clenshaw_n_eq_5(p[0],d5);

	d4[0] = c[9];
	d4[1] = c[13];
	d4[2] = c[18];
	d4[3] = c[24];
	q[3] = clenshaw_n_eq_4(p[0],d4);

	d3[0] = c[14];
	d3[1] = c[19];
	d3[2] = c[25];
	q[4] = clenshaw_n_eq_3(p[0],d3);

	d2[0] = c[20];
	d2[1] = c[26];
	q[5] = clenshaw_n_eq_2(p[0],d2);

	d1[0] = c[27];
	q[6] = clenshaw_n_eq_1(p[0],d1);

	return clenshaw_n_eq_7(p[1],q);
	}

uniform sampler2D tex0,matte;
uniform int w_in_px;
uniform int h_in_px;
uniform int w_out_px;
uniform int h_out_px;

/***************************************************************
* flow control parameters                                      *
***************************************************************/
// 0: image, 1: st-map, 2: jacobian, 3: jacobian diff-quot (Future 4: twist-vector), 5: twist-vector diff-quot, 6: red (testing
// render_mode 2 is mainly for testing and verifying
// the implementation of the jacobian.
uniform int render_mode;
// 0: distort, 1: undistort, 2: chebyshev, 3: identity
uniform int direction;
// 1: bilinear, 2: bicubic-catmull-rom, 3: bicubi-mitchell-netravali
uniform int reconstruction_filter;
// How initial values are provided.
// 0: by texture, 1: by polynomial
// Experimental!
uniform int initial_value_mode;
// input texture semantics, 0: rgba, 1: velocity
// uniform int semantics;

// initial values for direction "undistort". Small textures will
// do generally, e.g. 64x64, since lens distortion varies "slowly".
// for direction "distort" there is no need to specify tex_initial.
// The texture is interpreted in field-of-view coordinates.
// initial_value_mode "texture"
uniform sampler2D tex_initial;

// alternative method for providing initial values:
// This is an array of coefficients for a bi-variate
// Chebyshev polynomial. initial_value_mode "polynomial".

// This is our original code. Unfortunately it does not work on
// macOS, most likely due to an incomplete implementation of their
// glsl-120-compiler. We have to write these as distinct variables.
// uniform vec2 chebyshev[28];
uniform vec2 chebyshev0;
uniform vec2 chebyshev1;
uniform vec2 chebyshev2;
uniform vec2 chebyshev3;
uniform vec2 chebyshev4;
uniform vec2 chebyshev5;
uniform vec2 chebyshev6;
uniform vec2 chebyshev7;
uniform vec2 chebyshev8;
uniform vec2 chebyshev9;
uniform vec2 chebyshev10;
uniform vec2 chebyshev11;
uniform vec2 chebyshev12;
uniform vec2 chebyshev13;
uniform vec2 chebyshev14;
uniform vec2 chebyshev15;
uniform vec2 chebyshev16;
uniform vec2 chebyshev17;
uniform vec2 chebyshev18;
uniform vec2 chebyshev19;
uniform vec2 chebyshev20;
uniform vec2 chebyshev21;
uniform vec2 chebyshev22;
uniform vec2 chebyshev23;
uniform vec2 chebyshev24;
uniform vec2 chebyshev25;
uniform vec2 chebyshev26;
uniform vec2 chebyshev27;

/***************************************************************
* model-independent parameters                                 *
***************************************************************/
uniform float tde4_focal_length_cm;
uniform float tde4_filmback_width_cm;
uniform float tde4_filmback_height_cm;
uniform float tde4_lens_center_offset_x_cm;
uniform float tde4_lens_center_offset_y_cm;
uniform float tde4_pixel_aspect;
uniform float tde4_custom_focus_distance_cm;
// Field of View in unit-coordinates for input and output texture.
uniform vec2 a_fov_in_unit,b_fov_in_unit;
uniform vec2 a_fov_out_unit,b_fov_out_unit;

/***************************************************************
* model-independent uniform expressions                        *
***************************************************************/
// field of view
vec2 d_fov_in_unit = b_fov_in_unit - a_fov_in_unit;
vec2 d_fov_in_unit_inv = 1.0 / d_fov_in_unit;
vec2 d_fov_out_unit = b_fov_out_unit - a_fov_out_unit;
vec2 d_fov_out_unit_inv = 1.0 / d_fov_out_unit;

// filmback
vec2 lco_cm = vec2(tde4_lens_center_offset_x_cm,tde4_lens_center_offset_y_cm);
vec2 fb_cm = vec2(tde4_filmback_width_cm,tde4_filmback_height_cm);
float r_cm = length(fb_cm) / 2.0;
// filmback in diag-coordinates
vec2 fb_diag = fb_cm / (2.0 * r_cm);
// focal length in diag-coordinates (need for equisolid angle)
float fl_diag = tde4_focal_length_cm / r_cm;

// output buffer: Jacobian for mapping window coordinates to diag-coordinates.
// Size of filmback in pixel (filmback corresponds to field-of-view):
vec2 fb_out_px = d_fov_out_unit * vec2(float(w_out_px),float(h_out_px));

// Jacobian of pixel-coordinates to diag-coordinates
//vec2 d_px2diag = fb_out_px / fb_diag;
//vec2 d_px2diag = vec2(w_out_px,h_out_px);
// Jacobian for diag to fov
// THIS NEEDS TO BE TESTED FOR FOV_IN != FOU_OUT!
mat2 jac_diag2fov = mat2(2.0 * r_cm / fb_cm[0],0.0,0.0,2.0 * r_cm / fb_cm[1]);
mat2 jac_fov2diag = mat2(fb_cm[0] / (2.0 * r_cm),0.0,0.0,fb_cm[1] / (2.0 * r_cm));

/***************************************************************
* uniform expressions predefined by flame                      *
***************************************************************/
uniform float adsk_result_w, adsk_result_h;

/***************************************************************
* model-independent constants                                  *
***************************************************************/
const float pi = 3.1415926535;

/***************************************************************
* model-independent functions                                  *
***************************************************************/
vec2 unit2fov_out(vec2 p_unit)
	{ return (p_unit - a_fov_out_unit) * d_fov_out_unit_inv; }
vec2 fov2unit_in(vec2 p_fov)
	{ return p_fov * d_fov_in_unit + a_fov_in_unit; }
vec2 fov2diag(vec2 p_fov)
	{
// map fov-coordinates to diag-coordinates
	vec2 p_cm = (p_fov - 1.0/2.0) * fb_cm - lco_cm;
	vec2 p_diag = p_cm / r_cm;
	return p_diag;
	}
vec2 diag2fov(vec2 p_diag)
	{
// map diag-coordinates to fov-coordinates
	vec2 p_cm = p_diag * r_cm;
	vec2 p_fov = (p_cm + lco_cm) / fb_cm + 1.0/2.0;
	return p_fov;
	}
float norm2(vec2 v)
	{ return length(v); }
vec2 unit(vec2 v)
	{ return normalize(v); }
mat2 tensq(vec2 v)
	{ return mat2(v[0] * v[0],v[0] * v[1],v[0] * v[1],v[1] * v[1]); }
mat2 trans(mat2 m)
	{ return mat2(m[0][0],m[1][0],m[0][1],m[1][1]); }
mat2 invert(mat2 m)
	{ return mat2(m[1][1],-m[0][1],-m[1][0],m[0][0]) / (m[0][0] * m[1][1] - m[0][1] * m[1][0]); }


/***************************************************************
* model-dependent parameters                                   *
***************************************************************/
uniform float cx02_degree_2;
uniform float cy02_degree_2;
uniform float cx22_degree_2;
uniform float cy22_degree_2;

uniform float cx04_degree_4;
uniform float cy04_degree_4;
uniform float cx24_degree_4;
uniform float cy24_degree_4;
uniform float cx44_degree_4;
uniform float cy44_degree_4;

uniform float cx06_degree_6;
uniform float cy06_degree_6;
uniform float cx26_degree_6;
uniform float cy26_degree_6;
uniform float cx46_degree_6;
uniform float cy46_degree_6;
uniform float cx66_degree_6;
uniform float cy66_degree_6;

float cx_for_x2 = cx02_degree_2 + cx22_degree_2;
float cx_for_y2 = cx02_degree_2 - cx22_degree_2;

float cx_for_x4 = cx04_degree_4 + cx24_degree_4 + cx44_degree_4;
float cx_for_x2_y2 = 2.0 * cx04_degree_4 - 6.0 * cx44_degree_4;
float cx_for_y4 = cx04_degree_4 - cx24_degree_4 + cx44_degree_4;

float cx_for_x6 = cx06_degree_6 + cx26_degree_6 + cx46_degree_6 + cx66_degree_6;
float cx_for_x4_y2 = 3.0 * cx06_degree_6 + cx26_degree_6 - 5.0 * cx46_degree_6 - 15.0 * cx66_degree_6;
float cx_for_x2_y4 = 3.0 * cx06_degree_6 - cx26_degree_6 - 5.0 * cx46_degree_6 + 15.0 * cx66_degree_6;
float cx_for_y6 = cx06_degree_6 - cx26_degree_6 + cx46_degree_6 - cx66_degree_6;

float cy_for_x2 = cy02_degree_2 + cy22_degree_2;
float cy_for_y2 = cy02_degree_2 - cy22_degree_2;

float cy_for_x4 = cy04_degree_4 + cy24_degree_4 + cy44_degree_4;
float cy_for_x2_y2 = 2.0 * cy04_degree_4 - 6.0 * cy44_degree_4;
float cy_for_y4 = cy04_degree_4 - cy24_degree_4 + cy44_degree_4;

float cy_for_x6 = cy06_degree_6 + cy26_degree_6 + cy46_degree_6 + cy66_degree_6;
float cy_for_x4_y2 = 3.0 * cy06_degree_6 + cy26_degree_6 - 5.0 * cy46_degree_6 - 15.0 * cy66_degree_6;
float cy_for_x2_y4 = 3.0 * cy06_degree_6 - cy26_degree_6 - 5.0 * cy46_degree_6 + 15.0 * cy66_degree_6;
float cy_for_y6 = cy06_degree_6 - cy26_degree_6 + cy46_degree_6 - cy66_degree_6;

/***************************************************************
* model-independent functions                                  *
***************************************************************/
// In Autodesk(R) Flame the matte is passed as the blue channel of a separate
// sampler-object, so we extend our standard bicubic evaluator accordingly.
vec4 texture2d_bicubic_catmull_rom(sampler2D tex,sampler2D matte,	vec2 uv,int w_in_px,
										int h_in_px)
	{
	float w_in_px_inv = 1.0 / float(w_in_px);
	float h_in_px_inv = 1.0 / float(h_in_px);
// This is a 9-tap bicubic filter with catmull-rom weights.
	vec2 t = fract(vec2(w_in_px,h_in_px) * uv - (1.0/2.0));
	vec2 tt = t * t;
	vec2 ttt = tt * t;
// Das Zentrum des linken der beiden zentralen Pixel.
	vec2 base = (floor(vec2(w_in_px,h_in_px) * uv - (1.0/2.0)) + (1.0/2.0)) * vec2(w_in_px_inv,h_in_px_inv);
// Aus den Catmull-Rom-Gewichten kommt das hier.
// Der mittlere Gewichtsfaktor ist die Summe der beiden zentralen
// Catmull-Rom-Gewichte. Die aeusseren Gewichte entsprechen denen
// von Catmull-Rom.
	vec2 w0 = (1.0/2.0) * (-t + 2.0 * tt - ttt);
	vec2 w1 = (1.0/2.0) * (2.0 + t - tt);
	vec2 w2 = (1.0/2.0) * (-tt + ttt);
// Beim mittleren Tap wird linear interpoliert. Der Offset ist:
	vec2 o = (1.0/2.0) * (t + 4.0 * tt - 3.0 * ttt) / w1;
// Die neun Positionen, an denen wir die Textur auswerten, setzen sich
// aus der Basisposition und diesen Offsetkomponenten zusammen.
	vec3 x = vec3(-w_in_px_inv,o[0] * w_in_px_inv,2.0 * w_in_px_inv);
	vec3 y = vec3(-h_in_px_inv,o[1] * h_in_px_inv,2.0 * h_in_px_inv);

	vec4 t00 = vec4(texture2D(tex,base + vec2(x[0],y[0])).rgb,texture2D(matte,base + vec2(x[0],y[0])).b);
	vec4 t10 = vec4(texture2D(tex,base + vec2(x[1],y[0])).rgb,texture2D(matte,base + vec2(x[1],y[0])).b);
	vec4 t20 = vec4(texture2D(tex,base + vec2(x[2],y[0])).rgb,texture2D(matte,base + vec2(x[2],y[0])).b);
	vec4 t01 = vec4(texture2D(tex,base + vec2(x[0],y[1])).rgb,texture2D(matte,base + vec2(x[0],y[1])).b);
	vec4 t11 = vec4(texture2D(tex,base + vec2(x[1],y[1])).rgb,texture2D(matte,base + vec2(x[1],y[1])).b);
	vec4 t21 = vec4(texture2D(tex,base + vec2(x[2],y[1])).rgb,texture2D(matte,base + vec2(x[2],y[1])).b);
	vec4 t02 = vec4(texture2D(tex,base + vec2(x[0],y[2])).rgb,texture2D(matte,base + vec2(x[0],y[2])).b);
	vec4 t12 = vec4(texture2D(tex,base + vec2(x[1],y[2])).rgb,texture2D(matte,base + vec2(x[1],y[2])).b);
	vec4 t22 = vec4(texture2D(tex,base + vec2(x[2],y[2])).rgb,texture2D(matte,base + vec2(x[2],y[2])).b);

	return	  (t00 * w0[0] + t10 * w1[0] + t20 * w2[0]) * w0[1]
		+ (t01 * w0[0] + t11 * w1[0] + t21 * w2[0]) * w1[1]
		+ (t02 * w0[0] + t12 * w1[0] + t22 * w2[0]) * w2[1];
	}
// Mitchell-Netravali for B=C=1/3. Most likely this can be re-formulated
// as a 9-tap, but I don't know how.
vec4 texture2d_bicubic_mitchell_netravali(sampler2D tex,sampler2D matte,	vec2 uv,int w_in_px,
											int h_in_px)
	{
	float w_in_px_inv = 1.0 / float(w_in_px);
	float h_in_px_inv = 1.0 / float(h_in_px);
// Helper variable for splitting integer and fractional part.
// Note that fract(b) and floor(b) are undetermined because of
// numeric noise, but together they work nonetheless.
	vec2 b = vec2(w_in_px,h_in_px) * uv - (1.0/2.0);
// Should work similar to Catmull-Rom.
	vec2 t = fract(b);
	vec2 tt = t * t;
	vec2 ttt = tt * t;
// Das Zentrum des linken der beiden zentralen Pixel.
	vec2 base = (floor(b) + (1.0/2.0)) * vec2(w_in_px_inv,h_in_px_inv);
// Weight factors overscaled by factor 18. The idea was to simplify pre-factors
// in order to reduce numeric noise, but the result remains noisy.
	vec2 w0 =	1.0	- 9.0 * t	+ 15.0 * tt	-  7.0 * ttt;
	vec2 w1 =	16.0			- 36.0 * tt	+ 21.0 * ttt;
	vec2 w2 =	1.0	+ 9.0 * t	+ 27.0 * tt	- 21.0 * ttt;
	vec2 w3 = 				-  6.0 * tt	+  7.0 * ttt;
// Die sechzehn Positionen, an denen wir die Textur auswerten, setzen sich
// aus der Basisposition und diesen Offsetkomponenten zusammen.
	vec4 x = vec4(-w_in_px_inv,0.0,w_in_px_inv,2.0 * w_in_px_inv);
	vec4 y = vec4(-h_in_px_inv,0.0,h_in_px_inv,2.0 * h_in_px_inv);

	vec4 t00 = vec4(texture2D(tex,base + vec2(x[0],y[0])).rgb,texture2D(matte,base + vec2(x[0],y[0])).b);
	vec4 t10 = vec4(texture2D(tex,base + vec2(x[1],y[0])).rgb,texture2D(matte,base + vec2(x[1],y[0])).b);
	vec4 t20 = vec4(texture2D(tex,base + vec2(x[2],y[0])).rgb,texture2D(matte,base + vec2(x[2],y[0])).b);
	vec4 t30 = vec4(texture2D(tex,base + vec2(x[3],y[0])).rgb,texture2D(matte,base + vec2(x[3],y[0])).b);

	vec4 t01 = vec4(texture2D(tex,base + vec2(x[0],y[1])).rgb,texture2D(matte,base + vec2(x[0],y[1])).b);
	vec4 t11 = vec4(texture2D(tex,base + vec2(x[1],y[1])).rgb,texture2D(matte,base + vec2(x[1],y[1])).b);
	vec4 t21 = vec4(texture2D(tex,base + vec2(x[2],y[1])).rgb,texture2D(matte,base + vec2(x[2],y[1])).b);
	vec4 t31 = vec4(texture2D(tex,base + vec2(x[3],y[1])).rgb,texture2D(matte,base + vec2(x[3],y[1])).b);

	vec4 t02 = vec4(texture2D(tex,base + vec2(x[0],y[2])).rgb,texture2D(matte,base + vec2(x[0],y[2])).b);
	vec4 t12 = vec4(texture2D(tex,base + vec2(x[1],y[2])).rgb,texture2D(matte,base + vec2(x[1],y[2])).b);
	vec4 t22 = vec4(texture2D(tex,base + vec2(x[2],y[2])).rgb,texture2D(matte,base + vec2(x[2],y[2])).b);
	vec4 t32 = vec4(texture2D(tex,base + vec2(x[3],y[2])).rgb,texture2D(matte,base + vec2(x[3],y[2])).b);

	vec4 t03 = vec4(texture2D(tex,base + vec2(x[0],y[3])).rgb,texture2D(matte,base + vec2(x[0],y[3])).b);
	vec4 t13 = vec4(texture2D(tex,base + vec2(x[1],y[3])).rgb,texture2D(matte,base + vec2(x[1],y[3])).b);
	vec4 t23 = vec4(texture2D(tex,base + vec2(x[2],y[3])).rgb,texture2D(matte,base + vec2(x[2],y[3])).b);
	vec4 t33 = vec4(texture2D(tex,base + vec2(x[3],y[3])).rgb,texture2D(matte,base + vec2(x[3],y[3])).b);

	return	 ((t00 * w0[0] + t10 * w1[0] + t20 * w2[0] + t30 * w3[0]) * w0[1]
		+ (t01 * w0[0] + t11 * w1[0] + t21 * w2[0] + t31 * w3[0]) * w1[1]
		+ (t02 * w0[0] + t12 * w1[0] + t22 * w2[0] + t32 * w3[0]) * w2[1]
		+ (t03 * w0[0] + t13 * w1[0] + t23 * w2[0] + t33 * w3[0]) * w3[1]) * (1.0/(18.0*18.0))
		;
	}


/***************************************************************
* model-dependent functions                                    *
***************************************************************/
vec2 anamorphic(vec2 p_diag)
	{
// apply undistortion
	float x = p_diag[0];
	float y = p_diag[1];
	float x2 = x * x;
	float y2 = y * y;
	float x4 = x2 * x2;
	float y4 = y2 * y2;
	float x6 = x4 * x2;
	float y6 = y4 * y2;

	vec2 q_diag;
	q_diag[0] = x * (1.0
			+ x2 * cx_for_x2 + y2 * cx_for_y2
			+ x4 * cx_for_x4 + x2 * y2 * cx_for_x2_y2 + y4 * cx_for_y4
			+ x6 * cx_for_x6 + x4 * y2 * cx_for_x4_y2 + x2 * y4 * cx_for_x2_y4 + y6 * cx_for_y6);
	q_diag[1] = y * (1.0
			+ x2 * cy_for_x2 + y2 * cy_for_y2
			+ x4 * cy_for_x4 + x2 * y2 * cy_for_x2_y2 + y4 * cy_for_y4
			+ x6 * cy_for_x6 + x4 * y2 * cy_for_x4_y2 + x2 * y4 * cy_for_x2_y4 + y6 * cy_for_y6);
	return q_diag;
	}
vec2 undistort_diag(vec2 p_diag)
	{
	return anamorphic(p_diag);
	}
mat2 jacobian(vec2 p_diag)
	{
	mat2 m;
	float x = p_diag[0];
	float x2 = x * x;
	float x3 = x2 * x;
	float x4 = x2 * x2;
	float x5 = x4 * x;
	float x6 = x4 * x2;
	float y = p_diag[1];
	float y2 = y * y;
	float y3 = y2 * y;
	float y4 = y2 * y2;
	float y5 = y4 * y;
	float y6 = y4 * y2;
// Note that in glsl [i][j] is the component at column i and row j,
// whereas in the LDPK we use the notation [i][j] like in mathematics
// and physics, so that matrix multiplication is (a b)[i][k] = \sum_j a[i][j] b[j][k]
	m[0][0] = 1.0 + x2 * 3.0 * cx_for_x2 + y2 * cx_for_y2
		+ x4 * 5.0 * cx_for_x4 + x2 * y2 * 3.0 * cx_for_x2_y2 + y4 * cx_for_y4
		+ x6 * 7.0 * cx_for_x6 + x4 * y2 * 5.0 * cx_for_x4_y2 + x2 * y4 * 3.0 * cx_for_x2_y4 + y6 * cx_for_y6;
	m[1][1] = 1.0 + y2 * 3.0 * cy_for_y2 + x2 * cy_for_x2
		+ y4 * 5.0 * cy_for_y4 + x2 * y2 * 3.0 * cy_for_x2_y2 + x4 * cy_for_x4
		+ x6 * cy_for_x6 + x4 * y2 * 3.0 * cy_for_x4_y2 + x2 * y4 * 5.0 * cy_for_x2_y4 + y6 * 7.0 * cy_for_y6;
	m[1][0] = x * y * 2.0 * cx_for_y2
		+ x * y3 * 4.0 * cx_for_y4 + x3 * y * 2.0 * cx_for_x2_y2
		+ x5 * y * 2.0 * cx_for_x4_y2 + x3 * y3 * 4.0 * cx_for_x2_y4 + x * y5 * 6.0 * cx_for_y6;
	m[0][1] = x * y * 2.0 * cy_for_x2
		+ x3 * y * 4.0 * cy_for_x4 + x * y3 * 2.0 * cy_for_x2_y2
		+ x5 * y * 6.0 * cy_for_x6 + x3 * y3 * 4.0 * cy_for_x4_y2 + x * y5 * 2.0 * cy_for_x2_y4;
	return m;
	}

/***************************************************************
* model-independent functions                                  *
***************************************************************/
vec2 undistort(vec2 p_unit)
	{
// undistort, because distort for image processing is reading at undistorted position.
	vec2 q_unit = fov2unit_in(diag2fov(undistort_diag(fov2diag(unit2fov_out(p_unit)))));
	return q_unit;
	}
vec2 distort(vec2 q_unit)
	{
	vec2 q_diag = fov2diag(unit2fov_out(q_unit));
// initial value
	vec2 p_init_unit;
// In both cases we work on unit coordinates, not fov-coordinates.
	if(initial_value_mode == 0)
		{ p_init_unit = texture2D(tex_initial,q_unit).xy; }
	else
		{
		vec2 c[28];
		c[0]  = chebyshev0; c[1]  = chebyshev1; c[2]  = chebyshev2; c[3]  = chebyshev3; c[4]  = chebyshev4; c[5]  = chebyshev5; c[6]  = chebyshev6; c[7]  = chebyshev7; c[8]  = chebyshev8; c[9]  = chebyshev9;
		c[10] = chebyshev10;c[11] = chebyshev11;c[12] = chebyshev12;c[13] = chebyshev13;c[14] = chebyshev14;c[15] = chebyshev15;c[16] = chebyshev16;c[17] = chebyshev17;c[18] = chebyshev18;c[19] = chebyshev19;
		c[20] = chebyshev20;c[21] = chebyshev21;c[22] = chebyshev22;c[23] = chebyshev23;c[24] = chebyshev24;c[25] = chebyshev25;c[26] = chebyshev26;c[27] = chebyshev27;
		p_init_unit = (clenshaw_n_eq_7(2.0 * q_unit - 1.0,c) + 1.0) * (1.0/2.0);
		}
	vec2 p_iter_diag = fov2diag(unit2fov_out(p_init_unit));
// iterate n times
	for(int i = 0;i < 5;++i)
		{
		vec2 f_diag = undistort_diag(p_iter_diag) - q_diag;
		p_iter_diag -= invert(jacobian(p_iter_diag)) * f_diag;
		}
	vec2 p_unit = fov2unit_in(diag2fov(p_iter_diag));
	return p_unit;
	}
// Jacobian by difference quotient, for testing
mat2 jacobian_dq(vec2 p_diag)
	{
// We have float-precision, so we can't use a really tiny value.
	float epsilon = 0.001;
	mat2 j_trans;
	j_trans[0] = (undistort_diag(p_diag + vec2(epsilon,0.0)) - undistort_diag(p_diag - vec2(epsilon,0.0))) / (2.0 * epsilon);
	j_trans[1] = (undistort_diag(p_diag + vec2(0.0,epsilon)) - undistort_diag(p_diag - vec2(0.0,epsilon))) / (2.0 * epsilon);
	return j_trans;
	}
// Twist vector by difference quotient, for testing
vec2 twist_dq(vec2 p_diag)
	{
// Same here, even worse b/o 2nd derivative.
	float epsilon = 0.01;
	vec2 t;
	vec2 t00 = undistort_diag(p_diag + vec2(-epsilon,-epsilon));
	vec2 t10 = undistort_diag(p_diag + vec2( epsilon,-epsilon));
	vec2 t01 = undistort_diag(p_diag + vec2(-epsilon, epsilon));
	vec2 t11 = undistort_diag(p_diag + vec2( epsilon, epsilon));
	t[0] = (t11[0] - t01[0] - t10[0] + t00[0]) / (4.0 * epsilon * epsilon);
	t[1] = (t11[1] - t01[1] - t10[1] + t00[1]) / (4.0 * epsilon * epsilon);
	return t;
	}

/***************************************************************
* model-independent main entry point                           *
***************************************************************/
void main()
	{
	vec2 uv;
// unit coords in output image using predefined constants adsk_...
	vec2 coord = gl_FragCoord.xy / vec2(adsk_result_w,adsk_result_h);
// Generally, if-else is not good in shaders, but these are based
// only on uniform variables, so no problem. We reduce the amount
// of source code considerably.
	if(render_mode >= 2)
		{
// In this branch we investigate Jacobian and Twist.
		vec2 p_diag = fov2diag(unit2fov_out(coord));
// We'd like to have (row 0,col 0),(row 0,col 1),(row 1,col 0),(row 1,col 1),
// since it must be compatible to our CPU-version in the LDPK and to everything
// else we are doing at SDV.
		if(render_mode == 2)
			{
			mat2 j = jac_diag2fov * jacobian(p_diag) * jac_fov2diag;
			gl_FragColor = vec4(j[0][0],j[1][0],j[0][1],j[1][1]);
			}
		else if(render_mode == 3)
			{
			mat2 j = jac_diag2fov * jacobian_dq(p_diag) * jac_fov2diag;
			gl_FragColor = vec4(j[0][0],j[1][0],j[0][1],j[1][1]);
			}
		else if(render_mode == 4)
			{
// We don't have analytic expressions for all models, so we'll do this later.
			}
		else if(render_mode == 5)
			{
			vec2 t = vec2(fb_cm[1] / r_cm,fb_cm[0] / r_cm) * twist_dq(p_diag);
			gl_FragColor = vec4(t[0],t[1],0.0,1.0);
			}
		else if(render_mode == 6)
			{
// Render mode 6: constant output for testing and debugging.
			uv = coord;
			if((uv[0] >= a_fov_in_unit[0]) && (uv[0] < b_fov_in_unit[0]) && (uv[1] >= a_fov_in_unit[1]) && (uv[1] < b_fov_in_unit[1]))
				{ gl_FragColor = vec4(1.0,0.0,0.0,1.0); }
			else
				{ gl_FragColor = vec4(0.0,0.5,0.0,0.5); }
			}
		}
	else
		{
// This branch is render mode 0 (image) and render mode 1 (stmap)
		if(direction == 0)
			{
// distort invokes undistort-function in image-processing
			uv = undistort(coord);
			}
		else if(direction == 1)
			{
// undistort invokes distort-function in image-processing
			uv = distort(coord);
			}
		else if(direction == 2)
			{
// Visualizing the chebyshev polynomial for testing.
// The image resulting from this mode should *approximately*
// look like the undistort-image (yes, undistort).
			vec2 c[28];
			c[0]  = chebyshev0; c[1]  = chebyshev1; c[2]  = chebyshev2; c[3]  = chebyshev3; c[4]  = chebyshev4; c[5]  = chebyshev5; c[6]  = chebyshev6; c[7]  = chebyshev7; c[8]  = chebyshev8; c[9]  = chebyshev9;
			c[10] = chebyshev10;c[11] = chebyshev11;c[12] = chebyshev12;c[13] = chebyshev13;c[14] = chebyshev14;c[15] = chebyshev15;c[16] = chebyshev16;c[17] = chebyshev17;c[18] = chebyshev18;c[19] = chebyshev19;
			c[20] = chebyshev20;c[21] = chebyshev21;c[22] = chebyshev22;c[23] = chebyshev23;c[24] = chebyshev24;c[25] = chebyshev25;c[26] = chebyshev26;c[27] = chebyshev27;
			uv = (clenshaw_n_eq_7(2.0 * coord - 1.0,c) + 1.0) * (1.0/2.0);
			}
		else
			{
// identity for testing
			uv = coord;
			}
		if(reconstruction_filter == 1)
			{
// bilinear
			if(render_mode == 0)
				{
// Flame: Matte is the blue channel in sampler object matte.
				gl_FragColor = vec4(texture2D(tex0,uv).rgb,texture2D(matte,uv).b);
				}
			else if(render_mode == 1)
				{
				gl_FragColor = vec4(uv[0],uv[1],0.0,1.0);
				}
			}
		else if(reconstruction_filter == 2)
			{
// bicubic
			if(render_mode == 0)
				{
				gl_FragColor = texture2d_bicubic_catmull_rom(tex0,matte,uv,w_in_px,h_in_px);
				}
			else if(render_mode == 1)
				{
				gl_FragColor = vec4(uv[0],uv[1],0.0,1.0);
				}
			}
		else if(reconstruction_filter == 3)
			{
// bicubic
			if(render_mode == 0)
				{
				gl_FragColor = texture2d_bicubic_mitchell_netravali(tex0,matte,uv,w_in_px,h_in_px);
				}
			else if(render_mode == 1)
				{
				gl_FragColor = vec4(uv[0],uv[1],0.0,1.0);
				}
			}
		}
	}


