// This file is part of the Lens Distortion Plugin Kit
// Software is provided "as is" - no warranties implied.
// (C) 2011 - 2026,  Science-D-Visions. Current version: 2.14.0

#pragma once

#include <ldpk/ldpk_error.h>
#include <ldpk/ldpk_vec2d.h>
#include <ldpk/ldpk_lookup_table.h>
#include <ldpk/ldpk_plugin_loader.h>
#include <ldpk/ldpk_model_parser.h>
#include <ldpk/ldpk_table_generator.h>

// The model function classes
#include <ldpk/ldpk_generic_distortion_base.h>
#include <ldpk/ldpk_radial_decentered_distortion.h>
#include <ldpk/ldpk_generic_radial_distortion.h>
#include <ldpk/ldpk_generic_anamorphic_distortion.h>
#include <ldpk/ldpk_classic_ld_model_distortion.h>

//! @file ldpk.h

//! @brief The namespace of (most of the) things
//! related to the Lens Distortion Plugin Kit.
namespace ldpk
	{
//! text representation of parameter types
	inline const char* text(::tde4_ldp_ptype ptype)
		{
		switch(ptype)
			{
			case TDE4_LDP_STRING: return "string";
			case TDE4_LDP_DOUBLE: return "double";
			case TDE4_LDP_INT: return "int";
			case TDE4_LDP_FILE: return "file";
			case TDE4_LDP_TOGGLE: return "toggle";
			case TDE4_LDP_ADJUSTABLE_DOUBLE: return "adjustable double";
			}
		return "<error>";
		}
	}

