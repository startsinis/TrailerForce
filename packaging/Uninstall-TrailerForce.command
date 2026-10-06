#!/bin/bash
set -euo pipefail
printf 'Remove Trailer Force VST3, AU, MIDI FX and installed documentation? [y/N] '
read -r answer
if [[ "$answer" != "y" && "$answer" != "Y" ]]; then exit 0; fi
sudo rm -rf -- '/Library/Audio/Plug-Ins/VST3/Trailer Force.vst3' '/Library/Audio/Plug-Ins/Components/Trailer Force.component' '/Library/Audio/Plug-Ins/Components/Trailer Force MIDI FX.component' '/Library/Application Support/Vinci Sounds/Trailer Force'
sudo pkgutil --forget com.vincisounds.trailerforce || true
printf 'Trailer Force removed. Your DAW projects and exported files were preserved.\n'
