import ldpk

ldm = ldpk.anamorphic_standard_degree_4()
ldm.camera([0.157,0.64,0.36,0.0,0.0,1.0,300.0])
ldm.cx02(0.4)
ldm.cy02(1.2)
ldm.cx22(-0.5)
ldm.prepare()


nx = 11
ny = 11
"""for iy in range(ny):
	y_fov = float(iy) / (nx - 1)
	for ix in range(nx):
		x_fov = float(ix) / (nx - 1)
		p_fov = (x_fov,y_fov)
		q_fov = ldm.distort(p_fov)
		print(p_fov[0],p_fov[1],q_fov[0],q_fov[1])"""

y_fov = 1.0
for ix in range(nx):
	q = float(ix) / (nx - 1)
	x_fov = -0.5 + 2.0 * q
	p_fov = (x_fov,y_fov)
	q_fov = ldm.distort(p_fov)
	J = ldm.getJacobianMatrix(p_fov)
	det_J = J[0] * J[3] - J[1] * J[2]
	print(p_fov[0],p_fov[1],q_fov[0],q_fov[1])
# plot "/tmp/out.gp" using 1:3 with points pt 7 lc "red"


