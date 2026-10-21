// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <DDImage/Iop.h>
#include <DDImage/Box.h>
#include <DDImage/Filter.h>
#include <DDImage/Knobs.h>
#include <DDImage/Pixel.h>
#include <DDImage/Row.h>
#include <DDImage/Tile.h>
#include <DDImage/NukeWrapper.h>
#include <ldpk/ldpk_ldp_builtin.h>
#include <ldpk/ldpk_line_cache.h>

using namespace std;

struct nuke_ld_lut_entry
	{
	double _x,_y;
	bool _status;

	nuke_ld_lut_entry():_x(HUGE_VAL),_y(HUGE_VAL),_status(false)
		{ }
	nuke_ld_lut_entry(double x,double y,bool status):_x(x),_y(y),_status(status)
		{ }
	double x() const
		{ return _x; }
	double y() const
		{ return _y; }
	bool status() const
		{ return _status; }
	bool defd() const
		{ return (_x != HUGE_VAL) && (_y != HUGE_VAL); }
	};

//! @brief The baseclass for Nuke plugins around LDPK-based lens distortion models.
//! In order to implement a Nuke plugin, derive from this class.
class nuke_ld_3de4_base:public DD::Image::Iop
	{
private:
	DD::Image::Filter _filter;
// Anzahl der modellabhaengigen Parameter, wird im Konstruktor ermittelt.
	int _num_par;
// Der Wert des Richtungsknobs
	int _knob_direction;
// Current output mode (extension by RSP, see .C-file)
	int _knob_output_mode;
// With LUT-Caching
	int _knob_lut_cache;
// Bbox-Mode
	int _knob_bbox_mode;
// Boundary for bbox-mode 2: l,b,r,t
	float _knob_bbox_margin[2];
// Field-of-View
	double _xa_fov_unit,_ya_fov_unit;
	double _xb_fov_unit,_yb_fov_unit;
	double _xd_fov_unit,_yd_fov_unit;
	double _inv_xd_fov_unit,_inv_yd_fov_unit;
// Find out number of parameters per type and store here:
	int _num_par_adjustable_double;
	int _num_par_double;
	int _num_par_int;
	int _num_par_string;
	int _num_par_file;
	int _num_par_toggle;
// Wir verwenden hier absichtlich nicht std::vector, weil wir uns darauf verlassen
// muessen, dass die Werte nicht im Speicher verschoben werden. Es gibt einen Beispiel-Node,
// bei Nuke, der lustigerweise KnobParade heisst, da sehen wir, wie das geht.
	double* _values_adjustable_double;
	double* _values_double;
	int* _values_int;
	const char** _values_string;
	const char** _values_file;
	bool* _values_toggle;
// Default values for string- and file-knobs.
	char** _defaults_string;
	char** _defaults_file;
// The ld model instance.
	typedef tde4_ld_plugin ldm_type;
	ldm_type* _ldm;
// LUT-Caching
	DD::Image::Hash _lut_hash;
	ldpk::line_cache<nuke_ld_lut_entry> _lut_cache;
// In _request we compute a bounding box from our ldm, intersect it with
// input0 and request that box from input0. Nuke's Devguide says, we should
// never lock tiles larger than this request-box, so we keep it here and
// take it into account in engine().
	DD::Image::Box _box_requested;

// Map x-coordinate from unit cordinates to fov coordinates.
	double map_in_fov_x(double x_unit) const
		{ return  (x_unit - _xa_fov_unit) * _inv_xd_fov_unit; }
// Map y-coordinate from unit cordinates to fov coordinates.
	double map_in_fov_y(double y_unit) const
		{ return  (y_unit - _ya_fov_unit) * _inv_yd_fov_unit; }
// Map x-coordinate from fov cordinates to unit coordinates.
	double map_out_fov_x(double x_fov) const
		{ return x_fov * _xd_fov_unit + _xa_fov_unit; }
// Map y-coordinate from fov cordinates to unit coordinates.
	double map_out_fov_y(double y_fov) const
		{ return y_fov * _yd_fov_unit + _ya_fov_unit; }

// Model and parameter names in 3DE4 are human-readable, with characters
// like ",", "-" and space in it. This method eliminates maximum substrings containing
// other than [a-zA-Z0-9] and replaces them by a single underscore. The empty string is
// mapped to "_". A leading [0-9] is supplemented by a leading underscore.
// The result has the form of an identifier in C and Python.
	std::string nukify_name(const std::string& name);
// Anscheinend von keiner Basisklasse gefordert.
	enum direction_enum { distort = 0,undistort = 1 };
	DD::Image::Box bounds(direction_enum,int x,int y,int r,int t);
// LDM-Parameter auf default
	void set_default_values();
// Check for implementation errors
	void check_indices(int i_par_adjustable_double,int i_par_double,int i_par_int,int i_par_string,int i_par_file,int i_par_toggle,int i_debug) const;

protected:
/***** begin debugging ****************************************/
// Define the environment variable and get charming messages in stdout.
// Environment variable NUKE_DEBUG_REQUEST_SDV
	bool _verbose_request;
// Environment variable NUKE_DEBUG_VALIDATE_SDV
	bool _verbose_validate;
// Environment variable NUKE_DEBUG_CONSTRUCTOR_SDV
	bool _verbose_constructor;
// Environment variable NUKE_DEBUG_DESTRUCTOR_SDV
	bool _verbose_destructor;
/***** end debugging ******************************************/

public:
	nuke_ld_3de4_base(Node* node,ldm_type* ldm);
	virtual ~nuke_ld_3de4_base();

	void knobs(DD::Image::Knob_Callback f);
	void _validate(bool);
	void _request(int x,int y,int r,int t,DD::Image::ChannelMask channels,int count);
	void _open();
	void engine(int y,int x,int r,DD::Image::ChannelMask channels,DD::Image::Row& out);

	static const DD::Image::Iop::Description description;
	};
