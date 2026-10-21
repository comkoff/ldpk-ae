import sys
import ldpk
import svgwrite

# - 1.0.0 [2024-04-10]	More comments and docstrings
# - 0.3.0 [2023-11-29]	Visualizing lens center, deferred
# - 0.2.0 [2023-11-29]	Appearance even more configurable
# - 0.1.0 [2023-11-29]	Appearance configurable
# - 0.0.0 [2023-11-29]	Get stuff running
__version__ = "1.0.0"

#       10        20        30        40        50        60        70        80        90       100
#--------|---------|---------|---------|---------|---------|---------|---------|---------|---------|
class render_svg:
	"""
	A class for rendering distortion grids as SVG.
	Example:
	--------8<---------------------------------------------------------------
		import ldpk
		import ldpk_svg

		# Create lens distortion model instance
		ldm = ldpk.radial_standard_degree_4()
		ldm.c2(0.1)

		# Create renderer
		r = ldpk_svg.render_svg(ldm)

		# Render and write to file
		r.reset().render(21,21).write(open("/path/to/my.svg","w"))
	-------->8---------------------------------------------------------------
	and load "/path/to/my.svg" in your favourite web browser.

	param `ldm`:	An instance of a lens distortion model from module ldpk
			e.g. `radial_standard_degree_4`
	"""
	def __init__(self,ldm):
# Instance from svgwrite, see method render().
		self._dwg = None
		self._ldm = ldm
		self._mapping = "remove"
# These flags specify what is being rendered for visualizing
# distortion: dots, grid lines and maybe difference vectors.
		self._with_points = True
		self._with_diff = True
		self._with_grid = True
# Domain
		self._qx_overscan = 1.0;
		self._qy_overscan = 1.0;
# Image
		self._qx_oversize_img = 1.1;
		self._qy_oversize_img = 1.1;
# Appearance. Semantically we distinguish between the "original"
# grid and the "mapped" grid, e.g undistorted and distorted or vice versa,
# that depends on what you intend to visualize. Suffix "diff" refers to
# visualizing differences between original and mapped, e.g. by little line
# segments.
		self._point_radius_original = 3
		self._point_color_original = "#c00"
		self._point_radius_mapped = 3
		self._point_color_mapped = "#0c0"
# Suffic "lco" stands for lens center offset. More precisely,
# these are attributes for rendering the lens center (which is a
# a dot).
		self._point_radius_lco = 3
		self._point_color_lco = "#000"

		self._stroke_width_diff = 1
		self._stroke_color_diff = "#333"
		self._stroke_width_original = 1
		self._stroke_color_original = "#d77"
		self._stroke_width_mapped = 1
		self._stroke_color_mapped = "#7d7"

		self._stroke_width_fov = 2
		self._stroke_color_fov = "#000"
# Elements drawn later. In SVG this make sense because by the
# order of drawing we define which element is in the foreground
# or in the background.
		self._deferred = []
# Delete old drawing instance
	def reset(self):
		"""
		Reset instance for a new rendering. Appearance attributes remain unchanged.
		return: self
		"""
		self._deferred = []
		self._dwg = None
		return self
# One of {"apply","remove"}
	def set_mapping(self,m):
		"""
		Set mapping mode, one of {"apply","remove"}. The class semantically
		distinguishes between the categories "original" and the "mapped" grid,
		so that you can pick different rendering attributes for either category.
		For example, if you set the mapping mode to "apply", the "original" grid
		represents an undistorted image, while the "mapped" grid represents a distorted
		image.
		return: self
		"""
		if m not in ("apply","remove","remove_apply"):
			raise RuntimeError("undefined mapping, pick one of 'apply', 'remove'")
		self._mapping = m
		return self
	def set_overscan(self,qx,qy):
		"""
		A factor to be applied to the domain scanned. This helps to detect strange
		behaviour outside the field of view. While the lens distortion mapping
		function is usually defined on the field of view [0,1]x[0,1], models should
		be robust up to a certain degree even outside the field of view. The factors
		passed here enlarge the domain to be visualized. A value of 1.1 for `qx`
		for instance enlarges the domain by 10% horizontally, that is 5% on the left
		and 5% on the right hand side.
		param `qx`:	Domain enlargement factor in x-direction
		param `qy`:	Domain enlargement factor in y-direction
		return: self
		"""
		self._qx_overscan = qx
		self._qy_overscan = qy
		return self
	def set_oversize_image(self,qx,qy):
		"""
		Formally, the SVG image has a width of 1000 (dimensionless), which corresponds
		to filmback width of the camera specified in the lens distortion instance.
		These two factors allow to enlarge the image without scaling the content.
		By default there is a boundary of 10% around the content (q=1.1), but in order
		to render more extreme grid distortions you may want to enlarge the image so that
		outer regions of the mapped grid are still inside the image.
		param `qx`:	Image enlargement factor in x-direction
		param `qy`:	Image enlargement factor in y-direction
		return: self
		"""
		self._qx_oversize_img = qx
		self._qy_oversize_img = qy
		return self
# Draw points
	def with_points(self,e):
		"""
		Enable/disable rendering for dots on grid line intersections.
		param `e`: boolean
		return: self
		"""
		self._with_points = e
		return self
# Draw difference vectors between original and (un)distorted?
	def with_diff(self,e):
		"""
		Enable/disable rendering for difference vectors between "original" grid and
		"mapped" grid.
		param `e`: boolean
		return: self
		"""
		self._with_diff = e
		return self
# Draw grid vector for original and (un)distorted?
	def with_grid(self,e):
		"""
		Enable/disable rendering for grid lines.
		param `e`: boolean
		return: self
		"""
		self._with_grid = e
		return self
# Defaults are ok, but this is helpful if you draw one grid
# on top of another one.
	def set_point_radius_original(self,w):
		"""
		Set radius of dots for the original grid. The image has a width of 1000 (times
		overscan factor as specified by the user). The radius is specified in the same
		unit, i.e. a value of 10 corresponds to 1% of image width.
		param `w`: point radius in sdv image coordinates.
		return: self
		"""
		self._point_radius_original = float(w)
		return self
	def set_point_color_original(self,c):
		"""
		Set point color. `c` is directly passed to the underlying SVG writer, so all
		formats defined there are also allowed here, like e.g. "#fff", "#ffff", "#ffffff",
		"#ffffffff#, `rgb(255,255,255)`, `rgba(255,255,255,255)`, names like "red",
		or `hsl(...)` for hue (degree), saturation (percent), lightness (percent).
		param `c`: svg color specification
		return: self
		"""
		self._point_color_original = str(c)
		return self
	def set_point_radius_mapped(self,w):
		"""
		see: `set_point_radius_original`.
		param `w`: point radius in sdv image coordinates.
		return: self
		"""
		self._point_radius_mapped = float(w)
		return self
	def set_point_color_mapped(self,c):
		"""
		see: `set_point_color_original`
		param `c`: svg color specification
		return: self
		"""
		self._point_color_mapped = str(c)
		return self
	def set_point_radius_lco(self,w):
		"""
		see: `set_point_radius_original`.
		param `w`: point radius in sdv image coordinates.
		return: self
		"""
		self._point_radius_lco = float(w)
		return self
	def set_point_color_lco(self,c):
		"""
		see: `set_point_color_original`
		param `c`: svg color specification
		return: self
		"""
		self._point_color_lco = str(c)
		return self

	def set_stroke_width_diff(self,w):
		"""
		Set stroke width for difference vectors. The image has a width of 1000 (times
		overscan factor as specified by the user). The stroke width is specified in the
		same unit, i.e. a value of 10 corresponds to 1% of image width.
		param `w`: stroke width in sdv image coordinates.
		return: self
		"""
		self._stroke_width_diff = float(w)
		return self
	def set_stroke_color_diff(self,c):
		"""
		Set stroke color. `c` is directly passed to the underlying SVG writer, so all
		formats defined there are also allowed here, like e.g. "#fff", "#ffff", "#ffffff",
		"#ffffffff#, `rgb(255,255,255)`, `rgba(255,255,255,255)`, names like "red",
		or `hsl(...)` for hue (degree), saturation (percent), lightness (percent).
		param `c`: svg color specification
		return: self
		"""
		self._stroke_color_diff = str(c)
		return self
	def set_stroke_width_original(self,w):
		"""
		see: `set_stroke_width_diff`
		param `w`: stroke width in sdv image coordinates
		return: self
		"""
		self._stroke_width_original = float(w)
		return self
	def set_stroke_color_original(self,c):
		"""
		see: `set_stroke_color_diff`
		param `c`: svg color specification
		return: self
		"""
		self._stroke_color_original = str(c)
		return self
	def set_stroke_width_mapped(self,w):
		"""
		see: `set_stroke_width_diff`
		param `w`: stroke width in sdv image coordinates
		return: self
		"""
		self._stroke_width_mapped = float(w)
		return self
	def set_stroke_color_mapped(self,c):
		"""
		see: `set_stroke_color_diff`
		param `c`: svg color specification
		return: self
		"""
		self._stroke_color_mapped = str(c)
		return self
	def set_stroke_width_fov(self,w):
		"""
		see: `set_stroke_width_diff`
		param `w`: stroke width in sdv image coordinates
		return: self
		"""
		self._stroke_width_fov = float(w)
		return self
	def set_stroke_color_fov(self,c):
		"""
		see: `set_stroke_color_diff`
		param `c`: svg color specification
		return: self
		"""
		self._stroke_color_fov = str(c)
		return self

# Number of grid lines or points, e.g. 21,11.
	def render(self,num_points_x,num_points_y):
		"""
		Render all elements as specified in the instance to an internal buffer.
		see: `write`
		param `num_points_x`: Number of grid points in x-direction
		param `num_points_y`: Number of grid points in y-direction
		return: self
		"""
		self._ldm.prepare()
# Build scale factor for everything from filmback width.
		w_fb_cm = self._ldm.w_fb_cm()
		h_fb_cm = self._ldm.h_fb_cm()
		s = 1000.0 / w_fb_cm
# fov is for field of view, the sensor in unit coordinates.
		self.svg_size_image = (	self._qx_oversize_img * self._qx_overscan * s * w_fb_cm,
					self._qy_oversize_img * self._qy_overscan * s * h_fb_cm)
		self.svg_pos_fov = (	(self._qx_oversize_img * self._qx_overscan - 1.0) / 2.0 * s * w_fb_cm,
					(self._qy_oversize_img * self._qy_overscan - 1.0) / 2.0 * s * h_fb_cm)
		self.svg_size_fov = (s * w_fb_cm,s * h_fb_cm)

# If there is no drawing instance, create one.
		if self._dwg is None:
			self._dwg = svgwrite.Drawing(size = self.svg_size_image)
# Map unit coordinates to the fov in svg coordinates.
		map_x = lambda x: self.svg_pos_fov[0] + self.svg_size_fov[0] * x
		map_y = lambda y: self.svg_pos_fov[1] + self.svg_size_fov[1] * y
# Iterate over point pairs
		pairs = []
		if self._mapping == "remove":
			for iy in range(num_points_y):
				py = self._qy_overscan * float(iy) / (num_points_y - 1) - (self._qy_overscan - 1.0) / 2.0
				for ix in range(num_points_x):
					px = self._qx_overscan * float(ix) / (num_points_x - 1) - (self._qx_overscan - 1.0) / 2.0
					qx,qy = self._ldm.undistort((px,py))
					pairs.append((map_x(px),map_y(py),map_x(qx),map_y(qy)))
		elif self._mapping == "apply":
			for iy in range(num_points_y):
				py = self._qy_overscan * float(iy) / (num_points_y - 1) - (self._qy_overscan - 1.0) / 2.0
				for ix in range(num_points_x):
					px = self._qx_overscan * float(ix) / (num_points_x - 1) - (self._qx_overscan - 1.0) / 2.0
					qx,qy = self._ldm.distort((px,py))
					pairs.append((map_x(px),map_y(py),map_x(qx),map_y(qy)))
		elif self._mapping == "remove_apply":
			for iy in range(num_points_y):
				py = self._qy_overscan * float(iy) / (num_points_y - 1) - (self._qy_overscan - 1.0) / 2.0
				for ix in range(num_points_x):
					px = self._qx_overscan * float(ix) / (num_points_x - 1) - (self._qx_overscan - 1.0) / 2.0
					rx,ry = self._ldm.undistort((px,py))
#					if (rx < - (self._qx_overscan - 1.0) / 2.0) or (rx > self._qx_overscan - (self._qx_overscan - 1.0) / 2.0):
#						pairs.append(None)
#						continue
#					if (ry < - (self._qy_overscan - 1.0) / 2.0) or (ry > self._qy_overscan - (self._qy_overscan - 1.0) / 2.0):
#						pairs.append(None)
#						continue
					qx,qy = self._ldm.distort((rx,ry))
					pairs.append((map_x(px),map_y(py),map_x(qx),map_y(qy)))
# Render difference lines
		if self._with_diff:
			for iy in range(num_points_y):
				for ix in range(num_points_x):
					if pairs[ix + 0 + num_points_x * iy] is None:
						continue
					px_fov,py_fov,qx_fov,qy_fov = pairs[ix + 0 + num_points_x * iy]
# line from original to mapped point
					line = svgwrite.shapes.Line(	start		= (px_fov,py_fov),
									end		= (qx_fov,qy_fov),
									stroke		= self._stroke_color_diff,
									stroke_width	= self._stroke_width_diff)
					self._dwg.add(line)
# Render grids
		if self._with_grid:
# Horizontal lines
			for iy in range(num_points_y):
				for ix in range(num_points_x - 1):
					if pairs[ix + 0 + num_points_x * iy] is None:
						continue
					if pairs[ix + 1 + num_points_x * iy] is None:
						continue
					px1_fov,py1_fov,qx1_fov,qy1_fov = pairs[ix + 0 + num_points_x * iy]
					px2_fov,py2_fov,qx2_fov,qy2_fov = pairs[ix + 1 + num_points_x * iy]
# original point
					line = svgwrite.shapes.Line(	start		= (px1_fov,py1_fov),
									end		= (px2_fov,py2_fov),
									stroke		= self._stroke_color_original,
									stroke_width	= self._stroke_width_original)
					self._dwg.add(line)
# mapped point
					line = svgwrite.shapes.Line(	start		= (qx1_fov,qy1_fov),
									end		= (qx2_fov,qy2_fov),
									stroke		= self._stroke_color_mapped,
									stroke_width	= self._stroke_width_mapped)
					self._dwg.add(line)
# Vertical lines
			for iy in range(num_points_y - 1):
				for ix in range(num_points_x):
					if pairs[ix + num_points_x * (iy + 0)] is None:
						continue
					if pairs[ix + num_points_x * (iy + 1)] is None:
						continue
					px1_fov,py1_fov,qx1_fov,qy1_fov = pairs[ix + num_points_x * (iy + 0)]
					px2_fov,py2_fov,qx2_fov,qy2_fov = pairs[ix + num_points_x * (iy + 1)]
# original point
					line = svgwrite.shapes.Line(	start		= (px1_fov,py1_fov),
									end		= (px2_fov,py2_fov),
									stroke		= self._stroke_color_original,
									stroke_width	= self._stroke_width_original)
					self._dwg.add(line)
# mapped point
					line = svgwrite.shapes.Line(	start		= (qx1_fov,qy1_fov),
									end		= (qx2_fov,qy2_fov),
									stroke		= self._stroke_color_mapped,
									stroke_width	= self._stroke_width_mapped)
					self._dwg.add(line)
# Render points
		if self._with_points:
			for iy in range(num_points_y):
				for ix in range(num_points_x):
					px_fov,py_fov,qx_fov,qy_fov = pairs[ix + 0 + num_points_x * iy]
# original point
					circle = svgwrite.shapes.Circle(center		= (px_fov,py_fov),
									r		= self._point_radius_original,
									fill		= self._point_color_original)
					self._dwg.add(circle)
# mapped point
					circle = svgwrite.shapes.Circle(center		= (qx_fov,qy_fov),
									r		= self._point_radius_mapped,
									fill		= self._point_color_mapped)
					self._dwg.add(circle)
# Lens Center
		x_lco_fov = map_x(1.0 / 2.0 + self._ldm.x_lco_cm() / self._ldm.w_fb_cm())
		y_lco_fov = map_y(1.0 / 2.0 + self._ldm.y_lco_cm() / self._ldm.h_fb_cm())
		circle = svgwrite.shapes.Circle(center		= (x_lco_fov,y_lco_fov),
						r		= self._point_radius_lco,
						fill		= self._point_color_lco)
		self._deferred.append(circle)


# Draw the field of view last, so that it's always visible.
		rect = svgwrite.shapes.Rect(	insert		= self.svg_pos_fov,
						size		= self.svg_size_fov,
						stroke		= self._stroke_color_fov,
						stroke_width	= self._stroke_width_fov,
						fill		='none')
		self._dwg.add(rect)

		self._dwg['viewBox'] = '0 0 {} {}'.format(self.svg_size_image[0],self.svg_size_image[1])
		return self

	def write(self,fout = sys.stdout):
		"""
		Call this method for writing the SVG image to a file or stream object.
		param `fout`: file or stream object
		return: self
		"""
		for e in self._deferred:
			self._dwg.add(e)
		self._dwg.write(fout,pretty = True, indent = 2)
		return self

if __name__ == '__main__':
	print("version:",__version__)
