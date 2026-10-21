from __future__ import annotations

from typing import Any, TYPE_CHECKING, Protocol, Iterator, Tuple, ClassVar, cast
from typing_extensions import Self, TypeAlias

import lens_distortion_plugins as ldp
import math



__version__ = "1.2.0"

# Versions:
# - 1.2.0 [2024-07-02]	Stabilizer models; Python 3.8, 3.9, 3.10, 3.11, 3.12; Linux, MacOS, Windows
# - 1.1.0 [2022-12-10]	Math symbols for auto-generated reference documents..
# - 1.0.0 [2022-03-18]	Automated documentation; End of python2; ...
# Location:		$LDPK
# Maintainer:		uwe@sci-d-vis.com
# Scope:		public

# Versioning started late, but we try to provide a reasonable
# Changelog as of version 1.2.0.

# Status flags:
# external, experimental, deprecated.
# We can give full support for models which are marked:
# not external and not deprecated and not experimental.
# The flags are defined as follows:
# - external: The model has been defined by some organization 
#             outside SDV. The full mathematical specifications
#             should be provided by the organization as well, i.e.
#             we can only give an incomplete description of such a model.
# - experimental: These models are not hardcoded in 3DE4 but available
#             as plugins (Linux, OSX). The set of parameters may vary
#             in the future. Support for compositing systems may be incomplete.
# - deprecated: Some models are marked as deprecated, if it is considered as
#             "technologically insufficient". They may remain part of 3DE4
#             for quite some time, but support for compositing systems and
#             documentation may end gradually.

#----- begin Type checking ------------------------------------#
ParameterName: TypeAlias	= str
ParameterValue: TypeAlias	= float
ParameterIndex: TypeAlias	= int
GuiModelName: TypeAlias		= str
MathName: TypeAlias		= str
CameraTuple: TypeAlias		= Tuple[float,float,float,float,float,float,float]
ParameterRange: TypeAlias	= Tuple[float,float]
ModelClass: TypeAlias		= type[Any]
ModelRegistry: TypeAlias	= dict[GuiModelName,ModelClass]
ModelList: TypeAlias		= list[ModelClass]

if TYPE_CHECKING:
	class _LdpPluginApi(Protocol):
		def getModelName(self) -> str: ...
		def setParameterValueDouble(self, parameter_name: ParameterName, value: ParameterValue) -> None: ...
		def parameter_value(self, parameter_name: ParameterName) -> ParameterValue: ...
		def setCamera(
			self,
			fl_cm: float,
			w_fb_cm: float,
			h_fb_cm: float,
			x_lco_cm: float,
			y_lco_cm: float,
			r_pa: float,
			fd_cm: float,
		) -> None: ...
		def getCamera(self) -> CameraTuple: ...
		def getParameterName(self, index: ParameterIndex) -> ParameterName: ...
		def getNumParameters(self) -> int: ...
		def getParameterDefaultValue(self, parameter_name: ParameterName) -> ParameterValue: ...
		def getParameterRange(self, parameter_name: ParameterName) -> ParameterRange: ...
		def initializeParameters(self) -> None: ...
else:
	class _LdpPluginApi:
		pass

#----- end Type checking --------------------------------------#


# In python 3.9 we write c.isascii(),
# but python 3.6 does not have this.
def is_ascii(c: str) -> bool:
	return ord(c) < 128
# The official function for mapping GUI names into identifiers
# which can be used as function / method names, variables or
# XML element and attribute names.
def sdv_make_identifier(guiname: str) -> str:
# 1. Turn everything which is not alphanumeric into an underscore
	s = [c if c.isalnum() and is_ascii(c) else "_" for c in guiname]
# 2. Merge multiple underscores into a single one
	t = ""
	umode = False
	for c in s:
		if c == "_":
			if not umode:
				t += c
			umode = True
		else:
			t += c
			umode = False
# 3. If the result starts with a digit, prepend an underscore.
	if t[0].isnumeric():
		t = "_" + t
	return t

# With lower case conversion. My personal taste
# is to have lower case for variables and functions,
# even if this contradicts coding standards. Sorry...
def sdv_make_identifier_lc(guiname: str) -> str:
	return sdv_make_identifier(guiname).lower()

# We generate the math symbol by lots of cases. This is
# of course for the auto-generated document. If you want to
# render these symbols yourself, you need a few CSS-definitions,
# like
# ldpk {font-family:"Time New Roman",serif;}
# subsup {position:relative;}
# ldpk > subsup > sup {position:absolute;vertical-align:0;bottom:0.4em;left:0;font-size:70%;}
# ldpk > subsup > sub {position:absolute;vertical-align:0;top:0.4em;left:0;font-size:70%;}

def sdv_make_math_name(s: MathName) -> MathName:
# Do special cases first
	if s == "phi_mnt":
		return "<tde4_math><i>&varphi;</i><subsup><sub>mnt</sub></subsup></tde4_math>"
	elif s == "phi_bs":
		return "<tde4_math><i>&varphi;</i><subsup><sub>bs</sub></subsup></tde4_math>"
	elif s == "b_bs":
		return "<tde4_math><i>b</i><subsup><sub>bs</sub></subsup></tde4_math>"
# And now the more general ones
	elif s[0] in ("b","c","k","p","u","v","t"):
		if s[1] in ("x","y"):
			return "<tde4_math><i>" + s[0] + "</i><subsup><sub>" + s[2:] + "</sub><sup>(" + s[1] + ")</sup></subsup></tde4_math>"
		else:
			return "<tde4_math><i>" + s[0] + "</i><subsup><sub>" + s[1:] + "</sub></subsup></tde4_math>"
	elif s[0] in ("s",):
		if s[1] in ("x","y"):
			if len(s) == 2:
				return "<tde4_math><i>" + s[0] + "</i><subsup><sub>" + s[1] + "</sub></subsup></tde4_math>"
			else:
				return s
		elif len(s) == 4 and s[0:3] == "stb":
			if s[3] in ("x","y"):
				return "<tde4_math><i>" + s[3] + "</i><subsup><sub>stb,unit</sub></subsup></tde4_math>"
			else:
				return s
		elif s[1] == "_":
				return "<tde4_math><i>" + s[0] + "</i><subsup><sub>" + s[2:] + "</sub></subsup></tde4_math>"
		else:
			return s
	else:
		return s

# Convenience functions including get-functionality
# for the seven built-in parameters.
class builtin(_LdpPluginApi):
	parameters: ClassVar[list[ParameterName]] = []
	long_names: ClassVar[list[ParameterName]] = []
	short_names: ClassVar[list[ParameterName]] = []
	math_names: ClassVar[list[MathName]] = []

	def __init__(self) -> None:
		self._fl_cm = 2.0
		self._w_fb_cm = 1.6
		self._h_fb_cm = 0.9
		self._x_lco_cm = 0.0
		self._y_lco_cm = 0.0
		self._r_pa = 1.0
		self._fd_cm = 100.0
# Name of distortion model
	def name(self) -> str:
		return self.getModelName()
# Invoked by derived classes
	def init_builtin_parameters(self) -> None:
		self.setParameterValueDouble("tde4_focal_length_cm",self._fl_cm)
		self.setParameterValueDouble("tde4_filmback_width_cm",self._w_fb_cm)
		self.setParameterValueDouble("tde4_filmback_height_cm",self._h_fb_cm)
		self.setParameterValueDouble("tde4_lens_center_offset_x_cm",self._x_lco_cm )
		self.setParameterValueDouble("tde4_lens_center_offset_y_cm",self._y_lco_cm)
		self.setParameterValueDouble("tde4_pixel_aspect",self._r_pa)
		self.setParameterValueDouble("tde4_custom_focus_distance_cm",self._fd_cm)
# Getter in gui notation
	def getParameterValue(self,p: ParameterName) -> ParameterValue:
		return self.parameter_value(p)
# Setters and Getters in gui notation, i.e. long names, jquery-style
	def builtin_parameter_value(self,p: ParameterName,v: ParameterValue | None = None) -> Self | ParameterValue:
		if	p == "tde4_focal_length_cm":		return self.fl_cm(v)
		elif	p == "tde4_filmback_width_cm":		return self.w_fb_cm(v)
		elif	p == "tde4_filmback_height_cm":		return self.h_fb_cm(v)
		elif	p == "tde4_lens_center_offset_x_cm":	return self.x_lco_cm(v)
		elif	p == "tde4_lens_center_offset_y_cm":	return self.y_lco_cm(v)
		elif	p == "tde4_pixel_aspect":		return self.r_pa(v)
		elif	p == "tde4_custom_focus_distance_cm":	return self.fd_cm(v)
		raise KeyError(p)
# Setters and Getters in mathematical notation, jquery-style
	def fl_cm(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._fl_cm = q
			self.setParameterValueDouble("tde4_focal_length_cm",q)
			return self
		else:
			return self._fl_cm
	def w_fb_cm(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._w_fb_cm = q
			self.setParameterValueDouble("tde4_filmback_width_cm",q)
			return self
		else:
			return self._w_fb_cm
	def h_fb_cm(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._h_fb_cm = q
			self.setParameterValueDouble("tde4_filmback_height_cm",q)
			return self
		else:
			return self._h_fb_cm
	def x_lco_cm(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._x_lco_cm = q
			self.setParameterValueDouble("tde4_lens_center_offset_x_cm",q)
			return self
		else:
			return self._x_lco_cm
	def y_lco_cm(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._y_lco_cm = q
			self.setParameterValueDouble("tde4_lens_center_offset_y_cm",q)
			return self
		else:
			return self._y_lco_cm
	def r_pa(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._r_pa = q
			self.setParameterValueDouble("tde4_pixel_aspect",q)
			return self
		else:
			return self._r_pa
	def fd_cm(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._fd_cm = q
			self.setParameterValueDouble("tde4_custom_focus_distance_cm",q)
			return self
		else:
			return self._fd_cm
	def camera(self,c: CameraTuple | None = None) -> Self | CameraTuple:
		if c is not None:
			self.setCamera(*c)
			self._fl_cm = c[0]
			self._w_fb_cm = c[1]
			self._h_fb_cm = c[2]
			self._x_lco_cm = c[3]
			self._y_lco_cm = c[4]
			self._r_pa = c[5]
			self._fd_cm = c[6]
			return self
		else:
			return self.getCamera()
# Derived quanities: Lens center in cm with respect to lower left corner.
	def x_lc_cm(self) -> float:
		return cast(float, self.w_fb_cm()) * (1.0/2.0) + cast(float, self.x_lco_cm())
	def y_lc_cm(self) -> float:
		return cast(float, self.h_fb_cm()) * (1.0/2.0) + cast(float, self.y_lco_cm())
	def r_fb_cm(self) -> float:
		return math.sqrt(
			cast(float, self.w_fb_cm()) * cast(float, self.w_fb_cm()) +
			cast(float, self.h_fb_cm()) * cast(float, self.h_fb_cm())
		) * (1.0/2.0)
# Iterating over model dependent parameter names
	def __iter__(self) -> Iterator[str]:
		return iter(self.__class__.parameters)
# Convenience method: Get value by parameter index (as opposed to parameter name)
	def get_value(self,i: ParameterIndex) -> ParameterValue:
		return self.getParameterValue(self.getParameterName(i))
# Convenience method: Get default value by parameter index (as opposed to parameter name)
	def get_default_value(self,i: ParameterIndex) -> ParameterValue:
		return self.getParameterDefaultValue(self.getParameterName(i))
# Convenience method: Get parameter range by parameter index (as opposed to parameter name)
	def get_range(self,i: ParameterIndex) -> ParameterRange:
		return self.getParameterRange(self.getParameterName(i))
# Prepare for evaluation
	def prepare(self) -> None:
		"""Invoke this method after specifying camera and distortion parameters.
		This will call the tde4_ld_plugin-method initializeParameters()."""
		self.initializeParameters()

class anamorphic_standard_degree_4(builtin,ldp.anamorphic_standard_degree_4):
	"""A production proven model for anamorphic lenses with polynomial degree 4,
	with scaling and lens rotation."""

	short_names = ["cx02","cy02","cx22","cy22","cx04","cy04","cx24","cy24","cx44","cy44","phi_mnt","sx","sy"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.anamorphic_standard_degree_4.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
		self._cx02 = self.getParameterDefaultValue("Cx02 - Degree 2")
		self._cy02 = self.getParameterDefaultValue("Cy02 - Degree 2")
		self._cx22 = self.getParameterDefaultValue("Cx22 - Degree 2")
		self._cy22 = self.getParameterDefaultValue("Cy22 - Degree 2")
		self._cx04 = self.getParameterDefaultValue("Cx04 - Degree 4")
		self._cy04 = self.getParameterDefaultValue("Cy04 - Degree 4")
		self._cx24 = self.getParameterDefaultValue("Cx24 - Degree 4")
		self._cy24 = self.getParameterDefaultValue("Cy24 - Degree 4")
		self._cx44 = self.getParameterDefaultValue("Cx44 - Degree 4")
		self._cy44 = self.getParameterDefaultValue("Cy44 - Degree 4")
		self._phi_mnt = self.getParameterDefaultValue("Lens Rotation")
		self._sx = self.getParameterDefaultValue("Squeeze-X")
		self._sy = self.getParameterDefaultValue("Squeeze-Y")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Cx02 - Degree 2":	return self._cx02
		elif	p == "Cy02 - Degree 2":	return self._cy02
		elif	p == "Cx22 - Degree 2":	return self._cx22
		elif	p == "Cy22 - Degree 2":	return self._cy22
		elif	p == "Cx04 - Degree 4":	return self._cx04
		elif	p == "Cy04 - Degree 4":	return self._cy04
		elif	p == "Cx24 - Degree 4":	return self._cx24
		elif	p == "Cy24 - Degree 4":	return self._cy24
		elif	p == "Cx44 - Degree 4":	return self._cx44
		elif	p == "Cy44 - Degree 4":	return self._cy44
		elif	p == "Lens Rotation":	return self._phi_mnt
		elif	p == "Squeeze-X":	return self._sx
		elif	p == "Squeeze-Y":	return self._sy
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def cx02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx02 = q
			self.setParameterValueDouble("Cx02 - Degree 2",q)
			return self
		else:
			return self._cx02
	def cy02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy02 = q
			self.setParameterValueDouble("Cy02 - Degree 2",q)
			return self
		else:
			return self._cy02
	def cx22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx22 = q
			self.setParameterValueDouble("Cx22 - Degree 2",q)
			return self
		else:
			return self._cx22
	def cy22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy22 = q
			self.setParameterValueDouble("Cy22 - Degree 2",q)
			return self
		else:
			return self._cy22
	def cx04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx04 = q
			self.setParameterValueDouble("Cx04 - Degree 4",q)
			return self
		else:
			return self._cx04
	def cy04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy04 = q
			self.setParameterValueDouble("Cy04 - Degree 4",q)
			return self
		else:
			return self._cy04
	def cx24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx24 = q
			self.setParameterValueDouble("Cx24 - Degree 4",q)
			return self
		else:
			return self._cx24
	def cy24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy24 = q
			self.setParameterValueDouble("Cy24 - Degree 4",q)
			return self
		else:
			return self._cy24
	def cx44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx44 = q
			self.setParameterValueDouble("Cx44 - Degree 4",q)
			return self
		else:
			return self._cx44
	def cy44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy44 = q
			self.setParameterValueDouble("Cy44 - Degree 4",q)
			return self
		else:
			return self._cy44
	def phi_mnt(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._phi_mnt = q
			self.setParameterValueDouble("Lens Rotation",q)
			return self
		else:
			return self._phi_mnt
	def sx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sx = q
			self.setParameterValueDouble("Squeeze-X",q)
			return self
		else:
			return self._sx
	def sy(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sy = q
			self.setParameterValueDouble("Squeeze-Y",q)
			return self
		else:
			return self._sy

class anamorphic_rescaled_degree_4(builtin,ldp.anamorphic_rescaled_degree_4):
	"""Similar to anamorphic_standard_degree_4, but for images rescaled to a more
	convenient pixel aspect ratio for better representation in GUIs."""

	short_names = ["cx02","cy02","cx22","cy22","cx04","cy04","cx24","cy24","cx44","cy44","phi_mnt","sx","sy","s_rscl"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.anamorphic_rescaled_degree_4.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
		self._cx02 = self.getParameterDefaultValue("Cx02 - Degree 2")
		self._cy02 = self.getParameterDefaultValue("Cy02 - Degree 2")
		self._cx22 = self.getParameterDefaultValue("Cx22 - Degree 2")
		self._cy22 = self.getParameterDefaultValue("Cy22 - Degree 2")
		self._cx04 = self.getParameterDefaultValue("Cx04 - Degree 4")
		self._cy04 = self.getParameterDefaultValue("Cy04 - Degree 4")
		self._cx24 = self.getParameterDefaultValue("Cx24 - Degree 4")
		self._cy24 = self.getParameterDefaultValue("Cy24 - Degree 4")
		self._cx44 = self.getParameterDefaultValue("Cx44 - Degree 4")
		self._cy44 = self.getParameterDefaultValue("Cy44 - Degree 4")
		self._phi_mnt = self.getParameterDefaultValue("Lens Rotation")
		self._sx = self.getParameterDefaultValue("Squeeze-X")
		self._sy = self.getParameterDefaultValue("Squeeze-Y")
		self._s_rscl = self.getParameterDefaultValue("Rescale")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Cx02 - Degree 2":	return self._cx02
		elif	p == "Cy02 - Degree 2":	return self._cy02
		elif	p == "Cx22 - Degree 2":	return self._cx22
		elif	p == "Cy22 - Degree 2":	return self._cy22
		elif	p == "Cx04 - Degree 4":	return self._cx04
		elif	p == "Cy04 - Degree 4":	return self._cy04
		elif	p == "Cx24 - Degree 4":	return self._cx24
		elif	p == "Cy24 - Degree 4":	return self._cy24
		elif	p == "Cx44 - Degree 4":	return self._cx44
		elif	p == "Cy44 - Degree 4":	return self._cy44
		elif	p == "Lens Rotation":	return self._phi_mnt
		elif	p == "Squeeze-X":	return self._sx
		elif	p == "Squeeze-Y":	return self._sy
		elif	p == "Rescale":		return self._s_rscl
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def cx02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx02 = q
			self.setParameterValueDouble("Cx02 - Degree 2",q)
			return self
		else:
			return self._cx02
	def cy02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy02 = q
			self.setParameterValueDouble("Cy02 - Degree 2",q)
			return self
		else:
			return self._cy02
	def cx22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx22 = q
			self.setParameterValueDouble("Cx22 - Degree 2",q)
			return self
		else:
			return self._cx22
	def cy22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy22 = q
			self.setParameterValueDouble("Cy22 - Degree 2",q)
			return self
		else:
			return self._cy22
	def cx04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx04 = q
			self.setParameterValueDouble("Cx04 - Degree 4",q)
			return self
		else:
			return self._cx04
	def cy04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy04 = q
			self.setParameterValueDouble("Cy04 - Degree 4",q)
			return self
		else:
			return self._cy04
	def cx24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx24 = q
			self.setParameterValueDouble("Cx24 - Degree 4",q)
			return self
		else:
			return self._cx24
	def cy24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy24 = q
			self.setParameterValueDouble("Cy24 - Degree 4",q)
			return self
		else:
			return self._cy24
	def cx44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx44 = q
			self.setParameterValueDouble("Cx44 - Degree 4",q)
			return self
		else:
			return self._cx44
	def cy44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy44 = q
			self.setParameterValueDouble("Cy44 - Degree 4",q)
			return self
		else:
			return self._cy44
	def phi_mnt(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._phi_mnt = q
			self.setParameterValueDouble("Lens Rotation",q)
			return self
		else:
			return self._phi_mnt
	def sx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sx = q
			self.setParameterValueDouble("Squeeze-X",q)
			return self
		else:
			return self._sx
	def sy(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sy = q
			self.setParameterValueDouble("Squeeze-Y",q)
			return self
		else:
			return self._sy
	def s_rscl(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_rscl = q
			self.setParameterValueDouble("Rescale",q)
			return self
		else:
			return self._s_rscl

class anamorphic_standard_degree_6(builtin,ldp.anamorphic_standard_degree_6):
	"""Similar to anamorphic_standard_degree_4 but with additional degree-6 parameters."""

	short_names = ["cx02","cy02","cx22","cy22","cx04","cy04","cx24","cy24","cx44","cy44","cx06","cy06","cx26","cy26","cx46","cy46","cx66","cy66","phi_mnt","sx","sy"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.anamorphic_standard_degree_6.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
		self._cx02 = self.getParameterDefaultValue("Cx02 - Degree 2")
		self._cy02 = self.getParameterDefaultValue("Cy02 - Degree 2")
		self._cx22 = self.getParameterDefaultValue("Cx22 - Degree 2")
		self._cy22 = self.getParameterDefaultValue("Cy22 - Degree 2")
		self._cx04 = self.getParameterDefaultValue("Cx04 - Degree 4")
		self._cy04 = self.getParameterDefaultValue("Cy04 - Degree 4")
		self._cx24 = self.getParameterDefaultValue("Cx24 - Degree 4")
		self._cy24 = self.getParameterDefaultValue("Cy24 - Degree 4")
		self._cx44 = self.getParameterDefaultValue("Cx44 - Degree 4")
		self._cy44 = self.getParameterDefaultValue("Cy44 - Degree 4")
		self._cx06 = self.getParameterDefaultValue("Cx06 - Degree 6")
		self._cy06 = self.getParameterDefaultValue("Cy06 - Degree 6")
		self._cx26 = self.getParameterDefaultValue("Cx26 - Degree 6")
		self._cy26 = self.getParameterDefaultValue("Cy26 - Degree 6")
		self._cx46 = self.getParameterDefaultValue("Cx46 - Degree 6")
		self._cy46 = self.getParameterDefaultValue("Cy46 - Degree 6")
		self._cx66 = self.getParameterDefaultValue("Cx66 - Degree 6")
		self._cy66 = self.getParameterDefaultValue("Cy66 - Degree 6")
		self._phi_mnt = self.getParameterDefaultValue("Lens Rotation")
		self._sx = self.getParameterDefaultValue("Squeeze-X")
		self._sy = self.getParameterDefaultValue("Squeeze-Y")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Cx02 - Degree 2":	return self._cx02
		elif	p == "Cy02 - Degree 2":	return self._cy02
		elif	p == "Cx22 - Degree 2":	return self._cx22
		elif	p == "Cy22 - Degree 2":	return self._cy22
		elif	p == "Cx04 - Degree 4":	return self._cx04
		elif	p == "Cy04 - Degree 4":	return self._cy04
		elif	p == "Cx24 - Degree 4":	return self._cx24
		elif	p == "Cy24 - Degree 4":	return self._cy24
		elif	p == "Cx44 - Degree 4":	return self._cx44
		elif	p == "Cy44 - Degree 4":	return self._cy44
		elif	p == "Cx06 - Degree 6":	return self._cx06
		elif	p == "Cy06 - Degree 6":	return self._cy06
		elif	p == "Cx26 - Degree 6":	return self._cx26
		elif	p == "Cy26 - Degree 6":	return self._cy26
		elif	p == "Cx46 - Degree 6":	return self._cx46
		elif	p == "Cy46 - Degree 6":	return self._cy46
		elif	p == "Cx66 - Degree 6":	return self._cx66
		elif	p == "Cy66 - Degree 6":	return self._cy66
		elif	p == "Lens Rotation":	return self._phi_mnt
		elif	p == "Squeeze-X":	return self._sx
		elif	p == "Squeeze-Y":	return self._sy
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def cx02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx02 = q
			self.setParameterValueDouble("Cx02 - Degree 2",q)
			return self
		else:
			return self._cx02
	def cy02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy02 = q
			self.setParameterValueDouble("Cy02 - Degree 2",q)
			return self
		else:
			return self._cy02
	def cx22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx22 = q
			self.setParameterValueDouble("Cx22 - Degree 2",q)
			return self
		else:
			return self._cx22
	def cy22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy22 = q
			self.setParameterValueDouble("Cy22 - Degree 2",q)
			return self
		else:
			return self._cy22
	def cx04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx04 = q
			self.setParameterValueDouble("Cx04 - Degree 4",q)
			return self
		else:
			return self._cx04
	def cy04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy04 = q
			self.setParameterValueDouble("Cy04 - Degree 4",q)
			return self
		else:
			return self._cy04
	def cx24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx24 = q
			self.setParameterValueDouble("Cx24 - Degree 4",q)
			return self
		else:
			return self._cx24
	def cy24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy24 = q
			self.setParameterValueDouble("Cy24 - Degree 4",q)
			return self
		else:
			return self._cy24
	def cx44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx44 = q
			self.setParameterValueDouble("Cx44 - Degree 4",q)
			return self
		else:
			return self._cx44
	def cy44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy44 = q
			self.setParameterValueDouble("Cy44 - Degree 4",q)
			return self
		else:
			return self._cy44
	def cx06(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx06 = q
			self.setParameterValueDouble("Cx06 - Degree 6",q)
			return self
		else:
			return self._cx06
	def cy06(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy06 = q
			self.setParameterValueDouble("Cy06 - Degree 6",q)
			return self
		else:
			return self._cy06
	def cx26(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx26 = q
			self.setParameterValueDouble("Cx26 - Degree 6",q)
			return self
		else:
			return self._cx26
	def cy26(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy26 = q
			self.setParameterValueDouble("Cy26 - Degree 6",q)
			return self
		else:
			return self._cy26
	def cx46(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx46 = q
			self.setParameterValueDouble("Cx46 - Degree 6",q)
			return self
		else:
			return self._cx46
	def cy46(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy46 = q
			self.setParameterValueDouble("Cy46 - Degree 6",q)
			return self
		else:
			return self._cy46
	def cx66(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx66 = q
			self.setParameterValueDouble("Cx66 - Degree 6",q)
			return self
		else:
			return self._cx66
	def cy66(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy66 = q
			self.setParameterValueDouble("Cy66 - Degree 6",q)
			return self
		else:
			return self._cy66
	def phi_mnt(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._phi_mnt = q
			self.setParameterValueDouble("Lens Rotation",q)
			return self
		else:
			return self._phi_mnt
	def sx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sx = q
			self.setParameterValueDouble("Squeeze-X",q)
			return self
		else:
			return self._sx
	def sy(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sy = q
			self.setParameterValueDouble("Squeeze-Y",q)
			return self
		else:
			return self._sy

class anamorphic_rescaled_degree_6(builtin,ldp.anamorphic_rescaled_degree_6):
	"""Similar to anamorphic_rescaled_degree_4 but with additional degree-6 parameters."""

	short_names = ["cx02","cy02","cx22","cy22","cx04","cy04","cx24","cy24","cx44","cy44","cx06","cy06","cx26","cy26","cx46","cy46","cx66","cy66","phi_mnt","sx","sy","s_rscl"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.anamorphic_rescaled_degree_6.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
		self._cx02 = self.getParameterDefaultValue("Cx02 - Degree 2")
		self._cy02 = self.getParameterDefaultValue("Cy02 - Degree 2")
		self._cx22 = self.getParameterDefaultValue("Cx22 - Degree 2")
		self._cy22 = self.getParameterDefaultValue("Cy22 - Degree 2")
		self._cx04 = self.getParameterDefaultValue("Cx04 - Degree 4")
		self._cy04 = self.getParameterDefaultValue("Cy04 - Degree 4")
		self._cx24 = self.getParameterDefaultValue("Cx24 - Degree 4")
		self._cy24 = self.getParameterDefaultValue("Cy24 - Degree 4")
		self._cx44 = self.getParameterDefaultValue("Cx44 - Degree 4")
		self._cy44 = self.getParameterDefaultValue("Cy44 - Degree 4")
		self._cx06 = self.getParameterDefaultValue("Cx06 - Degree 6")
		self._cy06 = self.getParameterDefaultValue("Cy06 - Degree 6")
		self._cx26 = self.getParameterDefaultValue("Cx26 - Degree 6")
		self._cy26 = self.getParameterDefaultValue("Cy26 - Degree 6")
		self._cx46 = self.getParameterDefaultValue("Cx46 - Degree 6")
		self._cy46 = self.getParameterDefaultValue("Cy46 - Degree 6")
		self._cx66 = self.getParameterDefaultValue("Cx66 - Degree 6")
		self._cy66 = self.getParameterDefaultValue("Cy66 - Degree 6")
		self._phi_mnt = self.getParameterDefaultValue("Lens Rotation")
		self._sx = self.getParameterDefaultValue("Squeeze-X")
		self._sy = self.getParameterDefaultValue("Squeeze-Y")
		self._s_rscl = self.getParameterDefaultValue("Rescale")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Cx02 - Degree 2":	return self._cx02
		elif	p == "Cy02 - Degree 2":	return self._cy02
		elif	p == "Cx22 - Degree 2":	return self._cx22
		elif	p == "Cy22 - Degree 2":	return self._cy22
		elif	p == "Cx04 - Degree 4":	return self._cx04
		elif	p == "Cy04 - Degree 4":	return self._cy04
		elif	p == "Cx24 - Degree 4":	return self._cx24
		elif	p == "Cy24 - Degree 4":	return self._cy24
		elif	p == "Cx44 - Degree 4":	return self._cx44
		elif	p == "Cy44 - Degree 4":	return self._cy44
		elif	p == "Cx06 - Degree 6":	return self._cx06
		elif	p == "Cy06 - Degree 6":	return self._cy06
		elif	p == "Cx26 - Degree 6":	return self._cx26
		elif	p == "Cy26 - Degree 6":	return self._cy26
		elif	p == "Cx46 - Degree 6":	return self._cx46
		elif	p == "Cy46 - Degree 6":	return self._cy46
		elif	p == "Cx66 - Degree 6":	return self._cx66
		elif	p == "Cy66 - Degree 6":	return self._cy66
		elif	p == "Lens Rotation":	return self._phi_mnt
		elif	p == "Squeeze-X":	return self._sx
		elif	p == "Squeeze-Y":	return self._sy
		elif	p == "Rescale":		return self._s_rscl
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def cx02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx02 = q
			self.setParameterValueDouble("Cx02 - Degree 2",q)
			return self
		else:
			return self._cx02
	def cy02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy02 = q
			self.setParameterValueDouble("Cy02 - Degree 2",q)
			return self
		else:
			return self._cy02
	def cx22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx22 = q
			self.setParameterValueDouble("Cx22 - Degree 2",q)
			return self
		else:
			return self._cx22
	def cy22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy22 = q
			self.setParameterValueDouble("Cy22 - Degree 2",q)
			return self
		else:
			return self._cy22
	def cx04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx04 = q
			self.setParameterValueDouble("Cx04 - Degree 4",q)
			return self
		else:
			return self._cx04
	def cy04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy04 = q
			self.setParameterValueDouble("Cy04 - Degree 4",q)
			return self
		else:
			return self._cy04
	def cx24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx24 = q
			self.setParameterValueDouble("Cx24 - Degree 4",q)
			return self
		else:
			return self._cx24
	def cy24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy24 = q
			self.setParameterValueDouble("Cy24 - Degree 4",q)
			return self
		else:
			return self._cy24
	def cx44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx44 = q
			self.setParameterValueDouble("Cx44 - Degree 4",q)
			return self
		else:
			return self._cx44
	def cy44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy44 = q
			self.setParameterValueDouble("Cy44 - Degree 4",q)
			return self
		else:
			return self._cy44
	def cx06(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx06 = q
			self.setParameterValueDouble("Cx06 - Degree 6",q)
			return self
		else:
			return self._cx06
	def cy06(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy06 = q
			self.setParameterValueDouble("Cy06 - Degree 6",q)
			return self
		else:
			return self._cy06
	def cx26(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx26 = q
			self.setParameterValueDouble("Cx26 - Degree 6",q)
			return self
		else:
			return self._cx26
	def cy26(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy26 = q
			self.setParameterValueDouble("Cy26 - Degree 6",q)
			return self
		else:
			return self._cy26
	def cx46(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx46 = q
			self.setParameterValueDouble("Cx46 - Degree 6",q)
			return self
		else:
			return self._cx46
	def cy46(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy46 = q
			self.setParameterValueDouble("Cy46 - Degree 6",q)
			return self
		else:
			return self._cy46
	def cx66(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx66 = q
			self.setParameterValueDouble("Cx66 - Degree 6",q)
			return self
		else:
			return self._cx66
	def cy66(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy66 = q
			self.setParameterValueDouble("Cy66 - Degree 6",q)
			return self
		else:
			return self._cy66
	def phi_mnt(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._phi_mnt = q
			self.setParameterValueDouble("Lens Rotation",q)
			return self
		else:
			return self._phi_mnt
	def sx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sx = q
			self.setParameterValueDouble("Squeeze-X",q)
			return self
		else:
			return self._sx
	def sy(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sy = q
			self.setParameterValueDouble("Squeeze-Y",q)
			return self
		else:
			return self._sy
	def s_rscl(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_rscl = q
			self.setParameterValueDouble("Rescale",q)
			return self
		else:
			return self._s_rscl

class radial_fisheye_degree_8(builtin,ldp.radial_fisheye_degree_8):
	"""A fisheye model for equisolid angle lenses, degree 8."""

	short_names = ["c2","c4","c6","c8"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_fisheye_degree_8.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("Distortion - Degree 2")
		self._c4 = self.getParameterDefaultValue("Quartic Distortion - Degree 4")
		self._c6 = self.getParameterDefaultValue("C6 - Degree 6")
		self._c8 = self.getParameterDefaultValue("C8 - Degree 8")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Distortion - Degree 2":		return self._c2
		elif	p == "Quartic Distortion - Degree 4":	return self._c4
		elif	p == "C6 - Degree 6":			return self._c6
		elif	p == "C8 - Degree 8":			return self._c8
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("Distortion - Degree 2",q)
			return self
		else:
			return self._c2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("Quartic Distortion - Degree 4",q)
			return self
		else:
			return self._c4
	def c6(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c6 = q
			self.setParameterValueDouble("C6 - Degree 6",q)
			return self
		else:
			return self._c6
	def c8(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c8 = q
			self.setParameterValueDouble("C8 - Degree 8",q)
			return self
		else:
			return self._c8

class radial_standard_degree_4(builtin,ldp.radial_standard_degree_4):
	"""The standard model for radial lenses, including decentering
	and beam splitter correction. The decentering might be overpowered
	with a degree of 4, and the beam splitter correction is getting
	more and more obsolete. We recommend to use the degree-2 parameters
	for decentering and switch to degree-4 only if necessary. Beam splitter
	parameters should not be used for common lens systems."""

	short_names = ["c2","u2","v2","c4","u4","v4","phi_bs","b_bs"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_standard_degree_4.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("Distortion - Degree 2")
		self._u2 = self.getParameterDefaultValue("U - Degree 2")
		self._v2 = self.getParameterDefaultValue("V - Degree 2")
		self._c4 = self.getParameterDefaultValue("Quartic Distortion - Degree 4")
		self._u4 = self.getParameterDefaultValue("U - Degree 4")
		self._v4 = self.getParameterDefaultValue("V - Degree 4")
		self._phi_bs = self.getParameterDefaultValue("Phi - Cylindric Direction")
		self._b_bs = self.getParameterDefaultValue("B - Cylindric Bending")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Distortion - Degree 2":		return self._c2
		elif	p == "U - Degree 2":			return self._u2
		elif	p == "V - Degree 2":			return self._v2
		elif	p == "Quartic Distortion - Degree 4":	return self._c4
		elif	p == "U - Degree 4":			return self._u4
		elif	p == "V - Degree 4":			return self._v4
		elif	p == "Phi - Cylindric Direction":	return self._phi_bs
		elif	p == "B - Cylindric Bending":		return self._b_bs
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("Distortion - Degree 2",q)
			return self
		else:
			return self._c2
	def u2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._u2 = q
			self.setParameterValueDouble("U - Degree 2",q)
			return self
		else:
			return self._u2
	def v2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._v2 = q
			self.setParameterValueDouble("V - Degree 2",q)
			return self
		else:
			return self._v2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("Quartic Distortion - Degree 4",q)
			return self
		else:
			return self._c4
	def u4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._u4 = q
			self.setParameterValueDouble("U - Degree 4",q)
			return self
		else:
			return self._u4
	def v4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._v4 = q
			self.setParameterValueDouble("V - Degree 4",q)
			return self
		else:
			return self._v4
	def phi_bs(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._phi_bs = q
			self.setParameterValueDouble("Phi - Cylindric Direction",q)
			return self
		else:
			return self._phi_bs
	def b_bs(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._b_bs = q
			self.setParameterValueDouble("B - Cylindric Bending",q)
			return self
		else:
			return self._b_bs
			
class radial_stabilizer_degree_6(builtin,ldp.radial_stabilizer_degree_6):
	"""The standard model for radial lenses, including decentering
	and stabilizing. In parameter adjustment, it usually makes sense
	to optimize both stabilizer and decentering, in case the
	camera uses a sensor-shift stabilizer.
	"""
	short_names = ["c2","u2","v2","c4","c6","stbx","stby"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_stabilizer_degree_6.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("Distortion - Degree 2")
		self._u2 = self.getParameterDefaultValue("U - Degree 2")
		self._v2 = self.getParameterDefaultValue("V - Degree 2")
		self._c4 = self.getParameterDefaultValue("Distortion - Degree 4")
		self._c6 = self.getParameterDefaultValue("Distortion - Degree 6")
		self._stbx = self.getParameterDefaultValue("Stabilizer-X")
		self._stby = self.getParameterDefaultValue("Stabilizer-Y")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Distortion - Degree 2":		return self._c2
		elif	p == "U - Degree 2":			return self._u2
		elif	p == "V - Degree 2":			return self._v2
		elif	p == "Distortion - Degree 4":		return self._c4
		elif	p == "Distortion - Degree 6":		return self._c6
		elif	p == "Stabilizer-X":			return self._stbx
		elif	p == "Stabilizer-Y":			return self._stby
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("Distortion - Degree 2",q)
			return self
		else:
			return self._c2
	def u2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._u2 = q
			self.setParameterValueDouble("U - Degree 2",q)
			return self
		else:
			return self._u2
	def v2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._v2 = q
			self.setParameterValueDouble("V - Degree 2",q)
			return self
		else:
			return self._v2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("Distortion - Degree 4",q)
			return self
		else:
			return self._c4
	def c6(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c6 = q
			self.setParameterValueDouble("Distortion - Degree 6",q)
			return self
		else:
			return self._c6
	def stbx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._stbx = q
			self.setParameterValueDouble("Stabilizer-X",q)
			return self
		else:
			return self._stbx
	def stby(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._stby = q
			self.setParameterValueDouble("Stabilizer-Y",q)
			return self
		else:
			return self._stby
			
class classic_ld_model(builtin,ldp.classic_ld_model):
	"""A very old, deprecated degree-4 model with incomplete support
	for anamorphic lenses."""

	short_names = ["c2","s_anam","cx","cy","c4"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.classic_ld_model.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("Distortion")
		self._s_anam = self.getParameterDefaultValue("Anamorphic Squeeze")
		self._cx = self.getParameterDefaultValue("Curvature X")
		self._cy = self.getParameterDefaultValue("Curvature Y")
		self._c4 = self.getParameterDefaultValue("Quartic Distortion")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return True
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Distortion":		return self._c2
		elif	p == "Anamorphic Squeeze":	return self._s_anam
		elif	p == "Curvature X":		return self._cx
		elif	p == "Curvature Y":		return self._cy
		elif	p == "Quartic Distortion":	return self._c4
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("Distortion",q)
			return self
		else:
			return self._c2
	def s_anam(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_anam = q
			self.setParameterValueDouble("Anamorphic Squeeze",q)
			return self
		else:
			return self._s_anam
	def cx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx = q
			self.setParameterValueDouble("Curvature X",q)
			return self
		else:
			return self._cx
	def cy(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy = q
			self.setParameterValueDouble("Curvature Y",q)
			return self
		else:
			return self._cy
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("Quartic Distortion",q)
			return self
		else:
			return self._c4

################################################################
# Experimental                                                 #
################################################################
class radial_fisheye_equidistant_degree_8(builtin,ldp.radial_fisheye_equidistant_degree_8):
	"""An experimental model for equidistant fisheye lenses."""

	short_names = ["c2","c4","c6","c8","s_uni"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_fisheye_equidistant_degree_8.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("C2 - Degree 2")
		self._c4 = self.getParameterDefaultValue("C4 - Degree 4")
		self._c6 = self.getParameterDefaultValue("C6 - Degree 6")
		self._c8 = self.getParameterDefaultValue("C8 - Degree 8")
		self._s_uni = self.getParameterDefaultValue("Scale")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "C2 - Degree 2":	return self._c2
		elif	p == "C4 - Degree 4":	return self._c4
		elif	p == "C6 - Degree 6":	return self._c6
		elif	p == "C8 - Degree 8":	return self._c8
		elif	p == "Scale":		return self._s_uni
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("C2 - Degree 2",q)
			return self
		else:
			return self._c2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("C4 - Degree 4",q)
			return self
		else:
			return self._c4
	def c6(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c6 = q
			self.setParameterValueDouble("C6 - Degree 6",q)
			return self
		else:
			return self._c6
	def c8(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c8 = q
			self.setParameterValueDouble("C8 - Degree 8",q)
			return self
		else:
			return self._c8
	def s_uni(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_uni = q
			self.setParameterValueDouble("Scale",q)
			return self
		else:
			return self._s_uni

class radial_fisheye_equisolid_degree_8(builtin,ldp.radial_fisheye_equisolid_degree_8):
	"""An experimental model for equisolid angle fisheye lenses."""

	short_names = ["c2","c4","c6","c8","s_uni"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_fisheye_equisolid_degree_8.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("C2 - Degree 2")
		self._c4 = self.getParameterDefaultValue("C4 - Degree 4")
		self._c6 = self.getParameterDefaultValue("C6 - Degree 6")
		self._c8 = self.getParameterDefaultValue("C8 - Degree 8")
		self._s_uni = self.getParameterDefaultValue("Scale")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "C2 - Degree 2":	return self._c2
		elif	p == "C4 - Degree 4":	return self._c4
		elif	p == "C6 - Degree 6":	return self._c6
		elif	p == "C8 - Degree 8":	return self._c8
		elif	p == "Scale":		return self._s_uni
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("C2 - Degree 2",q)
			return self
		else:
			return self._c2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("C4 - Degree 4",q)
			return self
		else:
			return self._c4
	def c6(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c6 = q
			self.setParameterValueDouble("C6 - Degree 6",q)
			return self
		else:
			return self._c6
	def c8(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c8 = q
			self.setParameterValueDouble("C8 - Degree 8",q)
			return self
		else:
			return self._c8
	def s_uni(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_uni = q
			self.setParameterValueDouble("Scale",q)
			return self
		else:
			return self._s_uni

class radial_fisheye_orthographic_degree_8(builtin,ldp.radial_fisheye_orthographic_degree_8):
	"""An experimental model for orthographic fisheye lenses."""

	short_names = ["c2","c4","c6","c8","s_uni"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_fisheye_orthographic_degree_8.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("C2 - Degree 2")
		self._c4 = self.getParameterDefaultValue("C4 - Degree 4")
		self._c6 = self.getParameterDefaultValue("C6 - Degree 6")
		self._c8 = self.getParameterDefaultValue("C8 - Degree 8")
		self._s_uni = self.getParameterDefaultValue("Scale")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "C2 - Degree 2":	return self._c2
		elif	p == "C4 - Degree 4":	return self._c4
		elif	p == "C6 - Degree 6":	return self._c6
		elif	p == "C8 - Degree 8":	return self._c8
		elif	p == "Scale":		return self._s_uni
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("C2 - Degree 2",q)
			return self
		else:
			return self._c2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("C4 - Degree 4",q)
			return self
		else:
			return self._c4
	def c6(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c6 = q
			self.setParameterValueDouble("C6 - Degree 6",q)
			return self
		else:
			return self._c6
	def c8(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c8 = q
			self.setParameterValueDouble("C8 - Degree 8",q)
			return self
		else:
			return self._c8
	def s_uni(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_uni = q
			self.setParameterValueDouble("Scale",q)
			return self
		else:
			return self._s_uni

class radial_fisheye_stereographic_degree_8(builtin,ldp.radial_fisheye_stereographic_degree_8):
	"""An experimental model for stereographic fisheye lenses."""

	short_names = ["c2","c4","c6","c8","s_uni"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_fisheye_stereographic_degree_8.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("C2 - Degree 2")
		self._c4 = self.getParameterDefaultValue("C4 - Degree 4")
		self._c6 = self.getParameterDefaultValue("C6 - Degree 6")
		self._c8 = self.getParameterDefaultValue("C8 - Degree 8")
		self._s_uni = self.getParameterDefaultValue("Scale")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "C2 - Degree 2":	return self._c2
		elif	p == "C4 - Degree 4":	return self._c4
		elif	p == "C6 - Degree 6":	return self._c6
		elif	p == "C8 - Degree 8":	return self._c8
		elif	p == "Scale":		return self._s_uni
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("C2 - Degree 2",q)
			return self
		else:
			return self._c2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("C4 - Degree 4",q)
			return self
		else:
			return self._c4
	def c6(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c6 = q
			self.setParameterValueDouble("C6 - Degree 6",q)
			return self
		else:
			return self._c6
	def c8(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c8 = q
			self.setParameterValueDouble("C8 - Degree 8",q)
			return self
		else:
			return self._c8
	def s_uni(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_uni = q
			self.setParameterValueDouble("Scale",q)
			return self
		else:
			return self._s_uni

class anamorphic_stabilizer_degree_6(builtin,ldp.anamorphic_stabilizer_degree_6):
	"""Similar to anamorphic_rescaled_degree_4 but with additional degree-6 parameters and stabilizer."""

	short_names = ["cx02","cy02","cx22","cy22","cx04","cy04","cx24","cy24","cx44","cy44","cx06","cy06","cx26","cy26","cx46","cy46","cx66","cy66","phi_mnt","sx","sy","s_rscl","stbx","stby"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.anamorphic_stabilizer_degree_6.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
		self._cx02 = self.getParameterDefaultValue("Cx02 - Degree 2")
		self._cy02 = self.getParameterDefaultValue("Cy02 - Degree 2")
		self._cx22 = self.getParameterDefaultValue("Cx22 - Degree 2")
		self._cy22 = self.getParameterDefaultValue("Cy22 - Degree 2")
		self._cx04 = self.getParameterDefaultValue("Cx04 - Degree 4")
		self._cy04 = self.getParameterDefaultValue("Cy04 - Degree 4")
		self._cx24 = self.getParameterDefaultValue("Cx24 - Degree 4")
		self._cy24 = self.getParameterDefaultValue("Cy24 - Degree 4")
		self._cx44 = self.getParameterDefaultValue("Cx44 - Degree 4")
		self._cy44 = self.getParameterDefaultValue("Cy44 - Degree 4")
		self._cx06 = self.getParameterDefaultValue("Cx06 - Degree 6")
		self._cy06 = self.getParameterDefaultValue("Cy06 - Degree 6")
		self._cx26 = self.getParameterDefaultValue("Cx26 - Degree 6")
		self._cy26 = self.getParameterDefaultValue("Cy26 - Degree 6")
		self._cx46 = self.getParameterDefaultValue("Cx46 - Degree 6")
		self._cy46 = self.getParameterDefaultValue("Cy46 - Degree 6")
		self._cx66 = self.getParameterDefaultValue("Cx66 - Degree 6")
		self._cy66 = self.getParameterDefaultValue("Cy66 - Degree 6")
		self._phi_mnt = self.getParameterDefaultValue("Lens Rotation")
		self._sx = self.getParameterDefaultValue("Squeeze-X")
		self._sy = self.getParameterDefaultValue("Squeeze-Y")
		self._s_rscl = self.getParameterDefaultValue("Rescale")
		self._stbx = self.getParameterDefaultValue("Stabilizer-X")
		self._stby = self.getParameterDefaultValue("Stabilizer-Y")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return False
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Cx02 - Degree 2":	return self._cx02
		elif	p == "Cy02 - Degree 2":	return self._cy02
		elif	p == "Cx22 - Degree 2":	return self._cx22
		elif	p == "Cy22 - Degree 2":	return self._cy22
		elif	p == "Cx04 - Degree 4":	return self._cx04
		elif	p == "Cy04 - Degree 4":	return self._cy04
		elif	p == "Cx24 - Degree 4":	return self._cx24
		elif	p == "Cy24 - Degree 4":	return self._cy24
		elif	p == "Cx44 - Degree 4":	return self._cx44
		elif	p == "Cy44 - Degree 4":	return self._cy44
		elif	p == "Cx06 - Degree 6":	return self._cx06
		elif	p == "Cy06 - Degree 6":	return self._cy06
		elif	p == "Cx26 - Degree 6":	return self._cx26
		elif	p == "Cy26 - Degree 6":	return self._cy26
		elif	p == "Cx46 - Degree 6":	return self._cx46
		elif	p == "Cy46 - Degree 6":	return self._cy46
		elif	p == "Cx66 - Degree 6":	return self._cx66
		elif	p == "Cy66 - Degree 6":	return self._cy66
		elif	p == "Lens Rotation":	return self._phi_mnt
		elif	p == "Squeeze-X":	return self._sx
		elif	p == "Squeeze-Y":	return self._sy
		elif	p == "Rescale":		return self._s_rscl
		elif	p == "Stabilizer-X":	return self._stbx
		elif	p == "Stabilizer-Y":	return self._stby
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def cx02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx02 = q
			self.setParameterValueDouble("Cx02 - Degree 2",q)
			return self
		else:
			return self._cx02
	def cy02(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy02 = q
			self.setParameterValueDouble("Cy02 - Degree 2",q)
			return self
		else:
			return self._cy02
	def cx22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx22 = q
			self.setParameterValueDouble("Cx22 - Degree 2",q)
			return self
		else:
			return self._cx22
	def cy22(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy22 = q
			self.setParameterValueDouble("Cy22 - Degree 2",q)
			return self
		else:
			return self._cy22
	def cx04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx04 = q
			self.setParameterValueDouble("Cx04 - Degree 4",q)
			return self
		else:
			return self._cx04
	def cy04(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy04 = q
			self.setParameterValueDouble("Cy04 - Degree 4",q)
			return self
		else:
			return self._cy04
	def cx24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx24 = q
			self.setParameterValueDouble("Cx24 - Degree 4",q)
			return self
		else:
			return self._cx24
	def cy24(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy24 = q
			self.setParameterValueDouble("Cy24 - Degree 4",q)
			return self
		else:
			return self._cy24
	def cx44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx44 = q
			self.setParameterValueDouble("Cx44 - Degree 4",q)
			return self
		else:
			return self._cx44
	def cy44(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy44 = q
			self.setParameterValueDouble("Cy44 - Degree 4",q)
			return self
		else:
			return self._cy44
	def cx06(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx06 = q
			self.setParameterValueDouble("Cx06 - Degree 6",q)
			return self
		else:
			return self._cx06
	def cy06(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy06 = q
			self.setParameterValueDouble("Cy06 - Degree 6",q)
			return self
		else:
			return self._cy06
	def cx26(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx26 = q
			self.setParameterValueDouble("Cx26 - Degree 6",q)
			return self
		else:
			return self._cx26
	def cy26(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy26 = q
			self.setParameterValueDouble("Cy26 - Degree 6",q)
			return self
		else:
			return self._cy26
	def cx46(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx46 = q
			self.setParameterValueDouble("Cx46 - Degree 6",q)
			return self
		else:
			return self._cx46
	def cy46(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy46 = q
			self.setParameterValueDouble("Cy46 - Degree 6",q)
			return self
		else:
			return self._cy46
	def cx66(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx66 = q
			self.setParameterValueDouble("Cx66 - Degree 6",q)
			return self
		else:
			return self._cx66
	def cy66(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy66 = q
			self.setParameterValueDouble("Cy66 - Degree 6",q)
			return self
		else:
			return self._cy66
	def phi_mnt(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._phi_mnt = q
			self.setParameterValueDouble("Lens Rotation",q)
			return self
		else:
			return self._phi_mnt
	def sx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sx = q
			self.setParameterValueDouble("Squeeze-X",q)
			return self
		else:
			return self._sx
	def sy(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._sy = q
			self.setParameterValueDouble("Squeeze-Y",q)
			return self
		else:
			return self._sy
	def s_rscl(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._s_rscl = q
			self.setParameterValueDouble("Rescale",q)
			return self
		else:
			return self._s_rscl
	def stbx(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._stbx = q
			self.setParameterValueDouble("Stabilizer-X",q)
			return self
		else:
			return self._stbx
	def stby(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._stby = q
			self.setParameterValueDouble("Stabilizer-Y",q)
			return self
		else:
			return self._stby
################################################################
# Toy model                                                    #
################################################################
class radial_homomorphic_degree_2(builtin,ldp.radial_homomorphic_decentered_degree_2):
	"""An experimental model with focus on low number of parameters.
	When decentering is 0, the model has the following properties:
	1. Any two consecutive operations (applying or removing lens distortion
	by parameters <i>a</i> and <i>b</i> can be replaced by a single operation
	by <i>a</i>+<i>b</i>.
	2. As a consequence, removing distortion by some parameter value <i>a</i>
	is exactly the same as applying distortion by a value &minus;<i>d</i>,
	i.e. for each operation there is an exact inverse operation.
	Mathematically speaking the model function (without decentering)
	is a homomorphism from the real numbers to the set of distortion mappings.
	"""

	short_names = ["c2","u2","v2"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_homomorphic_decentered_degree_2.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("C - Degree 2")
		self._u2 = self.getParameterDefaultValue("U - Degree 2")
		self._v2 = self.getParameterDefaultValue("V - Degree 2")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "C - Degree 2":	return self._c2
		if	p == "U - Degree 2":	return self._u2
		if	p == "V - Degree 2":	return self._v2
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("C - Degree 2",q)
			return self
		else:
			return self._c2
	def u2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._u2 = q
			self.setParameterValueDouble("U - Degree 2",q)
			return self
		else:
			return self._u2
	def v2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._v2 = q
			self.setParameterValueDouble("V - Degree 2",q)
			return self
		else:
			return self._v2

class anamorphic_homomorphic_degree_2(builtin,ldp.anamorphic_homomorphic_degree_2):
	"""Similarly to '3DE4 Radial - Homomorphic, Degree 2', the model function
	of this model represents a homomorphism from the real numbers to the
	set of distortion mappings. Good for nothing, but beautiful."""

	short_names = ["cx2","cy2","b2"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.anamorphic_homomorphic_degree_2.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._cx2 = self.getParameterDefaultValue("Cx2 - Degree 2")
		self._cy2 = self.getParameterDefaultValue("Cy2 - Degree 2")
		self._b2 = self.getParameterDefaultValue("B2 - Degree 2")
	@staticmethod
	def is_external() -> bool:
		return False
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "C, Degree 2":	return self._c2
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def cx2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cx2 = q
			self.setParameterValueDouble("Cx2 - Degree 2",q)
			return self
		else:
			return self._cx2
	def cy2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._cy2 = q
			self.setParameterValueDouble("Cy2 - Degree 2",q)
			return self
		else:
			return self._cy2
	def b2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._b2 = q
			self.setParameterValueDouble("B2 - Degree 2",q)
			return self
		else:
			return self._b2

################################################################
# Original                                                     #
################################################################
class brown_conrady_degree_6_2(builtin,ldp.brown_conrady_degree_6_2):
	"""The original Brown-Conrady model for radial lenses,
	including decentering, radial degree 6, decentering degree 2,
	five parameters.
	"""

	short_names = ["c2","c4","c6","t1","t2"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.brown_conrady_degree_6_2.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._c2 = self.getParameterDefaultValue("C2")
		self._c4 = self.getParameterDefaultValue("C4")
		self._c6 = self.getParameterDefaultValue("C6")
		self._t1 = self.getParameterDefaultValue("T1")
		self._t2 = self.getParameterDefaultValue("T2")
	@staticmethod
	def is_external() -> bool:
		return True
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "C2":	return self._c2
		elif	p == "C4":	return self._c4
		elif	p == "C6":	return self._c6
		elif	p == "T1":	return self._t1
		elif	p == "T2":	return self._t2
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def c2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c2 = q
			self.setParameterValueDouble("C2",q)
			return self
		else:
			return self._c2
	def c4(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c4 = q
			self.setParameterValueDouble("C4",q)
			return self
		else:
			return self._c4
	def c6(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._c6 = q
			self.setParameterValueDouble("C6",q)
			return self
		else:
			return self._c6
	def t1(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._t1 = q
			self.setParameterValueDouble("T1",q)
			return self
		else:
			return self._t1
	def t2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._t2 = q
			self.setParameterValueDouble("T2",q)
			return self
		else:
			return self._t2
			
################################################################
# OpenCV                                                       #
################################################################
class opencv_radial_degree_6_decentered_degree_2(builtin,ldp.radial_degree_6_decentered_degree_2_opencv):

	short_names = ["k1","k2","k3","p1","p2"]
	math_names = [sdv_make_math_name(s) for s in short_names]

	def __init__(self) -> None:
		builtin.__init__(self)
		ldp.radial_degree_6_decentered_degree_2_opencv.__init__(self)
		builtin.init_builtin_parameters(self)
		self.__class__.parameters = [self.getParameterName(i) for i in range(self.getNumParameters())]
		self.__class__.long_names = [sdv_make_identifier_lc(guiname) for guiname in self.__class__.parameters]
# model-specific parameters in mathematical notation
		self._k1 = self.getParameterDefaultValue("Radial Distortion - Degree 2 - k1")
		self._k2 = self.getParameterDefaultValue("Radial Distortion - Degree 4 - k2")
		self._p1 = self.getParameterDefaultValue("Decentering Y - Degree 2 - p1")
		self._p2 = self.getParameterDefaultValue("Decentering X - Degree 2 - p2")
		self._k3 = self.getParameterDefaultValue("Radial Distortion - Degree 6 - k3")
	@staticmethod
	def is_external() -> bool:
		return True
	@staticmethod
	def is_experimental() -> bool:
		return True
	@staticmethod
	def is_deprecated() -> bool:
		return False
# Getter in gui notation
	def getParameterValue(self,p: str) -> Any:
		if	p == "Radial Distortion - Degree 2 - k1":	return self._k1
		elif	p == "Radial Distortion - Degree 4 - k2":	return self._k2
		elif	p == "Decentering Y - Degree 2 - p1":		return self._p1
		elif	p == "Decentering X - Degree 2 - p2":		return self._p2
		elif	p == "Radial Distortion - Degree 6 - k3":	return self._k3
		return builtin.getParameterValue(self,p)
# Setters and Getters in mathematical notation.
	def k1(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._k1 = q
			self.setParameterValueDouble("Radial Distortion - Degree 2 - k1",q)
			return self
		else:
			return self._k1
	def k2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._k2 = q
			self.setParameterValueDouble("Radial Distortion - Degree 4 - k2",q)
			return self
		else:
			return self._k2
	def p1(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._p1 = q
			self.setParameterValueDouble("Decentering Y - Degree 2 - p1",q)
			return self
		else:
			return self._p1
	def p2(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._p2 = q
			self.setParameterValueDouble("Decentering X - Degree 2 - p2",q)
			return self
		else:
			return self._p2
	def k3(self,q: ParameterValue | None = None) -> Self | ParameterValue:
		if q != None:
			self._k3 = q
			self.setParameterValueDouble("Radial Distortion - Degree 6 - k3",q)
			return self
		else:
			return self._k3

################################################################
# For automatic documentation                                  #
################################################################

models_by_gui_name: ModelRegistry = {
	"3DE4 Anamorphic - Standard, Degree 4":anamorphic_standard_degree_4,
	"3DE4 Anamorphic - Rescaled, Degree 4":anamorphic_rescaled_degree_4,
	"3DE4 Anamorphic - Standard, Degree 6":anamorphic_standard_degree_6,
	"3DE4 Anamorphic - Rescaled, Degree 6":anamorphic_rescaled_degree_6,
	"3DE4 Anamorphic - Stabilizer, Degree 6":anamorphic_stabilizer_degree_6,
	"3DE4 Radial - Fisheye, Degree 8":radial_fisheye_degree_8,
	"3DE4 Radial - Standard, Degree 4":radial_standard_degree_4,
	"3DE4 Radial - Stabilizer, Degree 4":radial_stabilizer_degree_6,
# Deprecated
	"3DE Classic LD Model":classic_ld_model,
# Experimental
	"3DE4 Radial - Fisheye, Equidistant, Degree 8":radial_fisheye_equidistant_degree_8,
	"3DE4 Radial - Fisheye, Equisolid, Degree 8":radial_fisheye_equisolid_degree_8,
	"3DE4 Radial - Fisheye, Orthographic, Degree 8":radial_fisheye_orthographic_degree_8,
	"3DE4 Radial - Fisheye, Stereographic, Degree 8":radial_fisheye_stereographic_degree_8,
# Toy model
	"3DE4 Radial - Homomorphic, Degree 2":radial_homomorphic_degree_2,
	"3DE4 Anamorphic - Homomorphic, Degree 2":anamorphic_homomorphic_degree_2,
# Original
	"3DE4 Radial - Brown-Conrady, Degree 6/2":brown_conrady_degree_6_2,
# OpenCV
	"OpenCV Radial Degree 6, Decentered Degree 2":opencv_radial_degree_6_decentered_degree_2,
# Obsolete
#	"3DE4 Anamorphic, Degree 6":anamorphic_degree_6,
	}

list_of_models: ModelList = [
	anamorphic_standard_degree_4,
	anamorphic_rescaled_degree_4,
	anamorphic_standard_degree_6,
	anamorphic_rescaled_degree_6,
	anamorphic_stabilizer_degree_6,
	radial_fisheye_degree_8,
	radial_standard_degree_4,
	radial_stabilizer_degree_6,
# Deprecated
	classic_ld_model,
# Experimental
	radial_fisheye_equidistant_degree_8,
	radial_fisheye_equisolid_degree_8,
	radial_fisheye_orthographic_degree_8,
	radial_fisheye_stereographic_degree_8,
# Toy model
	radial_homomorphic_degree_2,
	anamorphic_homomorphic_degree_2,
# Original
	brown_conrady_degree_6_2,
# Foreign
	opencv_radial_degree_6_decentered_degree_2,
# Obsolete
#	anamorphic_degree_6,
	]

# Too special for general use...
def build_dtd_code_for_math_names() -> None:
	global list_of_models
	s: dict[ParameterName,MathName] = {}
	for ldm in list_of_models:
		for name in ldm.short_names:
			s[name] = sdv_make_math_name(name)
	print(s)
	for e in s:
		print('<!ENTITY ldpar_%s "%s">' % (e,s[e]))

# Too special for general use...
def build_html_code_for_math_names(preline: str = "<tr>",postline: str = "</tr>") -> None:
	global list_of_models
	s: dict[ParameterName,MathName] = {}
	for ldm in list_of_models:
		for name in ldm.short_names:
			s[name] = sdv_make_math_name(name)
	for e in s:
		print(preline + '<td>%s</td><td>&ldpar_%s;</td><td><tde4_ent>ldpar_%s</tde4_ent></td>' % (e,e,e) + postline)

if __name__ == '__main__':
	print("version:",__version__ + ", ldp:",ldp.__file__)
