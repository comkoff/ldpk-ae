# ldpk-ae — Lens Distortion: LDPK → After Effects / Nuke

Lens distortion tooling based on LDPK 2.14.0 (Lens Distortion Plugin Kit, Science-D-Visions).

**Language:** English | [Русский](README.ru.md)

The centerpiece is the **After Effects effect "3DE4 Lens Distortion"**: it applies lens distortion computed in 3DE4 to a layer — either from a `.nk` file exported from 3DE4 (File → Export → Export Nuke LD_3DE4 Lens Distortion Node) or from parameters set manually. Animated parameters are supported (3DE4 curves → keyframes), Undistort/Distort directions, automatic composition fitting (Fit Comp), 8/16/32 bpc.

## Two builds: release and debug

The plugin is built in **two versions** that can be installed side by side:

| Build | Menu entry | Match name | Log file |
|---|---|---|---|
| **Release** | Lens Distortion → 3DE4 Lens Distortion | `SDV LD_3DE4 Lens Distortion` | none |
| **Debug** (diagnostic) | Lens Distortion → 3DE4 Lens Distortion Debug | `SDV LD_3DE4 Lens Distortion Debug` | always writes `%TEMP%\ld3de4_ae.log` (rotated at 10 MB, backup `ld3de4_ae.log.1`) |

For troubleshooting use the debug build: apply it to a copy of the composition, reproduce the problem and attach the log.

## Repository layout

| Directory | Contents |
|---|---|
| [`ae_ldpk/`](ae_ldpk) | LDPK → After Effects bridge: `.nk` parser ([`ldpk_nk_block.h`](ae_ldpk/ldpk_nk_block.h)), model registry ([`ldpk_model_registry.h`](ae_ldpk/ldpk_model_registry.h)), loader ([`ldpk_nk_loader.h`](ae_ldpk/ldpk_nk_loader.h)), parameter tables ([`ldpk_param_tables.h`](ae_ldpk/ldpk_param_tables.h)), Nuke cross-check utility ([`nk_ld_dump.cpp`](ae_ldpk/nk_ld_dump.cpp)), test ([`test_snapshot_roundtrip.cpp`](ae_ldpk/test_snapshot_roundtrip.cpp)), effect source ([`ae/`](ae_ldpk/ae)) and VS 2022 project ([`ae/Win/`](ae_ldpk/ae/Win)) |
| [`ldpk-2.14.0/`](ldpk-2.14.0) | Original LDPK 2.14.0 tree (Science-D-Visions), kept unmodified: header-only models ([`include/ldpk/`](ldpk-2.14.0/include/ldpk) — required for building the bridge), Nuke/3DE plugins ([`source/ldpk/`](ldpk-2.14.0/source/ldpk)), 3DE4 exporters ([`3de4/python/`](ldpk-2.14.0/3de4/python)), build scripts ([`script/`](ldpk-2.14.0/script)), tests ([`test/`](ldpk-2.14.0/test)), VS projects for Nuke ([`windows/`](ldpk-2.14.0/windows)), docs ([`doc/`](ldpk-2.14.0/doc)), license ([`license`](ldpk-2.14.0/license)) |
| [`releases/`](releases) | Release documentation; binaries are published on the Releases page |

## Downloads

Ready-to-use plugin binaries are published as assets of **GitHub Releases**: <https://github.com/comkoff/ldpk-ae/releases>. The repository tree contains sources and documentation only. Release assets contain **both builds** (release and debug) for **Windows and macOS** and the user manual in **English and Russian**. On macOS, remove the Gatekeeper quarantine right after unpacking: `xattr -dr com.apple.quarantine LD_3DE4_Lens_Distortion_v*.bundle`.

## Building the AE plugin (Windows)

- Visual Studio 2022 (toolset v143, x64), workload "Desktop development with C++".
- The After Effects SDK is **not part of this repository**: download AE SDK 26.5 and unpack it into `AfterEffectsSDK_26.5_win/` next to `ae_ldpk/`.
- LDPK headers are picked up from `ldpk-2.14.0/include` (configured in the [.vcxproj](ae_ldpk/ae/Win/LD3DE4_Lens_Distortion.vcxproj)).
- Detailed build commands — in [`ae_ldpk/readme.md`](ae_ldpk/readme.md): the `nk_ld_dump` utility, MSBuild for `LD3DE4_Lens_Distortion.sln`, the diagnostic build with the `%TEMP%\ld3de4_ae.log` log.

## Usage (quick start)

1. Apply the **Lens Distortion → 3DE4 Lens Distortion** effect to a layer.
2. **Choose .nk File...** — a file from 3DE4 (a full Nuke `.nk` works too); the model and keys are substituted automatically, the file is no longer needed (a snapshot is stored in the project).
3. **Direction**: Undistort / Distort.
4. **Fit Comp To Distortion...** — fits the composition to the full image.

Full user manual: [English](releases/AE_LD3DE4/readme_user.md) | [Русский](releases/AE_LD3DE4/readme_user_ru.md).

## License

The bridge code is provided "as is", without warranties. The LDPK models are free software © Uwe Sassenberg, Science-D-Visions (see [`ldpk-2.14.0/license`](ldpk-2.14.0/license)); the `tde4_ld_plugin.h` header belongs to Science-D-Visions. Adobe SDK examples are used as a structural reference.
