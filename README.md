# OpenMW VR Quest 3s
Managed to get OpenMW-VR to work natively in the Quest / Quest 2 so we can play Morrowind as we always dreamed.

Mixing https://gitlab.com/madsbuvi/openmw and https://github.com/xyzz/openmw-android

---

## Building (experimental branch)

### Requirements
- Android NDK (downloaded automatically by the build script)
- Android SDK / Gradle 7.3.3 + AGP 7.2.2
- A Linux host (build scripts are bash-based)

### Quick build (arm64, VR runtime)

```bash
cd buildscripts
./build.sh --arch arm64
```

The build script defaults to the local **OpenMW-VR** source (`buildscripts/openmw-vr/`) with
`-DBUILD_OPENMW_VR=ON`. If `libopenmw_vr.so` is not produced, the build will **fail fast** with
a clear error rather than silently packaging a non-VR runtime.

To override (non-VR, for debugging only):
```bash
./build.sh --arch arm64 --allow-non-vr
```

### Build all ABIs (release)
```bash
./full-build.sh
```

### Assemble APK after native build
```bash
./gradlew :app:assembleMainlineDebug --rerun-tasks
adb install -r app/build/outputs/apk/mainline/debug/omw_debug_0.47.0-45.apk
```

### What the build deploys
| File | Destination | Purpose |
|------|-------------|---------|
| `libopenmw_vr.so` → `libopenmw.so` | `app/src/main/jniLibs/<ABI>/` | VR game runtime (OpenXR + OpenMW) |
| `defaults.bin` | `assets/libopenmw/openmw/` | Settings defaults (includes [Stereo]/[VR] sections) |
| `settings-overrides-vr.cfg` | `assets/libopenmw/openmw/` | VR-only forced settings (stereo, no head bob, etc.) |
| `xrcontrollersuggestions.xml` | `assets/libopenmw/openmw/` | OpenXR controller binding profiles for Quest Touch, Index, Vive, WMR |
| `gamecontrollerdb.txt` | `assets/libopenmw/openmw/` | SDL gamepad database |
| `resources/` | `assets/libopenmw/resources/` | Shaders, MyGUI layouts, VFS |

### Quest launch notes
- App must launch as a **VR app** (`com.oculus.intent.category.VR`). See `VrEntryActivity`.
- Quest Touch controllers must be powered on or the shell will block launch.
- `GameActivity.onCreate()` calls `initOpenXRLoader()` before anything else — this is mandatory for Android OpenXR.
