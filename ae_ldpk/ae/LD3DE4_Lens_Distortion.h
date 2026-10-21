/*******************************************************************/
/*                                                                 */
/* LD_3DE4 Lens Distortion for After Effects                       */
/*                                                                 */
/* Reads a 3DE4-exported .nk lens distortion node block and        */
/* applies the distortion using the LDPK model headers.            */
/*                                                                 */
/* Per-model parameter groups ("tabs") with keyframe baking from   */
/* .nk curves onto the effect parameters. The Model popup is the   */
/* single source of truth (works without a .nk file); only the     */
/* selected model group is visible. The parsed .nk node is         */
/* snapshotted into the sequence data — the project is             */
/* self-sufficient, renders never touch the file. Smart render     */
/* (SMART_PRE_RENDER/SMART_RENDER) with native 8/16/32 bpc.        */
/*                                                                 */
/* Structure follows the Adobe SDK samples (Resizer: buffer        */
/* expansion, Shifter: subpixel sampling, PathMaster: sequence     */
/* data flatten/unflatten).                                        */
/* Software is provided "as is" - no warranties implied.           */
/*                                                                 */
/*******************************************************************/

#pragma once

#ifndef LD3DE4_LENS_DISTORTION_H
    #define LD3DE4_LENS_DISTORTION_H

    #define PF_DEEP_COLOR_AWARE 1 // make sure we get 16bpc pixels; AE_Effect.h checks for this.
    #include "AEConfig.h"
    #include "entry.h"
    #ifdef AE_OS_WIN
        #include "string.h"
    #endif
    #include "AE_Effect.h"
    #include "AE_EffectCB.h"
    #include "AE_EffectCBSuites.h"
    #include "AE_Macros.h"
    #include "Param_Utils.h"
    #include "String_Utils.h"
    #include "AE_GeneralPlug.h"
    #include "AEGP_SuiteHandler.h"

    #ifdef AE_OS_WIN
        #ifndef NOMINMAX
            #define NOMINMAX
        #endif
        #include <Windows.h>
        #include <commdlg.h>
    #endif

    #include "../ldpk_nk_loader.h"
    #include "../ldpk_param_tables.h"

    #include <atomic>
    #include <string>

/* Versioning information.
   NOTE: when bumping the version, update ALL of:
   1. MAJOR/MINOR/BUG/STAGE/BUILD here
   2. AE_Effect_Version in LD3DE4_Lens_DistortionPiPL.r (numeric)
   3. OutDir/TargetName in Win/LD3DE4_Lens_Distortion.vcxproj (release folder/file name) */

    #define MAJOR_VERSION 1
    #define MINOR_VERSION 0
    #define BUG_VERSION 0
    #define STAGE_VERSION PF_Stage_RELEASE
    #define BUILD_VERSION 1

    #define LD_VERSION_STRING "1.0.0"
#ifdef LD_ENABLE_DEBUG_LOG
    #define LD_NAME "3DE4 Lens Distortion Debug"
#else
    #define LD_NAME "3DE4 Lens Distortion"
#endif
    #define LD_DESCRIPTION "Reads a 3DE4 lens distortion node (.nk block)\rand applies it via LDPK models."

/* Parameter layout */

    #define LD_NK_PATH_MAX 1024

/* Fixed params (always present, before the model groups).
   The Model popup is the single source of truth for the active
   model class. */

enum
{
    LD_INPUT = 0,      // default input layer
    LD_NK_BUTTON,      // "Choose .nk File..." button
    LD_MODEL_POPUP,    // popup: active model class (source of truth)
    LD_DIRECTION,      // popup: undistort / distort
    LD_FIT_COMP,       // "Fit Comp To Distortion" button
    LD_FIXED_NUM       // == index of the first model group topic
};

/* The model groups fill the params AFTER the fixed ones, so the total
   param count (AE's num_params) depends on the model tables. Computed
   at build time in this header via constexpr helpers and reused by
   the .cpp. */

constexpr int LD_GroupParamCount(int num_model_params)
{
    return num_model_params + 2; // topic + end
}

// Only UI-visible models get parameter groups (hidden models are still
// loadable from .nk files but have no sliders in the Effect Controls).
constexpr int LD_ModelGroupsParamCount()
{
    int total = 0;
    for (int i = 0; i < ae_ldpk::LD_NUM_MODELS; ++i)
    {
        if (!ae_ldpk::LD_MODELS[i].ui_visible)
        {
            continue;
        }
        total += LD_GroupParamCount(ae_ldpk::LD_MODELS[i].num_params);
    }
    return total;
}

// Total number of params (including the input layer at index 0):
// fixed params + per-model groups ([topic][params...][GROUP_END]).
constexpr int LD_NUM_PARAMS()
{
    return LD_FIXED_NUM + LD_ModelGroupsParamCount();
}

/* Disk ids: fixed params first, then per-model groups are assigned
   dynamically in ParamsSetup (disk id = 1000 + model index * 100 + param). */

enum
{
    LD_NK_BUTTON_DISK_ID = 1,
    LD_MODEL_POPUP_DISK_ID,
    LD_DIRECTION_DISK_ID,
    LD_FIT_COMP_DISK_ID
};

enum
{
    LD_DIR_UNDISTORT = 0, // remove distortion (3DE4 exporter default)
    LD_DIR_DISTORT = 1    // apply distortion
};

/* Sequence data. The flat variant contains no pointers and can be
   written to the project file / replicated across render threads.
   The unflat variant holds the snapshot blob on the heap. Pattern
   taken from the SDK sample PathMaster.

   The flat data carries a serialized snapshot of
   the parsed .nk node (blob_size bytes following the header), so the
   project is self-sufficient — renders never read the file, and a
   moved/missing .nk cannot change the picture. nk_path is kept for
   re-import and diagnostics only. Live-reload by mtime is gone:
   updating the lens means re-importing the file. */

typedef struct
{
    A_Boolean flatB;
    A_long direction;
    A_char nk_path[LD_NK_PATH_MAX]; // source of the snapshot (re-import, diagnostics)
    A_char model_class[128];        // model class from the snapshot
    A_long blob_size;               // bytes of the serialized node that follow
    /* unsigned char blob[blob_size]; — variable part of the handle */
} LD_FlatSeqData;

typedef struct
{
    A_Boolean flatB;
    A_long direction;
    A_char nk_path[LD_NK_PATH_MAX];
    A_char model_class[128];
    A_long blob_size;
    unsigned char* blob; // heap copy of the snapshot (new[]), NULL when none
} LD_UnflatSeqData;

/* Per-iteration refcon for the pixel callbacks */

struct LD_RenderInfo
{
    const ae_ldpk::distortion_from_nk* dist;
    PF_InData* in_data;
    PF_SampPB samp_pb;
    A_long in_w, in_h;
    A_long border_x, border_y;
    std::atomic<int>* rejected; // pixels outside the model image (diagnostics)
};

extern "C"
{

    DllExport PF_Err EffectMain(
        PF_Cmd cmd, PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output, void* extra);
}

#endif // LD3DE4_LENS_DISTORTION_H
