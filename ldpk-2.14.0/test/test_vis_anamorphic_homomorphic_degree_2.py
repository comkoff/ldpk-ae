import ldpk
import ldpk_svg

ldm = ldpk.anamorphic_homomorphic_degree_2()
# gopro
ldm.camera((0.157583,0.64,0.36,0.0,0.0,1.0,91.44))

nx = 81
ny = 41

r = ldpk_svg.render_svg(ldm)
r.set_mapping("remove")
r.set_overscan(1.1,1.1)
r.set_oversize_image(3.0,3.0)
r.with_points(False)
r.with_diff(False)
r.with_grid(True)
r.set_stroke_color_mapped("#00f")

ldm.cx2(0.3).cy2(0.6).prepare()

r.render(nx,ny)
r.write()
