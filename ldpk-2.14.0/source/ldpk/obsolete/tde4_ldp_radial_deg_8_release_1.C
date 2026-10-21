#include <ldpk/tde4_ldp_radial_deg_8_release_1.h>

#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
extern "C" tde4_ld_plugin* tde4ldp_create_fct()
	{
	return new tde4_ldp_radial_deg_8_release_1<ldpk::vec2d,ldpk::mat2d>;
	}

extern "C" void tde4ldp_destroy_fct(tde4_ld_plugin* p)
	{
	delete p;
	}
#endif
