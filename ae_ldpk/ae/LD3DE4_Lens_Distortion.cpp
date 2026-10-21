/*******************************************************************/
/*                                                                 */
/* LD_3DE4 Lens Distortion for After Effects                       */
/*                                                                 */
/* Reads a 3DE4-exported .nk lens distortion node block and        */
/* applies the distortion using the LDPK model headers.            */
/* Per-model parameter groups ("tabs") with keyframe baking from   */
/* .nk curves onto the effect parameters.                          */
/*                                                                 */
/* Structure follows the Adobe SDK samples (Resizer: buffer        */
/* expansion, Shifter: subpixel sampling, PathMaster: sequence     */
/* data flatten/unflatten).                                        */
/* Software is provided "as is" - no warranties implied.           */
/*                                                                 */
/*******************************************************************/

#include "LD3DE4_Lens_Distortion.h"

#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <exception>
#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#ifdef AE_OS_MAC
    #include <sys/stat.h>
    #include <unistd.h> // confstr(_CS_DARWIN_USER_TEMP_DIR): macOS per-user temp folder
#endif

/* ---------------- helpers ---------------- */

static void LD_CopyPath(A_char* dst, size_t dst_size, const A_char* src)
{
#ifdef AE_OS_WIN
    strncpy_s(dst, dst_size, src ? src : "", _TRUNCATE);
#else
    strncpy(dst, src ? src : "", dst_size - 1);
    dst[dst_size - 1] = 0;
#endif
}

/* Diagnostic logging to %TEMP%\ld3de4_ae.log. Two builds come from these
   sources: the RELEASE build (default) compiles LD_Log into a no-op and
   never touches the log file; the DIAGNOSTIC build (MSBuild
   /p:LD_DEBUG_LOG=1, LD_ENABLE_DEBUG_LOG defined) always writes the full
   detail. Errors reach the user through return_msg in both builds. */

/* Plugin id for AEGP stream calls. Registered in GlobalSetup. */

static AEGP_PluginID G_plugin_id = 0;

// Log rotation: when the log exceeds LD_LOG_MAX_BYTES it is renamed to
// ld3de4_ae.log.1 (the previous .1 is overwritten) and a fresh log is
// started. One backup keeps at most ~2x the limit on disk while always
// preserving the most recent session for diagnostics. Checked before
// every append; the rename is cheap and happens at most once per
// LD_LOG_MAX_BYTES of output.
static const long long LD_LOG_MAX_BYTES = 10LL * 1024 * 1024;

#ifdef LD_ENABLE_DEBUG_LOG

/* Core writer (timestamped). */

static void LD_LogV(const char* fmt, va_list args)
{
#ifdef AE_OS_WIN
    // Multi-Frame Rendering renders on several threads at once.
    static std::mutex log_mutex;
    std::lock_guard<std::mutex> lock(log_mutex);

    char path[MAX_PATH];
    if (GetTempPathA(MAX_PATH, path) == 0)
    {
        return;
    }
    if (strncat_s(path, sizeof(path), "ld3de4_ae.log", _TRUNCATE) != 0)
    {
        return;
    }

    // Rotate when the current log grew past the limit.
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &fad))
    {
        const long long size =
            ((long long)fad.nFileSizeHigh << 32) | fad.nFileSizeLow;
        if (size > LD_LOG_MAX_BYTES)
        {
            char backup[MAX_PATH];
            if (strcpy_s(backup, sizeof(backup), path) == 0 &&
                strncat_s(backup, sizeof(backup), ".1", _TRUNCATE) == 0)
            {
                // A previous backup is silently overwritten (MoveFileEx).
                MoveFileExA(path, backup, MOVEFILE_REPLACE_EXISTING);
            }
        }
    }

    FILE* f = NULL;
    if (fopen_s(&f, path, "a") != 0 || !f)
    {
        return;
    }
    // Timestamp every entry — the log must be self-describing for support.
    {
        time_t now = time(NULL);
        struct tm tmv;
        localtime_s(&tmv, &now);
        char ts[32];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tmv);
        fprintf(f, "[%s] ", ts);
    }
    vfprintf(f, fmt, args);
    fputc('\n', f);
    fclose(f);
#elif defined(AE_OS_MAC)
    // Multi-Frame Rendering renders on several threads at once.
    static std::mutex log_mutex;
    std::lock_guard<std::mutex> lock(log_mutex);

    char path[1024];
    if (confstr(_CS_DARWIN_USER_TEMP_DIR, path, sizeof(path)) == 0)
    {
        return;
    }
    strlcat(path, "ld3de4_ae.log", sizeof(path));

    // Rotate when the current log grew past the limit.
    struct stat st;
    if (stat(path, &st) == 0 && st.st_size > LD_LOG_MAX_BYTES)
    {
        char backup[1024];
        strlcpy(backup, path, sizeof(backup));
        strlcat(backup, ".1", sizeof(backup));
        // A previous backup is silently overwritten (rename).
        rename(path, backup);
    }

    FILE* f = fopen(path, "a");
    if (!f)
    {
        return;
    }
    // Timestamp every entry — the log must be self-describing for support.
    {
        time_t now = time(NULL);
        struct tm tmv;
        localtime_r(&now, &tmv);
        char ts[32];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", &tmv);
        fprintf(f, "[%s] ", ts);
    }
    vfprintf(f, fmt, args);
    fputc('\n', f);
    fclose(f);
#else
    (void)fmt;
    (void)args;
#endif
}

// Always-on entry (diagnostic build).

static void LD_Log(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LD_LogV(fmt, args);
    va_end(args);
}

#else

// Release build: logging is compiled out, the log file is never touched.

static void LD_Log(const char* fmt, ...)
{
    (void)fmt;
}

#endif

/* Bounded write into out_data->return_msg. The host buffer is
   PF_MAX_EFFECT_MSG_LEN (255) bytes and PF_SPRINTF is a plain sprintf — a
   long .nk path or class name inside an error text overflowed it. The
   full text is formatted into a local buffer (and logged), then copied
   truncated into return_msg. */

static void LD_SetReturnMsg(PF_OutData* out_data, const char* fmt, ...)
{
    char full[2048];
    va_list args;
    va_start(args, fmt);
#ifdef AE_OS_WIN
    vsnprintf_s(full, sizeof(full), _TRUNCATE, fmt, args);
#else
    vsnprintf(full, sizeof(full), fmt, args);
#endif
    va_end(args);
    LD_Log("MSG: %s", full);
    LD_CopyPath(out_data->return_msg, sizeof(out_data->return_msg), full);
}

/* ---------------- .nk snapshot in the sequence data ----------------

   The parsed .nk node is serialized into the sequence data at import
   time, so the project is self-sufficient: renders never touch the file,
   a moved/renamed/missing .nk cannot change the picture, and the project
   renders identically on a farm. nk_path is kept for re-import and
   diagnostics only; updating the lens means re-importing the file. The blob
   format is documented next to nk_ld_node::serialize() in
   ldpk_nk_block.h. */

/* Forward declaration (defined below, used by LD_PrepareModel). */

static void LD_UpdateModelFromParams(int model_index, ae_ldpk::distortion_from_nk* dist, PF_ParamDef* params[]);

/* The snapshot blob follows the LD_FlatSeqData header inside the handle. */

static unsigned char* LD_FlatBlob(LD_FlatSeqData* flat)
{
    return reinterpret_cast<unsigned char*>(flat) + sizeof(LD_FlatSeqData);
}

static const unsigned char* LD_FlatBlobConst(const LD_FlatSeqData* flat)
{
    return reinterpret_cast<const unsigned char*>(flat) + sizeof(LD_FlatSeqData);
}

/* Read-only blob access for pre-render/render contexts. The sequence data
   handed to these contexts may arrive either FLATTENED (blob inline after
   the header) or as the LIVE UI-context unflat data (blob on the heap) —
   PF_GetConstSequenceData returns whatever the current context holds, and
   on the UI thread that is the unflat struct. Both layouts share the
   header prefix (flatB, direction, nk_path, model_class, blob_size), so
   flatB tells them apart reliably. Returns NULL + size 0 when there is no
   snapshot. */

static const unsigned char* LD_ConstBlobFromSeqData(const LD_FlatSeqData* flat, A_long& blob_size)
{
    blob_size = 0;
    if (!flat)
    {
        return NULL;
    }
    if (!flat->flatB)
    {
        // The live unflat data: the blob lives on the heap (the struct is
        // LD_UnflatSeqData — reading it as flat would hit the pointer).
        const LD_UnflatSeqData* unflat = reinterpret_cast<const LD_UnflatSeqData*>(flat);
        if (unflat->blob_size > 0 && unflat->blob)
        {
            blob_size = unflat->blob_size;
            return unflat->blob;
        }
        return NULL;
    }
    if (flat->blob_size > 0)
    {
        blob_size = flat->blob_size;
        return LD_FlatBlobConst(flat);
    }
    return NULL;
}

/* Current Nuke frame (1-based) for evaluating the snapshot's animated
   built-in parameters (e.g. an animated focus distance of a fisheye
   model). AE reports the frame duration in in_data->time_step
   (in time_scale units); Nuke frames are 1-based, AE times start at 0. */

static int LD_CurrentNukeFrame(const PF_InData* in_data)
{
    if (in_data->time_step <= 0 || in_data->time_scale == 0)
    {
        return 1;
    }
    const double frame = (double)in_data->current_time / (double)in_data->time_step;
    return (int)(frame + 0.5) + 1;
}

/* Read-only flattened sequence data. MFR-safe replacement for
   in_data->sequence_data, which is NULL on render threads. */

static const LD_FlatSeqData* LD_GetConstFlatData(PF_InData* in_data)
{
    PF_EffectSequenceDataSuite1* suite = NULL;
    SPBasicSuite* spb = in_data->pica_basicP;
    if (!spb)
    {
        return NULL;
    }
    const void* suite_ptr = NULL;
    if (spb->AcquireSuite(kPFEffectSequenceDataSuite, kPFEffectSequenceDataSuiteVersion1, &suite_ptr) || !suite_ptr)
    {
        return NULL;
    }
    suite = reinterpret_cast<PF_EffectSequenceDataSuite1*>(const_cast<void*>(suite_ptr));
    PF_ConstHandle h = NULL;
    PF_Err err = suite->PF_GetConstSequenceData(in_data->effect_ref, &h);
    spb->ReleaseSuite(kPFEffectSequenceDataSuite, kPFEffectSequenceDataSuiteVersion1);
    if (err || !h)
    {
        return NULL;
    }
    return reinterpret_cast<const LD_FlatSeqData*>(*h);
}

/* Map a popup (UI) selection, 1-based over the UI-visible models, to an
   LD_MODELS index. Returns -1 when out of range. The popup lists only the
   models of old/new 3DE; hidden LDPK models (experimental, toymodels,
   OpenCV, Brown-Conrady) remain loadable from .nk files but are not
   offered for manual selection. */

static int LD_PopupToModelIndex(A_long popup_value)
{
    return ae_ldpk::ld_ui_to_model((int)popup_value - 1); // popup is 1-based
}

/* Build the render model — the single source of truth for LD_PreRender
   (render borders + model stash), LD_SmartRender and
   LD_FitCompToDistortion:

   - no snapshot: the Model popup's class with LDPK defaults, FOV = [0..1]^2;
   - snapshot with a UI-visible class: the popup selects the model (the user
     may switch models after the import); coefficients come from the popup's
     parameter group; FOV and the 7 built-in parameters come from the snapshot;
   - snapshot with a hidden class (no group in the UI): the snapshot rules —
     class, coefficients, FOV and built-ins all come from it; the sliders are
     NOT pushed (they belong to another model's group).

   frame is the Nuke frame for the snapshot's animated built-ins. Returns an
   error message (empty on success); model_index receives the effective
   LD_MODELS index, coeffs_from_snapshot tells whether the sliders must be
   left alone. */

static std::string LD_PrepareModel(
    const LD_FlatSeqData* flat,
    PF_ParamDef* params[],
    int frame,
    ae_ldpk::distortion_from_nk& dist,
    int& model_index,
    bool& coeffs_from_snapshot)
{
    model_index = -1;
    coeffs_from_snapshot = false;

    model_index = LD_PopupToModelIndex(params[LD_MODEL_POPUP]->u.pd.value);
    if (model_index < 0)
    {
        return "invalid model selection";
    }

    A_long blob_size = 0;
    const unsigned char* blob = LD_ConstBlobFromSeqData(flat, blob_size);
    if (!blob)
    {
        // No snapshot: popup class with the CURRENT slider values (the
        // caller's params[]), LDPK defaults otherwise, FOV = [0..1]^2.
        std::string adopt_err = dist.adopt_class(ae_ldpk::LD_MODELS[model_index].nuke_class);
        if (!adopt_err.empty())
        {
            return adopt_err;
        }
        LD_UpdateModelFromParams(model_index, &dist, params);
        return "";
    }

    ae_ldpk::nk_ld_node node;
    std::string err;
    if (!node.deserialize(blob, (size_t)blob_size, err))
    {
        // Diagnostics: size + the first bytes of the blob (is it the
        // serialized node at all, or a foreign struct read as flat data?).
        char hex[3 * 16 + 1] = { 0 };
        const A_long n = flat->blob_size < 16 ? flat->blob_size : 16;
        for (A_long i = 0; i < n; ++i)
        {
            sprintf_s(hex + 3 * i, sizeof(hex) - 3 * i, "%02X ", LD_FlatBlobConst(flat)[i]);
        }
        unsigned long sum = 0;
        for (A_long i = 0; i < flat->blob_size; ++i)
        {
            sum += LD_FlatBlobConst(flat)[i];
        }
        LD_Log("PREPARE: deserialize FAILED size=%ld sum=%lu first=[%s]", (long)flat->blob_size, sum, hex);
        return "corrupt .nk snapshot: " + err;
    }
    int snap_index = -1;
    for (int m = 0; m < ae_ldpk::LD_NUM_MODELS; ++m)
    {
        if (node.nuke_class == ae_ldpk::LD_MODELS[m].nuke_class)
        {
            snap_index = m;
            break;
        }
    }
    if (snap_index < 0)
    {
        return "unknown node class: " + node.nuke_class;
    }
    if (ae_ldpk::LD_MODELS[snap_index].ui_visible)
    {
        // Popup is the source of truth for UI-visible classes: build the
        // popup's model on the snapshot's FOV/built-ins, then push the
        // popup group's (possibly keyframed) coefficients.
        err = dist.adopt_as(node, ae_ldpk::LD_MODELS[model_index].nuke_class, frame);
        if (!err.empty())
        {
            return err;
        }
        LD_UpdateModelFromParams(model_index, &dist, params);
    }
    else
    {
        // Hidden class: the snapshot rules, sliders are not pushed.
        err = dist.adopt(node, frame);
        if (!err.empty())
        {
            return err;
        }
        model_index = snap_index;
        coeffs_from_snapshot = true;
    }
    return "";
}

/* Forward declarations (defined later in this file). */

static PF_Err LD_BakeKeys(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[]);
static PF_Err LD_FitCompToDistortion(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[]);
static PF_Err UpdateParamsUI(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output);

/* Platform file dialog: Win32 GetOpenFileNameA / macOS NSOpenPanel
   (the macOS implementation lives in Mac/LD_FileDialog_mac.mm). AE has
   no built-in file-picker param type, so the path is chosen either from
   the supervised "Choose .nk File..." button (PF_Cmd_USER_CHANGED_PARAM)
   or from the Options button (PF_Cmd_DO_DIALOG). Both funnel into this
   helper with the same, single import path: parse and validate the file
   FIRST, then apply everything in one operation — path, snapshot,
   popup, keyframe baking. A failed import leaves the effect state
   unchanged. */

#ifdef AE_OS_MAC
/* Implemented in Mac/LD_FileDialog_mac.mm (Objective-C++, NSOpenPanel). */
extern bool LD_MacPickNkFile(char* path, size_t path_size);
#endif

static PF_Err LD_PickNkFile(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], A_Boolean bake_now)
{
    PF_Err err = PF_Err_NONE;

    LD_UnflatSeqData* seq = NULL;
    if (in_data->sequence_data)
    {
        seq = *reinterpret_cast<LD_UnflatSeqData**>(in_data->sequence_data);
    }
    if (!seq || seq->flatB)
    {
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    char file_name[LD_NK_PATH_MAX];
    file_name[0] = 0;
    if (seq->nk_path[0])
    {
        LD_CopyPath(file_name, sizeof(file_name), seq->nk_path);
    }

    bool picked = false;
#ifdef AE_OS_WIN
    OPENFILENAMEA ofn;
    AEFX_CLR_STRUCT(ofn);
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "Nuke Lens Distortion (*.nk)\0*.nk\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = LD_NK_PATH_MAX;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "nk";

    picked = GetOpenFileNameA(&ofn) ? true : false;
    if (!picked)
    {
        LD_Log("FILE PICK canceled (CommDlgExtendedError=%lu)", (unsigned long)CommDlgExtendedError());
    }
#elif defined(AE_OS_MAC)
    picked = LD_MacPickNkFile(file_name, sizeof(file_name));
    if (!picked)
    {
        LD_Log("FILE PICK canceled");
    }
#else
    (void)params;
    (void)bake_now;
#endif

    if (picked)
    {
        LD_Log("FILE PICKED: '%s'", file_name);

        // Parse and validate completely before touching the effect state.
        // The block may sit anywhere in the file.
        std::ifstream in(file_name);
        if (!in)
        {
            LD_SetReturnMsg(out_data, "LD_3DE4: cannot open file: %s", file_name);
            out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
            return PF_Err_BAD_CALLBACK_PARAM;
        }
        ae_ldpk::nk_ld_node node;
        std::string parse_err;
        if (!ae_ldpk::parse_nk_script(in, node, parse_err))
        {
            LD_Log("FILE PICKED: parse failed: %s", parse_err.c_str());
            LD_SetReturnMsg(out_data, "LD_3DE4: %s", parse_err.c_str());
            out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
            return PF_Err_BAD_CALLBACK_PARAM;
        }
        // Verbose dump of the parsed node (the support log carries the
        // full lens: class, direction, every parameter and key count).
        LD_Log("FILE PICKED: parsed class='%s' name='%s' direction='%s' params=%d",
               node.nuke_class.c_str(), node.name.c_str(), node.direction.c_str(), (int)node.params.size());
        for (std::map<std::string, ae_ldpk::nk_value>::const_iterator it = node.params.begin();
             it != node.params.end(); ++it)
        {
            if (it->second.animated)
            {
                LD_Log("FILE PICKED: param '%s' animated, %d key(s)", it->first.c_str(), (int)it->second.keys.size());
            }
            else
            {
                LD_Log("FILE PICKED: param '%s' static = %g", it->first.c_str(), it->second.static_value);
            }
        }
        int snap_index = -1;
        for (int m = 0; m < ae_ldpk::LD_NUM_MODELS; ++m)
        {
            if (node.nuke_class == ae_ldpk::LD_MODELS[m].nuke_class)
            {
                snap_index = m;
                break;
            }
        }
        if (snap_index < 0)
        {
            LD_SetReturnMsg(out_data, "LD_3DE4: unknown node class: %s", node.nuke_class.c_str());
            out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
            return PF_Err_BAD_CALLBACK_PARAM;
        }
        std::vector<unsigned char> blob;
        std::string ser_err;
        if (!node.serialize(blob, ser_err))
        {
            LD_SetReturnMsg(out_data, "LD_3DE4: cannot snapshot the node: %s", ser_err.c_str());
            out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
            return PF_Err_BAD_CALLBACK_PARAM;
        }

        // Validated — now apply everything in one operation.
        LD_CopyPath(seq->nk_path, sizeof(seq->nk_path), file_name);
        LD_CopyPath(seq->model_class, sizeof(seq->model_class), node.nuke_class.c_str());
        delete[] seq->blob;
        seq->blob = NULL;
        seq->blob_size = 0;
        if (!blob.empty())
        {
            seq->blob = new (std::nothrow) unsigned char[blob.size()];
            if (!seq->blob)
            {
                return PF_Err_OUT_OF_MEMORY;
            }
            memcpy(seq->blob, &blob[0], blob.size());
            seq->blob_size = (A_long)blob.size();
        }
        LD_Log("FILE PICKED: snapshot %ld bytes, class='%s'", (long)seq->blob_size, node.nuke_class.c_str());

        // Point the popup at the model class from the file — but only when
        // that model is offered in the UI. A hidden model (e.g. an
        // experimental fisheye) keeps the current popup selection; the
        // render still uses the snapshot's class.
        if (params)
        {
            if (ae_ldpk::LD_MODELS[snap_index].ui_visible)
            {
                int ui_index = ae_ldpk::ld_model_to_ui(snap_index);
                if (ui_index >= 0)
                {
                    params[LD_MODEL_POPUP]->u.pd.value = (PF_Fixed)(ui_index + 1); // popup is 1-based
                    params[LD_MODEL_POPUP]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
                    LD_Log("FILE PICKED: model popup -> %s", ae_ldpk::LD_MODELS[snap_index].ui_label);
                }
            }
            else
            {
                LD_Log("FILE PICKED: model '%s' is hidden from the UI, popup unchanged", node.nuke_class.c_str());
            }
            // Re-collapse the model groups right away (output is not used
            // by UpdateParamsUI, NULL is fine).
            ERR(UpdateParamsUI(in_data, out_data, params, NULL));
        }

        // Bake keys immediately: sliders/keyframes then carry the .nk values,
        // so renders (which read params[]) reproduce the distortion without
        // an extra manual step. Always the full curve range, LINEAR, no
        // offset.
        if (!err && bake_now && params)
        {
            err = LD_BakeKeys(in_data, out_data, params);
        }

        // AE 13.5+ splits UI and render threads: sequence_data changes made
        // in a UI selector only reach the render project when the effect
        // requests a forced re-render (see AE_Effect.h on FORCE_RERENDER).
        out_data->out_flags |= PF_OutFlag_FORCE_RERENDER | PF_OutFlag_REFRESH_UI;
    }

    return err;
}

/* ---------------- sequence data ---------------- */

static PF_Err LD_SequenceSetup(PF_InData* in_data, PF_OutData* out_data)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    if (out_data->sequence_data)
    {
        suites.HandleSuite1()->host_dispose_handle(out_data->sequence_data);
        out_data->sequence_data = NULL;
    }

    PF_Handle seqH = suites.HandleSuite1()->host_new_handle(sizeof(LD_UnflatSeqData));
    if (!seqH)
    {
        return PF_Err_OUT_OF_MEMORY;
    }

    LD_UnflatSeqData* seq = reinterpret_cast<LD_UnflatSeqData*>(suites.HandleSuite1()->host_lock_handle(seqH));
    if (!seq)
    {
        suites.HandleSuite1()->host_dispose_handle(seqH);
        return PF_Err_OUT_OF_MEMORY;
    }

    AEFX_CLR_STRUCT(*seq);
    seq->flatB = FALSE;
    seq->direction = LD_DIR_UNDISTORT;
    seq->blob = NULL;
    seq->blob_size = 0;
    LD_Log("SEQ: setup");

    out_data->sequence_data = seqH;
    suites.HandleSuite1()->host_unlock_handle(seqH);
    return err;
}

static PF_Err LD_SequenceSetdown(PF_InData* in_data, PF_OutData* out_data)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    if (in_data->sequence_data)
    {
        LD_UnflatSeqData* seq = *reinterpret_cast<LD_UnflatSeqData**>(in_data->sequence_data);
        if (seq && !seq->flatB && seq->blob)
        {
            delete[] seq->blob;
            seq->blob = NULL;
        }
        suites.HandleSuite1()->host_dispose_handle(in_data->sequence_data);
        out_data->sequence_data = NULL;
    }
    return err;
}

static PF_Err LD_SequenceFlatten(PF_InData* in_data, PF_OutData* out_data)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    if (in_data->sequence_data)
    {
        LD_UnflatSeqData* seq = *reinterpret_cast<LD_UnflatSeqData**>(in_data->sequence_data);
        if (seq && !seq->flatB)
        {
            // The flat handle is variable-length: header + snapshot blob.
            const A_long blob_size = (seq->blob_size > 0 && seq->blob) ? seq->blob_size : 0;
            PF_Handle flatH = suites.HandleSuite1()->host_new_handle(sizeof(LD_FlatSeqData) + blob_size);
            if (!flatH)
            {
                return PF_Err_OUT_OF_MEMORY;
            }
            LD_FlatSeqData* flat = reinterpret_cast<LD_FlatSeqData*>(suites.HandleSuite1()->host_lock_handle(flatH));
            if (!flat)
            {
                suites.HandleSuite1()->host_dispose_handle(flatH);
                return PF_Err_OUT_OF_MEMORY;
            }
            AEFX_CLR_STRUCT(*flat);
            flat->flatB = TRUE;
            flat->direction = seq->direction;
            LD_CopyPath(flat->nk_path, sizeof(flat->nk_path), seq->nk_path);
            LD_CopyPath(flat->model_class, sizeof(flat->model_class), seq->model_class);
            flat->blob_size = blob_size;
            if (blob_size > 0)
            {
                memcpy(LD_FlatBlob(flat), seq->blob, (size_t)blob_size);
            }
            LD_Log("SEQ: flatten blob=%ld", (long)blob_size);
            // Free the unflat data (the blob dies with its handle).
            suites.HandleSuite1()->host_dispose_handle(in_data->sequence_data);

            out_data->sequence_data = flatH;
            suites.HandleSuite1()->host_unlock_handle(flatH);
        }
    }
    return err;
}

static PF_Err LD_GetFlattenedSequenceData(PF_InData* in_data, PF_OutData* out_data)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    if (in_data->sequence_data)
    {
        LD_UnflatSeqData* seq = *reinterpret_cast<LD_UnflatSeqData**>(in_data->sequence_data);
        if (seq && !seq->flatB)
        {
            // The flat handle is variable-length: header + snapshot blob.
            const A_long blob_size = (seq->blob_size > 0 && seq->blob) ? seq->blob_size : 0;
            PF_Handle flatH = suites.HandleSuite1()->host_new_handle(sizeof(LD_FlatSeqData) + blob_size);
            if (!flatH)
            {
                return PF_Err_OUT_OF_MEMORY;
            }
            LD_FlatSeqData* flat = reinterpret_cast<LD_FlatSeqData*>(suites.HandleSuite1()->host_lock_handle(flatH));
            if (!flat)
            {
                suites.HandleSuite1()->host_dispose_handle(flatH);
                return PF_Err_OUT_OF_MEMORY;
            }
            AEFX_CLR_STRUCT(*flat);
            flat->flatB = TRUE;
            flat->direction = seq->direction;
            LD_CopyPath(flat->nk_path, sizeof(flat->nk_path), seq->nk_path);
            LD_CopyPath(flat->model_class, sizeof(flat->model_class), seq->model_class);
            flat->blob_size = blob_size;
            if (blob_size > 0)
            {
                memcpy(LD_FlatBlob(flat), seq->blob, (size_t)blob_size);
            }
            LD_Log("SEQ: get_flattened blob=%ld", (long)blob_size);
            // Keep the unflat data alive (do not dispose).
            out_data->sequence_data = flatH;
            suites.HandleSuite1()->host_unlock_handle(flatH);
        }
    }
    return err;
}

static PF_Err LD_SequenceResetup(PF_InData* in_data, PF_OutData* out_data)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    if (!in_data->sequence_data)
    {
        return LD_SequenceSetup(in_data, out_data);
    }

    LD_UnflatSeqData* seq = *reinterpret_cast<LD_UnflatSeqData**>(in_data->sequence_data);
    if (!seq)
    {
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    if (seq->flatB)
    {
        // Inflate the flat data back into the unflat form.
        LD_FlatSeqData* flat = reinterpret_cast<LD_FlatSeqData*>(seq);
        const A_long blob_size = (flat->blob_size > 0) ? flat->blob_size : 0;

        PF_Handle unflatH = suites.HandleSuite1()->host_new_handle(sizeof(LD_UnflatSeqData));
        if (!unflatH)
        {
            return PF_Err_OUT_OF_MEMORY;
        }
        LD_UnflatSeqData* unflat = reinterpret_cast<LD_UnflatSeqData*>(suites.HandleSuite1()->host_lock_handle(unflatH));
        if (!unflat)
        {
            suites.HandleSuite1()->host_dispose_handle(unflatH);
            return PF_Err_OUT_OF_MEMORY;
        }
        AEFX_CLR_STRUCT(*unflat);
        unflat->flatB = FALSE;
        unflat->direction = flat->direction;
        LD_CopyPath(unflat->nk_path, sizeof(unflat->nk_path), flat->nk_path);
        LD_CopyPath(unflat->model_class, sizeof(unflat->model_class), flat->model_class);
        unflat->blob_size = 0;
        unflat->blob = NULL;
        if (blob_size > 0)
        {
            unflat->blob = new (std::nothrow) unsigned char[blob_size];
            if (!unflat->blob)
            {
                suites.HandleSuite1()->host_unlock_handle(unflatH);
                suites.HandleSuite1()->host_dispose_handle(unflatH);
                return PF_Err_OUT_OF_MEMORY;
            }
            memcpy(unflat->blob, LD_FlatBlobConst(flat), (size_t)blob_size);
            unflat->blob_size = blob_size;
        }

        LD_Log("SEQ: resetup blob=%ld", (long)blob_size);

        // SDK (AE_Effect.h, PF_OutFlag_SEQUENCE_DATA_NEEDS_FLATTENING):
        // after unflattening, free the flattened handle.
        suites.HandleSuite1()->host_dispose_handle(in_data->sequence_data);

        out_data->sequence_data = unflatH;
        suites.HandleSuite1()->host_unlock_handle(unflatH);
        return err;
    }
    return err;
}

/* ---------------- params / global ---------------- */

static PF_Err About(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output)
{
    (void)in_data;
    (void)params;
    (void)output;
    LD_SetReturnMsg(out_data, "%s v%s\r\r%s", LD_NAME, LD_VERSION_STRING, LD_DESCRIPTION);
    return PF_Err_NONE;
}

static PF_Err GlobalSetup(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output)
{
    out_data->my_version = PF_VERSION(MAJOR_VERSION, MINOR_VERSION, BUG_VERSION, STAGE_VERSION, BUILD_VERSION);

    // Must match AE_Effect_Global_OutFlags in LD3DE4_Lens_DistortionPiPL.r (0x06000630).
    // PF_OutFlag_I_HAVE_EXTERNAL_DEPENDENCIES is not set — the .nk snapshot
    // lives in the sequence data, renders never touch the file.
    // The buffer expansion is declared by BOTH
    // I_EXPAND_BUFFER (the engine's hint for the layer extent / request
    // computation — without it AE clips the request to the layer raster)
    // and the pre-render result rects (LD_PreRender). USE_OUTPUT_EXTENT
    // is not set (the result rects govern).
    out_data->out_flags = PF_OutFlag_DEEP_COLOR_AWARE | PF_OutFlag_I_EXPAND_BUFFER |
                          PF_OutFlag_PIX_INDEPENDENT | PF_OutFlag_I_DO_DIALOG |
                          PF_OutFlag_SEQUENCE_DATA_NEEDS_FLATTENING | PF_OutFlag_SEND_UPDATE_PARAMS_UI;

    // Must match AE_Effect_Global_OutFlags_2 in LD3DE4_Lens_DistortionPiPL.r (0x8801409).
    // SUPPORTS_THREADED_RENDERING (Multi-Frame Rendering). Render never touches
    // in_data->sequence_data (it is NULL on render threads); the snapshot comes
    // from read-only flat data and each render builds its own model.
    // PARAM_GROUP_START_COLLAPSED_FLAG makes AE honor the initial
    // collapsed state of the model groups set in ParamsSetup.
    // SUPPORTS_SMART_RENDER + FLOAT_COLOR_AWARE — AE
    // requires the pair (internal verification failure 25::248 otherwise);
    // 8/16/32 bpc are dispatched by the checked-out worlds' pixel format
    // in LD_SmartRender.
    out_data->out_flags2 = PF_OutFlag2_SUPPORTS_QUERY_DYNAMIC_FLAGS |
                           PF_OutFlag2_SUPPORTS_GET_FLATTENED_SEQUENCE_DATA |
                           PF_OutFlag2_SUPPORTS_THREADED_RENDERING |
                           PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG |
                           PF_OutFlag2_SUPPORTS_SMART_RENDER |
                           PF_OutFlag2_FLOAT_COLOR_AWARE;

    LD_Log(
        "GLOBAL_SETUP: my_version=%lu out_flags=0x%lx out_flags2=0x%lx num_params=%d",
        (unsigned long)out_data->my_version,
        (unsigned long)out_data->out_flags,
        (unsigned long)out_data->out_flags2,
        (int)LD_NUM_PARAMS());

    // Register for AEGP (needed for keyframe baking). Same pattern as HistoGrid.
    if (in_data->appl_id != kAppID_Premiere && G_plugin_id == 0)
    {
        AEGP_SuiteHandler suites(in_data->pica_basicP);
        PF_Err reg_err = suites.UtilitySuite3()->AEGP_RegisterWithAEGP(NULL, "3DE4 Lens Distortion", &G_plugin_id);
        if (reg_err)
        {
            LD_Log("GLOBAL_SETUP: AEGP_RegisterWithAEGP failed err=%ld", (long)reg_err);
        }
        else
        {
            LD_Log("GLOBAL_SETUP: registered AEGP plugin id %ld", (long)G_plugin_id);
        }
    }

    return PF_Err_NONE;
}

/* Build one parameter from the model table. Names longer than
   PF_MAX_EFFECT_PARAM_NAME_LEN (31) are truncated. The macro
   PF_ADD_FLOAT_SLIDERX returns on error, hence the PF_Err return. */

static PF_Err LD_AddModelParam(PF_InData* in_data, PF_ParamDef& def, const ae_ldpk::ld_param_info& pi, A_long disk_id)
{
    char name[PF_MAX_EFFECT_PARAM_NAME_LEN + 1];
    strncpy_s(name, sizeof(name), pi.name, _TRUNCATE);

    // Precision 5 keeps small radial coefficients readable.
    PF_ADD_FLOAT_SLIDERX(name, pi.min_value, pi.max_value, pi.min_value, pi.max_value, pi.default_value, 5, 0, 0, disk_id);
    (void)def;
    return PF_Err_NONE;
}

static PF_Err ParamsSetup(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output)
{
    PF_Err err = PF_Err_NONE;
    PF_ParamDef def;

    AEFX_CLR_STRUCT(def);

    PF_ADD_BUTTON("Lens Distortion .nk File", "Choose .nk File...", 0, PF_ParamFlag_SUPERVISE, LD_NK_BUTTON_DISK_ID);

    // Model popup — the single source of truth for the active model class.
    // Works without a .nk file; picking a .nk overwrites it.
    // SUPERVISE so PF_Cmd_USER_CHANGED_PARAM fires on manual model switch:
    // AE does NOT send PF_Cmd_UPDATE_PARAMS_UI for a plain popup change,
    // so without this the model groups would never re-collapse.
    AEFX_CLR_STRUCT(def);
    {
        // Build the '|' separated choices string from the model table —
        // UI-visible models only (old/new 3DE set). Hidden models stay
        // loadable from .nk files but are not offered for manual selection.
        static std::string choices;
        if (choices.empty())
        {
            for (int m = 0; m < ae_ldpk::LD_NUM_MODELS; ++m)
            {
                if (!ae_ldpk::LD_MODELS[m].ui_visible)
                {
                    continue;
                }
                if (!choices.empty())
                {
                    choices += "|";
                }
                choices += ae_ldpk::LD_MODELS[m].ui_label;
            }
        }
        PF_ADD_POPUPX("Model", ae_ldpk::ld_num_ui_models(), 1, choices.c_str(), PF_ParamFlag_SUPERVISE, LD_MODEL_POPUP_DISK_ID);
    }

    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUP("Direction", 2, 1, "Undistort|Distort", LD_DIRECTION_DISK_ID);

    AEFX_CLR_STRUCT(def);
    PF_ADD_BUTTON("Fit Comp To Distortion", "Fit Comp To Distortion...", 0, PF_ParamFlag_SUPERVISE, LD_FIT_COMP_DISK_ID);

    // One collapsible group per UI-visible model ("tab"). All start
    // collapsed; UpdateParamsUI opens the one selected in the popup.
    // Hidden models get no group (no sliders in the Effect Controls).
    for (int m = 0; m < ae_ldpk::LD_NUM_MODELS; ++m)
    {
        if (!ae_ldpk::LD_MODELS[m].ui_visible)
        {
            continue;
        }
        const ae_ldpk::ld_model_info& mi = ae_ldpk::LD_MODELS[m];
        char group_name[PF_MAX_EFFECT_PARAM_NAME_LEN + 1];
        strncpy_s(group_name, sizeof(group_name), mi.ui_label, _TRUNCATE);

        AEFX_CLR_STRUCT(def);
        A_long group_id = 1000 + m * 100;
        PF_ADD_TOPICX(group_name, PF_ParamFlag_COLLAPSE_TWIRLY, group_id);
        for (int p = 0; p < mi.num_params; ++p)
        {
            ERR(LD_AddModelParam(in_data, def, mi.params[p], group_id + 1 + p));
        }
        PF_END_TOPIC(group_id);
    }

    out_data->num_params = LD_NUM_PARAMS();

    // Dump of the computed parameter layout (model group indices).
    A_long first = LD_FIXED_NUM;
    for (int m = 0; m < ae_ldpk::LD_NUM_MODELS; ++m)
    {
        if (!ae_ldpk::LD_MODELS[m].ui_visible)
        {
            continue;
        }
        LD_Log("PARAMS_SETUP: group '%s' at params[%ld..%ld]",
               ae_ldpk::LD_MODELS[m].ui_label, (long)first,
               (long)(first + ae_ldpk::LD_MODELS[m].num_params + 1));
        first += ae_ldpk::LD_MODELS[m].num_params + 2;
    }
    return err;
}

/* ---------------- baking (.nk curves -> AE keyframes) ---------------- */

/* Map .nk curve frames to comp times and insert keyframes on the
   parameter stream. All keys are LINEAR to match Nuke's default
   interpolation for exported curves.

   The key time uses the comp's exact frame duration
   (AEGP_GetCompFrameDuration), not an fps rounded to an integer.
   Existing keyframes are removed before writing, so a re-import REPLACES
   the old curve; a parameter missing from the new file is reset to the
   table default with its animation removed; LINEAR is applied to the
   added keys only. Baking runs only when the snapshot class matches the
   Model popup and is UI-visible — hidden classes have no sliders. The
   curves come from the sequence-data snapshot, not from the file. */

static PF_Err LD_BakeKeys(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[])
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    if (!in_data->sequence_data)
    {
        return PF_Err_BAD_CALLBACK_PARAM;
    }
    LD_UnflatSeqData* seq = *reinterpret_cast<LD_UnflatSeqData**>(in_data->sequence_data);
    if (!seq || seq->flatB)
    {
        return PF_Err_BAD_CALLBACK_PARAM;
    }
    if (seq->blob_size <= 0 || !seq->blob)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: import a .nk file before baking keys");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return PF_Err_BAD_CALLBACK_PARAM;
    }
    ae_ldpk::nk_ld_node node;
    std::string deser_err;
    if (!node.deserialize(seq->blob, (size_t)seq->blob_size, deser_err))
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: corrupt .nk snapshot: %s", deser_err.c_str());
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }

    // Baking rule: the snapshot class must match the Model
    // popup and be UI-visible; hidden classes have no parameter group.
    int model_index = LD_PopupToModelIndex(params[LD_MODEL_POPUP]->u.pd.value);
    if (model_index < 0)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: invalid model selection");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return PF_Err_BAD_CALLBACK_PARAM;
    }
    int snap_index = -1;
    for (int m = 0; m < ae_ldpk::LD_NUM_MODELS; ++m)
    {
        if (node.nuke_class == ae_ldpk::LD_MODELS[m].nuke_class)
        {
            snap_index = m;
            break;
        }
    }
    if (snap_index < 0)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: unknown node class in snapshot: %s", node.nuke_class.c_str());
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return PF_Err_BAD_CALLBACK_PARAM;
    }
    if (!ae_ldpk::LD_MODELS[snap_index].ui_visible)
    {
        LD_Log("BAKE: model '%s' is hidden from the UI - baking skipped", node.nuke_class.c_str());
        return PF_Err_NONE;
    }
    if (snap_index != model_index)
    {
        LD_Log("BAKE: snapshot class '%s' != popup model '%s' - baking skipped",
               node.nuke_class.c_str(), ae_ldpk::LD_MODELS[model_index].nuke_class);
        return PF_Err_NONE;
    }
    const ae_ldpk::ld_model_info* mi = &ae_ldpk::LD_MODELS[model_index];

    A_long first_param = LD_FIXED_NUM;
    for (int m = 0; m < model_index; ++m)
    {
        if (!ae_ldpk::LD_MODELS[m].ui_visible)
        {
            continue; // hidden models have no parameter group
        }
        first_param += ae_ldpk::LD_MODELS[m].num_params + 2; // topic + end
    }

    // Exact frame duration of the comp — rational, no fps
    // rounding. Fallback 1/25 s.
    A_Time frame_dur;
    frame_dur.value = 1;
    frame_dur.scale = 25;
    {
        AEGP_LayerH layerH = NULL;
        AEGP_CompH compH = NULL;
        if (!suites.PFInterfaceSuite1()->AEGP_GetEffectLayer(in_data->effect_ref, &layerH) && layerH)
        {
            if (!suites.LayerSuite5()->AEGP_GetLayerParentComp(layerH, &compH) && compH)
            {
                A_Time dur;
                if (!suites.CompSuite13()->AEGP_GetCompFrameDuration(compH, &dur) && dur.scale > 0 && dur.value > 0)
                {
                    frame_dur = dur;
                }
            }
        }
    }
    LD_Log("BAKE: frame duration = %ld/%ld s", (long)frame_dur.value, (long)frame_dur.scale);

    // Effect ref for stream access.
    AEGP_EffectRefH effectH = NULL;
    ERR(suites.PFInterfaceSuite1()->AEGP_GetNewEffectForEffect(G_plugin_id, in_data->effect_ref, &effectH));
    if (err || !effectH)
    {
        LD_Log("BAKE: AEGP_GetNewEffectForEffect failed (err=%ld, plugin_id=%ld)", (long)err, (long)G_plugin_id);
        LD_SetReturnMsg(out_data, "LD_3DE4: AEGP registration failed, cannot bake keys");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    // One undo group for the whole bake.
    ERR(suites.UtilitySuite3()->AEGP_StartUndoGroup("LD_3DE4: Bake Keys"));

    A_long keys_total = 0;
    LD_Log("BAKE: model='%s' first_param=%ld params=%d", mi->nuke_class, (long)first_param, mi->num_params);

    for (int p = 0; p < mi->num_params && !err; ++p)
    {
        const ae_ldpk::ld_param_info& pi = mi->params[p];
        const ae_ldpk::nk_value* v = node.find(ae_ldpk::nukify_name(std::string(pi.name)));

        PF_ParamDef* defP = params[first_param + 1 + p];
        A_long param_index = (A_long)(first_param + 1 + p);

        // Open the stream for EVERY parameter of the group: existing
        // keyframes must be removed even when the new value is static
        // (a static value must silence the old animation).
        AEGP_StreamRefH streamH = NULL;
        if (suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(
                G_plugin_id, effectH, param_index, &streamH) || !streamH)
        {
            LD_Log("BAKE: cannot get stream for param %ld - skipped", (long)param_index);
            continue;
        }

        // Re-import: remove the existing keyframes first (from
        // the end — indices shift), so the new curve replaces the old one.
        A_long num_kf = 0;
        if (!suites.KeyframeSuite4()->AEGP_GetStreamNumKFs(streamH, &num_kf) && num_kf > 0)
        {
            for (A_long k = num_kf - 1; k >= 0; --k)
            {
                PF_Err del_err = suites.KeyframeSuite4()->AEGP_DeleteKeyframe(streamH, k);
                if (del_err)
                {
                    LD_Log("BAKE: delete keyframe failed for param %ld, key %ld (err=%ld)",
                           (long)param_index, (long)k, (long)del_err);
                    break;
                }
            }
        }

        if (!v)
        {
            // Parameter absent from the new file: reset to the table
            // default, animation already removed above.
            defP->u.fs_d.value = (PF_FpLong)pi.default_value;
            defP->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
            LD_Log("BAKE: param '%s' absent from file - reset to default %g", pi.name, pi.default_value);
            suites.StreamSuite2()->AEGP_DisposeStream(streamH);
            continue;
        }

        // Static value: no keys needed, AE holds it in the param itself.
        if (!v->animated)
        {
            defP->u.fs_d.value = (PF_FpLong)v->static_value;
            defP->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
            ++keys_total;
            LD_Log("BAKE: param '%s' static = %g", pi.name, v->static_value);
            suites.StreamSuite2()->AEGP_DisposeStream(streamH);
            continue;
        }

        // Animated: write keyframes through AEGP.
        A_long keys_param = 0;
        AEGP_AddKeyframesInfoH akH = NULL;
        PF_Err ak_err = suites.KeyframeSuite4()->AEGP_StartAddKeyframes(streamH, &akH);
        if (ak_err || !akH)
        {
            LD_Log("BAKE: StartAddKeyframes failed for param %ld (err=%ld)", (long)param_index, (long)ak_err);
            suites.StreamSuite2()->AEGP_DisposeStream(streamH);
            continue;
        }

        PF_Err kf_err = PF_Err_NONE;
        for (size_t k = 0; k < v->keys.size() && !kf_err; ++k)
        {
            const A_long frame = v->keys[k].first;
            // Nuke frame -> comp time: (frame - 1) * frame duration, kept
            // rational in the A_Time (value/scale seconds).
            // Guard against an A_long overflow at absurd frame numbers.
            const long long t_val = (long long)(frame - 1) * (long long)frame_dur.value;
            if (t_val > (long long)0x7FFFFFFF)
            {
                LD_Log("BAKE: key time overflow at frame %ld - key skipped", (long)frame);
                continue;
            }
            A_Time t;
            t.scale = frame_dur.scale;
            t.value = (A_long)t_val;
            A_long key_index = -1;
            kf_err = suites.KeyframeSuite4()->AEGP_AddKeyframes(akH, AEGP_LTimeMode_CompTime, &t, &key_index);
            if (!kf_err && key_index >= 0)
            {
                // AEGP_StreamValue2 carries the stream ref — AE validates
                // it in AEGP_SetAddKeyframe ("AEGP magic error" otherwise).
                AEGP_StreamValue2 sv;
                AEFX_CLR_STRUCT(sv);
                sv.streamH = streamH;
                sv.val.one_d = v->keys[k].second;
                kf_err = suites.KeyframeSuite4()->AEGP_SetAddKeyframe(akH, key_index, &sv);
                if (!kf_err)
                {
                    ++keys_total;
                    ++keys_param;
                }
            }
            if (kf_err)
            {
                LD_Log("BAKE: keyframe failed for param %ld, key %d (err=%ld)", (long)param_index, (int)k, (long)kf_err);
            }
        }
        // Commit whatever was added (FALSE would discard all of it). Not
        // routed through ERR(): the stream must be disposed regardless.
        suites.KeyframeSuite4()->AEGP_EndAddKeyframes(kf_err ? FALSE : TRUE, akH);

        // LINEAR interpolation on the added keys only (the stream was empty
        // before, so these are all keys present now) — a re-import does not
        // clobber the interpolation of unrelated streams.
        if (!kf_err)
        {
            A_long num_kf_new = 0;
            if (!suites.KeyframeSuite4()->AEGP_GetStreamNumKFs(streamH, &num_kf_new) && num_kf_new > 0)
            {
                for (A_long k = 0; k < num_kf_new; ++k)
                {
                    if (suites.KeyframeSuite4()->AEGP_SetKeyframeInterpolation(
                            streamH, k, AEGP_KeyInterp_LINEAR, AEGP_KeyInterp_LINEAR))
                    {
                        break;
                    }
                }
            }
        }

        LD_Log("BAKE: param '%s' -> %ld key(s)", pi.name, (long)keys_param);
        suites.StreamSuite2()->AEGP_DisposeStream(streamH);
    }

    // End the undo group unconditionally: ERR() would return
    // early and leave the group open.
    {
        PF_Err undo_err = suites.UtilitySuite3()->AEGP_EndUndoGroup();
        if (!err)
        {
            err = undo_err;
        }
    }

    // Resource release must run regardless of errors.
    suites.EffectSuite5()->AEGP_DisposeEffect(effectH);

    if (!err)
    {
        // No informational popup — the result is visible in the
        // Effect Controls / comp; only the diagnostic log records it.
        LD_Log("BAKE: %ld keys on model '%s'", (long)keys_total, mi->nuke_class);
        out_data->out_flags |= PF_OutFlag_REFRESH_UI | PF_OutFlag_FORCE_RERENDER;
    }
    return err;
}

/* ---------------- render ---------------- */

/* Per-side pixel overshoot of the distorted unit bbox beyond the frame.
   Shared by LD_PreRender (render buffer) and LD_FitCompToDistortion
   (comp size) so both stay identical.
   top is yb>1, bottom is ya<0. NOTE: bottom must be -ya, NOT (1-ya):
   (1-ya) measures from the frame TOP and always eats ~one frame height.
   Overshoots below kUnitEps are FP noise of the model/FOV round-trip:
   ceil() turns even 1e-13 into a full pixel, which made an identity
   model (all sliders 0) report a 1px border — the comp could then never
   return to its original resolution. */
static void LD_SideBorders(double xa, double ya, double xb, double yb, A_long in_w, A_long in_h,
    A_long& left, A_long& top, A_long& right, A_long& bottom)
{
    left = top = right = bottom = 0;
    // Extreme fisheye coefficients can drive the LDPK bbox
    // accumulator to NaN/huge values (it ignores undistort() failures).
    // Sanitize BEFORE any conversion to A_long.
    if (!std::isfinite(xa) || !std::isfinite(ya) || !std::isfinite(xb) || !std::isfinite(yb))
    {
        return;
    }
    // Enforce ordering — a degenerate model may swap the corners.
    if (xa > xb)
    {
        std::swap(xa, xb);
    }
    if (ya > yb)
    {
        std::swap(ya, yb);
    }
    const double kUnitEps = 1e-6;
    double ov_left = std::max(0.0, -xa);
    double ov_top = std::max(0.0, yb - 1.0);
    double ov_right = std::max(0.0, xb - 1.0);
    double ov_bottom = std::max(0.0, -ya);
    if (ov_left < kUnitEps)
    {
        ov_left = 0.0;
    }
    if (ov_top < kUnitEps)
    {
        ov_top = 0.0;
    }
    if (ov_right < kUnitEps)
    {
        ov_right = 0.0;
    }
    if (ov_bottom < kUnitEps)
    {
        ov_bottom = 0.0;
    }
    // Clamp before the cast: a finite but absurd bbox must not overflow
    // A_long in ceil(); the callers clamp to kMaxBorder afterwards.
    const double kMaxOvershootPx = 1e9;
    double px;
    px = std::ceil(ov_left * in_w);
    left = (A_long)std::min(px, kMaxOvershootPx);
    px = std::ceil(ov_top * in_h);
    top = (A_long)std::min(px, kMaxOvershootPx);
    px = std::ceil(ov_right * in_w);
    right = (A_long)std::min(px, kMaxOvershootPx);
    px = std::ceil(ov_bottom * in_h);
    bottom = (A_long)std::min(px, kMaxOvershootPx);
}

/* Push the current (possibly keyframed) parameter values into the model.
   The model class comes from the Model popup (source of truth),
   so the caller passes the popup index instead of a class string. */

static void LD_UpdateModelFromParams(int model_index, ae_ldpk::distortion_from_nk* dist, PF_ParamDef* params[])
{
    if (model_index < 0 || model_index >= ae_ldpk::LD_NUM_MODELS)
    {
        return;
    }
    const ae_ldpk::ld_model_info* mi = &ae_ldpk::LD_MODELS[model_index];
    if (!dist || !dist->model())
    {
        return;
    }
    A_long first_param = LD_FIXED_NUM;
    for (int m = 0; m < model_index; ++m)
    {
        if (!ae_ldpk::LD_MODELS[m].ui_visible)
        {
            continue; // hidden models have no parameter group
        }
        first_param += ae_ldpk::LD_MODELS[m].num_params + 2; // topic + end
    }

    char name_par[256];
    int num_par;
    dist->model()->getNumParameters(num_par);
    for (int i = 0; i < num_par; ++i)
    {
        dist->model()->getParameterName(i, name_par);
        // Find the matching table entry (original names).
        for (int p = 0; p < mi->num_params; ++p)
        {
            A_long idx = first_param + 1 + p;
            if (idx < 0 || idx >= LD_NUM_PARAMS())
            {
                LD_Log("UPDATE_PARAMS: index %ld out of range (model='%s')", (long)idx, mi->nuke_class);
                break;
            }
            if (strcmp(name_par, mi->params[p].name) == 0)
            {
                PF_FpLong val = params[idx]->u.fs_d.value;
                dist->model()->setParameterValue(name_par, (double)val);
                break;
            }
        }
    }
    dist->model()->initializeParameters();
}

/* Same as LD_UpdateModelFromParams, but the values come from a table-order
   array (the coefficients stashed by LD_PreRender for SMART_RENDER, where
   params[] are not available). */

static void LD_UpdateModelFromValues(int model_index, ae_ldpk::distortion_from_nk* dist, const double* coeffs)
{
    if (model_index < 0 || model_index >= ae_ldpk::LD_NUM_MODELS)
    {
        return;
    }
    const ae_ldpk::ld_model_info* mi = &ae_ldpk::LD_MODELS[model_index];
    if (!dist || !dist->model() || !coeffs)
    {
        return;
    }
    char name_par[256];
    int num_par;
    dist->model()->getNumParameters(num_par);
    for (int i = 0; i < num_par; ++i)
    {
        dist->model()->getParameterName(i, name_par);
        for (int p = 0; p < mi->num_params; ++p)
        {
            if (strcmp(name_par, mi->params[p].name) == 0)
            {
                dist->model()->setParameterValue(name_par, coeffs[p]);
                break;
            }
        }
    }
    dist->model()->initializeParameters();
}

static PF_Err LD_RenderPixel8(void* refcon, A_long x, A_long y, PF_Pixel* in, PF_Pixel* out)
{
    LD_RenderInfo* ri = (LD_RenderInfo*)refcon;
    PF_Err err = PF_Err_NONE;

    // Output pixel center in input-buffer coordinates.
    double ox = x - ri->border_x + 0.5;
    double oy = y - ri->border_y + 0.5;

    // To unit coordinates (Nuke convention: origin bottom-left, y up).
    double ux = ox / ri->in_w;
    double uy = 1.0 - oy / ri->in_h;

    double qx, qy;
    if (ri->dist && ri->dist->apply_checked(ux, uy, qx, qy))
    {
        // Back to AE pixel coordinates (top-left origin, y down).
        double px = qx * ri->in_w - 0.5;
        double py = (1.0 - qy) * ri->in_h - 0.5;

        // LDPK's Newton iteration (eval_inv) diverges for strong
        // negative distortion (e.g. Degree 6 = -0.32): the polynomial folds
        // at large radii, the Jacobian degenerates and eval_inv() returns
        // NaN/huge coordinates while still reporting success. FLOAT2FIX
        // then overflows and subpixel_sample reads a random in-buffer
        // pixel -> salt-and-pepper noise at the frame border. Reject
        // non-finite or absurdly far points (legitimate points stay within
        // ~1 frame; the sampler fills out-of-bounds with transparency via
        // PF_SampleEdgeBehav_ZERO).
        if (!std::isfinite(px) || !std::isfinite(py) ||
            px < -(double)ri->in_w || px > 2.0 * (double)ri->in_w ||
            py < -(double)ri->in_h || py > 2.0 * (double)ri->in_h)
        {
            if (ri->rejected)
            {
                ri->rejected->fetch_add(1, std::memory_order_relaxed);
            }
            out->alpha = out->red = out->green = out->blue = 0;
            return PF_Err_NONE;
        }

        PF_Fixed fx = FLOAT2FIX(px);
        PF_Fixed fy = FLOAT2FIX(py);
        err = ri->in_data->utils->subpixel_sample(ri->in_data->effect_ref, fx, fy, &ri->samp_pb, out);
        if (err)
        {
            out->alpha = out->red = out->green = out->blue = 0;
            err = PF_Err_NONE;
        }
    }
    else
    {
        // apply_checked failed: the point is outside the model image.
        if (ri->rejected)
        {
            ri->rejected->fetch_add(1, std::memory_order_relaxed);
        }
        out->alpha = out->red = out->green = out->blue = 0;
    }
    return err;
}

static PF_Err LD_RenderPixel16(void* refcon, A_long x, A_long y, PF_Pixel16* in, PF_Pixel16* out)
{
    LD_RenderInfo* ri = (LD_RenderInfo*)refcon;
    PF_Err err = PF_Err_NONE;

    double ox = x - ri->border_x + 0.5;
    double oy = y - ri->border_y + 0.5;
    double ux = ox / ri->in_w;
    double uy = 1.0 - oy / ri->in_h;

    double qx, qy;
    if (ri->dist && ri->dist->apply_checked(ux, uy, qx, qy))
    {
        double px = qx * ri->in_w - 0.5;
        double py = (1.0 - qy) * ri->in_h - 0.5;

        // Reject diverged Newton results (see LD_RenderPixel8).
        if (!std::isfinite(px) || !std::isfinite(py) ||
            px < -(double)ri->in_w || px > 2.0 * (double)ri->in_w ||
            py < -(double)ri->in_h || py > 2.0 * (double)ri->in_h)
        {
            if (ri->rejected)
            {
                ri->rejected->fetch_add(1, std::memory_order_relaxed);
            }
            out->alpha = out->red = out->green = out->blue = 0;
            return PF_Err_NONE;
        }

        PF_Fixed fx = FLOAT2FIX(px);
        PF_Fixed fy = FLOAT2FIX(py);
        err = ri->in_data->utils->subpixel_sample16(ri->in_data->effect_ref, fx, fy, &ri->samp_pb, out);
        if (err)
        {
            out->alpha = out->red = out->green = out->blue = 0;
            err = PF_Err_NONE;
        }
    }
    else
    {
        // apply_checked failed: the point is outside the model image.
        if (ri->rejected)
        {
            ri->rejected->fetch_add(1, std::memory_order_relaxed);
        }
        out->alpha = out->red = out->green = out->blue = 0;
    }
    return err;
}

static PF_Err LD_RenderPixelFloat(void* refcon, A_long x, A_long y, PF_PixelFloat* in, PF_PixelFloat* out)
{
    LD_RenderInfo* ri = (LD_RenderInfo*)refcon;
    PF_Err err = PF_Err_NONE;

    double ox = x - ri->border_x + 0.5;
    double oy = y - ri->border_y + 0.5;
    double ux = ox / ri->in_w;
    double uy = 1.0 - oy / ri->in_h;

    double qx, qy;
    if (ri->dist && ri->dist->apply_checked(ux, uy, qx, qy))
    {
        double px = qx * ri->in_w - 0.5;
        double py = (1.0 - qy) * ri->in_h - 0.5;

        // Reject diverged Newton results (see LD_RenderPixel8).
        if (!std::isfinite(px) || !std::isfinite(py) ||
            px < -(double)ri->in_w || px > 2.0 * (double)ri->in_w ||
            py < -(double)ri->in_h || py > 2.0 * (double)ri->in_h)
        {
            if (ri->rejected)
            {
                ri->rejected->fetch_add(1, std::memory_order_relaxed);
            }
            out->alpha = out->red = out->green = out->blue = 0;
            return PF_Err_NONE;
        }

        PF_Fixed fx = FLOAT2FIX(px);
        PF_Fixed fy = FLOAT2FIX(py);
        AEGP_SuiteHandler suites(ri->in_data->pica_basicP);
        err = suites.SamplingFloatSuite1()->subpixel_sample_float(
            ri->in_data->effect_ref, fx, fy, &ri->samp_pb, out);
        if (err)
        {
            out->alpha = out->red = out->green = out->blue = 0;
            err = PF_Err_NONE;
        }
    }
    else
    {
        // apply_checked failed: the point is outside the model image.
        if (ri->rejected)
        {
            ri->rejected->fetch_add(1, std::memory_order_relaxed);
        }
        out->alpha = out->red = out->green = out->blue = 0;
    }
    return err;
}

/* ---------------- smart render ----------------

   AE drives smart effects through PF_Cmd_SMART_PRE_RENDER + PF_Cmd_SMART_RENDER
   instead of PF_Cmd_FRAME_SETUP + PF_Cmd_RENDER. The buffer expansion (the
   former PF_OutFlag_I_EXPAND_BUFFER) is expressed through the pre-render
   result rects; the pixel format (8/16/32 bpc) is queried from the
   checked-out worlds, which also report their origin in layer coordinates
   (PF_LayerDef.origin_x/y — "smart effect checkouts only").

   PreRender:
   - checks out the Model popup, Direction and the active group's sliders
     (AE auto-checks-in everything checked out here);
   - probes the input extent, then checks the input out over its FULL
     extent — the warp can fetch any pixel of the frame from any output
     pixel, a sub-rect request would produce transparent edges;
   - computes the borders exactly like the render will (LD_PrepareModel);
   - reports result_rect = max_result_rect = frame + borders and stashes
     the model selection + coefficient values for SMART_RENDER.

   SMART_RENDER rebuilds the model per call (LDPK models build lookup
   tables lazily — concurrent renders must not share one) from the
   snapshot + the stashed values, and iterates the output world with
   the pixel callbacks above. */

struct LD_PreRenderData
{
    int model_index;             // effective LD_MODELS index (-1: no model)
    bool coeffs_from_snapshot;   // hidden snapshot class: sliders not pushed
    A_long direction;            // 0 = undistort, 1 = distort
    double coeffs[ae_ldpk::LD_MAX_MODEL_PARAMS]; // active group values at this frame
};

static void LD_DeletePreRenderData(void* data)
{
    delete reinterpret_cast<LD_PreRenderData*>(data);
}

/* Checkout ids for the input layer (unique per layer, chosen by us). */

enum
{
    LD_CHECKOUT_PROBE = 1, // PreRender probe: learn the input's full extent
    LD_CHECKOUT_FULL = 2   // the real checkout: the input's whole extent
};

static PF_Err LD_PreRender(PF_InData* in_data, PF_OutData* out_data, PF_PreRenderExtra* extra)
{
    PF_Err err = PF_Err_NONE;

    const PF_LRect req_rect = extra->input->output_request.rect;
    if (req_rect.left >= req_rect.right || req_rect.top >= req_rect.bottom)
    {
        return PF_Err_NONE; // nothing requested
    }

    PF_RenderRequest req = extra->input->output_request;
    req.preserve_rgb_of_zero_alpha = TRUE; // we care about zero-alpha RGB
    req.field = PF_Field_FRAME;            // complete frames for sampling

    // Probe: learn the input's full extent (upstream effects may have
    // expanded it beyond the frame).
    PF_CheckoutResult in_probe;
    AEFX_CLR_STRUCT(in_probe);
    ERR(extra->cb->checkout_layer(in_data->effect_ref, LD_INPUT, LD_CHECKOUT_PROBE, &req,
        in_data->current_time, in_data->time_step, in_data->time_scale, &in_probe));

    // Full input checkout: the frame = the input's whole extent.
    PF_RenderRequest req_full = req;
    if (in_probe.max_result_rect.left < in_probe.max_result_rect.right &&
        in_probe.max_result_rect.top < in_probe.max_result_rect.bottom)
    {
        req_full.rect = in_probe.max_result_rect;
    }
    PF_CheckoutResult in_full;
    AEFX_CLR_STRUCT(in_full);
    ERR(extra->cb->checkout_layer(in_data->effect_ref, LD_INPUT, LD_CHECKOUT_FULL, &req_full,
        in_data->current_time, in_data->time_step, in_data->time_scale, &in_full));

    // Frame dimensions (the input's available extent; fall back to the
    // maximum/requested extent when the checkout came back empty).
    PF_LRect frame_rect = in_full.result_rect;
    if (!(frame_rect.left < frame_rect.right && frame_rect.top < frame_rect.bottom))
    {
        frame_rect = in_full.max_result_rect;
    }
    if (!(frame_rect.left < frame_rect.right && frame_rect.top < frame_rect.bottom))
    {
        frame_rect = req_rect;
    }
    const A_long frame_w = frame_rect.right - frame_rect.left;
    const A_long frame_h = frame_rect.bottom - frame_rect.top;

    // Parameter values at this frame: popup, direction, active group
    // (AE auto-checks-in everything checked out during PreRender).
    std::vector<PF_ParamDef> tmp(LD_NUM_PARAMS());
    for (size_t i = 0; i < tmp.size(); ++i)
    {
        AEFX_CLR_STRUCT(tmp[i]);
    }
    // LD_PrepareModel takes a PF_ParamDef*[] — point into tmp.
    std::vector<PF_ParamDef*> tmp_ptr(LD_NUM_PARAMS());
    for (size_t i = 0; i < tmp_ptr.size(); ++i)
    {
        tmp_ptr[i] = &tmp[i];
    }
    ERR(PF_CHECKOUT_PARAM(in_data, LD_MODEL_POPUP, in_data->current_time, in_data->time_step, in_data->time_scale, &tmp[LD_MODEL_POPUP]));
    ERR(PF_CHECKOUT_PARAM(in_data, LD_DIRECTION, in_data->current_time, in_data->time_step, in_data->time_scale, &tmp[LD_DIRECTION]));

    int popup_index = LD_PopupToModelIndex(tmp[LD_MODEL_POPUP].u.pd.value);
    A_long first_param = LD_FIXED_NUM;
    if (popup_index >= 0)
    {
        for (int m = 0; m < popup_index; ++m)
        {
            if (!ae_ldpk::LD_MODELS[m].ui_visible)
            {
                continue; // hidden models have no parameter group
            }
            first_param += ae_ldpk::LD_MODELS[m].num_params + 2; // topic + end
        }
        for (int p = 0; p < ae_ldpk::LD_MODELS[popup_index].num_params; ++p)
        {
            ERR(PF_CHECKOUT_PARAM(in_data, first_param + 1 + p, in_data->current_time, in_data->time_step, in_data->time_scale, &tmp[first_param + 1 + p]));
        }
    }

    // Borders — the same model construction the render will use
    // (one source of truth).
    A_long border_x = 0, border_y = 0;
    int model_index = -1;
    bool coeffs_from_snapshot = false;
    {
        const LD_FlatSeqData* flat = LD_GetConstFlatData(in_data);
        ae_ldpk::distortion_from_nk dist;
        std::string prep_err = LD_PrepareModel(flat, tmp_ptr.data(), LD_CurrentNukeFrame(in_data), dist, model_index, coeffs_from_snapshot);
        if (!prep_err.empty())
        {
            LD_Log("PRE_RENDER: model preparation failed (%s)", prep_err.c_str());
        }
        else if (frame_w > 0 && frame_h > 0)
        {
            const A_long direction = tmp[LD_DIRECTION].u.pd.value - 1;
            dist.set_direction(direction != LD_DIR_DISTORT);
            double xa, ya, xb, yb;
            if (dist.bounding_box(direction != LD_DIR_DISTORT, 0.0, 0.0, 1.0, 1.0, xa, ya, xb, yb))
            {
                A_long left, top, right, bottom;
                LD_SideBorders(xa, ya, xb, yb, frame_w, frame_h, left, top, right, bottom);
                // Exactly to the last pixel of the distorted bounds, no
                // safety padding; clamp pathological bboxes.
                const A_long kMaxBorder = 8192;
                border_x = std::min(std::max(left, right), kMaxBorder);
                border_y = std::min(std::max(top, bottom), kMaxBorder);
            }
        }
    }

    LD_Log("PRE_RENDER: model='%s' coeffs_from_snapshot=%d direction=%ld",
           model_index >= 0 ? ae_ldpk::LD_MODELS[model_index].nuke_class : "<none>",
           (int)coeffs_from_snapshot, (long)(tmp[LD_DIRECTION].u.pd.value - 1));

    // Output extent: the frame expanded by the borders (layer coordinates).
    extra->output->result_rect.left = frame_rect.left - border_x;
    extra->output->result_rect.top = frame_rect.top - border_y;
    extra->output->result_rect.right = frame_rect.right + border_x;
    extra->output->result_rect.bottom = frame_rect.bottom + border_y;
    extra->output->max_result_rect = extra->output->result_rect;
    extra->output->flags |= PF_RenderOutputFlag_RETURNS_EXTRA_PIXELS;

    LD_Log(
        "PRE_RENDER: req=(%ld,%ld)-(%ld,%ld) probe_max=(%ld,%ld)-(%ld,%ld) full_res=(%ld,%ld)-(%ld,%ld) frame=%ldx%ld borders=%ld,%ld reported=(%ld,%ld)-(%ld,%ld)",
        (long)req_rect.left, (long)req_rect.top, (long)req_rect.right, (long)req_rect.bottom,
        (long)in_probe.max_result_rect.left, (long)in_probe.max_result_rect.top,
        (long)in_probe.max_result_rect.right, (long)in_probe.max_result_rect.bottom,
        (long)in_full.result_rect.left, (long)in_full.result_rect.top,
        (long)in_full.result_rect.right, (long)in_full.result_rect.bottom,
        (long)frame_w, (long)frame_h, (long)border_x, (long)border_y,
        (long)extra->output->result_rect.left, (long)extra->output->result_rect.top,
        (long)extra->output->result_rect.right, (long)extra->output->result_rect.bottom);

    // Stash everything SMART_RENDER needs (params[] are not passed there).
    if (LD_PreRenderData* pd = new (std::nothrow) LD_PreRenderData())
    {
        pd->model_index = model_index;
        pd->coeffs_from_snapshot = coeffs_from_snapshot;
        pd->direction = tmp[LD_DIRECTION].u.pd.value - 1;
        if (popup_index >= 0)
        {
            for (int p = 0; p < ae_ldpk::LD_MODELS[popup_index].num_params; ++p)
            {
                pd->coeffs[p] = (double)tmp[first_param + 1 + p].u.fs_d.value;
            }
        }
        extra->output->pre_render_data = pd;
        extra->output->delete_pre_render_data_func = LD_DeletePreRenderData;
    }
    else
    {
        err = PF_Err_OUT_OF_MEMORY;
    }
    return err;
}

static PF_Err LD_SmartRender(PF_InData* in_data, PF_OutData* out_data, PF_SmartRenderExtra* extra)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    LD_PreRenderData* pd = reinterpret_cast<LD_PreRenderData*>(extra->input->pre_render_data);
    if (!pd)
    {
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    // Pixels for this render. The worlds report their origin in layer
    // coordinates — that is the whole coordinate contract we need.
    PF_EffectWorld* in_world = NULL;
    ERR(extra->cb->checkout_layer_pixels(in_data->effect_ref, LD_CHECKOUT_FULL, &in_world));
    if (err || !in_world)
    {
        return (err ? err : PF_Err_BAD_CALLBACK_PARAM);
    }
    PF_EffectWorld* out_world = NULL;
    ERR(extra->cb->checkout_output(in_data->effect_ref, &out_world));
    if (err || !out_world)
    {
        extra->cb->checkin_layer_pixels(in_data->effect_ref, LD_CHECKOUT_FULL);
        return (err ? err : PF_Err_BAD_CALLBACK_PARAM);
    }

    // Pixel format of THIS render (8/16/32 bpc). Fallback when the world
    // suite is unavailable: DEEP -> 16 bpc.
    PF_PixelFormat format = PF_PixelFormat_ARGB32;
    {
        SPBasicSuite* spb = in_data->pica_basicP;
        const void* suite_ptr = NULL;
        if (spb && !spb->AcquireSuite(kPFWorldSuite, kPFWorldSuiteVersion2, &suite_ptr) && suite_ptr)
        {
            PF_WorldSuite2* wsP = reinterpret_cast<PF_WorldSuite2*>(const_cast<void*>(suite_ptr));
            wsP->PF_GetPixelFormat(out_world, &format);
            spb->ReleaseSuite(kPFWorldSuite, kPFWorldSuiteVersion2);
        }
        else if (PF_WORLD_IS_DEEP(out_world))
        {
            format = PF_PixelFormat_ARGB64;
        }
    }

    // Rebuild the model per render (thread safety): the snapshot supplies
    // the class (for hidden ones), FOV and the 7 built-ins; the stashed
    // values stand in for params[] (not available in SMART_RENDER).
    const LD_FlatSeqData* flat = LD_GetConstFlatData(in_data);
    A_long blob_size = 0;
    const unsigned char* blob = LD_ConstBlobFromSeqData(flat, blob_size);
    ae_ldpk::distortion_from_nk dist;
    bool have_model = false;
    if (pd->model_index >= 0)
    {
        const char* model_class = ae_ldpk::LD_MODELS[pd->model_index].nuke_class;
        std::string build_err;
        if (blob)
        {
            ae_ldpk::nk_ld_node node;
            std::string deser_err;
            if (!node.deserialize(blob, (size_t)blob_size, deser_err))
            {
                build_err = "corrupt .nk snapshot: " + deser_err;
            }
            else
            {
                build_err = dist.adopt_as(node, model_class, LD_CurrentNukeFrame(in_data));
            }
        }
        else
        {
            build_err = dist.adopt_class(model_class);
        }
        if (build_err.empty())
        {
            if (!pd->coeffs_from_snapshot)
            {
                LD_UpdateModelFromValues(pd->model_index, &dist, pd->coeffs);
            }
            // Direction: undistort (popup 1) / distort (popup 2).
            dist.set_direction(pd->direction != LD_DIR_DISTORT);
            have_model = true;
        }
        else
        {
            LD_Log("SMART_RENDER: model build failed (%s)", build_err.c_str());
            LD_SetReturnMsg(out_data, "LD_3DE4: %s", build_err.c_str());
            out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        }
    }
    else
    {
        LD_Log("SMART_RENDER: no model from PreRender");
    }

    if (!have_model)
    {
        // No usable model (corrupt snapshot is the only realistic trigger —
        // imports are validated): transparent output + error message.
        PF_Pixel transparent;
        transparent.alpha = transparent.red = transparent.green = transparent.blue = 0;
        PF_FILL(&transparent, NULL, out_world);
        extra->cb->checkin_layer_pixels(in_data->effect_ref, LD_CHECKOUT_FULL);
        return PF_Err_NONE;
    }

    LD_RenderInfo ri;
    AEFX_CLR_STRUCT(ri);
    ri.dist = &dist;
    ri.in_data = in_data;
    ri.samp_pb.src = in_world;
    ri.samp_pb.x_radius = 0;
    ri.samp_pb.y_radius = 0;
    ri.samp_pb.area = 0;
    ri.samp_pb.samp_behave = PF_SampleEdgeBehav_ZERO;
    ri.samp_pb.allow_asynch = 0;
    std::atomic<int> rejected_cnt(0);
    ri.rejected = &rejected_cnt; // diagnostics: pixels outside the model image
    ri.in_w = in_world->width;
    ri.in_h = in_world->height;
    // The input world's (0,0) sits at (border_x, border_y) in the output
    // world (both origins are reported in layer coordinates).
    ri.border_x = in_world->origin_x - out_world->origin_x;
    ri.border_y = in_world->origin_y - out_world->origin_y;

    LD_Log(
        "SMART_RENDER: time=%ld model='%s' coeffs=%s in=%ldx%ld@(%ld,%ld) out=%ldx%ld@(%ld,%ld) dir=%s",
        (long)in_data->current_time,
        ae_ldpk::LD_MODELS[pd->model_index].nuke_class,
        pd->coeffs_from_snapshot ? "snapshot" : "params",
        (long)ri.in_w,
        (long)ri.in_h,
        (long)in_world->origin_x,
        (long)in_world->origin_y,
        (long)out_world->width,
        (long)out_world->height,
        (long)out_world->origin_x,
        (long)out_world->origin_y,
        pd->direction == LD_DIR_DISTORT ? "distort" : "undistort");

    // Iterate over the WHOLE output world. Plain iterate_origin CLIPS the
    // iteration to the src world placed at `origin`, leaving the rest
    // unfilled (transparent). iterate_origin_non_clip_src has the same
    // coordinate contract but does not clip to src; the callbacks never
    // read the `in` pixel — they resample samp_pb.src themselves.
    PF_Rect iter_rect;
    iter_rect.left = 0;
    iter_rect.top = 0;
    iter_rect.right = (A_short)out_world->width;
    iter_rect.bottom = (A_short)out_world->height;

    PF_Point origin;
    origin.h = (A_short)ri.border_x;
    origin.v = (A_short)ri.border_y;

    A_long linesL = out_world->height;

    if (format == PF_PixelFormat_ARGB128)
    {
        ERR(suites.IterateFloatSuite2()->iterate_origin_non_clip_src(
            in_data, 0, linesL, in_world, &iter_rect, &origin, (void*)&ri, LD_RenderPixelFloat, out_world));
    }
    else if (format == PF_PixelFormat_ARGB64)
    {
        ERR(suites.Iterate16Suite2()->iterate_origin_non_clip_src(
            in_data, 0, linesL, in_world, &iter_rect, &origin, (void*)&ri, LD_RenderPixel16, out_world));
    }
    else
    {
        ERR(suites.Iterate8Suite2()->iterate_origin_non_clip_src(
            in_data, 0, linesL, in_world, &iter_rect, &origin, (void*)&ri, LD_RenderPixel8, out_world));
    }

    LD_Log("SMART_RENDER: rejected=%d (outside the model image)",
           rejected_cnt.load(std::memory_order_relaxed));

    // Free the input pixels early (the output can be large).
    extra->cb->checkin_layer_pixels(in_data->effect_ref, LD_CHECKOUT_FULL);

    return err;
}

/* UpdateParamsUI: open the group selected in the Model popup, collapse
   the rest. The popup is the source of truth (works without a
   .nk file and follows manual model switching). */

static PF_Err UpdateParamsUI(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    (void)out_data;
    (void)output;

    // Active model = Model popup value (1-based over the UI-visible models).
    const int active_model = LD_PopupToModelIndex(params[LD_MODEL_POPUP]->u.pd.value);

    A_long p = LD_FIXED_NUM;
    for (int m = 0; m < ae_ldpk::LD_NUM_MODELS; ++m)
    {
        if (!ae_ldpk::LD_MODELS[m].ui_visible)
        {
            continue; // hidden models have no parameter group
        }
        const ae_ldpk::ld_model_info& mi = ae_ldpk::LD_MODELS[m];
        PF_ParamDef* group_def = params[p];
        const A_long new_flags = (m == active_model)
            ? (group_def->flags & ~PF_ParamFlag_COLLAPSE_TWIRLY)
            : (group_def->flags | PF_ParamFlag_COLLAPSE_TWIRLY);
        // Call PF_UpdateParamUI ONLY when the collapse state actually
        // changes (model switch). PF_Cmd_UPDATE_PARAMS_UI also arrives on
        // every comp-time change (SEND_UPDATE_PARAMS_UI); re-pushing the
        // group defs there would interrupt AE's own redraw of the animated
        // slider values, so the numbers in the Effect Controls would freeze
        // until the next click. Idempotent skip keeps that redraw alive.
        if (new_flags != group_def->flags)
        {
            group_def->flags = new_flags;
            group_def->uu.change_flags = PF_ChangeFlag_NONE;
            LD_Log("UPDATE_PARAMS_UI: group '%s' %s", mi.ui_label, (m == active_model) ? "opened" : "closed");
            // Apply immediately via PF_UpdateParamUI: in the
            // PF_Cmd_USER_CHANGED_PARAM context a plain flags change on
            // params[] is ignored — the SDK Supervisor sample routes every
            // cosmetic change through PF_UpdateParamUI (the only field it
            // honors in flags is PF_ParamFlag_COLLAPSE_TWIRLY).
            ERR(suites.ParamUtilsSuite3()->PF_UpdateParamUI(in_data->effect_ref, p, group_def));
        }
        p += mi.num_params + 2;
    }

    return err;
}

/* Fit Comp To Distortion: resize the composition canvas so it exactly
   covers the (expanded) output buffer of the effect —
   growing it when the distortion spills outside the frame and shrinking it
   when the distortion fits inside. The buffer is always expanded to the
   distorted frame bounds, but AE clips it to the comp frame — matching the
   comp size to the buffer makes the full undistorted image visible with no
   wasted pixels. Layers are NOT touched (pixels stay 1:1, the frame
   "grows" or "shrinks" padding around them). UI-thread only (AEGP).

   Animated distortion: when the model sliders (or the .nk curves)
   are keyframed, the comp is fitted to the UNION of the distorted bounds
   over every keyframe — the widest undistorted image across time — so the
   resized comp covers all frames, not just the current one. */

/* Collect the comp-frame times at which the distortion must be evaluated:
   every keyframe of every model slider of the active group (the user may
   edit keys by hand — the effect params are the source of truth), every
   keyframe of the .nk curves, plus the current time. Times are returned
   as A_Time in comp time (rational seconds), sorted and deduplicated.

   The .nk curve frames are converted through the
   comp's exact frame duration (not through time_scale), and the
   LD_FIT_MAX_SAMPLES limit applies to UNIQUE times — if there are still
   more, they are thinned evenly so the whole time range stays covered. */

static const size_t LD_FIT_MAX_SAMPLES = 2048;

static void LD_CollectFitTimes(
    AEGP_SuiteHandler& suites,
    AEGP_PluginID plugin_id,
    AEGP_EffectRefH effectH,
    A_long first_param,
    int num_params,
    const ae_ldpk::nk_ld_node* node, // NULL when there is no snapshot
    double frame_dur_secs,           // exact comp frame duration (seconds)
    PF_InData* in_data,
    std::vector<A_Time>& times)
{
    std::vector<A_FpLong> secs;

    // Current time first (always present). current_time is in comp time
    // units: value/time_scale seconds.
    secs.push_back((A_FpLong)in_data->current_time / (A_FpLong)(in_data->time_scale ? in_data->time_scale : 1));

    // Keyframes on the model sliders (AE streams).
    for (int p = 0; p < num_params; ++p)
    {
        AEGP_StreamRefH streamH = NULL;
        if (suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(
                plugin_id, effectH, first_param + 1 + p, &streamH) || !streamH)
        {
            continue;
        }
        A_long num_kf = 0;
        if (!suites.KeyframeSuite4()->AEGP_GetStreamNumKFs(streamH, &num_kf) && num_kf > 0)
        {
            for (A_long k = 0; k < num_kf; ++k)
            {
                A_Time t;
                if (!suites.KeyframeSuite4()->AEGP_GetKeyframeTime(streamH, k, AEGP_LTimeMode_CompTime, &t) && t.scale > 0)
                {
                    secs.push_back((A_FpLong)t.value / (A_FpLong)t.scale);
                }
            }
        }
        suites.StreamSuite2()->AEGP_DisposeStream(streamH);
    }

    // Keyframes of the .nk curves (frames are 1-based; convert
    // via the comp's frame duration, not via time_scale).
    if (node)
    {
        for (std::map<std::string, ae_ldpk::nk_value>::const_iterator i = node->params.begin();
             i != node->params.end(); ++i)
        {
            if (!i->second.animated)
            {
                continue;
            }
            for (size_t k = 0; k < i->second.keys.size(); ++k)
            {
                secs.push_back((A_FpLong)(i->second.keys[k].first - 1) * (A_FpLong)frame_dur_secs);
            }
        }
    }

    // Sort + dedup with a small epsilon (half a frame).
    std::sort(secs.begin(), secs.end());
    const A_FpLong eps = 0.5 * (A_FpLong)frame_dur_secs;
    times.clear();
    for (size_t i = 0; i < secs.size(); ++i)
    {
        if (!times.empty())
        {
            const A_FpLong prev = (A_FpLong)times.back().value / (A_FpLong)times.back().scale;
            if (secs[i] - prev < eps)
            {
                continue;
            }
        }
        A_Time t;
        t.scale = in_data->time_scale;
        t.value = (A_long)(secs[i] * (A_FpLong)in_data->time_scale + 0.5);
        times.push_back(t);
    }

    // The sample limit applies to the UNIQUE times; thin evenly
    // so the end of the animation is not silently dropped.
    if (times.size() > LD_FIT_MAX_SAMPLES)
    {
        std::vector<A_Time> thinned;
        thinned.reserve(LD_FIT_MAX_SAMPLES);
        const double step = (double)(times.size() - 1) / (double)(LD_FIT_MAX_SAMPLES - 1);
        for (size_t i = 0; i < LD_FIT_MAX_SAMPLES; ++i)
        {
            thinned.push_back(times[(size_t)((double)i * step + 0.5)]);
        }
        times.swap(thinned);
    }
}

/* Evaluate the model at one comp time and union its distorted bounds into
   xa/ya/xb/yb. For UI-visible classes the slider values are sampled at
   that time from the AE streams (keyframed or static); for hidden snapshot
   classes the coefficients already pushed by LD_PrepareModel (from the
   snapshot, evaluated at this frame) stay untouched. */

static bool LD_UnionBoundsAtTime(
    AEGP_SuiteHandler& suites,
    AEGP_PluginID plugin_id,
    AEGP_EffectRefH effectH,
    const ae_ldpk::ld_model_info* mi,
    A_long first_param,
    bool coeffs_from_snapshot, // true: the snapshot rules, sliders not pushed
    ae_ldpk::distortion_from_nk& dist,
    const A_Time& t,
    bool undistort_dir,
    double& xa, double& ya, double& xb, double& yb)
{
    if (!dist.model())
    {
        return false;
    }

    if (!coeffs_from_snapshot)
    {
        // Push slider values at time t into the model (hidden snapshot
        // classes keep the coefficients from the snapshot).
        char name_par[256];
        int num_par;
        dist.model()->getNumParameters(num_par);
        for (int i = 0; i < num_par; ++i)
        {
            dist.model()->getParameterName(i, name_par);
            for (int p = 0; p < mi->num_params; ++p)
            {
                if (strcmp(name_par, mi->params[p].name) != 0)
                {
                    continue;
                }
                AEGP_StreamRefH streamH = NULL;
                double val = mi->params[p].default_value;
                if (!suites.StreamSuite2()->AEGP_GetNewEffectStreamByIndex(
                        plugin_id, effectH, first_param + 1 + p, &streamH) && streamH)
                {
                    AEGP_StreamValue2 sv;
                    AEFX_CLR_STRUCT(sv);
                    if (!suites.StreamSuite6()->AEGP_GetNewStreamValue(
                            plugin_id, streamH, AEGP_LTimeMode_CompTime, &t, FALSE, &sv))
                    {
                        val = (double)sv.val.one_d;
                        suites.StreamSuite6()->AEGP_DisposeStreamValue(&sv);
                    }
                    suites.StreamSuite2()->AEGP_DisposeStream(streamH);
                }
                dist.model()->setParameterValue(name_par, val);
                break;
            }
        }
        dist.model()->initializeParameters();
    }

    double xa_f, ya_f, xb_f, yb_f;
    if (!dist.bounding_box(undistort_dir, 0.0, 0.0, 1.0, 1.0, xa_f, ya_f, xb_f, yb_f))
    {
        return false;
    }
    xa = std::min(xa, xa_f);
    ya = std::min(ya, ya_f);
    xb = std::max(xb, xb_f);
    yb = std::max(yb, yb_f);
    return true;
}

static PF_Err LD_FitCompToDistortion(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[])
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    // Popup model layout (for the slider keyframe scan below). For a hidden
    // snapshot class the sliders are not pushed; the scan is then harmless.
    int model_index = LD_PopupToModelIndex(params[LD_MODEL_POPUP]->u.pd.value);
    if (model_index < 0)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: invalid model selection");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    // Layer + comp handles first (needed for the layer size too).
    AEGP_LayerH layerH = NULL;
    ERR(suites.PFInterfaceSuite1()->AEGP_GetEffectLayer(in_data->effect_ref, &layerH));
    if (err || !layerH)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: cannot access the layer");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return err ? err : PF_Err_BAD_CALLBACK_PARAM;
    }
    AEGP_CompH compH = NULL;
    ERR(suites.LayerSuite5()->AEGP_GetLayerParentComp(layerH, &compH));
    if (err || !compH)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: cannot access the composition");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return err ? err : PF_Err_BAD_CALLBACK_PARAM;
    }
    AEGP_ItemH comp_itemH = NULL;
    ERR(suites.CompSuite4()->AEGP_GetItemFromComp(compH, &comp_itemH));
    if (err || !comp_itemH)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: cannot access the comp item");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return err ? err : PF_Err_BAD_CALLBACK_PARAM;
    }

    // Exact frame duration of the comp: rational frame <-> time
    // conversion for the .nk curve frames. Fallback 1/25 s.
    A_Time frame_dur;
    frame_dur.value = 1;
    frame_dur.scale = 25;
    {
        A_Time dur;
        if (!suites.CompSuite13()->AEGP_GetCompFrameDuration(compH, &dur) && dur.scale > 0 && dur.value > 0)
        {
            frame_dur = dur;
        }
    }
    const double frame_dur_secs = (double)frame_dur.value / (double)frame_dur.scale;

    // Layer size. NOTE: params[LD_INPUT]->u.ld.width/height are NOT filled
    // in the UI thread (only during render) — using them here gave
    // need=4x4 (0 + safety padding). The reliable source is the layer's
    // source item (same pattern as the SDK sample Artie).
    A_long in_w = 0, in_h = 0;
    {
        AEGP_ItemH src_itemH = NULL;
        ERR(suites.LayerSuite5()->AEGP_GetLayerSourceItem(layerH, &src_itemH));
        if (!err && src_itemH)
        {
            ERR(suites.ItemSuite1()->AEGP_GetItemDimensions(src_itemH, &in_w, &in_h));
        }
        if (err || in_w <= 0 || in_h <= 0)
        {
            // Fallback: comp dimensions (e.g. text/shape layers without
            // a meaningful source size).
            in_w = 0;
            in_h = 0;
            ERR(suites.ItemSuite1()->AEGP_GetItemDimensions(comp_itemH, &in_w, &in_h));
        }
        if (err || in_w <= 0 || in_h <= 0)
        {
            LD_SetReturnMsg(out_data, "LD_3DE4: cannot read layer dimensions");
            out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
            return err ? err : PF_Err_BAD_CALLBACK_PARAM;
        }
    }

    // Layer Scale at the CURRENT frame only. The effect renders in layer
    // space and AE applies the layer transform AFTER the effect, so the
    // distorted buffer is scaled by the layer's Scale when it lands in the
    // comp. Scale keyframes are deliberately NOT scanned: the comp size
    // itself cannot be animated in AE, so a union of scales over time
    // would be meaningless.
    double layer_scale_x = 1.0, layer_scale_y = 1.0;
    {
        AEGP_StreamRefH scaleH = NULL;
        A_Time fit_now;
        fit_now.value = in_data->current_time;
        fit_now.scale = in_data->time_scale;
        if (!suites.StreamSuite2()->AEGP_GetNewLayerStream(G_plugin_id, layerH, AEGP_LayerStream_SCALE, &scaleH) && scaleH)
        {
            AEGP_StreamValue2 val;
            AEFX_CLR_STRUCT(val);
            if (!suites.StreamSuite6()->AEGP_GetNewStreamValue(G_plugin_id, scaleH, AEGP_LTimeMode_CompTime, &fit_now, FALSE, &val))
            {
                AEGP_StreamType st = AEGP_StreamType_NO_DATA;
                if (!suites.StreamSuite2()->AEGP_GetStreamType(scaleH, &st) &&
                    (st == AEGP_StreamType_ThreeD_SPATIAL || st == AEGP_StreamType_ThreeD))
                {
                    layer_scale_x = (double)val.val.three_d.x / 100.0;
                    layer_scale_y = (double)val.val.three_d.y / 100.0;
                }
                else
                {
                    layer_scale_x = (double)val.val.two_d.x / 100.0;
                    layer_scale_y = (double)val.val.two_d.y / 100.0;
                }
                suites.StreamSuite6()->AEGP_DisposeStreamValue(&val);
            }
            suites.StreamSuite2()->AEGP_DisposeStream(scaleH);
        }
        // Flips are fine (absolute value); a collapsed layer (0%) has no
        // image to fit — keep the comp unchanged.
        layer_scale_x = std::fabs(layer_scale_x);
        layer_scale_y = std::fabs(layer_scale_y);
        if (layer_scale_x < 1e-6 || layer_scale_y < 1e-6)
        {
            LD_Log("FIT_COMP: degenerate layer scale %.4f,%.4f - comp unchanged", layer_scale_x, layer_scale_y);
            out_data->out_flags |= PF_OutFlag_REFRESH_UI;
            return PF_Err_NONE;
        }
    }

    const bool undistort_dir = ((params[LD_DIRECTION]->u.pd.value - 1) != LD_DIR_DISTORT);

    // Popup model layout (slider keyframe scan + per-time slider push).
    const ae_ldpk::ld_model_info* mi = &ae_ldpk::LD_MODELS[model_index];
    A_long first_param = LD_FIXED_NUM;
    for (int m = 0; m < model_index; ++m)
    {
        if (!ae_ldpk::LD_MODELS[m].ui_visible)
        {
            continue; // hidden models have no parameter group
        }
        first_param += ae_ldpk::LD_MODELS[m].num_params + 2; // topic + end
    }

    const LD_FlatSeqData* flat = LD_GetConstFlatData(in_data);

    // Snapshot node (if any) — for the .nk curve keyframe scan below.
    ae_ldpk::nk_ld_node snap_node;
    bool have_snap_node = false;
    {
        A_long snap_blob_size = 0;
        const unsigned char* snap_blob = LD_ConstBlobFromSeqData(flat, snap_blob_size);
        if (snap_blob)
        {
            std::string deser_err;
            have_snap_node = snap_node.deserialize(snap_blob, (size_t)snap_blob_size, deser_err);
            if (!have_snap_node)
            {
                LD_Log("FIT_COMP: corrupt snapshot (%s)", deser_err.c_str());
            }
        }
    }

    // Distorted bounds. With keyframes (on the sliders or in the .nk
    // curves) the comp is fitted to the UNION of the bounds over all
    // keyframes — the widest image across time. Without keyframes this
    // reduces to the single current-time bbox. The model is rebuilt per
    // time through LD_PrepareModel (one source of truth).
    double xa = 1e30, ya = 1e30, xb = -1e30, yb = -1e30;
    bool bounds_ok = false;
    {
        // Effect ref for stream access (keyframe times + values at times).
        AEGP_EffectRefH effectH = NULL;
        bool have_effect = !suites.PFInterfaceSuite1()->AEGP_GetNewEffectForEffect(G_plugin_id, in_data->effect_ref, &effectH) && effectH;

        std::vector<A_Time> times;
        if (have_effect)
        {
            LD_CollectFitTimes(suites, G_plugin_id, effectH, first_param, mi->num_params,
                have_snap_node ? &snap_node : NULL, frame_dur_secs, in_data, times);
        }
        LD_Log("FIT_COMP: %d sample time(s)", (int)times.size());

        for (size_t i = 0; i < times.size(); ++i)
        {
            // Convert comp time back to a 1-based Nuke frame
            // (via the exact frame duration).
            const double secs = (double)times[i].value / (double)times[i].scale;
            const int frame = (int)(secs / frame_dur_secs + 0.5) + 1;

            int model_index_i = -1;
            bool coeffs_from_snapshot = false;
            ae_ldpk::distortion_from_nk dist;
            if (!LD_PrepareModel(flat, params, frame, dist, model_index_i, coeffs_from_snapshot).empty())
            {
                continue;
            }
            // Honor the Direction popup: without set_direction the model
            // stays in its default undistort state and the bbox is computed
            // for the wrong direction.
            dist.set_direction(undistort_dir);
            if (LD_UnionBoundsAtTime(suites, G_plugin_id, effectH, &ae_ldpk::LD_MODELS[model_index_i],
                    first_param, coeffs_from_snapshot, dist, times[i], undistort_dir, xa, ya, xb, yb))
            {
                bounds_ok = true;
                LD_Log("FIT_COMP: t=%ld/%ld frame=%d bounds=[%g %g %g %g]",
                       (long)times[i].value, (long)times[i].scale, frame, xa, ya, xb, yb);
            }
        }
        if (effectH)
        {
            suites.EffectSuite5()->AEGP_DisposeEffect(effectH);
        }
    }
    if (!bounds_ok)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: bounding box failed");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return PF_Err_BAD_CALLBACK_PARAM;
    }

    // Required buffer size in pixels (same math as LD_PreRender):
    // the frame plus the expansion needed to cover the distorted bounds.
    // No +2 safety padding — the comp must match the render buffer
    // exactly (LD_PreRender never adds padding either).
    A_long left, top, right, bottom;
    LD_SideBorders(xa, ya, xb, yb, in_w, in_h, left, top, right, bottom);
    A_long exp_x = std::max(left, right);
    A_long exp_y = std::max(top, bottom);
    // Same clamp as LD_PreRender — the comp must match the render
    // buffer exactly, also for pathological bboxes.
    const A_long kMaxBorder = 8192;
    exp_x = std::min(exp_x, kMaxBorder);
    exp_y = std::min(exp_y, kMaxBorder);
    // Multiply by the layer's Scale (current frame) — AE
    // applies the layer transform after the effect, so the distorted buffer
    // occupies scale× as many comp pixels.
    const double need_w = ((double)in_w + 2.0 * exp_x) * layer_scale_x;
    const double need_h = ((double)in_h + 2.0 * exp_y) * layer_scale_y;

    // Current comp dimensions.
    A_long comp_w = 0, comp_h = 0;
    ERR(suites.ItemSuite1()->AEGP_GetItemDimensions(comp_itemH, &comp_w, &comp_h));
    if (err || comp_w <= 0 || comp_h <= 0)
    {
        LD_SetReturnMsg(out_data, "LD_3DE4: cannot read comp dimensions");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
        return err ? err : PF_Err_BAD_CALLBACK_PARAM;
    }

    // The comp is resized to EXACTLY the required buffer,
    // per dimension (no uniform scaling, which would overshoot one axis
    // and prevent returning to the original comp resolution).
    const A_long new_w = (A_long)std::ceil(need_w - 1e-6);
    const A_long new_h = (A_long)std::ceil(need_h - 1e-6);
    LD_Log("FIT_COMP: comp=%ldx%ld need=%.2fx%.2f (layer scale %.4f,%.4f) -> %ldx%ld",
           (long)comp_w, (long)comp_h, need_w, need_h, layer_scale_x, layer_scale_y, (long)new_w, (long)new_h);
    if (new_w == comp_w && new_h == comp_h)
    {
        // Comp already matches the distortion exactly — nothing to do,
        // no popup (informational messages are log-only).
        LD_Log("FIT_COMP: comp already matches the distortion");
        out_data->out_flags |= PF_OutFlag_REFRESH_UI;
        return PF_Err_NONE;
    }

    // One undo group for the resize + layer shift.
    ERR(suites.UtilitySuite3()->AEGP_StartUndoGroup("LD_3DE4: Fit Comp To Distortion"));
    ERR(suites.CompSuite4()->AEGP_SetCompDimensions(compH, new_w, new_h));

    // Shift every layer by half the growth so the content stays centered —
    // the same behavior as resizing the comp via Composition Settings.
    const A_long dx = (new_w - comp_w) / 2;
    const A_long dy = (new_h - comp_h) / 2;
    if (dx != 0 || dy != 0)
    {
        A_long num_layers = 0;
        ERR(suites.LayerSuite5()->AEGP_GetCompNumLayers(compH, &num_layers));
        A_Time now;
        now.value = in_data->current_time;
        now.scale = in_data->time_scale;
        for (A_long i = 0; i < num_layers && !err; ++i)
        {
            AEGP_LayerH layer_iH = NULL;
            ERR(suites.LayerSuite5()->AEGP_GetCompLayerByIndex(compH, i, &layer_iH));
            if (err || !layer_iH)
            {
                break;
            }
            // A layer with a parent is already dragged along by
            // the parent's shift — shifting it again would move it twice.
            AEGP_LayerH parentH = NULL;
            if (!suites.LayerSuite9()->AEGP_GetLayerParent(layer_iH, &parentH) && parentH)
            {
                continue;
            }
            AEGP_StreamRefH posH = NULL;
            ERR(suites.StreamSuite2()->AEGP_GetNewLayerStream(G_plugin_id, layer_iH, AEGP_LayerStream_POSITION, &posH));
            if (err || !posH)
            {
                err = PF_Err_NONE; // no position stream (rare) — skip the layer
                continue;
            }
            A_long num_kf = 0;
            ERR(suites.KeyframeSuite4()->AEGP_GetStreamNumKFs(posH, &num_kf));
            if (!err && num_kf == 0)
            {
                // Static position: read, shift, write back.
                AEGP_StreamValue2 val;
                AEFX_CLR_STRUCT(val);
                ERR(suites.StreamSuite6()->AEGP_GetNewStreamValue(G_plugin_id, posH, AEGP_LTimeMode_CompTime, &now, FALSE, &val));
                if (!err)
                {
                    AEGP_StreamType st = AEGP_StreamType_NO_DATA;
                    ERR(suites.StreamSuite2()->AEGP_GetStreamType(posH, &st));
                    if (!err)
                    {
                        if (st == AEGP_StreamType_ThreeD_SPATIAL || st == AEGP_StreamType_ThreeD)
                        {
                            val.val.three_d.x += dx;
                            val.val.three_d.y += dy;
                        }
                        else
                        {
                            val.val.two_d.x += dx;
                            val.val.two_d.y += dy;
                        }
                        ERR(suites.StreamSuite6()->AEGP_SetStreamValue(G_plugin_id, posH, &val));
                    }
                    suites.StreamSuite6()->AEGP_DisposeStreamValue(&val);
                }
            }
            else if (!err)
            {
                // Animated position: shift every keyframe.
                for (A_long k = 0; k < num_kf && !err; ++k)
                {
                    AEGP_StreamValue2 kval;
                    AEFX_CLR_STRUCT(kval);
                    ERR(suites.KeyframeSuite5()->AEGP_GetNewKeyframeValue(G_plugin_id, posH, k, &kval));
                    if (!err)
                    {
                        AEGP_StreamType st = AEGP_StreamType_NO_DATA;
                        ERR(suites.StreamSuite2()->AEGP_GetStreamType(posH, &st));
                        if (!err)
                        {
                            if (st == AEGP_StreamType_ThreeD_SPATIAL || st == AEGP_StreamType_ThreeD)
                            {
                                kval.val.three_d.x += dx;
                                kval.val.three_d.y += dy;
                            }
                            else
                            {
                                kval.val.two_d.x += dx;
                                kval.val.two_d.y += dy;
                            }
                            ERR(suites.KeyframeSuite5()->AEGP_SetKeyframeValue(posH, k, &kval));
                        }
                        suites.StreamSuite6()->AEGP_DisposeStreamValue(&kval);
                    }
                }
            }
            suites.StreamSuite2()->AEGP_DisposeStream(posH);
        }
        LD_Log("FIT_COMP: shifted %ld layers by (%ld,%ld)", (long)num_layers, (long)dx, (long)dy);
    }
    // End the undo group unconditionally: ERR() would return
    // early and leave the group open.
    {
        PF_Err undo_err = suites.UtilitySuite3()->AEGP_EndUndoGroup();
        if (!err)
        {
            err = undo_err;
        }
    }

    // No informational popup — the resize is visible in the comp;
    // only the diagnostic log records it.
    LD_Log("FIT_COMP: resized comp %ldx%ld -> %ldx%ld", (long)comp_w, (long)comp_h, (long)new_w, (long)new_h);
    out_data->out_flags |= PF_OutFlag_REFRESH_UI | PF_OutFlag_FORCE_RERENDER;
    return err;
}

// Distinct Match Name for the diagnostic build, so both builds can live
// in the AE Plug-ins folder at the same time (AE identifies plugins by
// the Match Name; duplicates are not loadable side by side).
#ifdef LD_ENABLE_DEBUG_LOG
    #define LD_MATCH_NAME "SDV LD_3DE4 Lens Distortion Debug"
#else
    #define LD_MATCH_NAME "SDV LD_3DE4 Lens Distortion"
#endif

extern "C" DllExport PF_Err PluginDataEntryFunction2(
    PF_PluginDataPtr inPtr,
    PF_PluginDataCB2 inPluginDataCallBackPtr,
    SPBasicSuite* inSPBasicSuitePtr,
    const char* inHostName,
    const char* inHostVersion)
{
    PF_Err result = PF_Err_INVALID_CALLBACK;

    result = PF_REGISTER_EFFECT_EXT2(
        inPtr,
        inPluginDataCallBackPtr,
        LD_NAME,                          // Name
        LD_MATCH_NAME,                    // Match Name
        "Lens Distortion",                // Category
        AE_RESERVED_INFO,                 // Reserved Info
        "EffectMain",                     // Entry point
        "https://www.sci-d-vis.com");     // support URL

    return result;
}

PF_Err EffectMain(
    PF_Cmd cmd, PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output, void* extra)
{
    PF_Err err = PF_Err_NONE;

    try
    {
        // Diagnostic trace (diagnostic build only; compiled out in the
        // release build). QUERY_DYNAMIC_FLAGS is omitted — AE fires it
        // hundreds of times per second.
        if (cmd != PF_Cmd_QUERY_DYNAMIC_FLAGS)
        {
            LD_Log("CMD: %d", (int)cmd);
        }
        switch (cmd)
    {
        case PF_Cmd_ABOUT:
            err = About(in_data, out_data, params, output);
            break;
        case PF_Cmd_GLOBAL_SETUP:
            LD_Log("GLOBAL_SETUP v%s", LD_VERSION_STRING);
            err = GlobalSetup(in_data, out_data, params, output);
            break;
        case PF_Cmd_PARAMS_SETUP:
            err = ParamsSetup(in_data, out_data, params, output);
            break;
        case PF_Cmd_SEQUENCE_SETUP:
            err = LD_SequenceSetup(in_data, out_data);
            break;
        case PF_Cmd_SEQUENCE_SETDOWN:
            err = LD_SequenceSetdown(in_data, out_data);
            break;
        case PF_Cmd_SEQUENCE_FLATTEN:
            err = LD_SequenceFlatten(in_data, out_data);
            break;
        case PF_Cmd_GET_FLATTENED_SEQUENCE_DATA:
            err = LD_GetFlattenedSequenceData(in_data, out_data);
            break;
        case PF_Cmd_SEQUENCE_RESETUP:
            err = LD_SequenceResetup(in_data, out_data);
            break;
        case PF_Cmd_DO_DIALOG:
            LD_Log("DO_DIALOG command received (Options button)");
            // The Options button follows the same import path as
            // the supervised .nk button (parse -> validate -> apply -> bake).
            err = LD_PickNkFile(in_data, out_data, params, true);
            break;
        case PF_Cmd_USER_CHANGED_PARAM:
        {
            // Supervised buttons arrive here, not as PF_Cmd_DO_DIALOG.
            const PF_UserChangedParamExtra* ucpe = reinterpret_cast<const PF_UserChangedParamExtra*>(extra);
            LD_Log("USER_CHANGED_PARAM for param %ld", (long)ucpe->param_index);
            // NOTE: ucpe->param_index is the INDEX into params[] (0-based,
            // 0 = input layer), NOT the disk id. Compare against the
            // computed indices of the supervised buttons.
            if (ucpe->param_index == LD_NK_BUTTON)
            {
                err = LD_PickNkFile(in_data, out_data, params, true);
            }
            else if (ucpe->param_index == LD_FIT_COMP)
            {
                err = LD_FitCompToDistortion(in_data, out_data, params);
            }
            else if (ucpe->param_index == LD_MODEL_POPUP)
            {
                // Manual model switch: re-collapse the model
                // groups immediately. AE does not send
                // PF_Cmd_UPDATE_PARAMS_UI for a plain popup change, so
                // update the group flags right here.
                err = UpdateParamsUI(in_data, out_data, params, output);
            }
            break;
        }
        case PF_Cmd_UPDATE_PARAMS_UI:
            err = UpdateParamsUI(in_data, out_data, params, output);
            break;
        case PF_Cmd_SMART_PRE_RENDER:
            err = LD_PreRender(in_data, out_data, reinterpret_cast<PF_PreRenderExtra*>(extra));
            break;
        case PF_Cmd_SMART_RENDER:
            err = LD_SmartRender(in_data, out_data, reinterpret_cast<PF_SmartRenderExtra*>(extra));
            break;
        default:
            break;
        }
    }
    catch (const PF_Err& thrown_err)
    {
        err = thrown_err;
    }
    catch (const std::exception& e)
    {
        // An escaping C++ exception must not cross the plugin
        // boundary.
        LD_Log("EXCEPTION: %s", e.what());
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    catch (...)
    {
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    return err;
}
