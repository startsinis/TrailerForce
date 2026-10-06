# Native build and packaging

## Prerequisites

JUCE 8.0.6 is pinned through CMake FetchContent. CMake >= 3.22; C++17 compiler; Xcode on Mac; Visual Studio 2022/2025 with Desktop C++ workload on Windows. The Actions Windows runner provides a supported Visual Studio toolchain and Inno Setup 6. No JavaScript runtime or browser is part of the product.

## Windows x64

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel 3
ctest --test-dir build -C Release --output-on-failure
& "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" packaging/windows.iss
```

## macOS universal

```sh
cmake -S . -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13
cmake --build build --config Release --parallel 3
ctest --test-dir build -C Release --output-on-failure
bash packaging/macos.sh build dist 0.1.0
```

The deployment target is the compile target, not a guarantee of testing on every macOS release. Apple Silicon requires macOS 11 or newer. Windows targets x64; Windows ARM-native and 32-bit builds are not included.

## Signing and distribution

The Mac package script ad-hoc signs each plug-in and verifies both architectures. It does not impersonate a publisher certificate. For public commercial distribution, replace ad-hoc signing with your Developer ID Application identity; sign the PKG with Developer ID Installer; submit with `xcrun notarytool` and staple the accepted ticket. Windows distribution should use your Authenticode certificate with `signtool`. Private signing keys must be stored in encrypted CI secrets, never committed.

JUCE licenses are separate from this source. Review the included JUCE license and your applicable JUCE entitlement before commercial distribution.

## GitHub Actions

Push to main or run workflow_dispatch. The core job uses AddressSanitizer and UndefinedBehaviorSanitizer. Native jobs compile and test independently on macOS and Windows. Mac packaging verifies signatures and universal slices; auval validates both AU types. Installers and raw bundles are uploaded separately. A successful `v*` tag build publishes prerelease installers.

No Mac/Windows binary is considered validated merely because a source file or packaging script exists. Inspect the actual workflow result and complete the DAW checklist before calling a release production ready.
