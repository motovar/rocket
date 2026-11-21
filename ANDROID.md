# Rocket Mania Deluxe for Android

An Android build of the byo-pop port of Rocket Mania Deluxe (https://github.com/allisonpelton/byo-pop-rocketmania). GitHub
Actions builds the APK; nothing has to be installed on your computer.

**The APK holds no game files.** Like the Windows build, it plays your own copy of Rocket Mania Deluxe (the Steam release or the
PopCap Game Collection disc) and reads its pictures, sounds, music and texts. This repository must never contain them either.

## Getting the APK

1. Push this repository to GitHub (github.com/motovar/rocket).
2. Open the **Actions** tab. The *Android APK* workflow runs on every push (or start it with *Run workflow*).
3. When it is green (about 10 to 15 minutes), open the run and download the **rocket-android-apk** artifact. It is a zip that
   holds `rocket-android.apk`.
4. To get a permanent download link, push a tag: `git tag v1.0 && git push origin v1.0`. The APK is attached to a GitHub release.

## Installing and the first start

1. Copy `rocket-android.apk` to the phone and open it. Allow "install unknown apps" for the app you opened it from.
2. Copy your Rocket Mania Deluxe folder from the PC to the phone (USB cable, cloud drive...). It holds the folders `images`,
   `data`, `sounds`, `music`, `properties` and the file `RocketMania.exe`.
3. Start the app, tap **Choose the game folder** and pick that folder. The game's files are copied into the app once
   (a few seconds to a minute), then the game starts. Later starts go straight into the game.

The files live in `Android/data/com.motovar.rocket/files/rocketmania/`. You can also put them there by hand (USB, `adb push`)
instead of using the folder picker. Saves, high scores and settings are in the app's private storage; uninstalling the app
removes both.

## Controls on a touch screen

| Gesture | Does |
|---|---|
| Tap | left click: turn a tile a quarter turn |
| Press and hold (about 0.4 s) without moving | right click: turn a tile the other way |
| Press and drag | drag (Arcade: move a tile to the next column) |

A tap reaches the game when the finger lifts, so that a long press can be told from a tap. The Android back button leaves the
game. A mouse on a Chromebook or a Bluetooth mouse works as on the PC.

## Signing: keep updates installable

Every Android app must be signed, and a new APK can only be installed over an old one if both have the same key. Without
setup, the workflow signs each build with a throwaway key, so you must uninstall the old app first (and copy the game files in
again). To avoid that, make a key once and give it to GitHub:

```
cd android && ./make-keystore.sh
```

It prints four values. Add them as repository secrets (Settings > Secrets and variables > Actions): `ANDROID_KEYSTORE_BASE64`,
`ANDROID_KEYSTORE_PASSWORD`, `ANDROID_KEY_ALIAS`, `ANDROID_KEY_PASSWORD`. Keep `rocket-release.jks` safe and out of git (the
`.gitignore` already excludes it). The version code is the workflow's run number, so each build is newer than the last.

## What was changed from the Windows source

- `android/`: the Gradle project, the launcher (`SetupActivity`, the folder picker and copier), the game activity (SDL's
  `SDLActivity`), `app/jni/CMakeLists.txt` (builds SDL 2.32.10 from source plus the game into `libmain.so`) and
  `fetch-sdl.sh` (downloads the pinned, checksummed SDL release; CI runs it).
- `.github/workflows/android.yml`: the build.
- `games/rocketmania/src/main.cpp`: looks for the game files in the app's folder on Android.
- `framework/SexyAppBase.cpp`: always full screen on Android, and the touch gestures above (`#ifdef __ANDROID__`).
- `framework/DDInterface.cpp`: asks SDL for its OpenGL ES 2 renderer on Android.
- **64-bit support.** The framework assumed a 32-bit `unsigned long` (true for 32-bit Windows). On 64-bit Android that is 64 bits,
  and the build stopped at the framework's own `static_assert`s. The 32-bit pixel/DWORD type is now `ulong32` (`framework/Common.h`:
  the old `unsigned long` on Windows and 32-bit ARM, `uint32_t` on 64-bit), `MTRand` works on `uint32_t`, and
  `Buffer::ReadLong` returns a sign-extended 32-bit value. On Windows these resolve to exactly the old types.
- Floating point is built without fused multiply-add (`-ffp-contract=off`) so doubles round as they did in the original.

The ABIs are `arm64-v8a` (current phones; many new ones cannot run 32-bit code at all), `armeabi-v7a` and `x86_64` (emulator).
Minimum Android is 5.0 (API 21); OpenGL ES 2.0 is required.

## Not verified

The whole tree was compile-checked and built with a 64-bit Linux compiler (the same data model as arm64), and the CMake
project was configured there. It has **not** been run on a phone, and the Android toolchain itself has not been run on this
code, so expect the first GitHub run to need a fix or two. Things to look at if the game starts but misbehaves: screen
scaling and letterboxing on unusual aspect ratios, audio latency, resuming after the app was in the background, and text if
your copy of the game is a different version than Steam's.

## Building locally

Needs JDK 17, the Android SDK with NDK 26.3.11579264 and CMake 3.22.1.

```
./android/fetch-sdl.sh
cd android
./gradlew assembleRelease        # app/build/outputs/apk/release/app-release.apk
```

Without the signing variables the release build is signed with the local debug key.

## Credits

The port is allisonpelton's byo-pop (https://github.com/allisonpelton/byo-pop). Rocket Mania Deluxe is by PopCap Games and
Nuclide. SDL is zlib-licensed. This repository does not include any of the game's own files.
