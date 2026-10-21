#include <ldpk/tde4_ldp_anamorphic_standard_stabilizer_degree_6.experimental.h>

#ifdef LDPK_COMPILE_AS_PLUGIN_SDV
extern "C" tde4_ld_plugin* tde4ldp_create_fct()
	{
	return new tde4_ldp_anamorphic_standard_stabilizer_degree_6<ldpk::vec2d,ldpk::mat2d>;
	}

extern "C" void tde4ldp_destroy_fct(tde4_ld_plugin* p)
	{
	delete p;
	}
#endif
