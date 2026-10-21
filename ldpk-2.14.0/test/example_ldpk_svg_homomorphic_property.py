import ldpk
import ldpk_svg

# This example demonstrates the homomorphic property of the
# toy model '3DE4 Anamorphic - Homomorphic, Degree 2".

ldm = ldpk.anamorphic_homomorphic_degree_2()
ldm.camera((3.0,2.0,1.0,0.0,0.0,2.0,100.0))

nx = 21
ny = 11

r = ldpk_svg.render_svg(ldm)
r.set_mapping("apply")
r.with_points(True)
r.with_diff(True)
r.with_grid(True)

r.set_point_color_original("#f00")
r.set_point_radius_original(3)
r.set_stroke_width_fov(1)

r.set_stroke_color_diff("#cf0")
r.set_stroke_width_diff(1)
r.set_stroke_color_original("#fdd")
r.set_stroke_width_original(3)

# Pick some distortion, paint in green
ldm.cx2(0.1).cy2(0.15).b2(0.05).prepare()
r.set_mapping("remove")
r.set_point_color_mapped("#7f0")
r.set_point_radius_mapped(5)
r.set_stroke_color_mapped("#afa")
r.set_stroke_width_mapped(5)
r.render(nx,ny)

# Now pick negative distortion and revert mapping, paint in blue.
# The blue grid is painted exactly on top of the green one.
ldm.cx2(-0.1).cy2(-0.15).b2(-0.05).prepare()
r.set_mapping("apply")
r.set_point_color_mapped("#007")
r.set_point_radius_mapped(2)
r.set_stroke_color_mapped("#007")
r.set_stroke_width_mapped(1)
r.render(nx,ny)

r.write()
