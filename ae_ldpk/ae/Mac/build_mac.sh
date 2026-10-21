#!/bin/bash
# ---------------------------------------------------------------------------
# Build "LD_3DE4 Lens Distortion" for After Effects (macOS).
#
# Produces a universal (x86_64 + arm64) .bundle with the PiPL resource,
# mirroring the Windows build (Win/LD3DE4_Lens_Distortion.vcxproj):
#   release: releases/AE_LD3DE4/<version>/LD_3DE4_Lens_Distortion_v<version>.bundle
#   debug:   releases/AE_LD3DE4/<version>_debug/LD_3DE4_Lens_Distortion_v<version>_debug.bundle
#
# Usage:
#   ./build_mac.sh [release|debug]
#
# The After Effects SDK must be available in one of the ways:
#   AE_SDK=<sdk root>            — folder containing Examples/Headers (or Headers)
#   AE_SDK_HEADERS=<Headers dir> — explicit paths (AE_SDK_RESOURCES / AE_SDK_UTIL too)
# The SDK subset needed for compilation:
#   Examples/Headers/   (all .h)
#   Examples/Resources/ (all .r — Rez needs them for the PiPL)
#   Examples/Util/      (AEGP_SuiteHandler.cpp, MissingSuiteError.cpp)
# ---------------------------------------------------------------------------
set -euo pipefail

CONFIG="${1:-release}"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"   # ae_ldpk/ae/Mac
AE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"        # ae_ldpk/ae
AE_LDPK="$(cd "$AE_DIR/.." && pwd)"           # ae_ldpk
REPO="$(cd "$AE_LDPK/.." && pwd)"             # repo root
LDPK_INC="$REPO/ldpk-2.14.0/include"

# --- locate the SDK ---------------------------------------------------------
if [ -n "${AE_SDK_HEADERS:-}" ]; then
    SDK_HEADERS="$AE_SDK_HEADERS"
    SDK_RESOURCES="${AE_SDK_RESOURCES:-$(dirname "$SDK_HEADERS")/Resources}"
    SDK_UTIL="${AE_SDK_UTIL:-$(dirname "$SDK_HEADERS")/Util}"
else
    SDK="${AE_SDK:-$REPO/AfterEffectsSDK_26.5_mac}"
    if [ -d "$SDK/Examples/Headers" ]; then
        SDK_HEADERS="$SDK/Examples/Headers"
        SDK_RESOURCES="$SDK/Examples/Resources"
        SDK_UTIL="$SDK/Examples/Util"
    elif [ -d "$SDK/Headers" ]; then
        SDK_HEADERS="$SDK/Headers"
        SDK_RESOURCES="$SDK/Resources"
        SDK_UTIL="$SDK/Util"
    else
        echo "ERROR: AE SDK not found at '$SDK'." >&2
        echo "Set AE_SDK=<sdk root> or AE_SDK_HEADERS=<Headers dir>." >&2
        exit 1
    fi
fi
for d in "$SDK_HEADERS" "$SDK_RESOURCES" "$SDK_UTIL"; do
    [ -d "$d" ] || { echo "ERROR: SDK folder missing: $d" >&2; exit 1; }
done

# --- version (kept in sync with LD3DE4_Lens_Distortion.h / PiPL) ------------
H="$AE_DIR/LD3DE4_Lens_Distortion.h"
MAJOR=$(sed -n 's/^[[:space:]]*#define MAJOR_VERSION \([0-9]*\)$/\1/p' "$H")
MINOR=$(sed -n 's/^[[:space:]]*#define MINOR_VERSION \([0-9]*\)$/\1/p' "$H")
BUG=$(sed -n 's/^[[:space:]]*#define BUG_VERSION \([0-9]*\)$/\1/p' "$H")
VERSION="${MAJOR:-1}.${MINOR:-0}.${BUG:-0}"

if [ "$CONFIG" = "debug" ]; then
    NAME="LD_3DE4_Lens_Distortion_v${VERSION}_debug"
    OUT="$REPO/releases/AE_LD3DE4/${VERSION}_debug"
    CXX_DEFINES="-DLD_ENABLE_DEBUG_LOG=1"
    REZ_DEFINES="-d LD_ENABLE_DEBUG_LOG=1"   # Rez wants -d, not -D
else
    NAME="LD_3DE4_Lens_Distortion_v${VERSION}"
    OUT="$REPO/releases/AE_LD3DE4/${VERSION}"
    CXX_DEFINES=""
    REZ_DEFINES=""
fi

BUNDLE="$OUT/$NAME.bundle"
echo "SDK headers:   $SDK_HEADERS"
echo "Configuration: $CONFIG"
echo "Bundle:        $BUNDLE"

rm -rf "$BUNDLE"
mkdir -p "$BUNDLE/Contents/MacOS" "$BUNDLE/Contents/Resources"

# --- 1. compile (universal binary) ------------------------------------------
# Note: AE_OS_MAC is defined by the SDK's AEConfig.h itself; defining it here
# as well only produces a macro-redefined warning.
CXX_FLAGS=(
    -std=c++17 -O2 -fexceptions -frtti -fPIC
    -arch x86_64 -arch arm64
    -mmacosx-version-min=11.0
    $CXX_DEFINES
    -I"$SDK_HEADERS" -I"$SDK_HEADERS/SP" -I"$SDK_UTIL"
    -I"$LDPK_INC" -I"$AE_LDPK"
)

echo "[1/3] Compiling (universal x86_64 + arm64)..."
clang++ "${CXX_FLAGS[@]}" -bundle \
    -framework AppKit -framework CoreFoundation \
    -o "$BUNDLE/Contents/MacOS/$NAME" \
    "$AE_DIR/LD3DE4_Lens_Distortion.cpp" \
    "$SCRIPT_DIR/LD_FileDialog_mac.mm" \
    "$SDK_UTIL/AEGP_SuiteHandler.cpp" \
    "$SDK_UTIL/MissingSuiteError.cpp"

# --- 2. PiPL resource (Rez) ---------------------------------------------------
echo "[2/3] Compiling the PiPL (Rez)..."
Rez -o "$BUNDLE/Contents/Resources/$NAME.rsrc" \
    -I "$SDK_HEADERS" -I "$SDK_RESOURCES" -I "$AE_DIR" \
    -d AE_OS_MAC=1 $REZ_DEFINES \
    "$AE_DIR/LD3DE4_Lens_DistortionPiPL.r"

# --- 3. Info.plist ------------------------------------------------------------
echo "[3/3] Info.plist..."
cat > "$BUNDLE/Contents/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleInfoDictionaryVersion</key><string>6.0</string>
    <key>CFBundlePackageType</key><string>BNDL</string>
    <key>CFBundleExecutable</key><string>$NAME</string>
    <key>CFBundleName</key><string>$NAME</string>
    <key>CFBundleIdentifier</key><string>com.sdv.ld3de4-lens-distortion</string>
    <key>CFBundleVersion</key><string>$VERSION</string>
    <key>LSMinimumSystemVersion</key><string>11.0</string>
</dict>
</plist>
PLIST

echo "OK: $BUNDLE"
