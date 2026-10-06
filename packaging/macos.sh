#!/bin/bash
set -euo pipefail
BUILD=${1:-build}
OUT=${2:-dist}
VERSION=${3:-0.4.0}
ROOT="$OUT/macos-root"
mkdir -p "$ROOT/Library/Audio/Plug-Ins/VST3" "$ROOT/Library/Audio/Plug-Ins/Components" "$ROOT/Library/Application Support/Vinci Sounds/Trailer Force"
cp -R "$BUILD/TrailerForce_artefacts/Release/VST3/Trailer Force.vst3" "$ROOT/Library/Audio/Plug-Ins/VST3/"
cp -R "$BUILD/TrailerForce_artefacts/Release/AU/Trailer Force.component" "$ROOT/Library/Audio/Plug-Ins/Components/"
cp -R "$BUILD/TrailerForceMidi_artefacts/Release/AU/Trailer Force MIDI FX.component" "$ROOT/Library/Audio/Plug-Ins/Components/"
for BUNDLE in "$ROOT/Library/Audio/Plug-Ins/VST3/Trailer Force.vst3" "$ROOT/Library/Audio/Plug-Ins/Components/Trailer Force.component" "$ROOT/Library/Audio/Plug-Ins/Components/Trailer Force MIDI FX.component"; do
  codesign --force --deep --sign - "$BUNDLE"
  codesign --verify --deep --strict "$BUNDLE"
  BIN=$(find "$BUNDLE/Contents/MacOS" -type f -print -quit)
  lipo "$BIN" -verify_arch x86_64 arm64
done
cp docs/PRODUCTION-RESEARCH.md "$ROOT/Library/Application Support/Vinci Sounds/Trailer Force/"
cp docs/USER-MANUAL.md "$ROOT/Library/Application Support/Vinci Sounds/Trailer Force/"
cp packaging/Uninstall-TrailerForce.command "$ROOT/Library/Application Support/Vinci Sounds/Trailer Force/"
chmod +x "$ROOT/Library/Application Support/Vinci Sounds/Trailer Force/Uninstall-TrailerForce.command"
pkgbuild --root "$ROOT" --identifier com.vincisounds.trailerforce --version "$VERSION" --install-location / "$OUT/TrailerForce-$VERSION-macOS-universal.pkg"
shasum -a 256 "$OUT/TrailerForce-$VERSION-macOS-universal.pkg" > "$OUT/TrailerForce-$VERSION-macOS-universal.pkg.sha256"
