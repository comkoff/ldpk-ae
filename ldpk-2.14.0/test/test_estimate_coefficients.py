import numpy as np
import ldpk
import vl_sdv as vl
import math

# Base functions for the OpenCV model in OpenCV coordinates.
# p is of type vl.vec2d, the return type is vl.vec2d as well.
def B(i,p_und_ocv):
	r_pow_2 = vl.dotsq(p_und_ocv)
	r_pow_4 = r_pow_2 * r_pow_2
	r_pow_6 = r_pow_2 * r_pow_4
	x = p_und_ocv[0]
	y = p_und_ocv[1]
	if i == 0:
		return p_und_ocv * r_pow_2
	elif i == 1:
		return p_und_ocv * r_pow_4
	elif i == 2:
		return p_und_ocv * r_pow_6
	elif i == 3:
		return vl.vec2d(2.0 * x * y,r_pow_2 + 2.0 * y * y)
	elif i == 4:
		return vl.vec2d(r_pow_2 + 2.0 * x * x,2.0 * x * y)

def fov2ocv(w_fb_cm,h_fb_cm,fl_cm,p_fov):
	return vl.had((p_fov - vl.vec2d(.5,.5)),vl.vec2d(w_fb_cm,-h_fb_cm)) / fl_cm

# Point sets
P_und = []
P_dis = []

# The OpenCV model. We us this as proxy code in order to
# create point pairs. The task is then to reproduce the
# distortion coefficients we set in ldm.
ldm = ldpk.opencv_radial_degree_6_decentered_degree_2()
w_fb_cm = 2.0
h_fb_cm = 1.0
fl_cm = 5.0
ldm.fl_cm(fl_cm).w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).x_lco_cm(0.0).y_lco_cm(0.0).r_pa(1.0)
ldm.k1(0.2).k2(0.1).k3(0.05).p1(-0.05).p2(0.1)
ldm.prepare()

# Let's start with a sample of 121 points, which is more than enough.
nx = 11
ny = 11
for iy in range(0,ny):
# y in SDV field-of-view coordinates
	y = float(iy) / (ny - 1)
	for ix in range(0,nx):
# x in SDV field-of-view coordinates
		x = float(ix) / (nx - 1)
# undistorted, feed this into ST-map
		p_und = vl.vec2d(x,y)
# distorted, get this from ST-map
		p_dis = vl.vec2d(ldm.distort(p_und))
		print(f"{p_und} -> {p_dis}")
# build point sets
		P_und.append(fov2ocv(w_fb_cm,h_fb_cm,fl_cm,p_und))
		P_dis.append(fov2ocv(w_fb_cm,h_fb_cm,fl_cm,p_dis))

# Initialize M as 5x5 matrix, since we have 5 base functions.
M = np.zeros((5, 5))
# Compute matrix M[k][i]
for i in range(5):
	for k in range(5):
		for j in range(nx * ny):
			M[k][i] += vl.dot(B(k,P_und[j]),B(i,P_und[j]))
# Initialize V as vector with 5 components, since we have 5 base functions.
V = np.zeros(5)
# Compute vector V[i]
for i in range(5):
	for j in range(nx * ny):
		V[i] += vl.dot(B(i,P_und[j]),P_dis[j] - P_und[j])

# Solve system of linear equations -> L
#L = np.linalg.solve(M, V)
# We can use Cholesky's method because the matrix
# is symmetric and positive-definit.
L = np.linalg.solve(np.linalg.cholesky(M).T, 
                    np.linalg.solve(np.linalg.cholesky(M), V))
print(L)
print(M.dot(L) - V)

residual = 0.0
for j in range(nx * ny):
# Make sure we create a copy(!) of P_und[j].
	p_pred = +P_und[j]
	for i in range(5):
		p_pred += B(i, P_und[j]) * L[i]
	diff = P_dis[j] - p_pred
	residual += vl.dotsq(diff)

# Root-Mean-Square-Error:
rmse = np.sqrt(residual / (nx * ny))
print("RMSE:",rmse)
