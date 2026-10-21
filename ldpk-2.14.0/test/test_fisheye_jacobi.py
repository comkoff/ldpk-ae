import ldpk

ldm = ldpk.radial_fisheye_equisolid_degree_8()
ldm.camera([0.5,2.0,1.0,0.0,0.0,100.0,1.0])
ldm.initializeParameters()

print(ldm.getJacobianMatrix([.25,.15]))
print(ldm.getJacobianMatrixDQ([.25,.15]))

ldm = ldpk.radial_fisheye_equidistant_degree_8()
ldm.camera([0.5,2.0,1.0,0.0,0.0,100.0,1.0])
ldm.initializeParameters()

print(ldm.getJacobianMatrix([.25,.15]))
print(ldm.getJacobianMatrixDQ([.25,.15]))

ldm = ldpk.radial_fisheye_orthographic_degree_8()
ldm.camera([0.5,2.0,1.0,0.0,0.0,100.0,1.0])
ldm.initializeParameters()

print(ldm.getJacobianMatrix([.25,.15]))
print(ldm.getJacobianMatrixDQ([.25,.15]))

ldm = ldpk.radial_fisheye_stereographic_degree_8()
ldm.camera([0.5,2.0,1.0,0.0,0.0,100.0,1.0])
ldm.initializeParameters()

print(ldm.getJacobianMatrix([.25,.15]))
print(ldm.getJacobianMatrixDQ([.25,.15]))
