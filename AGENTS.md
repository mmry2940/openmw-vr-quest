# AGENTS.md

This file provides guidance to WARP (warp.dev) when working with code in this repository.

## Scope and primary code paths
- This repo packages a Quest-targeted Android app around a VR-enabled OpenMW native runtime.
- Day-to-day changes usually touch one of:
  - Android app shell and launch/config flow: `app/`
  - Native cross-build orchestration and dependency patching: `buildscripts/`
  - Embedded OpenMW-VR engine source used by default for native builds: `buildscripts/openmw-vr/`
- `ai/writer_reviewer_workflow/` is a separate Python sample workflow and not part of Android/OpenMW runtime packaging.

## Common commands
- Build native runtime for Quest arm64 (default VR path, fails fast if VR runtime artifact is missing):
  - `cd buildscripts && ./build.sh --arch arm64`
- Build native runtimes for all ABIs:
  - `cd buildscripts && ./full-build.sh`
- Clean native build artifacts:
  - `cd buildscripts && ./clean.sh`
  - `cd buildscripts && ./clean.sh --all` (also removes downloaded source archives)
- Assemble Android APK (after native libs/assets are staged by `buildscripts/build.sh`):
  - `./gradlew :app:assembleMainlineDebug --rerun-tasks`
- Install debug APK to connected device:
  - `adb install -r app/build/outputs/apk/mainline/debug/omw_debug_0.47.0-45.apk`

## Lint and tests
- Lint Android app module:
  - `./gradlew :app:lint`
- Run app unit tests:
  - `./gradlew :app:testMainlineDebugUnitTest`
- Run a single app unit test class:
  - `./gradlew :app:testMainlineDebugUnitTest --tests file.IniConverterTest`
- Run storagechooser module unit tests:
  - `./gradlew :storagechooser:test`

## Architecture overview
### 1) Android shell + VR entry flow
- `app/src/main/AndroidManifest.xml` declares Quest/VR intent categories and routes launcher entry through `ui.activity.VrEntryActivity`.
- `VrEntryActivity` decides whether to:
  - auto-start native runtime immediately (`EXTRA_AUTO_START_GAME`), or
  - open `LauncherActivity` for data-path setup and UI-based launch.
- `LauncherActivity` is a lightweight front-end for selecting game data, validating install shape (`Morrowind.ini` + `Data Files`), then forwarding to `startGame()` via `VrEntryActivity`.

### 2) Launch preparation and config generation
- `MainActivity.startGame()` performs most pre-launch work on a background thread:
  - validates packaged runtime payload (`libopenmw.so` + bundled assets),
  - (re)installs static bundled assets/config into app-private storage when needed,
  - regenerates `openmw.cfg` by merging base/fallback config and enabled mods,
  - ensures VR-specific settings/config files are present.
- `GameInstaller` handles game-path validation and converts `Morrowind.ini` into OpenMW fallback config entries.

### 3) Java/Kotlin ↔ SDL ↔ native OpenMW boundary
- `GameActivity` extends the local SDL activity implementation and controls native bootstrap:
  - loads required shared libs (`c++_shared`, `openal`, `SDL2`, `GL`, optional `openxr_loader`, `openmw`),
  - calls `initOpenXRLoader()` before gameplay initialization,
  - passes command-line args from preferences through `CommandlineParser`.
- `app/src/main/java/org/libsdl/app/SDLActivity.java` is a customized SDL Java layer:
  - lifecycle/state machine for the native thread,
  - Quest/headlook/controller input handling,
  - JNI bridge methods used by native OpenMW entrypoints.

### 4) Native build pipeline and VR source selection
- `buildscripts/build.sh` is the authoritative native build entry point:
  - bootstraps Android NDK/toolchain,
  - configures/builds with CMake,
  - stages `libopenmw.so` and dependency `.so` files into `app/src/main/jniLibs/<ABI>/`,
  - stages resources/config into `app/src/main/assets/libopenmw/`.
- `buildscripts/CMakeLists.txt` builds third-party dependencies via `ExternalProject_Add` and then builds OpenMW itself.
- By default, CMake uses local `buildscripts/openmw-vr/` with `-DBUILD_OPENMW_VR=ON`; fallback to upstream non-VR OpenMW is explicitly gated (`ALLOW_OPENMW_UPSTREAM_FALLBACK`).
- `buildscripts/patches/openmw/android_main.cpp` is copied into OpenMW during upstream fallback path and defines key JNI/SDL integration points.

## Working safely in this repo
- For launch/runtime issues, check both sides of the boundary:
  - Android activity flow/config generation in `app/src/main/java/ui/activity/`
  - native packaging/staging logic in `buildscripts/build.sh`
- If APK launches but engine fails, verify:
  - `app/src/main/jniLibs/<ABI>/libopenmw.so` exists after native build,
  - `app/src/main/assets/libopenmw/openmw/` and `.../resources/` were deployed.
