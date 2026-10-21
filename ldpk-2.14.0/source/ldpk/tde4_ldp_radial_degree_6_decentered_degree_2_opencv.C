// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2022,  Science-D-Visions. Current version: 2.8

#include <ldpk/tde4_ldp_radial_degree_6_decentered_degree_2_opencv.h>

#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
extern "C" tde4_ld_plugin* tde4ldp_create_fct()
	{
	return new tde4_ldp_radial_degree_6_decentered_degree_2_opencv<ldpk::vec2d,ldpk::mat2d>;
	}

extern "C" void tde4ldp_destroy_fct(tde4_ld_plugin* p)
	{
	delete p;
	}
#endif
