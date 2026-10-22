# 3DE4 Lens Distortion for After Effects

**Language:** English | [Русский](readme.ru.md)

A bridge between LDPK (Lens Distortion Plugin Kit, Science-D-Visions) and After Effects:
the effect reads a `.nk` file with a lens distortion node exported from 3DE4 by the
`ldpk-2.14.0/3de4/python/export_nuke_LD_3DE4_Lens_Distortion_Node.py` script and applies
the distortion to the layer using the header-only LDPK models.

## Contents

| File | Purpose |
|---|---|
| [`ldpk_nk_block.h`](ldpk_nk_block.h) | Parser of the `LD_3DE4_* { ... }` block from a `.nk` file: static parameters and `{{curve x<frame> <value>}}` curves with linear interpolation |
| [`ldpk_model_registry.h`](ldpk_model_registry.h) | Mapping "Nuke node name → LDPK class" (all current 3DE4 models) |
| [`ldpk_nk_loader.h`](ldpk_nk_loader.h) | Model loading from a file: 7 built-in parameters + model parameters + FOV mapping (as in the LDPK reference implementation) |
| [`nk_ld_dump.cpp`](nk_ld_dump.cpp) | Console utility: prints parameters and a coordinate grid — for cross-checking the model math |
| [`test_snapshot_roundtrip.cpp`](test_snapshot_roundtrip.cpp) | Test: .nk snapshot serialization (round-trip), block scanner within a full script, `apply`/`apply_checked` consistency |
| [`ae/LD3DE4_Lens_Distortion.h`](ae/LD3DE4_Lens_Distortion.h) / [`.cpp`](ae/LD3DE4_Lens_Distortion.cpp) | The After Effects (AEX) effect |
| [`ae/LD3DE4_Lens_DistortionPiPL.r`](ae/LD3DE4_Lens_DistortionPiPL.r) | Plugin PiPL resource |
| [`ae/Win/LD3DE4_Lens_Distortion.sln`](ae/Win/LD3DE4_Lens_Distortion.sln) / [`.vcxproj`](ae/Win/LD3DE4_Lens_Distortion.vcxproj) | Visual Studio 2022 project (v143, x64) |
| [`ae/Mac/LD_FileDialog_mac.mm`](ae/Mac/LD_FileDialog_mac.mm) / [`build_mac.sh`](ae/Mac/build_mac.sh) | macOS: NSOpenPanel file dialog and the universal `.bundle` build script |

## Building

Requires Visual Studio 2022 (toolset v143) with the "Desktop development with C++" workload.
The SDK lives in `AfterEffectsSDK_26.5_win`.

```cmd
:: console utility
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd ae_ldpk
cl /nologo /EHsc /MD /D_WINDOWS /I..\ldpk-2.14.0\include nk_ld_dump.cpp /Fe:nk_ld_dump.exe

:: AE plugin
set AE_PLUGIN_BUILD_DIR=%TEMP%\ae_ldpk_plugins
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" ^
  ae_ldpk\ae\Win\LD3DE4_Lens_Distortion.sln /p:Configuration=Release /p:Platform=x64

:: diagnostic build (the %TEMP%\ld3de4_ae.log log is always enabled)
"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" ^
  ae_ldpk\ae\Win\LD3DE4_Lens_Distortion.sln /p:Configuration=Release /p:Platform=x64 /p:LD_DEBUG_LOG=1
```

The diagnostic build gets a separate name ("3DE4 Lens Distortion
Debug") and Match Name (`SDV LD_3DE4 Lens Distortion Debug`) — both versions
can be kept in Plug-ins simultaneously.

The ready plugin: `releases/AE_LD3DE4/<version>/LD_3DE4_Lens_Distortion_v<version>.aex`
(the project OutDir) — copy it to
`C:\Program Files\Adobe\Adobe After Effects <version>\Support Files\Plug-ins\`.

## Building (macOS)

Requires Xcode command line tools and the After Effects SDK for macOS; only
three SDK folders are needed for compilation: `Examples/Headers`,
`Examples/Resources`, `Examples/Util`.

```bash
./ae_ldpk/ae/Mac/build_mac.sh release   # releases/AE_LD3DE4/<version>/LD_3DE4_Lens_Distortion_v<version>.bundle
./ae_ldpk/ae/Mac/build_mac.sh debug     # ..._debug.bundle, the always-on log
```

The SDK is auto-detected in `AfterEffectsSDK_26.5_mac` next to `ae_ldpk/`, or
set explicitly: `AE_SDK=<sdk root>` / `AE_SDK_HEADERS=<Headers dir>`. The
result is a universal binary (x86_64 + arm64); install by copying the
`.bundle` to `/Applications/Adobe After Effects <year>/Plug-ins/`. A
downloaded (not locally built) `.bundle` carries the Gatekeeper quarantine —
remove it right after unpacking: `xattr -dr com.apple.quarantine <name>.bundle`.

GitHub Actions builds both configurations on every push touching `ae_ldpk/`
([`.github/workflows/build-mac.yml`](../.github/workflows/build-mac.yml)). The
workflow downloads the SDK from a private repository (the SDK license forbids
open redistribution) — the one-time setup steps are listed in the workflow
header.

## Usage in AE

1. Apply the **Lens Distortion → 3DE4 Lens Distortion** effect to a layer.
2. Pick a model in the **Model** drop-down — the effect works immediately,
   without a .nk file (model parameters and FOV = [0..1]² — the LDPK defaults).
3. Press **Choose .nk File...** and pick a file exported from 3DE4
   (or a full `.nk` script containing an `LD_3DE4_*` node — the block is found
   inside it): the Model popup switches to the model from the file, the static
   parameters are filled in, and the .nk curves are automatically placed as keys
   (LINEAR, the whole curve range, no offset).
   The file is parsed **before** being applied: on error the effect state does
   not change. A successful import stores a **snapshot** of the parsed node inside
   the project — the file is no longer needed (see "The .nk snapshot" below).
4. **Direction**: `Undistort` (remove the distortion — the exporter's default)
   or `Distort` (apply it back).
5. **Fit Comp To Distortion...**: fits the composition canvas exactly to the image
   after the effect is applied — enlarges when the distortion exceeds the frame and
   shrinks when the distorted image occupies fewer pixels than the frame. Layers
   are not touched (pixels stay 1:1, the frame "grows" or "shrinks" margins). With
   animated distortion (slider keys or .nk curves) the canvas is fitted to the
   union of the boundaries across all keyframes — the widest image over time. The
   layer Scale is taken into account (the value at the current frame): the effect
   renders in layer space, AE applies layer transforms after effects, so a
   non-precomposed downscaled layer yields a proportionally smaller comp gain.
   Scale animation is not scanned — the comp size cannot be animated in AE.

### Buffer and coverage

- The effect output buffer **always** expands to the distorted frame boundaries
  (LDPK `getBoundingBox*`; smart render: PreRender reports
  `result_rect`/`max_result_rect` = frame + expansion).
- AE crops the expanded buffer to the composition frame, so before pressing
  **Fit Comp To Distortion...** the picture looks like the "Original Frame".
- After pressing the button the comp becomes exactly the buffer size (no margin) —
  the full undistorted image is visible entirely, no extra pixels.
- With animation, Fit Comp scans all keys of the effect sliders and .nk curves
  and fits the comp to the union of the distorted boundaries (up to 2048 scan points).
- The layer Scale is taken into account by its value at the current frame (no key
  scanning: the comp size cannot be animated in AE). The required comp size is
  multiplied by the layer Scale; Scale 0% — the comp does not change.
- The comp size is fitted exactly per side (no proportional scaling). With zero
  coefficients the button returns the comp to the layer size taking Scale into
  account; FP noise of the bbox (< 1e-6 in unit coordinates) is cut off so that
  an identity model yields exactly the original size without an extra pixel.
- Limitation: the render output buffer itself expands by the current frame —
  on frames with less distortion the comp edges will be transparent (no image
  there), which is correct.

## Coordinates

- LDPK/`.nk`: unit coordinates, (0,0) — bottom-left corner, y up.
- AE: pixels, (0,0) — top-left corner, y down.
- The conversion happens in the pixel callbacks
  [`LD_RenderPixel8/16/Float`](ae/LD3DE4_Lens_Distortion.cpp):
  `uy = 1 - oy/h` and back `py = (1-qy)*h - 0.5`.
- Sampling is subpixel (`subpixel_sample` / `subpixel_sample16` /
  `subpixel_sample_float`), as in the SDK Shifter example.

## Supported models

The UI (Model popup and parameter groups) shows 10 models corresponding
to the built-in set of old and new 3DE:

`LD_3DE_Classic_LD_Model`, `LD_3DE4_Radial_Standard_Degree_4`,
`LD_3DE4_Radial_Stabilizer_Degree_6`, `LD_3DE4_Anamorphic_Degree_6`,
`LD_3DE4_Anamorphic_Standard_Degree_4/6`, `LD_3DE4_Anamorphic_Rescaled_Degree_4/6`,
`LD_3DE4_Anamorphic_Stabilizer_Degree_6`, `LD_3DE4_Radial_Fisheye_Degree_8`.

Hidden from the UI (the `ui_visible = false` flag in
[`ldpk_param_tables.h`](ldpk_param_tables.h)) but still registered and loaded
from `.nk` files: `LD_3DE4_Brown_Conrady_Degree_6_2`,
`LD_3DE4_Radial_Fisheye_{Equidistant,Equisolid,Orthographic,Stereographic}_Degree_8`
(experimental), `LD_3DE4_{Radial,Anamorphic}_Homomorphic_Degree_2` (toymodels),
`LD_OpenCV_Radial_Degree_6_Decentered_Degree_2`. If such a class occurs in a
`.nk`, the effect renders it with the parameters from the file; the Model popup
does not switch.

Not supported (obsolete/incompatible with MSVC): `LD_3DE4_Noop_Debugging`
(zero-length array — C2466), `LD_3DE4_Radial_Fisheye_Degree_8_Release_1`.

## The .nk snapshot

- On import the parsed node is serialized into the effect sequence data
  (a binary blob, ~1 KB per static node, tens of KB with animation).
  The project is **self-sufficient**: the render never touches the .nk file;
  moving the project and rendering on a farm give the same geometry; losing
  the file does not change the image.
- The snapshot is the source of truth; updating the lens means
  re-importing the file (the button or Options). Re-import replaces the old
  keys with new ones; a parameter missing from the new file is reset to the
  table default and its animation is removed. The Options button follows the
  same path as the file button (with baking).
- A single model source for the render, buffer boundaries and Fit Comp
  (`LD_PrepareModel`): the class — from the popup if the snapshot class is
  visible in the UI, otherwise from the snapshot (hidden classes); the
  coefficients — from the selected model group (visible) or from the snapshot
  (hidden, sliders are not pushed); FOV and the 7 built-in camera parameters —
  from the snapshot, without a snapshot — the LDPK defaults. Changing Model
  after an import switches the model; coefficients are taken from the selected
  model group; FOV and camera parameters — from the .nk.
- Baking happens only when the snapshot class matches the popup and is visible
  in the UI; for hidden classes — skipped with a log entry.
- The key time during baking uses the exact comp frame duration
  (`AEGP_GetCompFrameDuration`), without fps rounding.

## Limitations

- Parameter curves are interpolated linearly (the 3DE4 exporter writes keys
  without spline handles; linear is the default interpolation for such curves).
- FOV animation is not supported (FOV is static in the export; without a .nk
  FOV = [0..1]²). Animated built-in camera parameters (e.g. the fisheye focus)
  are supported — the snapshot stores their curves.
- The composition pixel aspect is assumed to be 1.
- The render is smart render (`SMART_PRE_RENDER`/`SMART_RENDER`): 8/16/32 bpc
  are supported natively, the format is determined by the issued worlds
  (`PF_GetPixelFormat`), 32 bpc via float sampling. AE requires the
  `SUPPORTS_SMART_RENDER` + `FLOAT_COLOR_AWARE` flag pair (otherwise 25::248).
- Fit Comp takes the layer Scale at the current frame into account; layer
  rotation and off-center layer position are not taken into account.
- For an exact cross-check of the model math use `nk_ld_dump.exe`
  (prints the parsed parameters and a mapping grid).

## License

The bridge code is "as is", without warranties. The LDPK models are free software
(see `license` in the LDPK root); the `tde4_ld_plugin.h` header belongs to
Science-D-Visions. The Adobe SDK examples are used as a structural reference
(Resizer, Shifter, PathMaster, Gamma_Table).
