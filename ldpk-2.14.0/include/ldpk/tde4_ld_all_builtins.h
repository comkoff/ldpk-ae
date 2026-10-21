// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

//! @file
//! This file is required by an internal class of 3DE4.
//! The headers of all built-in models must be included
//! and there must be a template instantiation.

#include <ldpk/tde4_ldp_classic_ld_model.h>
#include <ldpk/tde4_ldp_radial_fisheye_degree_8.h>
#include <ldpk/tde4_ldp_radial_standard_degree_4.h>
#include <ldpk/tde4_ldp_radial_stabilizer_degree_6.h>
#include <ldpk/tde4_ldp_anamorphic_standard_degree_4.h>
#include <ldpk/tde4_ldp_anamorphic_rescaled_degree_4.h>
#include <ldpk/tde4_ldp_anamorphic_standard_degree_6.h>
#include <ldpk/tde4_ldp_anamorphic_rescaled_degree_6.h>
#include <ldpk/tde4_ldp_anamorphic_stabilizer_degree_6.h>

typedef tde4_ldp_classic_ld_model<ldpk::vec2d,ldpk::mat2d>			tde4_ldp_classic_ld_model_builtin;
typedef tde4_ldp_radial_fisheye_degree_8<ldpk::vec2d,ldpk::mat2d>		tde4_ldp_radial_fisheye_degree_8_builtin;
typedef tde4_ldp_radial_standard_degree_4<ldpk::vec2d,ldpk::mat2d>		tde4_ldp_radial_standard_degree_4_builtin;
typedef tde4_ldp_radial_stabilizer_degree_6<ldpk::vec2d,ldpk::mat2d>		tde4_ldp_radial_stabilizer_degree_6_builtin;
typedef tde4_ldp_anamorphic_standard_degree_4<ldpk::vec2d,ldpk::mat2d>		tde4_ldp_anamorphic_standard_degree_4_builtin;
typedef tde4_ldp_anamorphic_rescaled_degree_4<ldpk::vec2d,ldpk::mat2d>		tde4_ldp_anamorphic_rescaled_degree_4_builtin;
typedef tde4_ldp_anamorphic_standard_degree_6<ldpk::vec2d,ldpk::mat2d>		tde4_ldp_anamorphic_standard_degree_6_builtin;
typedef tde4_ldp_anamorphic_rescaled_degree_6<ldpk::vec2d,ldpk::mat2d>		tde4_ldp_anamorphic_rescaled_degree_6_builtin;
typedef tde4_ldp_anamorphic_stabilizer_degree_6<ldpk::vec2d,ldpk::mat2d>	tde4_ldp_anamorphic_stabilizer_degree_6_builtin;
