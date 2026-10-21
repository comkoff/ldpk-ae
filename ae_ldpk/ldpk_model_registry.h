// LDPK-to-AfterEffects bridge.
// Registry mapping Nuke node class names (as written by the 3DE4 exporter)
// to LDPK model instances. Mirrors the CLASS names used by the Nuke plugins
// in source/ldpk/nuke_ld_3de4_*.C.
//
// Software is provided "as is" - no warranties implied.

#pragma once

#include <ldpk/tde4_ld_plugin.h>
#include <ldpk/tde4_ldp_classic_ld_model.h>
#include <ldpk/tde4_ldp_radial_standard_degree_4.h>
#include <ldpk/tde4_ldp_radial_stabilizer_degree_6.h>
#include <ldpk/tde4_ldp_anamorphic_degree_6.h>
#include <ldpk/tde4_ldp_anamorphic_standard_degree_4.h>
#include <ldpk/tde4_ldp_anamorphic_standard_degree_6.h>
#include <ldpk/tde4_ldp_anamorphic_rescaled_degree_4.h>
#include <ldpk/tde4_ldp_anamorphic_rescaled_degree_6.h>
#include <ldpk/tde4_ldp_anamorphic_stabilizer_degree_6.h>
#include <ldpk/tde4_ldp_brown_conrady_degree_6_2.h>
#include <ldpk/tde4_ldp_radial_fisheye_degree_8.h>
#include <ldpk/tde4_ldp_radial_fisheye_equidistant_degree_8.experimental.h>
#include <ldpk/tde4_ldp_radial_fisheye_equisolid_degree_8.experimental.h>
#include <ldpk/tde4_ldp_radial_fisheye_orthographic_degree_8.experimental.h>
#include <ldpk/tde4_ldp_radial_fisheye_stereographic_degree_8.experimental.h>
#include <ldpk/tde4_ldp_radial_homomorphic_decentered_degree_2.experimental.h>
#include <ldpk/tde4_ldp_anamorphic_homomorphic_degree_2.experimental.h>
#include <ldpk/tde4_ldp_radial_degree_6_decentered_degree_2_opencv.h>

#include <map>
#include <string>

namespace ae_ldpk
	{
	typedef ldpk::vec2d vec2_type;
	typedef ldpk::mat2d mat2_type;
	typedef tde4_ld_plugin ldm_type;

//! @brief Create an LDPK model instance from a Nuke node class name.
//! Returns 0 for unknown class names.
	inline ldm_type* create_model_by_nuke_class(const std::string& nuke_class)
		{
		typedef ldm_type ldm;
		if(nuke_class == "LD_3DE_Classic_LD_Model")
			{ return new tde4_ldp_classic_ld_model<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Standard_Degree_4")
			{ return new tde4_ldp_radial_standard_degree_4<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Stabilizer_Degree_6")
			{ return new tde4_ldp_radial_stabilizer_degree_6<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Anamorphic_Degree_6")
			{ return new tde4_ldp_anamorphic_degree_6<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Anamorphic_Standard_Degree_4")
			{ return new tde4_ldp_anamorphic_standard_degree_4<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Anamorphic_Standard_Degree_6")
			{ return new tde4_ldp_anamorphic_standard_degree_6<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Anamorphic_Rescaled_Degree_4")
			{ return new tde4_ldp_anamorphic_rescaled_degree_4<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Anamorphic_Rescaled_Degree_6")
			{ return new tde4_ldp_anamorphic_rescaled_degree_6<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Anamorphic_Stabilizer_Degree_6")
			{ return new tde4_ldp_anamorphic_stabilizer_degree_6<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Brown_Conrady_Degree_6_2")
			{ return new tde4_ldp_brown_conrady_degree_6_2<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Fisheye_Degree_8")
			{ return new tde4_ldp_radial_fisheye_degree_8<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Fisheye_Equidistant_Degree_8")
			{ return new tde4_ldp_radial_fisheye_equidistant_degree_8<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Fisheye_Equisolid_Degree_8")
			{ return new tde4_ldp_radial_fisheye_equisolid_degree_8<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Fisheye_Orthographic_Degree_8")
			{ return new tde4_ldp_radial_fisheye_orthographic_degree_8<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Fisheye_Stereographic_Degree_8")
			{ return new tde4_ldp_radial_fisheye_stereographic_degree_8<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Radial_Homomorphic_Degree_2")
			{ return new tde4_ldp_radial_homomorphic_decentered_degree_2<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_3DE4_Anamorphic_Homomorphic_Degree_2")
			{ return new tde4_ldp_anamorphic_homomorphic_degree_2<vec2_type,mat2_type>(); }
		if(nuke_class == "LD_OpenCV_Radial_Degree_6_Decentered_Degree_2")
			{ return new tde4_ldp_radial_degree_6_decentered_degree_2_opencv<vec2_type,mat2_type>(); }
// Note: "LD_3DE4_Noop_Debugging" is not registered here: its header uses a
// zero-length static array, which MSVC rejects (C2466).
// Note: "LD_3DE4_Radial_Fisheye_Degree_8_Release_1" is obsolete and not
// supported here (its header does not implement the current interface).
		return 0;
		}
	}
