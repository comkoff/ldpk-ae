import ldpk
import vl_sdv as vl

ldm = ldpk.radial_fisheye_degree_8()
ldm.w_fb_cm(2.0).h_fb_cm(1.0).fl_cm(1.5)
ldm.c6(-0.1)
ldm.c8( 0.02)
ldm.initializeParameters()

# Lens center
lc = vl.vec2d(.5,.5)

for i in range(100):
	y = 0.5 + 2.0 * i / 100.0

	v = vl.vec2d(.5,y)
	m = vl.mat2d(ldm.getJacobianMatrix(v))
	print("undistort:",ldm.undistort(v),", jacobian:",m,", derivative:",m * (v - lc),", orientation:",vl.dot(v - lc,m * (v - lc)))
