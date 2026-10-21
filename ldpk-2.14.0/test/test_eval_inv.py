import vl_sdv as vl
import ldpk
import lens_distortion_plugins

ldm = ldpk.anamorphic_standard_degree_6()
# gopro
ldm.camera((0.157583,0.64,0.36,0.0,0.0,1.0,91.44))

ldm.cx02(0.4769811).cy02(1.3779209).cx22(-0.8935339).cy22(-0.4353864)
ldm.cx04(0.944127).cy04(-0.1719167).cx24(-0.2163173).cy24(-0.6751548).cx44(0.2440836).cy44(-0.0261635)
ldm.cx06(0.7185876).cy06(1.21891).cx26(-1.2089749).cy26(-0.0229961).cx46(-0.082658).cy46(-0.6337921).cx66(0.1238686).cy66(0.2647172)
ldm.phi_mnt(0.8813548)
ldm.sx(0.8878144).sy(0.8893092)
ldm.prepare()

print(lens_distortion_plugins.__file__)

c = vl.vec2d(0.5,0.5)

p = vl.vec2d(0.3,0.3)
q = vl.vec2d(ldm.undistort(p))
r = vl.vec2d(ldm.distort(q))
print("p:",p)
print("q:",q)
print("r:",r)
print("(p-c)/(q-c):",vl.norm2(p-c) / vl.norm2(q-c))

p = vl.vec2d(-0.08,-0.08)
q = vl.vec2d(ldm.undistort(p))
r = vl.vec2d(ldm.distort(q))
print("p:",p)
print("q:",q)
print("r:",r)

