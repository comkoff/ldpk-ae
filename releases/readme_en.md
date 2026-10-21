# Releases

Ready-to-use builds are published on the **GitHub Releases** page:
<https://github.com/comkoff/ldpk-ae/releases>

The repository tree contains sources and documentation only, no binaries.

**Language:** English | [Русский](readme.md)

## Release archive contents (AE_LD3DE4)

`LD_3DE4_Lens_Distortion_v<version>.zip`:

| File | Description |
|---|---|
| `LD_3DE4_Lens_Distortion_v<version>.aex` | release build (no log) |
| `LD_3DE4_Lens_Distortion_v<version>_debug.aex` | diagnostic build (log always enabled) |
| `readme_user.md` | user manual (Russian) |
| `readme_user_en.md` | user manual (English) |

## AE_LD3DE4

The **3DE4 Lens Distortion** plugin for After Effects (category
"Lens Distortion", Match Name `SDV LD_3DE4 Lens Distortion`).

- Reads a `.nk` file with a lens distortion node exported from 3DE4
  and applies the distortion through the header-only LDPK models.
- **Model popup is the source of truth**: the effect works right after
  being applied, without a .nk file (model parameters are the LDPK
  defaults, FOV = [0..1]², built-in parameters — Nuke node defaults).
  Choosing a .nk file switches the popup, fills the parameters and
  automatically places keys from the curves (LINEAR, full curve range).
- **The UI shows 10 models** (the set of old and new 3DE): experimental,
  toymodel and OpenCV LDPK models are hidden from the Model popup and
  Effect Controls, but stay in the registry and load from .nk files.
- **Direction**: Undistort / Distort.
- **Fit Comp To Distortion...** fits the composition canvas exactly to
  the distorted image: enlarges when the distortion exceeds the frame,
  shrinks when the image occupies fewer pixels than the frame. Layers
  are never moved twice (parenting is taken into account). With animated
  distortion the canvas is fitted to the widest image across all keyframes.
- The imported node is stored as a **snapshot** inside the project: the
  render never touches the .nk file, the project is self-sufficient and
  farm-friendly.
- **Multi-Frame Rendering**: the render is thread-safe, frames render
  in parallel.
- 8 / 16 / 32 bpc are supported natively (smart render).
- **Render robustness**: with strong negative distortion the model
  polynomial folds — points outside the image of the model are rejected
  (round-trip residual check + Jacobian orientation check) and stay
  transparent; the Newton iteration starts deterministically, without
  lookup tables.
- **Diagnostics**: ships in two builds — regular and diagnostic
  (`*_debug.aex`, menu entry "3DE4 Lens Distortion Debug", separate Match
  Name `SDV LD_3DE4 Lens Distortion Debug`). The diagnostic build always
  writes a detailed technical log to `%TEMP%\ld3de4_ae.log` (rotated at
  10 MB, backup `ld3de4_ae.log.1`); the regular build writes no log at
  all. Both builds can be kept in Plug-ins simultaneously. For bug
  hunting: ask the user to apply the diagnostic effect, reproduce the
  problem and send the log.

## Installation

Copy the desired `.aex` (both builds can be installed at once) to
`C:\Program Files\Adobe\Adobe After Effects <version>\Support Files\Plug-ins\`
and restart AE.

User manual: [`AE_LD3DE4/readme_user_en.md`](AE_LD3DE4/readme_user_en.md)
(English) / [`AE_LD3DE4/readme_user.md`](AE_LD3DE4/readme_user.md)
(Russian). Building from sources is described in
[`ae_ldpk/readme.md`](../ae_ldpk/readme.md).
