import ldpk

ldm = ldpk.anamorphic_stabilizer_degree_6()

w_fb_cm = 2.0
h_fb_cm = 1.0
fl_cm = 4.0

ldm.w_fb_cm(w_fb_cm).h_fb_cm(h_fb_cm).fl_cm(fl_cm)

x_lco_cm = 0.2
y_lco_cm = -0.1

ldm.x_lco_cm(0.0).y_lco_cm(0.0)

ldm.cx02(0.05)
ldm.cy02(0.05)

stbx = 0.5
stby = 0.0

ldm.stbx(stbx).stby(stby).prepare()


print(ldm.undistort((0.5,0.5)))
print("Test Jacobian")
print(ldm.getJacobianMatrixDQ((0.5,0.5)))
print(ldm.getJacobianMatrix((0.5,0.5)))
print()
print(ldm.getJacobianMatrixDQ((0.0,0.0)))
print(ldm.getJacobianMatrix((0.0,0.0)))
print()
print(ldm.getJacobianMatrixDQ((1.0,0.0)))
print(ldm.getJacobianMatrix((1.0,0.0)))
print()
print(ldm.getJacobianMatrixDQ((0.0,1.0)))
print(ldm.getJacobianMatrix((0.0,1.0)))
print()
print(ldm.getJacobianMatrixDQ((1.0,1.0)))
print(ldm.getJacobianMatrix((1.0,1.0)))

