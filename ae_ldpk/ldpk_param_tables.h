// LDPK-to-AfterEffects bridge.
// Static parameter tables for every supported LD_3DE4 model class.
// Used by the AE plugin to build per-model parameter groups ("tabs") at
// PF_Cmd_PARAMS_SETUP time and to map .nk curves onto effect parameters.
//
// The tables mirror the LDPK model headers exactly:
//   - names:      tde4_ldp_*::_para[]
//   - ranges:     tde4_ldp_*::getParameterRange()
//   - defaults:   tde4_ldp_*::getParameterDefaultValue()
// nk_ld_dump prints the parsed .nk node and a mapping grid for
// cross-checking against Nuke.
//
// Software is provided "as is" - no warranties implied.

#pragma once

#include <string>

namespace ae_ldpk
	{
	static const int LD_MAX_MODEL_PARAMS = 24;

	struct ld_param_info
		{
		const char* name;	// original LDPK parameter name (as in the model)
		double min_value;
		double max_value;
		double default_value;
		};

	struct ld_model_info
		{
		const char* nuke_class;		// node class as written by the 3DE4 exporter
		const char* ui_label;		// group label in the AE effect controls
		int num_params;
		bool ui_visible;			// offered in the AE UI (Model popup + group);
									// hidden models stay loadable from .nk files
		ld_param_info params[LD_MAX_MODEL_PARAMS];
		};

// Shared range shorthands (values from the LDPK headers).
//   radial coeffs:      [-0.5, 0.5], default 0
//   decentering U/V:    [-0.1, 0.1], default 0
//   lens rotation deg:  [-2.0, 2.0], default 0
//   squeeze X/Y:        [0.9, 1.1],  default 1
//   rescale:            [0.25, 4.0], default 1
//   stabilizer X/Y:     [-0.5, 0.5], default 0
//   classic squeeze:    [0.25, 4.0], default 1

#define LD_P(name_,mn,mx,df) { name_, mn, mx, df }
#define LD_RADIAL(name_)     LD_P(name_, -0.5, 0.5, 0.0)
#define LD_DECENT(name_)     LD_P(name_, -0.1, 0.1, 0.0)
#define LD_ROT(name_)        LD_P(name_, -2.0, 2.0, 0.0)
#define LD_SQUEEZE(name_)    LD_P(name_, 0.9, 1.1, 1.0)
#define LD_RESCALE(name_)    LD_P(name_, 0.25, 4.0, 1.0)
#define LD_STAB(name_)       LD_P(name_, -0.5, 0.5, 0.0)

// 18 anamorphic coefficients, shared by all anamorphic models.
#define LD_ANAMORPHIC_18 \
	LD_RADIAL("Cx02 - Degree 2"),LD_RADIAL("Cy02 - Degree 2"), \
	LD_RADIAL("Cx22 - Degree 2"),LD_RADIAL("Cy22 - Degree 2"), \
	LD_RADIAL("Cx04 - Degree 4"),LD_RADIAL("Cy04 - Degree 4"), \
	LD_RADIAL("Cx24 - Degree 4"),LD_RADIAL("Cy24 - Degree 4"), \
	LD_RADIAL("Cx44 - Degree 4"),LD_RADIAL("Cy44 - Degree 4"), \
	LD_RADIAL("Cx06 - Degree 6"),LD_RADIAL("Cy06 - Degree 6"), \
	LD_RADIAL("Cx26 - Degree 6"),LD_RADIAL("Cy26 - Degree 6"), \
	LD_RADIAL("Cx46 - Degree 6"),LD_RADIAL("Cy46 - Degree 6"), \
	LD_RADIAL("Cx66 - Degree 6"),LD_RADIAL("Cy66 - Degree 6")

// 10 anamorphic degree-4 coefficients.
#define LD_ANAMORPHIC_10 \
	LD_RADIAL("Cx02 - Degree 2"),LD_RADIAL("Cy02 - Degree 2"), \
	LD_RADIAL("Cx22 - Degree 2"),LD_RADIAL("Cy22 - Degree 2"), \
	LD_RADIAL("Cx04 - Degree 4"),LD_RADIAL("Cy04 - Degree 4"), \
	LD_RADIAL("Cx24 - Degree 4"),LD_RADIAL("Cy24 - Degree 4"), \
	LD_RADIAL("Cx44 - Degree 4"),LD_RADIAL("Cy44 - Degree 4")

	static const ld_model_info LD_MODELS[] =
		{
		{
		"LD_3DE4_Radial_Standard_Degree_4","Radial Standard Degr. 4",8,true,
			{
			LD_RADIAL("Distortion - Degree 2"),
			LD_DECENT("U - Degree 2"),
			LD_DECENT("V - Degree 2"),
			LD_RADIAL("Quartic Distortion - Degree 4"),
			LD_DECENT("U - Degree 4"),
			LD_DECENT("V - Degree 4"),
			LD_P("Phi - Cylindric Direction",-90.0,90.0,0.0),
			LD_P("B - Cylindric Bending",-0.1,0.1,0.0),
			}
		},
		{
		"LD_3DE4_Radial_Stabilizer_Degree_6","Radial Stabilizer Degr. 6",7,true,
			{
			LD_RADIAL("Distortion - Degree 2"),
			LD_DECENT("U - Degree 2"),
			LD_DECENT("V - Degree 2"),
			LD_RADIAL("Distortion - Degree 4"),
			LD_RADIAL("Distortion - Degree 6"),
			LD_STAB("Stabilizer-X"),
			LD_STAB("Stabilizer-Y"),
			}
		},
		{
		"LD_3DE4_Anamorphic_Degree_6","Anamorphic Degr. 6",18,true,
			{
			LD_ANAMORPHIC_18
			}
		},
		{
		"LD_3DE4_Anamorphic_Standard_Degree_4","Anamorphic Standard Degr. 4",13,true,
			{
			LD_ANAMORPHIC_10,
			LD_ROT("Lens Rotation"),
			LD_SQUEEZE("Squeeze-X"),
			LD_SQUEEZE("Squeeze-Y"),
			}
		},
		{
		"LD_3DE4_Anamorphic_Standard_Degree_6","Anamorphic Standard Degr. 6",21,true,
			{
			LD_ANAMORPHIC_18,
			LD_ROT("Lens Rotation"),
			LD_SQUEEZE("Squeeze-X"),
			LD_SQUEEZE("Squeeze-Y"),
			}
		},
		{
		"LD_3DE4_Anamorphic_Rescaled_Degree_4","Anamorphic Rescaled Degr. 4",14,true,
			{
			LD_ANAMORPHIC_10,
			LD_ROT("Lens Rotation"),
			LD_SQUEEZE("Squeeze-X"),
			LD_SQUEEZE("Squeeze-Y"),
			LD_RESCALE("Rescale"),
			}
		},
		{
		"LD_3DE4_Anamorphic_Rescaled_Degree_6","Anamorphic Rescaled Degr. 6",22,true,
			{
			LD_ANAMORPHIC_18,
			LD_ROT("Lens Rotation"),
			LD_SQUEEZE("Squeeze-X"),
			LD_SQUEEZE("Squeeze-Y"),
			LD_RESCALE("Rescale"),
			}
		},
		{
		"LD_3DE4_Anamorphic_Stabilizer_Degree_6","Anamorphic Stabilizer Degr. 6",24,true,
			{
			LD_ANAMORPHIC_18,
			LD_ROT("Lens Rotation"),
			LD_SQUEEZE("Squeeze-X"),
			LD_SQUEEZE("Squeeze-Y"),
			LD_RESCALE("Rescale"),
			LD_STAB("Stabilizer-X"),
			LD_STAB("Stabilizer-Y"),
			}
		},
		{
		"LD_3DE4_Brown_Conrady_Degree_6_2","Brown-Conrady Degr. 6-2",5,false,
			{
			LD_RADIAL("C2"),
			LD_RADIAL("C4"),
			LD_RADIAL("C6"),
			LD_DECENT("T1"),
			LD_DECENT("T2"),
			}
		},
		{
		"LD_3DE4_Radial_Fisheye_Degree_8","Radial Fisheye Degr. 8",4,true,
			{
			LD_RADIAL("Distortion - Degree 2"),
			LD_RADIAL("Quartic Distortion - Degree 4"),
			LD_RADIAL("Degree 6"),
			LD_RADIAL("Degree 8"),
			}
		},
		{
		"LD_3DE4_Radial_Fisheye_Equidistant_Degree_8","Radial Fisheye Equidistant Degr. 8",4,false,
			{
			LD_RADIAL("Distortion - Degree 2"),
			LD_RADIAL("Quartic Distortion - Degree 4"),
			LD_RADIAL("Degree 6"),
			LD_RADIAL("Degree 8"),
			}
		},
		{
		"LD_3DE4_Radial_Fisheye_Equisolid_Degree_8","Radial Fisheye Equisolid Degr. 8",4,false,
			{
			LD_RADIAL("Distortion - Degree 2"),
			LD_RADIAL("Quartic Distortion - Degree 4"),
			LD_RADIAL("Degree 6"),
			LD_RADIAL("Degree 8"),
			}
		},
		{
		"LD_3DE4_Radial_Fisheye_Orthographic_Degree_8","Radial Fisheye Orthographic Degr. 8",4,false,
			{
			LD_RADIAL("Distortion - Degree 2"),
			LD_RADIAL("Quartic Distortion - Degree 4"),
			LD_RADIAL("Degree 6"),
			LD_RADIAL("Degree 8"),
			}
		},
		{
		"LD_3DE4_Radial_Fisheye_Stereographic_Degree_8","Radial Fisheye Stereographic Degr. 8",4,false,
			{
			LD_RADIAL("Distortion - Degree 2"),
			LD_RADIAL("Quartic Distortion - Degree 4"),
			LD_RADIAL("Degree 6"),
			LD_RADIAL("Degree 8"),
			}
		},
		{
		"LD_3DE4_Radial_Homomorphic_Degree_2","Radial Homomorphic Degr. 2",3,false,
			{
			LD_RADIAL("C - Degree 2"),
			LD_DECENT("U - Degree 2"),
			LD_DECENT("V - Degree 2"),
			}
		},
		{
		"LD_3DE4_Anamorphic_Homomorphic_Degree_2","Anamorphic Homomorphic Degr. 2",3,false,
			{
			LD_RADIAL("Cx2 - Degree 2"),
			LD_RADIAL("Cy2 - Degree 2"),
			LD_RADIAL("B2 - Degree 2"),
			}
		},
		{
		"LD_OpenCV_Radial_Degree_6_Decentered_Degree_2","OpenCV Radial Degr. 6 Decentered Degr. 2",5,false,
			{
			LD_RADIAL("Radial Distortion - Degree 2 - k1"),
			LD_RADIAL("Radial Distortion - Degree 4 - k2"),
			LD_RADIAL("Decentering Y - Degree 2 - p1"),
			LD_RADIAL("Decentering X - Degree 2 - p2"),
			LD_RADIAL("Radial Distortion - Degree 6 - k3"),
			}
		},
		{
		"LD_3DE_Classic_LD_Model","Classic LD Model",5,true,
			{
			LD_RADIAL("Distortion"),
			LD_P("Anamorphic Squeeze",0.25,4.0,1.0),
			LD_RADIAL("Curvature X"),
			LD_RADIAL("Curvature Y"),
			LD_RADIAL("Quartic Distortion"),
			}
		},
		};

	static const int LD_NUM_MODELS = (int)(sizeof(LD_MODELS) / sizeof(LD_MODELS[0]));

// Number of models offered in the UI (Model popup + parameter groups):
// the 3DE4 built-ins plus the legacy "3DE Classic LD Model" — 10 total.
	inline int ld_num_ui_models()
		{
		int n = 0;
		for(int i = 0;i < LD_NUM_MODELS;++i)
			{
			if(LD_MODELS[i].ui_visible)
				{ ++n; }
			}
		return n;
		}

//! @brief Map a UI (popup) index, 0-based over the visible models,
//! to an LD_MODELS index. Returns -1 if out of range.
	inline int ld_ui_to_model(int ui_index)
		{
		int n = 0;
		for(int i = 0;i < LD_NUM_MODELS;++i)
			{
			if(!LD_MODELS[i].ui_visible)
				{ continue; }
			if(n == ui_index)
				{ return i; }
			++n;
			}
		return -1;
		}

//! @brief Map an LD_MODELS index to a UI (popup) index, 0-based.
//! Returns -1 for models hidden from the UI.
	inline int ld_model_to_ui(int model_index)
		{
		if(model_index < 0 || model_index >= LD_NUM_MODELS || !LD_MODELS[model_index].ui_visible)
			{ return -1; }
		int n = 0;
		for(int i = 0;i < model_index;++i)
			{
			if(LD_MODELS[i].ui_visible)
				{ ++n; }
			}
		return n;
		}

//! @brief Find the model table by Nuke node class. Returns 0 if unknown.
	inline const ld_model_info* find_model_info(const std::string& nuke_class)
		{
		for(int i = 0;i < LD_NUM_MODELS;++i)
			{
			if(nuke_class == LD_MODELS[i].nuke_class)
				{ return &LD_MODELS[i]; }
			}
		return 0;
		}

#undef LD_P
#undef LD_RADIAL
#undef LD_DECENT
#undef LD_ROT
#undef LD_SQUEEZE
#undef LD_RESCALE
#undef LD_STAB
#undef LD_ANAMORPHIC_18
#undef LD_ANAMORPHIC_10
	}
