#
# 3DE4.script.name:	Export Distortion as Flame Matchbox Node
#
# 3DE4.script.version:	v1.7.3
#
# 3DE4.script.gui:	Main Window::3DE4::File::Export
#
# 3DE4.script.comment:	Export Lens Distortion Data to Flame
# 3DE4.script.comment:	The script will generate a batch flow graph
# 3DE4.script.comment:	consisting of two resize nodes for adding
# 3DE4.script.comment:	and removing overscan margin, and an undistort
# 3DE4.script.comment:	and a distort node.

# Versions:
# - 1.7.3 [2024-04-03]	Rename script to "Export Distortion as Flame Matchbox Node"
# - 1.7.2 [2023-09-06]	Bugfix in matchbox export: w/h_img_px < w/h_img_overscan_px
# - 1.7.1 [2023-08-30]	purify_name able to handle slash
# - 1.7.0 [2023-08-29]	Approved by the SVA - For all time, always.
# - 1.6.0 [2021-11-19]	Bug fix, Python 2->3, dividing integers
# - 1.5.0 [2021-04-28]	Minor changes
# - 1.4.0 [2021-04-13]	Python 3
# - 1.3.0 [2020-01-23]	textfields for absolute overscan added
# - 1.2.0 [0000-00-00]	gui cosmetics
# - 1.1.0 [0000-00-00]	fixed a bug under windows, strftime formatstring.
# - 1.0.0 [0000-00-00]	initial
# Internal comment: prefix xflld
# Internal comment: Part of 3DE4 installation, keep uptodate!
# Internal comment: Original in $LDPK/3de4/python

# Location: sys_data/py_scripts
# Source: $LDPK/3de4/python
# Maintainer: uwe@sci-d-vis.com
# Scope:	public

#
# DO NOT ADD ANY CUSTOM CODE BEYOND THIS POINT!
#

try:
	requester	= _ExportFlameLensDistortion_requester
except (ValueError,NameError,TypeError):
	requester = tde4.createCustomRequester()
	tde4.addFileWidget(requester,"fl_batch_file","Batch file","*","/tmp/my_batch")
	tde4.setWidgetOffsets(requester,"fl_batch_file",25,10,20,0)
	tde4.setWidgetAttachModes(requester,"fl_batch_file","ATTACH_POSITION","ATTACH_WINDOW","ATTACH_WINDOW","ATTACH_NONE")
	tde4.setWidgetSize(requester,"fl_batch_file",200,20)
	tde4.addTextFieldWidget(requester,"tf_overscan_margin_x","","100")
	tde4.setWidgetOffsets(requester,"tf_overscan_margin_x",0,5,10,0)
	tde4.setWidgetAttachModes(requester,"tf_overscan_margin_x","ATTACH_WIDGET","ATTACH_WIDGET","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"tf_overscan_margin_x",200,20)
	tde4.setWidgetCallbackFunction(requester,"tf_overscan_margin_x","xflld_callback_tf_overscan_margin_x")
	tde4.addTextFieldWidget(requester,"tf_overscan_margin_y","","100")
	tde4.setWidgetOffsets(requester,"tf_overscan_margin_y",0,10,10,0)
	tde4.setWidgetAttachModes(requester,"tf_overscan_margin_y","ATTACH_WIDGET","ATTACH_WINDOW","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"tf_overscan_margin_y",200,20)
	tde4.setWidgetCallbackFunction(requester,"tf_overscan_margin_y","xflld_callback_tf_overscan_margin_y")
	tde4.addTextFieldWidget(requester,"tf_overscan_absolute_x","","")
	tde4.setWidgetOffsets(requester,"tf_overscan_absolute_x",0,0,10,0)
	tde4.setWidgetAttachModes(requester,"tf_overscan_absolute_x","ATTACH_OPPOSITE_WIDGET","ATTACH_OPPOSITE_WIDGET","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"tf_overscan_absolute_x",200,20)
	tde4.setWidgetCallbackFunction(requester,"tf_overscan_absolute_x","xflld_callback_tf_overscan_absolute_x")
	tde4.addTextFieldWidget(requester,"tf_overscan_absolute_y","","")
	tde4.setWidgetOffsets(requester,"tf_overscan_absolute_y",0,0,10,0)
	tde4.setWidgetAttachModes(requester,"tf_overscan_absolute_y","ATTACH_OPPOSITE_WIDGET","ATTACH_OPPOSITE_WIDGET","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"tf_overscan_absolute_y",200,20)
	tde4.setWidgetCallbackFunction(requester,"tf_overscan_absolute_y","xflld_callback_tf_overscan_absolute_y")
	tde4.addLabelWidget(requester,"w005","y","ALIGN_LABEL_LEFT")
	tde4.setWidgetOffsets(requester,"w005",62,0,10,0)
	tde4.setWidgetAttachModes(requester,"w005","ATTACH_POSITION","ATTACH_NONE","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"w005",20,20)
	tde4.addLabelWidget(requester,"w006","x","ALIGN_LABEL_LEFT")
	tde4.setWidgetOffsets(requester,"w006",25,600,10,0)
	tde4.setWidgetAttachModes(requester,"w006","ATTACH_POSITION","ATTACH_NONE","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"w006",20,20)
	tde4.addLabelWidget(requester,"w007","Overscan margin (px)","ALIGN_LABEL_RIGHT")
	tde4.setWidgetOffsets(requester,"w007",10,10,10,0)
	tde4.setWidgetAttachModes(requester,"w007","ATTACH_WINDOW","ATTACH_WIDGET","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"w007",200,20)
	tde4.addLabelWidget(requester,"w010","y","ALIGN_LABEL_LEFT")
	tde4.setWidgetOffsets(requester,"w010",0,0,10,0)
	tde4.setWidgetAttachModes(requester,"w010","ATTACH_OPPOSITE_WIDGET","ATTACH_OPPOSITE_WIDGET","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"w010",200,20)
	tde4.addLabelWidget(requester,"w012","x","ALIGN_LABEL_LEFT")
	tde4.setWidgetOffsets(requester,"w012",0,0,10,0)
	tde4.setWidgetAttachModes(requester,"w012","ATTACH_OPPOSITE_WIDGET","ATTACH_OPPOSITE_WIDGET","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"w012",200,20)
	tde4.addLabelWidget(requester,"w013","Overscan absolute (px)","ALIGN_LABEL_RIGHT")
	tde4.setWidgetOffsets(requester,"w013",0,0,10,0)
	tde4.setWidgetAttachModes(requester,"w013","ATTACH_OPPOSITE_WIDGET","ATTACH_OPPOSITE_WIDGET","ATTACH_WIDGET","ATTACH_NONE")
	tde4.setWidgetSize(requester,"w013",200,20)
	tde4.setWidgetLinks(requester,"fl_batch_file","","","tf_overscan_margin_x","")
	tde4.setWidgetLinks(requester,"tf_overscan_margin_x","w006","w005","fl_batch_file","")
	tde4.setWidgetLinks(requester,"tf_overscan_margin_y","w005","tf_overscan_margin_x","fl_batch_file","")
	tde4.setWidgetLinks(requester,"tf_overscan_absolute_x","tf_overscan_margin_x","tf_overscan_margin_x","tf_overscan_margin_x","")
	tde4.setWidgetLinks(requester,"tf_overscan_absolute_y","tf_overscan_margin_y","tf_overscan_margin_y","tf_overscan_margin_y","")
	tde4.setWidgetLinks(requester,"w005","","tf_overscan_margin_y","fl_batch_file","")
	tde4.setWidgetLinks(requester,"w006","fl_batch_file","","fl_batch_file","")
	tde4.setWidgetLinks(requester,"w007","","w006","fl_batch_file","")
	tde4.setWidgetLinks(requester,"w010","w005","w005","w005","")
	tde4.setWidgetLinks(requester,"w012","w006","w006","w006","")
	tde4.setWidgetLinks(requester,"w013","w007","w007","w007","")
	_ExportFlameLensDistortion_requester = requester

#
# DO NOT ADD ANY CUSTOM CODE UP TO THIS POINT!
#

import re
import os
import sys
import time
import shutil
import string
import calendar
import datetime
import getpass
import math

# these modules are located in 3DE4's file hierarchy.
import vl_sdv as vl

def xflld_int_from_widget(id_req,name_wdg):
	try:
		q = int(tde4.getWidgetValue(id_req,name_wdg))
	except:
		try:
			q = int(float(tde4.getWidgetValue(id_req,name_wdg)))
		except:
			q = 0
	return q

def xflld_callback_tf_overscan_margin_x(a,wdg,c):
	id_req = _ExportFlameLensDistortion_requester
	id_cam = tde4.getCurrentCamera()
	w_img_px = tde4.getCameraImageWidth(id_cam)
	q = xflld_int_from_widget(id_req,"tf_overscan_margin_x")
	tde4.setWidgetValue(id_req,"tf_overscan_absolute_x",str(2 * q + w_img_px))
def xflld_callback_tf_overscan_margin_y(a,wdg,c):
	id_req = _ExportFlameLensDistortion_requester
	id_cam = tde4.getCurrentCamera()
	h_img_px = tde4.getCameraImageHeight(id_cam)
	q = xflld_int_from_widget(id_req,"tf_overscan_margin_y")
	tde4.setWidgetValue(id_req,"tf_overscan_absolute_y",str(2 * q + h_img_px))
def xflld_callback_tf_overscan_absolute_x(a,wdg,c):
	id_req = _ExportFlameLensDistortion_requester
	id_cam = tde4.getCurrentCamera()
	w_img_px = tde4.getCameraImageWidth(id_cam)
	q = xflld_int_from_widget(id_req,"tf_overscan_absolute_x")
# Bugfix Python 2->3: int(math.floor(...)) inserted.
	tde4.setWidgetValue(id_req,"tf_overscan_margin_x",str(int(math.floor((q - w_img_px) / 2))))
def xflld_callback_tf_overscan_absolute_y(a,wdg,c):
	id_req = _ExportFlameLensDistortion_requester
	id_cam = tde4.getCurrentCamera()
	h_img_px = tde4.getCameraImageHeight(id_cam)
	q = xflld_int_from_widget(id_req,"tf_overscan_absolute_y")
# Bugfix Python 2->3: int(math.floor(...)) inserted.
	tde4.setWidgetValue(id_req,"tf_overscan_margin_y",str(int(math.floor((q - h_img_px) / 2))))
def xflld_update_tf_overscan_absolute():
	id_req = _ExportFlameLensDistortion_requester
	id_cam = tde4.getCurrentCamera()
	w_img_px = tde4.getCameraImageWidth(id_cam)
	h_img_px = tde4.getCameraImageHeight(id_cam)
	q = xflld_int_from_widget(id_req,"tf_overscan_margin_x")
	tde4.setWidgetValue(id_req,"tf_overscan_absolute_x",str(2 * q + w_img_px))
	q = xflld_int_from_widget(id_req,"tf_overscan_margin_y")
	tde4.setWidgetValue(id_req,"tf_overscan_absolute_y",str(2 * q + h_img_px))



# A simple xml-xflld_writer without any configuration.
# Does not generate processing instruction.
# For parsing xml-data use ElementTree!
# We'll extend this class as needed.
# API methods return self, so commands can be concatenated jquery-style.
class xflld_writer:
# Caller is responsible for open and close of outfile.
	def __init__(self,outfile):
		self._indent = 0
		self._tab = ""
		self._file = outfile
		self._tagstack = []
# Encode '<', '>' and '&' to entities.
	def encode(self,a):
		return a.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;")
	def build_attr_str(self,attr):
		if attr != None:
			return " " + " ".join(["%s='%s'" % (key,attr[key]) for key in attr])
		else:
			return ""
	def inc(self):
		self._indent += 1
		self._tab = "\t" * self._indent
	def dec(self):
		self._indent -= 1
		self._tab = "\t" * self._indent
# **************************************************************
# * API                                                        *
# **************************************************************
# write open tag. Attributes can be passed as dictionary.
	def begin(self,tag,attr = None):
		self._file.write(self._tab + "<%s%s>\n" % (tag,self.build_attr_str(attr)))
		self._tagstack.append(tag)
		self.inc()
		return self
# write close tag. The second argument is optional. It can be passed to increase
# clarity in implementation. If it is passed it will be checked against opening tag.
	def end(self,tag = None):
		if tag != None:
			if tag != self._tagstack[-1]:
				print("xml_writer::warning: closing tag </%s> for element opened by <%s>." % (tag,self._tagstack[-1]))
		self.dec()
		self._file.write(self._tab + "</%s>\n" % self._tagstack[-1])
		self._tagstack.pop()
		return self
# insert 'void' element, like <br/> or <meta .../>
	def void(self,tag,attr = None):
		self._file.write(self._tab + "<%s%s/>\n" % (tag,self.build_attr_str(attr)))
		return self
# write element with content in one single line
	def inline(self,tag,text,attr = None):
		self._file.write(self._tab + "<%s%s>%s</%s>\n" % (tag,self.build_attr_str(attr),self.encode(str(text)),tag))
		return self
	def comment_noindent(self,text):
		self._file.write("<!-- %s -->\n" % (self.encode(str(text))))
		return self
# close file, convenient, if you construct like w = xflld_writer(open(...,"w")).
	def close(self):
		self._file.close()


################################################################
# Exceptions                                                   #
################################################################
class xflld_error_path_exists(BaseException):
	pass
class xflld_error_bad_fingerprint(BaseException):
	pass
class xflld_reenter_batch_file(BaseException):
	pass
class xflld_failed(BaseException):
	pass

################################################################
# Supplementary classes                                        #
################################################################
# This represents the Chebyshev coefficients for a single frame.
# We have 28 parameters (per frame) because we approximate by
# means of a bi-variate degree-6 Chebyshev polynomial.
class xflld_chebyshev_coefficient_set:
# Default is the identity polynomial, i.e. (x,y) mapsto (x,y).
	def __init__(self):
		self._c = [[0,0]] * 28
		self._c[1] = [1,0]
		self._c[2] = [0,1]
	def __getitem__(self,i):
		return self._c[i]
	def __setitem__(self,i,val):
		self._c[i] = val

# This one is currently pretty simple. In Flame's preset parameters
# are simply listed one by one. Parameter names don't seem to be important.
class xflld_model_parameter_set:
	def __init__(self,n):
		self._p = [0] * n
		self._num_parameters = n
	def __getitem__(self,i):
		return self._p[i]
	def __setitem__(self,i,val):
		self._p[i] = val
	def __len__(self):
		return self._num_parameters

################################################################
# Main class: access 3DE4 database and generate batch          #
################################################################
class xflld_accessor:
# pass 3de4 camera here
	def __init__(self,id_cam,tde4_flame_path,target_directory,batch_name,overscan_margin_x,overscan_margin_y):
		self._id_cam = id_cam
		self._id_lens = tde4.getCameraLens(self._id_cam)
		self._id_model = tde4.getLensLDModel(self._id_lens)

		self._is_lco_dynamic = False
		self._is_fl_dynamic = (tde4.getCameraFocalLengthMode(self._id_cam) == "FOCAL_DYNAMIC")
		self._is_fd_dynamic = (tde4.getCameraFocusMode(self._id_cam) == "FOCUS_DYNAMIC")
		self._is_ld_dynamic = (tde4.getLensDynamicDistortionMode(self._id_lens) != "DISTORTION_STATIC")

		self._tde4_flame_path = tde4_flame_path
		self._target_directory = target_directory
		self._batch_name = batch_name
		self._batch_path = os.path.join(self._target_directory,self._batch_name)

		self._user = getpass.getuser()

		self._modelpar_index = None
		self._chebyshev_index = None

# As of version 1.7 this is supposed to work for plugins as well,
# so we have to remove the plugin marker from the name
		self._ldm_name = "LD" + self.purify_name(self._id_model).replace("_[Plugin]","")
		self._ldm_name_glsl = self._ldm_name + ".glsl"
		self._ldm_name_xml = self._ldm_name + ".xml"

# proxycode attach this to 3de4's database
		start,end,step = tde4.getCameraSequenceAttr(self._id_cam)
		self._start_frame = start
		self._end_frame = end
		self._num_frames = end - start + 1
# proxycode
		self._num_parameters = tde4.getLDModelNoParameters(self._id_model)
# Images size in px in 3DE4.
		self._w_img_px = tde4.getCameraImageWidth(self._id_cam)
		self._h_img_px = tde4.getCameraImageHeight(self._id_cam)
# Field of View from 3DE4.
		xa,xb,ya,yb = tde4.getCameraFOV(self._id_cam)
		self._a_fov_unit = vl.vec2d(xa,ya)
		self._b_fov_unit = vl.vec2d(xb,yb)
# Overscan horizontal/vertical (symmetric) as specified by user, in px.
# A value of 100 means 100 pixels are added on the left/bottom AND on the right/top.
		self._w_overscan_px = overscan_margin_x
		self._h_overscan_px = overscan_margin_y
# Image area including overscan margin
		self._w_img_overscan_px = self._w_img_px + 2 * self._w_overscan_px
		self._h_img_overscan_px = self._h_img_px + 2 * self._h_overscan_px
# The image area can be described by a field-of-view with respect to the overscan area.
		self._a_fov_overscan_unit = vl.vec2d(float(self._w_overscan_px) / float(self._w_img_overscan_px),float(self._h_overscan_px) / float(self._h_img_overscan_px))
		self._b_fov_overscan_unit = vl.vec2d(float(self._w_overscan_px + self._w_img_px) / float(self._w_img_overscan_px),float(self._h_overscan_px + self._h_img_px) / float(self._h_img_overscan_px))
# Each field-of-view box represents an affine transform (x,y) -> a + (b - a) * (x,y).
# We now need the concatenation of the two affine transforms given by overscan and
# 3DE4's fov. The result is an affine transform as well and represents our total fov.
		d_fov_overscan_unit = self._b_fov_overscan_unit - self._a_fov_overscan_unit
		self._a_fov_total_unit = vl.had(d_fov_overscan_unit,self._a_fov_unit) + self._a_fov_overscan_unit
		self._b_fov_total_unit = vl.had(d_fov_overscan_unit,self._b_fov_unit) + self._a_fov_overscan_unit
# Lens Center Offset
		if self.is_lco_dynamic():
			self._x_lco_cm = [0.0] * self._num_frames
			self._y_lco_cm = [0.0] * self._num_frames
			for i in range(self._num_frames):
				frame = i + 1
# proxy code until lco is dynamic (kingdom come).
				self._x_lco_cm[i] = tde4.getLensLensCenterX()
				self._y_lco_cm[i] = tde4.getLensLensCenterY()
		else:
			self._x_lco_cm = tde4.getLensLensCenterX(self._id_lens)
			self._y_lco_cm = tde4.getLensLensCenterY(self._id_lens)
# Focal length
		if self.is_fl_dynamic():
			self._fl_cm = [3.0] * self._num_frames
			for i in range(self._num_frames):
				frame = i + 1
				self._fl_cm[i] = tde4.getCameraFocalLength(self._id_cam,frame)
		else:
			self._fl_cm = tde4.getCameraFocalLength(self._id_cam,1)
# Focus distance
		if self.is_fd_dynamic():
			self._fd_cm = [100.0] * self._num_frames
			for i in range(self._num_frames):
				frame = i + 1
				self._fd_cm[i] = tde4.getCameraFocus(self._id_cam,frame)
		else:
			self._fd_cm = tde4.getCameraFocus(self._id_cam,1)

# Depending on dynamic or not we have an array or a single instance for the model parameter set.
		if self.is_ld_dynamic():
			self._modelpar = [None] * self._num_frames
# Iterate over frames
			for i in range(self._num_frames):
# 3DE4 frames start at 1.
				frame = i + 1
# Create object for storing per-frame-data
				self._modelpar[i] = xflld_model_parameter_set(self._num_parameters)
# Get focal length and focus distance for this frame
				fl_cm = tde4.getCameraFocalLength(self._id_cam,frame)
				fd_cm = tde4.getCameraFocus(self._id_cam,frame)
# Iterate over parameters
				for i_par in range(self._num_parameters):
# Parameter name and type
					par_name = self.get_parameter_name(i_par)
					par_type = tde4.getLDModelParameterType(self._id_model,par_name)
# Get parameter value for this frame.
					self._modelpar[i][i_par] = tde4.getLensLDAdjustableParameter(self._id_lens,par_name,fl_cm,fd_cm)
		else:
			self._modelpar = xflld_model_parameter_set(self._num_parameters)
# Iterate over parameters
			for i_par in range(self._num_parameters):
# Parameter name and type
				par_name = self.get_parameter_name(i_par)
				par_type = tde4.getLDModelParameterType(self._id_model,par_name)
# Get parameter value
				self._modelpar[i_par] = tde4.getLensLDAdjustableParameter(self._id_lens,par_name,1.0,1.0)
# depending on dynamic or not we have an array or a single instance for the chebyshev coefficient set.
		if self.is_ld_dynamic():
			self._chebyshev = [None] * self._num_frames
			for i in range(self._num_frames):
				frame = i + 1
				self._chebyshev[i] = xflld_chebyshev_coefficient_set()
				points = []
				a_fov = self._a_fov_total_unit
				d_fov = self._b_fov_total_unit - self._a_fov_total_unit
# We generate a grid of point pairs (undistorted / distorted) in symmetric unit coordinates
# and feed them into the polynomial approximator. 11x11 should be enough
				for iy in range(11):
# x_- and y_dist_shader run from 0 to 1.
					y_dist_shader = iy / 10.0
					for ix in range(11):
						x_dist_shader = ix / 10.0
# The Clenshaw evaluator in our shaders feeds image unit coordinates into the polynomial
# and interprets the result the same way. That means we have to build our point pairs
# like that. We weave in the total fov.
						x_dist = (x_dist_shader - a_fov[0]) / d_fov[0]
						y_dist = (y_dist_shader - a_fov[1]) / d_fov[1]
						x_undist,y_undist = tde4.removeDistortion2D(self._id_cam,frame,[x_dist,y_dist])
						x_undist_shader = x_undist * d_fov[0] + a_fov[0]
						y_undist_shader = y_undist * d_fov[1] + a_fov[1]

#						print x_dist,y_dist," -> ",x_undist,y_undist

# We are interested in a polynomial which map undistorted points to distorted points.
# This polynomial will provide our shader with initial values for the iterative function inversion.
						points.append([2.0 * x_undist - 1.0,2.0 * y_undist - 1.0])
						points.append([2.0 * x_dist - 1.0,2.0 * y_dist - 1.0])
# This is not nice. For non-gnomonic cameras we cannot calculate the Chebyshevs, since the python API
# won't let us, so we use the identity polynomial and hope the best.
				if self._id_model != "3DE4 Radial - Fisheye, Degree 8":
					coeff = tde4.approximateByPolynomial2(6,"CHEBYSHEV1",points)
					for j in range(len(coeff)):
						self._chebyshev[i][j] = coeff[j]
				else:
					if i == 1:
						print("Warning, no initial values calculated for model %s." % self._id_model)
		else:
			self._chebyshev = xflld_chebyshev_coefficient_set()
			points = []
			a_fov = self._a_fov_total_unit
			d_fov = self._b_fov_total_unit - self._a_fov_total_unit
			for iy in range(11):
# x_- and y_dist_shader run from 0 to 1.
				y_dist_shader = iy / 10.0
				for ix in range(11):
					x_dist_shader = ix / 10.0
# The Clenshaw evaluator in our shaders feeds image unit coordinates into the polynomial
# and interprets the result the same way. That means we have to build our point pairs
# like that. We weave in the total fov.
					x_dist = (x_dist_shader - a_fov[0]) / d_fov[0]
					y_dist = (y_dist_shader - a_fov[1]) / d_fov[1]
					x_undist,y_undist = tde4.removeDistortion2D(self._id_cam,1,[x_dist,y_dist])
					x_undist_shader = x_undist * d_fov[0] + a_fov[0]
					y_undist_shader = y_undist * d_fov[1] + a_fov[1]

#					print x_dist,y_dist," -> ",x_undist,y_undist

# We are interested in a polynomial which map undistorted points to distorted points.
# This polynomial will provide our shader with initial values for the iterative function inversion.
					points.append([2.0 * x_undist - 1.0,2.0 * y_undist - 1.0])
					points.append([2.0 * x_dist - 1.0,2.0 * y_dist - 1.0])
			coeff = tde4.approximateByPolynomial2(6,"CHEBYSHEV1",points)
			for j in range(len(coeff)):
				self._chebyshev[j] = coeff[j]


# Convert a human readable name into a reasonable file system compatible name.
	@staticmethod
	def purify_name(s):
		if s == "":
			return "_"
		if s[0] in "0123456789":
			t = "_"
		else:
			t = ""
		t += " ".join(re.sub("[/+,:; _-]+","_",s.strip()).split())
		return t
# properties
	def start_frame(self):
		return self._start_frame
	def end_frame(self):
		return self._end_frame
	def num_frames(self):
		return self._num_frames
	def num_parameters(self):
		return self._num_parameters
	def frame(self,index):
		return index + 1
	def is_fl_dynamic(self):
		return self._is_fl_dynamic
	def is_fd_dynamic(self):
		return self._is_fd_dynamic
	def is_ld_dynamic(self):
		return self._is_ld_dynamic
# future use
	def is_lco_dynamic(self):
		return self._is_lco_dynamic
# field-of-view
	def a_fov_total_unit(self):
		return self._a_fov_total_unit
	def b_fov_total_unit(self):
		return self._b_fov_total_unit
# controlling the proxies behaviour
	def set_modelpar_index(self,i):
		self._modelpar_index = i
	def set_chebyshev_index(self,i):
		self._chebyshev_index = i
	def get_parameter_name(self,i):
# proxycode
		return tde4.getLDModelParameterName(self._id_model,i)
	def get_func(self,name):
		if name == "x_lco_cm":
			if self.is_lco_dynamic():
				return lambda index: self._x_lco_cm[index]
			else:
				return lambda: self._x_lco_cm
		elif name == "y_lco_cm":
			if self.is_lco_dynamic():
				return lambda index: self._y_lco_cm[index]
			else:
				return lambda: self._y_lco_cm
		elif name == "tde4_focal_length_cm":
			if self.is_fl_dynamic():
				return lambda index: self._fl_cm[index]
			else:
				return lambda: self._fl_cm
		elif name == "tde4_focus_distance_cm":
			if self.is_fd_dynamic():
				return lambda index: self._fd_cm[index]
			else:
				return lambda: self._fd_cm
		elif name == "chebyshev":
			if self.is_ld_dynamic():
				return lambda index: self._chebyshev[index][self._chebyshev_index]
			else:
				return lambda: self._chebyshev[self._chebyshev_index]
		elif name == "modelpar":
			if self.is_ld_dynamic():
				return lambda index: self._modelpar[index][self._modelpar_index]
			else:
				return lambda: self._modelpar[self._modelpar_index]
		print("get_func: unreachable,",name)
		return lambda frame:3.0
# built-ins in math notation
	def w_img_px(self):
		return self._w_img_px
	def h_img_px(self):
		return self._h_img_px
# Non-dynamic camera data: filmback and pixel aspect
	def w_fb_cm(self):
		return tde4.getLensFBackWidth(self._id_lens)
	def h_fb_cm(self):
		return tde4.getLensFBackHeight(self._id_lens)
	def r_pa(self):
		return tde4.getLensPixelAspect(self._id_lens)

	def insert_single_static_float_parameter(self,w,name,value):
		w.begin("Parameter")
		w.begin("Data")
		w.begin("Channel",{"Name":name})
		w.inline("Extrap","constant")
		w.inline("Value",value)
		w.void("Uncollapsed")
		w.end()
		w.end()
		w.end()

	def insert_single_static_int_parameter(self,w,name,value):
		w.begin("Parameter")
		w.begin("Data")
		w.begin("Channel",{"Name":name})
		w.inline("Extrap","constant")
		w.inline("Value",value)
		w.void("Uncollapsed")
		w.end()
		w.end()
		w.end()

	def insert_single_static_menu_parameter(self,w,name,value):
		w.begin("Parameter")
		w.inline("value",value)
		w.end()

	def insert_xy_static_float_parameter(self,w,name,xy):
		w.begin("Parameter")
		w.begin("X")
		w.begin("Channel",{"Name":"X"})
		w.inline("Extrap","constant")
		w.inline("Value",xy[0])
		w.void("Uncollapsed")
		w.end()
		w.end()
		w.begin("Y")
		w.begin("Channel",{"Name":"Y"})
		w.inline("Extrap","constant")
		w.inline("Value",xy[1])
		w.void("Uncollapsed")
		w.end()
		w.end()
		w.inline("Icon","False")
		w.inline("Action3D","False")
		w.end()

	def insert_single_dynamic_float_parameter(self,w,name,func = None):
		if func == None:
			func = self.get_func(name)

		w.begin("Parameter")
		w.begin("Data")
		w.begin("Channel",{"Name":name})
		w.inline("Extrap","constant")
# unclear what it's good for
		w.inline("Value",0.0)
# Number of keyframes
		w.inline("Size",self.num_frames())
		w.inline("KeyVersion",2)
# Keyframes
		w.begin("KFrames")
		for index in range(self.num_frames()):
			w.begin("Key",{"Index":index})
			w.inline("Frame",self.frame(index))
			w.inline("Value",func(index))
			w.inline("RHandle_dX",0.25)
			w.inline("RHandle_dY",0.0)
			w.inline("LHandle_dX",0.25)
			w.inline("LHandle_dY",0.0)
			w.inline("CurveMode","hermite")
			w.inline("CurveOrder","linear")
			w.end()
		w.end()
		w.void("Uncollapsed")
		w.end()
		w.end()
		w.end()

	def insert_xy_dynamic_float_parameter(self,w,name,func = None):
		if func == None:
			func = self.get_func(name)

		w.begin("Parameter")
# X
		w.begin("X")
		w.begin("Channel",{"Name":"X"})
		w.inline("Extrap","constant")
# Number of keyframes
		w.inline("Size",self.num_frames())
		w.inline("KeyVersion",2)
# Keyframes
		w.begin("KFrames")
		for index in range(self.num_frames()):
			w.begin("Key",{"Index":index})
			w.inline("Frame",self.frame(index))
			w.inline("Value",func(index)[0])
			w.inline("RHandle_dX",0.25)
			w.inline("RHandle_dY",0.0)
			w.inline("LHandle_dX",0.25)
			w.inline("LHandle_dY",0.0)
			w.inline("CurveMode","hermite")
			w.inline("CurveOrder","linear")
			w.end()
		w.end()
		w.void("Uncollapsed")
		w.end()
		w.end()
# Y
		w.begin("Y")
		w.begin("Channel",{"Name":"Y"})
		w.inline("Extrap","constant")
# Number of keyframes
		w.inline("Size",self.num_frames())
		w.inline("KeyVersion",2)
# Keyframes
		w.begin("KFrames")
		for index in range(self.num_frames()):
			w.begin("Key",{"Index":index})
			w.inline("Frame",self.frame(index))
			w.inline("Value",func(index)[1])
			w.inline("RHandle_dX",0.25)
			w.inline("RHandle_dY",0.0)
			w.inline("LHandle_dX",0.25)
			w.inline("LHandle_dY",0.0)
			w.inline("CurveMode","hermite")
			w.inline("CurveOrder","linear")
			w.end()
		w.end()
		w.void("Uncollapsed")
		w.end()
		w.end()
		w.inline("Icon","False")
		w.inline("Action3D","False")
		w.end()

# invoked in the (empty) target directory.
	def create_matchbox_node(self,out_name,direction):
#		w = xmlw.xflld_writer(open("/dev/tty","w"))
		w = xflld_writer(open(out_name + ".matchbox_node","w"))
		w.begin("Setup")
# Base
		w.begin("Base")
		w.inline("Version",17)
		w.inline("NAME",self._ldm_name)
		w.void("Note")
		w.inline("Expanded","False")
		w.inline("ScrollBar","0")
		w.inline("Frames","0")
		w.inline("Current_Time","1")
		w.inline("Input_DataType","4")
		w.inline("ClampMode",0)
		w.inline("AdapDegrad","False")
		w.inline("ReadOnly","False")
# Eventuell Span weglassen, ist disable'd.
		w.void("Range",{"Before":2,"After":2,"Start":self._start_frame,"End":self._end_frame,"SpanEnable":"False","Span":self._num_frames})
		w.inline("NoMediaHandling","1")
		w.inline("UsedAsTransition","False")
		w.void("FrameBounds",{"W":self._w_img_overscan_px,"H":self._h_img_overscan_px,"X":0,"Y":0,"SX":self._w_img_overscan_px / 120.0,"SY":self._h_img_overscan_px / 120.0})
		w.end()
# State
		w.begin("State")
		w.inline("NbPasses",1)
		w.inline("Regen","True")
		w.inline("OutputResMode",1)
# - Output Resolution
		w.begin("OutputResolution")
		w.inline("Width",self._w_img_overscan_px)
		w.inline("Height",self._h_img_overscan_px)
		w.inline("Format",124)
		w.inline("ScanFormat",2)
		w.inline("AspectRatio",float(self._w_img_overscan_px) / float(self._h_img_overscan_px))
		w.inline("ColourSpace","Unknown")
		w.end()
# - Top Channel
		w.begin("TopChannel")
		w.begin("Channel",{"Name":self._ldm_name})
		w.void("Uncollapsed")
		w.end()
		w.end()
# - Shader Parameters
		w.begin("ShaderParameters")

# The seven 3de4-built-in parameters.
		w.comment_noindent("Built-in parameters")
		self.insert_single_static_float_parameter(w,"tde4_filmback_width_cm",self.w_fb_cm())
		self.insert_single_static_float_parameter(w,"tde4_filmback_height_cm",self.h_fb_cm())
		self.insert_single_static_float_parameter(w,"tde4_lens_center_offset_x_cm",self.get_func("x_lco_cm")())
		self.insert_single_static_float_parameter(w,"tde4_lens_center_offset_y_cm",self.get_func("y_lco_cm")())
		self.insert_single_static_float_parameter(w,"tde4_pixel_aspect",self.r_pa())

		if self.is_fl_dynamic():
			self.insert_single_dynamic_float_parameter(w,"tde4_focal_length_cm")
		else:
			self.insert_single_static_float_parameter(w,"tde4_focal_length_cm",self.get_func("tde4_focal_length_cm")())
		if self.is_fd_dynamic():
			self.insert_single_dynamic_float_parameter(w,"tde4_focus_distance_cm")
		else:
			self.insert_single_static_float_parameter(w,"tde4_focus_distance_cm",self.get_func("tde4_focus_distance_cm")())
# Fov. We will construct our matchbox shader thus that input and output image size are the same.
# So, input and output fov should be the same as well. Both are the "product" of the embedding mapping
# due to resize for overscan and the fov coming from the 3de4 project.
		w.comment_noindent("Field of View")
		self.insert_xy_static_float_parameter(w,"a_fov_in_unit",self.a_fov_total_unit())
		self.insert_xy_static_float_parameter(w,"b_fov_in_unit",self.b_fov_total_unit())
		self.insert_xy_static_float_parameter(w,"a_fov_out_unit",self.a_fov_total_unit())
		self.insert_xy_static_float_parameter(w,"b_fov_out_unit",self.b_fov_total_unit())
# Flow control
		w.comment_noindent("Flow control")
		w.comment_noindent("Direction: distort, undistort, chebyshev, identity")
		self.insert_single_static_menu_parameter(w,"direction",direction)
		w.comment_noindent("Render Mode: image, stmap, jacobi, jacobi diffquot, twist vector (nyi), twist vector diffquot")
		self.insert_single_static_menu_parameter(w,"render_mode",0)
# By default, we'd like to use the bi-cubic filter.
		w.comment_noindent("Reconstruction filter: bilinear, bicubic")
		self.insert_single_static_menu_parameter(w,"reconstruction_filter",2)
# Our shaders expect that this mode is provided. In Flame we always use
# a polynomial for approximating the distort-function (i.e. initial_value_mode 1).
		w.comment_noindent("Initial value mode: image, polynomial")
		self.insert_single_static_menu_parameter(w,"initial_value_mode",1)
# Input image. There was a bug here!
		self.insert_single_static_int_parameter(w,"w_in_px",self._w_img_overscan_px)
		self.insert_single_static_int_parameter(w,"h_in_px",self._h_img_overscan_px)
# Model dependent parameters. For single component entries, the Name attribute in <Channel> really contains the parameter name.
		w.comment_noindent("Model dependent parameters")
		if self.is_ld_dynamic():
			for i in range(self.num_parameters()):
				self.set_modelpar_index(i)
				self.insert_single_dynamic_float_parameter(w,self.get_parameter_name(i),self.get_func("modelpar"))
		else:
			for i in range(self.num_parameters()):
				self.set_modelpar_index(i)
				self.insert_single_static_float_parameter(w,self.get_parameter_name(i),self.get_func("modelpar")())
# Chebishev coefficients
		w.comment_noindent("Chebyshev coefficients")
		if self.is_ld_dynamic():
			for i in range(28):
				self.set_chebyshev_index(i)
				self.insert_xy_dynamic_float_parameter(w,"chebyshev",self.get_func("chebyshev"))
		else:
			for i in range(28):
				self.set_chebyshev_index(i)
				self.insert_xy_static_float_parameter(w,"chebyshev",self.get_func("chebyshev")())

		w.end()

		w.inline("InputPage",0)
		w.inline("RenderContext",0)
		w.begin("InputRewiring")
		w.inline("InputIndex",0,{"SamplerIndex":0})
		w.inline("InputIndex",1,{"SamplerIndex":1})
		w.inline("InputIndex",2,{"SamplerIndex":2})
		w.end()
		w.inline("ShaderPresetIndex",0)
		w.end()
		w.end()
		w.close()
		pass

# load the template, expand placeholders and write.
	def create_resize_node_add_margin(self):
		path = os.path.join(self._tde4_flame_path,"margin.resize_node.template.xml")
		fin = open(path,"r")
		content = fin.read()
		content = content.replace("$BASE_WIDTH",str(self._w_img_px))
		content = content.replace("$BASE_HEIGHT",str(self._h_img_px))
		content = content.replace("$BASE_SX",str(self._w_img_px / 120.0))
		content = content.replace("$BASE_SY",str(self._h_img_px / 120.0))
		content = content.replace("$SAVED_WIDTH",str(self._w_img_px))
		content = content.replace("$SAVED_HEIGHT",str(self._h_img_px))
		content = content.replace("$DESTINATION_WIDTH",str(self._w_img_overscan_px))
		content = content.replace("$DESTINATION_HEIGHT",str(self._h_img_overscan_px))
		content = content.replace("$DESTINATION_ASPECT",str(float(self._w_img_overscan_px) / float(self._h_img_overscan_px)))
		fout = open("add_margin.resize_node","w")
		fout.write(content)
# load the template, expand placeholders and write.
	def create_resize_node_remove_margin(self):
		path = os.path.join(self._tde4_flame_path,"margin.resize_node.template.xml")
		fin = open(path,"r")
		content = fin.read()
		content = content.replace("$BASE_WIDTH",str(self._w_img_overscan_px))
		content = content.replace("$BASE_HEIGHT",str(self._h_img_overscan_px))
		content = content.replace("$BASE_SX",str(self._w_img_overscan_px / 120.0))
		content = content.replace("$BASE_SY",str(self._h_img_overscan_px / 120.0))
		content = content.replace("$SAVED_WIDTH",str(self._w_img_overscan_px))
		content = content.replace("$SAVED_HEIGHT",str(self._h_img_overscan_px))
		content = content.replace("$DESTINATION_WIDTH",str(self._w_img_px))
		content = content.replace("$DESTINATION_HEIGHT",str(self._h_img_px))
		content = content.replace("$DESTINATION_ASPECT",str(float(self._w_img_px) / float(self._h_img_px)))
		fout = open("remove_margin.resize_node","w")
		fout.write(content)
# load the template, expand placeholders and write.
	def create_root_node(self):
		path = os.path.join(self._tde4_flame_path,"root.root_node.template.xml")
		fin = open(path,"r")
		content = fin.read()
		content = content.replace("$ROOT_WIDTH",str(self._w_img_px))
		content = content.replace("$ROOT_HEIGHT",str(self._h_img_px))
		content = content.replace("$ROOT_SX",str(self._w_img_px / 120.0))
		content = content.replace("$ROOT_SY",str(self._h_img_px / 120.0))
		fout = open("root.root_node","w")
		fout.write(content)
# load the template, expand placeholders and write.
	def create_pipeline(self):
# timestamp
		t = calendar.timegm(time.gmtime())
# This only works under unix.
#		t_var = datetime.datetime.now().strftime('%a %b %d %H:%M:%S %G')
# This works under unix and windows.
		t_var = datetime.datetime.now().strftime('%a %b %d %H:%M:%S') + " " + str(time.gmtime().tm_year)

		fin = open(os.path.join(self._tde4_flame_path,"pipeline.batch.template.xml"),"r")
		content = fin.read()
		content = content.replace("$HEADER_CREATION_DATE",str(t_var))
		content = content.replace("$ROOT_BATCH_LENGTH",str(self._num_frames))
		content = content.replace("$ROOT_USER",self._user)
		content = content.replace("$ROOT_TIME_IN_SEC",str(int(t)))
		content = content.replace("$ADD_MARGIN_RES_W",str(self._w_img_px))
		content = content.replace("$ADD_MARGIN_RES_H",str(self._h_img_px))
		content = content.replace("$ADD_MARGIN_RES_PA",str(self.r_pa()))
		content = content.replace("$REMOVE_MARGIN_RES_W",str(self._w_img_overscan_px))
		content = content.replace("$REMOVE_MARGIN_RES_H",str(self._h_img_overscan_px))
		content = content.replace("$REMOVE_MARGIN_RES_PA",str(self.r_pa()))
		content = content.replace("$UNDISTORT_RES_W",str(self._w_img_overscan_px))
		content = content.replace("$UNDISTORT_RES_H",str(self._h_img_overscan_px))
		content = content.replace("$UNDISTORT_RES_PA",str(self.r_pa()))
		content = content.replace("$REDISTORT_RES_W",str(self._w_img_overscan_px))
		content = content.replace("$REDISTORT_RES_H",str(self._h_img_overscan_px))
		content = content.replace("$REDISTORT_RES_PA",str(self.r_pa()))
		fout = open(self._batch_name + ".batch","w")
		fout.write(content)

#--------------------------------------------------------------#
# Begin API                                                    #
#--------------------------------------------------------------#
# A batch in Flame consists of a batch-file and a batch-directory.
# This method creates the batch-directory, sets permissions and
# adds a "fingerprint" file. This is a safety feature from our
# side to avoid unintended writing in the file system.
	def create_top_level_directory(self):
# Create the directory and set permissions
		os.mkdir(self._batch_path)
		os.chmod(self._batch_path,0o775)
		os.chdir(self._batch_path)
# Create fingerprint file. We give out a warning in case the user
# is about to delete someone else's batch directory.
		fout = open("fingerprint","w")
		print("user",self._user, file=fout)
		fout.close()
		os.chmod("fingerprint",0o775)

	def create_batch(self):
# Filenames and Paths
		out_undistort_name = "undistort"
		out_redistort_name = "redistort"
		in_glsl_path = os.path.join(self._tde4_flame_path,self._ldm_name_glsl)
		in_xml_path = os.path.join(self._tde4_flame_path,self._ldm_name_xml)
# Delete directory tree!
		if os.path.isdir(self._batch_path):
			shutil.rmtree(self._batch_path,ignore_errors = True)
		elif os.path.isfile(self._batch_path):
			os.remove(self._batch_path)

		self.create_top_level_directory()
# The plan is to generate a batch with four nodes forming a pipeline.
# One for adding overscan margin:
		self.create_resize_node_add_margin()

# One for removing overscan margin
		self.create_resize_node_remove_margin()

# One for undistorting
		self.create_matchbox_node(out_undistort_name,1)
		shutil.copyfile(in_glsl_path,out_undistort_name + ".1.glsl")
		shutil.copyfile(in_xml_path,out_undistort_name + ".xml")

# One for redistorting
		self.create_matchbox_node(out_redistort_name,0)
		shutil.copyfile(in_glsl_path,out_redistort_name + ".1.glsl")
		shutil.copyfile(in_xml_path,out_redistort_name + ".xml")

# And additionally the root node
		self.create_root_node()

# And the pipeline
		os.chdir(self._target_directory)
		self.create_pipeline()
#--------------------------------------------------------------#
# End API                                                      #
#--------------------------------------------------------------#

################################################################
# Script class: GUI and control methods                        #
################################################################
class xflld_script:
# Error classes
	class error(Exception):
		def __init__(self,value):
			self.value = value
		def __str__(self):
			return repr(self.value)
	def __init__(self):
# This is the path in 3DE4's installation where we
# keep templates for the batch to be created.
		self._tde4_flame_path = os.path.join(tde4.get3DEInstallPath(),"sys_data","flame","2019.1")
# Values obtained from widgets
		self._gui_batch_path = "/tmp/my.batch"
		self._overscan_margin_x = 0
		self._overscan_margin_y = 0

		self._target_directory = ""
		self._batch_name = "my_batch"

		self._message = ""
		self._ask = True
# Script name
	def name(self):
			return "Export Lens Distortion Batch to Flame"
# Main entry point, called from top-level.
	def main(self):
		if tde4.getCameraNoFrames(tde4.getCurrentCamera()) == 0:
			tde4.postQuestionRequester("Error","No frames defined for selected camera.","Close")
			return
		try:
			self.build_dialog()
		except self.error as e:
# Error class defined for this script
			tde4.postQuestionRequester("Error",str(e),"Close")
			return
		except:
# Any other error class
			tde4.postQuestionRequester("Error","An error has occurred, see python console window","Close")
			raise
# Check Buttons of dislog
# Cancel means success, Ok means success if no error has occurred.
		success = False
# In case of trouble the dialog keeps popping up again until
# the user solves the problem or cancels the entire process.
		while success == False:
# Set values in overscan textfields
			xflld_update_tf_overscan_absolute()

			if tde4.postCustomRequester(self.id_req,self.name(),650,200,"Ok","Cancel") == 1:
# Ok was pressed. Evaluate widgets of dialog here
				self._gui_batch_path = tde4.getWidgetValue(self.id_req,"fl_batch_file")
# More robust as of 1.6:
				self._overscan_margin_x = xflld_int_from_widget(self.id_req,"tf_overscan_margin_x")
				self._overscan_margin_y = xflld_int_from_widget(self.id_req,"tf_overscan_margin_y")

				if (self._gui_batch_path == None) or (self._gui_batch_path == ""):
					tde4.postQuestionRequester("Error","Batch file: please enter a path to a batch (existing or to be created)","Close")
					continue
# At this point, all data are gathered and prepared.
				try:
					self.run()
					success = True
				except(xflld_reenter_batch_file):
					continue
				except(xflld_failed):
					break
			else:
# Cancel was pressed
				success = True

# Does path have suffix .batch?
	def is_batch_file(self,path):
		return path.endswith(".batch")
	def strip_batch_suffix(self,path):
		if path[-6:].lower() == ".batch":
			return path[:-6]
		else:
			return path
	def strip_separator(self,path):
		if path[-1:] == "/":
			return path[:-1]
		else:
			return path
	def set_target_directory(self,path):
		self._target_directory = path
# The method ensures that the name is stored without suffix.
	def set_batch_name(self,name):
		if name[-6:].lower() == ".batch":
			self._batch_name = name[:-6]
		else:
			self._batch_name = name
	def check_fingerprint(self,path):
		try:
			fin = open(os.path.join(path,"fingerprint"),"r")
		except:
			return False
		line = fin.readline()
		tag,user = line.split()
# Everything fine? Then just return
		if tag == "user":
			if user == getpass.getuser():
				return True
# Something wrong
		return False

# Method called when all required data are prepared. In case of trouble
# the method raises an exception, and the caller decides whether to
# pop up the dialog again or to exit without action.
	def run(self):
# The following can happen:
# 1. user selects an existing plain file
#    1. It's a batch file "path/to/<batch>.batch"
#      -  Set target_directory to basename("/path/to") and _batch_name with "<batch>"
#      1. There is a directory "<batch>"
#         - Ask if it's ok to replace <batch> and <batch>.batch
#      2. There is NO directory "<batch>"
#         - Ask if it's ok to create <batch> and replace <batch>.batch
#    2. It's NOT a batch file "path/to/<batch>.batch" but has name "path/to/<name>"
#       - Fill fl_base_directory with basename("/path/to") and tf_batch_name with "<name>"
#       - Ask if it's ok to create directory "/path/to/<name>" and file "/path/to/<name>.batch"
# 2. user selects an existing directory "path/to/name"
#    1. There is a batch file "path/to/name.batch"
#       - Fill fl_base_directory with basename("/path/to") and tf_batch_name with "<name>"
#       - Ask if it's ok to replace <batch> and <batch>.batch
#    2. There is NO batch file "path/to/name.batch"
#       ! We intepret this as unique sign that "path/to/name" is not a batch directory.
#       - Fill fl_base_directory with basename("/path/to/name") and tf_batch_name with ""
#       - Ask if it's ok to replace <batch> and create <batch>.batch
# 3. user specifies a path that does not exist: /path/to/name
#    -  Set target_directory to /path/to and batch_name to name
#    1. target_directory exists
#       - Ask if it's ok to replace /path/to/name and create name.batch
#    2. target_directory does NOT exist
#       - Ask if it's ok to create /path/to/name and create name.batch
# 4. user selects a file of unexpected type "path/to/name"
#    - fatal error
		path_to_name = self.strip_separator(self._gui_batch_path)
		self._message = ""
		self._ask = True

# Extract target directory and batch name.
		self.set_target_directory(self.strip_batch_suffix(os.path.dirname(path_to_name)))
		self.set_batch_name(self.strip_batch_suffix(os.path.basename(path_to_name)))
# Base message
		self._message = "The script will now do the following in directory\n%s:\n" % self._target_directory
# Case 1.
		if os.path.isfile(path_to_name):
# Case 1.1

			if self.is_batch_file(path_to_name):
# Case 1.1.1
				if os.path.exists(self.strip_batch_suffix(path_to_name)):
					self._message += "- replace directory %s\n- replace file %s.batch\n (case 1.1.1)" % (self._batch_name,self._batch_name)
# Case 1.1.2
				else:
					self._message += "- create directory %s\n- replace file %s.batch\n (case 1.1.2)" % (self._batch_name,self._batch_name)
# Case 1.2
			else:
				self._message += "- create directory %s\n- create file %s.batch\n (case 1.2)" % (self._batch_name,self._batch_name)
# Case 2.
		elif os.path.isdir(path_to_name):
# Case 2.1
			if os.path.exists(path_to_name + ".batch"):
				self._message += "- replace directory %s\n- replace file %s.batch\n (case 2.1)" % (self._batch_name,self._batch_name)
# Case 2.2
			else:
				if self.check_fingerprint(path_to_name):
					self._message += "- replace directory %s\n- create file %s.batch\n (case 2.2)" % (self._batch_name,self._batch_name)
				else:
					tde4.postQuestionRequester("Error","'%s' is a (non-batch) directory.\nPlease enter a batch name." % path_to_name,"Ok")
					raise xflld_reenter_batch_file()
# Case 3.
		elif not os.path.exists(path_to_name):
# Case 3.1
			if os.path.exists(os.path.join(self._target_directory,self._batch_name)):
				self._message += "- replace directory %s\n- create file %s.batch\n (case 3.1)" % (self._batch_name,self._batch_name)
# Case 3.2
			else:
				self._message += "- create directory %s\n- create file %s.batch\n (case 3.2)" % (self._batch_name,self._batch_name)
# Case 4
		else:
			tde4.postQuestionRequester("Fatal error","File %s exists but has unexpected file type.\nCannot overwrite.\n (case 4)" % path_to_name,"Back")
			raise xflld_reenter_batch_file()

# self._ask and self._message are set in the file widgets callback method.
		if self._ask:
			if tde4.postQuestionRequester("Please check",self._message,"Ok","Cancel") == 2:
				raise xflld_reenter_batch_file()

# We create an instance of our main class for accessing 3DE4's database.
		proxy = xflld_accessor(tde4.getCurrentCamera(),self._tde4_flame_path,self._target_directory,self._batch_name,self._overscan_margin_x,self._overscan_margin_y)

# We did what we could do to ensure the user wants to overwrite the directory, so let's go ahead.				
		try:
			proxy.create_batch()
		except OSError as e:
# most likely a permissions problem
			tde4.postQuestionRequester("Error","Could not create batch directory or file: %s (OSError)" % str(e),"Back")
			raise xflld_reenter_batch_file()
		except IOError as e:
# not sure if this can occur logically.
			tde4.postQuestionRequester("Error","Could not create batch directory or file: %s (IOError)" % str(e),"Back")
			raise xflld_reenter_batch_file()


	def build_dialog(self):
		self.id_req = _ExportFlameLensDistortion_requester

# Singleton
try:
	the_xflld_script
except:
	the_xflld_script = xflld_script()

# del-commands during development ensure that
# the script is re-loaded again within 3DE4
# after editing. When in use, objects are not deleted
# so that 3DE4 remembers entries in the dialog from one
# script call to the next (which is useful).
try:
	the_xflld_script.main()
except:
# During development:
#	del the_xflld_script
#	del xflld_script
#end
	raise
else:
# During development:
#	del the_xflld_script
#	del xflld_script
#end
	pass



