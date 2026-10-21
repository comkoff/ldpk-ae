# 3DE4 Lens Distortion — User Guide (After Effects)

The **Lens Distortion → 3DE4 Lens Distortion** effect applies lens distortion computed in 3DE4 (Science-D-Visions) to a layer. The distortion is taken either from a `.nk` file exported from 3DE4 or from the effect parameters set manually.

Requirements: After Effects 2021 or newer, Windows x64. Project color depth: 8 / 16 / 32 bpc (float).

---

## Effect panel

```
Lens Distortion .nk File   [Choose .nk File...]
Model                      [ Radial Standard Degr. 4 ▼ ]
Direction                  [ Undistort ▼ ]
                           [Fit Comp To Distortion...]
  ▼ Radial Standard Degr. 4        ← parameter group of the active model
      Distortion - Degree 2        0.00000
      U - Degree 2                 0.00000
      ...
```

---

## Controls

### Choose .nk File...

Opens a file dialog for a `.nk` file exported from 3DE4
(File → Export → Export Nuke LD_3DE4 Lens Distortion Node). A full Nuke `.nk` script works as well — the lens node is found inside it.

**What happens when a file is selected:**

1. The file is fully parsed and validated; on error nothing changes (it is safe to try any file).
2. The effect reads the lens model and its parameters from the file and stores a **snapshot** of the node inside the project.
3. The **Model** list automatically switches to the model from the file.
4. Static parameters are written to the sliders of the model group.
5. Animated parameters (3DE4 curves) are placed as keyframes on the sliders — the full length of the curve, linear interpolation, no offset. Keys are visible in the Graph Editor and can be edited by hand.
6. The composition redraws automatically.

After that the effect works: the layer image is distorted/restored according to the 3DE4 lens. **The `.nk` file is no longer needed**: the snapshot is stored inside the project — the project can be moved to another machine, sent to a render farm, and the file can be renamed or deleted, the image will not change.

**Re-import** (the same or another `.nk`): old keys are replaced with new ones; a parameter missing from the new file is reset to its default value and its animation is removed. The **Options** button of the effect does the same as **Choose .nk File...**

### Model (drop-down list)

Selecting the lens model — the **main switch** of the effect. Works without a `.nk` file too: in that case the model parameters equal the defaults (zero distortion) and FOV = the full frame.

The list contains 10 models — the standard set of old and new 3DE:

- 3DE Classic LD Model (old 3DE)
- Radial Standard Degr. 4
- Radial Stabilizer Degr. 6
- Anamorphic Degr. 6
- Anamorphic Standard Degr. 4 / 6
- Anamorphic Rescaled Degr. 4 / 6
- Anamorphic Stabilizer Degr. 6
- Radial Fisheye Degr. 8

When the model changes, the parameter group of the selected model expands in the effect panel, the other groups collapse. If a `.nk` file has been loaded, the render takes FOV and the built-in parameters (focal length, filmback etc.) from the file, and the model coefficients from the sliders (with keys, if they have been placed).

### Direction (Undistort / Distort)

Direction of the transform:

- **Undistort** — remove the lens distortion: footage shot with the lens is brought to an "ideal" (rectilinear) view. This is the default direction of the 3DE4 exporter — this is how footage is prepared for tracking/compositing.
- **Distort** — apply the distortion back: an ideal frame (e.g. produced by a render or composite) is distorted the way the lens would distort it. This is how a final render is prepared for insertion into shot footage.

### Fit Comp To Distortion...

Fits the composition size to the image after distortion.

**Algorithm:**

1. The effect computes where the frame corners "escape" when distorted (or what stays inside in the reverse direction).
2. If the distorted image **exceeds the frame**, the composition is **enlarged** exactly to the size of the distorted image, so that the whole image fits.
3. If the distorted image **occupies less space** than the frame (typical for Undistort with a "barrel"), the composition is **reduced**, so there are no empty margins.
4. Layers are not moved or scaled: pixels stay 1:1, only the canvas size changes. The content stays centered — when enlarged the canvas "grows" margins, when reduced it is cropped evenly on all sides.
5. Layers that have a parent (parenting) are not shifted separately — they are already moved by the parent, there is no double shift.
6. The whole action is a single Undo entry (Ctrl+Z reverts it entirely).

**With animated distortion** (keys on the effect sliders or curves in the `.nk`): the button scans **all keyframes** and fits the composition to the widest image over time. Result: one composition size covers the whole animation; on individual frames the edges may be transparent — there is no image there physically.

**Layer Scale.** If a layer is reduced or enlarged by its Scale parameter (without pre-composing), the button takes the current Scale value into account: the required composition size is multiplied by the layer scale. The value at the current frame is used — Scale animation is not scanned (the composition size cannot be animated in AE). At Scale = 0% the button changes nothing.

**Zero coefficients.** If all model parameters are zero, the distortion is identity: the button returns the composition to the layer size (taking Scale into account) — convenient for rolling the composition back to the original resolution after a Fit with distortion.

**Before pressing the button** the image in the composition looks like the original frame: the effect internally computes the expanded image, but AE shows only the composition frame. After pressing, the frame matches the full image.

### Model parameter group

Sliders of the coefficients of the selected model (Distortion, Squeeze, Stabilizer etc.). Without a `.nk` file this is manual distortion input; with a file — the values written from 3DE4 (possibly with keys). Slider ranges correspond to the parameter ranges in 3DE4.

---

## Typical workflows

### Undistorting footage for tracking

1. Apply the effect to the footage layer.
2. **Choose .nk File...** → select the `.nk` from 3DE4 (model and keys are substituted).
3. Direction = **Undistort** (default).
4. **Fit Comp To Distortion...** — the composition enlarges to the full image.
5. Track/composite on the "ideal" frame.

### Inserting a render into shot footage

1. Apply the effect to the layer with the ideal render.
2. **Choose .nk File...** → the same `.nk`.
3. Direction = **Distort**.
4. **Fit Comp To Distortion...** if necessary.
5. Overlay the layer on the footage in Normal mode.

### Manual distortion without 3DE4

1. Apply the effect, select a **Model**.
2. Move the sliders of the model group (e.g. Distortion - Degree 2).
3. Keyframe the parameters like any AE property — animation works.

---

## FAQ

**The effect does not distort the image.** Check: is a model selected (Model), is a `.nk` loaded (if needed), are all coefficients zero? With zero coefficients the distortion is identity — this is normal behavior.

**AE says "cannot open file: …"** when selecting a file — the path is not readable (network dropped, file renamed). Select the file again. The render itself never touches the file: after a successful import the lens snapshot is stored in the project.

**Transparency at the composition edges.** There is no image there: the distortion "pulled" the frame inward (Undistort) or the image extends beyond the footage. Press **Fit Comp To Distortion...** so the composition matches the full image.

**Keys were not placed over the whole composition length.** Keys are placed over the whole range of the curve from the `.nk` (as in 3DE4). If the curve in 3DE4 is shorter than the composition — beyond the last key the value holds at the last key.

**Diagnostics.** The plugin ships in two builds: regular and diagnostic (`LD_3DE4_Lens_Distortion_v*_debug.aex`). The diagnostic build always writes a detailed technical log to `%TEMP%\ld3de4_ae.log` (previous copy — `ld3de4_ae.log.1`); the regular build writes no log. Both builds can be kept in Plug-ins simultaneously — the diagnostic effect appears in the menu as **Lens Distortion → 3DE4 Lens Distortion Debug**. When contacting support: apply the diagnostic effect (on a copy of the composition), reproduce the problem and attach the log.
