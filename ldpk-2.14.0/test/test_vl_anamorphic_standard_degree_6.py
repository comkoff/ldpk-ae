import vl_sdv as vl
import ldpk
import ldpk_svg

ldm = ldpk.anamorphic_standard_degree_6()
# gopro
ldm.camera((0.157583,0.64,0.36,0.0,0.0,1.0,91.44))

nx = 81
ny = 41

rnd = ldpk_svg.render_svg(ldm)
rnd.set_mapping("remove_apply")
rnd.set_overscan(1.0,1.0)
rnd.set_oversize_image(2.0,2.0)
rnd.with_points(False)
rnd.with_diff(False)
rnd.with_grid(True)
rnd.set_stroke_color_mapped("#00f")

ldm.cx02(0.4769811).cy02(1.3779209).cx22(-0.8935339).cy22(-0.4353864)
ldm.cx04(0.944127).cy04(-0.1719167).cx24(-0.2163173).cy24(-0.6751548).cx44(0.2440836).cy44(-0.0261635)
ldm.cx06(0.7185876).cy06(1.21891).cx26(-1.2089749).cy26(-0.0229961).cx46(-0.082658).cy46(-0.6337921).cx66(0.1238686).cy66(0.2647172)
ldm.phi_mnt(0.8813548)
ldm.sx(0.8878144).sy(0.8893092)
ldm.prepare()

#for i in range(11):
#	p = vl.vec2d(i / 10.0,i / 10.0)
#	print(p,":")
#	q = ldm.undistort(p)
#	r = ldm.distort(q)


#p = vl.vec2d(1.0,1.0)
#q = ldm.undistort(p)

rnd.render(nx,ny)

rnd.set_mapping("remove")
rnd.set_stroke_color_mapped("#09f")
rnd.render(nx,ny)

rnd.write()

# Convert to png like so:
# convert -density 300 /tmp/out.svg -resize 800x600 -quality 90 /tmp/out.png
