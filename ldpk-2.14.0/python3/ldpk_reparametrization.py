# This module implements the reparametrization procedure for
# modifying the filmback, as described in the white paper
# "The standard models for lens distortion in 3DE4",
# in $LDPK/doc/tex/standard/tde4_ldm_standard.pdf
#
# version 1.0: initial
#
# todo: The "classic" model (kingdom come)
# todo: reparametrization for flip and flop.
#

import math

class indices:
	pass

class reparametrization:
	def __init__(self,model):
		self._uptodate = False
		self._model = model
		self.i = indices()
		if self._model == "3DE4 Radial - Standard, Degree 4":
			self.i.c2 = 0
			self.i.u2 = 1
			self.i.v2 = 2
			self.i.c4 = 3
			self.i.u4 = 4
			self.i.v4 = 5
			self.i.phi_bs = 6
			self.i.b_bs = 7
		elif self._model == "3DE4 Anamorphic - Standard, Degree 4":
			self.i.cx02 = 0
			self.i.cy02 = 1
			self.i.cx22 = 2
			self.i.cy22 = 3
			self.i.cx04 = 4
			self.i.cy04 = 5
			self.i.cx24 = 6
			self.i.cy24 = 7
			self.i.cx44 = 8
			self.i.cy44 = 9
			self.i.phi_mnt = 10
			self.i.sx = 11
			self.i.sy = 12
		elif self._model == "3DE4 Anamorphic - Rescaled, Degree 4":
			self.i.cx02 = 0
			self.i.cy02 = 1
			self.i.cx22 = 2
			self.i.cy22 = 3
			self.i.cx04 = 4
			self.i.cy04 = 5
			self.i.cx24 = 6
			self.i.cy24 = 7
			self.i.cx44 = 8
			self.i.cy44 = 9
			self.i.phi_mnt = 10
			self.i.sx = 11
			self.i.sy = 12
			self.i.s_rscl = 13
		elif self._model == "3DE4 Anamorphic - Degree 6":
			self.i.cx02 = 0
			self.i.cy02 = 1
			self.i.cx22 = 2
			self.i.cy22 = 3
			self.i.cx04 = 4
			self.i.cy04 = 5
			self.i.cx24 = 6
			self.i.cy24 = 7
			self.i.cx44 = 8
			self.i.cy44 = 9
			self.i.cx06 = 10
			self.i.cy06 = 11
			self.i.cx26 = 12
			self.i.cy26 = 13
			self.i.cx46 = 14
			self.i.cy46 = 15
			self.i.cx66 = 16
			self.i.cy66 = 17
		elif self._model == "3DE4 Radial - Fisheye, Degree 8":
			self.i.c2 = 0
			self.i.c4 = 1
			self.i.c6 = 2
			self.i.c8 = 3

	def r_fb_old_cm(self):
		self.done()
		return self._r_fb_old_cm
	def r_fb_new_cm(self):
		self.done()
		return self._r_fb_new_cm
	def rho(self):
		self.done()
		return self._rho

# parameters is a tuple of numbers, same order as in the distortion model.
# return value is the same kind of tuple, yet with reparametrized values.
	def _reparametrize(self,p):
		self.done()
		rho = self._rho
# All models are hard-coded here; reparametrization is model-dependent.
		if self._model == "3DE4 Radial - Standard, Degree 4":
			q = [0] * 8
			q[self.i.c2]		= p[self.i.c2] * rho ** 2
			q[self.i.u2]		= p[self.i.u2] * rho ** 1
			q[self.i.v2]		= p[self.i.v2] * rho ** 1
			q[self.i.c4]		= p[self.i.c4] * rho ** 4
			q[self.i.u4]		= p[self.i.u4] * rho ** 3
			q[self.i.v4]		= p[self.i.v4] * rho ** 3
			q[self.i.phi_bs]	= p[self.i.phi_bs]
			q[self.i.b_bs]		= p[self.i.b_bs]
		elif self._model == "3DE4 Anamorphic - Standard, Degree 4":
			q = [0] * 13
			q[self.i.cx02]		= p[self.i.cx02] * rho ** 2
			q[self.i.cy02]		= p[self.i.cy02] * rho ** 2
			q[self.i.cx22]		= p[self.i.cx22] * rho ** 2
			q[self.i.cy22]		= p[self.i.cy22] * rho ** 2
			q[self.i.cx04]		= p[self.i.cx04] * rho ** 4
			q[self.i.cy04]		= p[self.i.cy04] * rho ** 4
			q[self.i.cx24]		= p[self.i.cx24] * rho ** 4
			q[self.i.cy24]		= p[self.i.cy24] * rho ** 4
			q[self.i.cx44]		= p[self.i.cx44] * rho ** 4
			q[self.i.cy44]		= p[self.i.cy44] * rho ** 4
			q[self.i.phi_mnt]	= p[self.i.phi_mnt]
			q[self.i.sx]		= p[self.i.sx]
			q[self.i.sy]		= p[self.i.sy]
		elif self._model == "3DE4 Anamorphic - Rescaled, Degree 4":
			q = [0] * 14
			q[self.i.cx02]		= p[self.i.cx02] * rho ** 2
			q[self.i.cy02]		= p[self.i.cy02] * rho ** 2
			q[self.i.cx22]		= p[self.i.cx22] * rho ** 2
			q[self.i.cy22]		= p[self.i.cy22] * rho ** 2
			q[self.i.cx04]		= p[self.i.cx04] * rho ** 4
			q[self.i.cy04]		= p[self.i.cy04] * rho ** 4
			q[self.i.cx24]		= p[self.i.cx24] * rho ** 4
			q[self.i.cy24]		= p[self.i.cy24] * rho ** 4
			q[self.i.cx44]		= p[self.i.cx44] * rho ** 4
			q[self.i.cy44]		= p[self.i.cy44] * rho ** 4
			q[self.i.phi_mnt]	= p[self.i.phi_mnt]
			q[self.i.sx]		= p[self.i.sx]
			q[self.i.sy]		= p[self.i.sy]
			q[self.i.s_rscl]	= p[self.i.s_rscl]
		elif self._model == "3DE4 Anamorphic - Degree 6":
			q = [0] * 18
			q[self.i.cx02]		= p[self.i.cx02] * rho ** 2
			q[self.i.cy02]		= p[self.i.cy02] * rho ** 2
			q[self.i.cx22]		= p[self.i.cx22] * rho ** 2
			q[self.i.cy22]		= p[self.i.cy22] * rho ** 2
			q[self.i.cx04]		= p[self.i.cx04] * rho ** 4
			q[self.i.cy04]		= p[self.i.cy04] * rho ** 4
			q[self.i.cx24]		= p[self.i.cx24] * rho ** 4
			q[self.i.cy24]		= p[self.i.cy24] * rho ** 4
			q[self.i.cx44]		= p[self.i.cx44] * rho ** 4
			q[self.i.cy44]		= p[self.i.cy44] * rho ** 4
			q[self.i.cx06]		= p[self.i.cx06] * rho ** 6
			q[self.i.cy06]		= p[self.i.cy06] * rho ** 6
			q[self.i.cx26]		= p[self.i.cx26] * rho ** 6
			q[self.i.cy26]		= p[self.i.cy26] * rho ** 6
			q[self.i.cx46]		= p[self.i.cx46] * rho ** 6
			q[self.i.cy46]		= p[self.i.cy46] * rho ** 6
			q[self.i.cx66]		= p[self.i.cx66] * rho ** 6
			q[self.i.cy66]		= p[self.i.cy66] * rho ** 6
		elif self._model == "3DE4 Radial - Fisheye, Degree 8":
			q = [0] * 4
			q[self.i.c2]		= p[self.i.c2] * rho ** 2
			q[self.i.c4]		= p[self.i.c4] * rho ** 4
			q[self.i.c6]		= p[self.i.c6] * rho ** 6
			q[self.i.c8]		= p[self.i.c8] * rho ** 8
			pass
		return q

class reparametrization_by_filmback(reparametrization):
# internal
	def done(self):
# process-on-demand
		if self._uptodate == True:
			return
# First of all, we calculate rho.
		self._r_fb_old_cm = math.sqrt(self._w_fb_old_cm ** 2 + self._h_fb_old_cm ** 2) / 2.0
		self._r_fb_new_cm = math.sqrt(self._w_fb_new_cm ** 2 + self._h_fb_new_cm ** 2) / 2.0
		self._rho = self._r_fb_new_cm / self._r_fb_old_cm
# process-on-demand
		self._uptodate = True
# API begins here:
	def __init__(self,model):
		reparametrization.__init__(self,model)
		self._w_fb_old_cm = 1.6
		self._h_fb_old_cm = 0.9
		self._x_lco_old_cm = 0.0
		self._y_lco_old_cm = 0.0
		self._w_fb_new_cm = 1.6
		self._h_fb_new_cm = 0.9
		self._x_lco_new_cm = 0.0
		self._y_lco_new_cm = 0.0
# Input for this class is:
# - old filmback
# - old lens center offset
# - new filmback
# - new lens center offset
	def set_old_filmback_cm(self,w_fb_cm,h_fb_cm):
		self._w_fb_old_cm = w_fb_cm
		self._h_fb_old_cm = h_fb_cm
		self._uptodate = False
	def set_old_lens_center_offset_cm(self,x_lco_cm,y_lco_cm):
		self._x_lco_old_cm = x_lco_cm
		self._y_lco_old_cm = y_lco_cm
		self._uptodate = False
	def set_new_filmback_cm(self,w_fb_cm,h_fb_cm):
		self._w_fb_new_cm = w_fb_cm
		self._h_fb_new_cm = h_fb_cm
		self._uptodate = False
	def set_new_lens_center_offset_cm(self,x_lco_cm,y_lco_cm):
		self._x_lco_new_cm = x_lco_cm
		self._y_lco_new_cm = y_lco_cm
		self._uptodate = False
	def reparametrize(self,parameters_old):
		return self._reparametrize(parameters_old)

class reparametrization_by_fov(reparametrization):
# internal
	def done(self):
		if self._uptodate == True:
			return
		dx_old = self._xb_fov_old_unit - self._xa_fov_old_unit
		dy_old = self._yb_fov_old_unit - self._ya_fov_old_unit
		dx_new = self._xb_fov_new_unit - self._xa_fov_new_unit
		dy_new = self._yb_fov_new_unit - self._ya_fov_new_unit
		self._w_fb_new_cm = self._w_fb_old_cm * dx_new / dx_old
		self._h_fb_new_cm = self._h_fb_old_cm * dy_new / dy_old
# First of all, we calculate rho.
		self._r_fb_old_cm = math.sqrt(self._w_fb_old_cm ** 2 + self._h_fb_old_cm ** 2) / 2.0
		self._r_fb_new_cm = math.sqrt(self._w_fb_new_cm ** 2 + self._h_fb_new_cm ** 2) / 2.0
		self._rho = self._r_fb_new_cm / self._r_fb_old_cm
# New lens center offset, will be correct in case the old lens center offset is correct.
		self._x_lco_new_cm = self._x_lco_old_cm + self._w_fb_old_cm * (1.0/2.0) * (self._xb_fov_old_unit - self._xb_fov_new_unit + self._xa_fov_old_unit - self._xa_fov_new_unit) / dx_old
		self._y_lco_new_cm = self._y_lco_old_cm + self._h_fb_old_cm * (1.0/2.0) * (self._yb_fov_old_unit - self._yb_fov_new_unit + self._ya_fov_old_unit - self._ya_fov_new_unit) / dy_old
		self._uptodate = True
# API begins here:
	def __init__(self,model):
		reparametrization.__init__(self,model)
		self._w_fb_old_cm = 1.6
		self._h_fb_old_cm = 0.9
		self._x_lco_old_cm = 0.0
		self._y_lco_old_cm = 0.0
		self._w_fb_new_cm = 1.6
		self._h_fb_new_cm = 0.9
		self._x_lco_new_cm = 0.0
		self._y_lco_new_cm = 0.0
		self._xa_fov_old_unit = 0
		self._ya_fov_old_unit = 0
		self._xb_fov_old_unit = 1
		self._yb_fov_old_unit = 1
		self._xa_fov_new_unit = 0
		self._ya_fov_new_unit = 0
		self._xb_fov_new_unit = 1
		self._yb_fov_new_unit = 1
# Input for this class is:
# - old filmback
# - old lens center offset
# - old field of view
# - new field of view
#   if you're interested in the new lens center
#   offset, otherwise leave as it is.
	def set_old_filmback_cm(self,w_fb_cm,h_fb_cm):
		self._w_fb_old_cm = w_fb_cm
		self._h_fb_old_cm = h_fb_cm
		self._uptodate = False
	def set_old_lens_center_offset_cm(self,x_lco_cm,y_lco_cm):
		self._x_lco_old_cm = x_lco_cm
		self._y_lco_old_cm = y_lco_cm
		self._uptodate = False
	def set_old_field_of_view(self,xa,ya,xb,yb):
		self._xa_fov_old_unit = xa
		self._ya_fov_old_unit = ya
		self._xb_fov_old_unit = xb
		self._yb_fov_old_unit = yb
		self._uptodate = False
	def set_new_field_of_view(self,xa,ya,xb,yb):
		self._xa_fov_new_unit = xa
		self._ya_fov_new_unit = ya
		self._xb_fov_new_unit = xb
		self._yb_fov_new_unit = yb
		self._uptodate = False
# Modifying the field of view leads to a new lens center offset, so the result is:
# - new lens center offset
# - new filmback
	def get_new_lens_center_offset_cm(self):
		self.done()
		return self._x_lco_new_cm,self._y_lco_new_cm
	def get_new_filmback_cm(self):
		self.done()
		return self._w_fb_new_cm,self._h_fb_new_cm
	def reparametrize(self,parameters_old):
		return self._reparametrize(parameters_old)

