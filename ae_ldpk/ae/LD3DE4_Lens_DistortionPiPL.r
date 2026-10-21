#include "AEConfig.h"
#include "AE_EffectVers.h"

#ifndef AE_OS_WIN
	#include "AE_General.r"
#endif

resource 'PiPL' (16000) {
	{	/* array properties: 13 elements */
		/* [1] */
		Kind {
			AEEffect
		},
		/* [2] */
		Name {
#ifdef LD_ENABLE_DEBUG_LOG
			"3DE4 Lens Distortion Debug"
#else
			"3DE4 Lens Distortion"
#endif
		},
		/* [3] */
		Category {
			"Lens Distortion"
		},
#ifdef AE_OS_WIN
    #if defined(AE_PROC_INTELx64)
		CodeWin64X86 {"EffectMain"},
    #elif defined(AE_PROC_ARM64)
		CodeWinARM64 {"EffectMain"},
    #endif
#elif defined(AE_OS_MAC)
		CodeMacIntel64 {"EffectMain"},
		CodeMacARM64 {"EffectMain"},
#endif
		/* [6] */
		AE_PiPL_Version {
			2,
			0
		},
		/* [7] */
		AE_Effect_Spec_Version {
			PF_PLUG_IN_VERSION,
			PF_PLUG_IN_SUBVERS
		},
		/* [8] */
		AE_Effect_Version {
			525825 /* 1.0.0 = PF_VERSION(1,0,0,RELEASE,1), must match out_data->my_version */
		},
		/* [9] */
		AE_Effect_Info_Flags {
			0
		},
		/* [10] */
		AE_Effect_Global_OutFlags {
			0x06000630 /* DEEP_COLOR_AWARE(1<<25) | SEND_UPDATE_PARAMS_UI(1<<26) |
			              PIX_INDEPENDENT(1<<10) | I_EXPAND_BUFFER(1<<9) |
			              I_DO_DIALOG(1<<5) | SEQUENCE_DATA_NEEDS_FLATTENING(1<<4).
			  I_HAVE_EXTERNAL_DEPENDENCIES is not set — the .nk
			  snapshot lives in the sequence data, renders never touch the file.
			  I_EXPAND_BUFFER kept — the engine's hint for the layer
			  extent / request computation in the smart-render protocol too
			  (without it AE clips the request to the layer raster);
			  USE_OUTPUT_EXTENT(1<<6) is not set — the pre-render result rects
			  govern.
			  Must equal out_data->out_flags in GlobalSetup() exactly. */
		},
		/* [11] */
		AE_Effect_Global_OutFlags_2 {
			0x8801409 /* SUPPORTS_QUERY_DYNAMIC_FLAGS | SUPPORTS_GET_FLATTENED_SEQUENCE_DATA |
			              SUPPORTS_THREADED_RENDERING (Multi-Frame Rendering) |
			              PARAM_GROUP_START_COLLAPSED_FLAG | SUPPORTS_SMART_RENDER |
			              FLOAT_COLOR_AWARE (32 bpc; AE requires the pair) */
		},
		/* [12] */
		AE_Effect_Match_Name {
#ifdef LD_ENABLE_DEBUG_LOG
			"SDV LD_3DE4 Lens Distortion Debug"
#else
			"SDV LD_3DE4 Lens Distortion"
#endif
		},
		/* [13] */
		AE_Reserved_Info {
			8
		},
		/* [14] */
		AE_Effect_Support_URL {
			"https://www.sci-d-vis.com"
		}
	}
};
