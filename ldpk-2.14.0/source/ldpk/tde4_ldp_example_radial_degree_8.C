// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#include <ldpk/tde4_ldp_example_radial_degree_8.h>

const char* tde4_ldp_example_radial_degree_8::_para[4] = {
	"Degree 2",
	"Degree 4",
	"Degree 6",
	"Degree 8"
	};

// C-Style creator and destroyer, required for plugin.
// Please leave these functions as they are, because 3DE4
// needs them to load the plugin.
extern "C" tde4_ld_plugin* tde4ldp_create_fct()
	{
	return new tde4_ldp_example_radial_degree_8;
	}

extern "C" void tde4ldp_destroy_fct(tde4_ld_plugin* p)
	{
	delete p;
	}

