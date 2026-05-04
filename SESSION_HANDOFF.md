# Soundscape Session Handoff

## Current State

- The desktop JUCE app has been recovered onto a CMake plus Visual Studio 2022 workflow and launches successfully on Windows.
- The latest validated desktop binary is the release build under `build\vs2022\SoundscapeApplication_artefacts\Release\Soundscape Application.exe`.
- The Android wrapper package id is `uk.fivereasons.soundscape`.
- The Android wrapper version is `0.3.0` with `versionCode 4`.
- A recent debug APK exists under `android-wrapper\app\build\outputs\apk\debug\app-debug.apk`.
- The phone UI includes named preset save/load/delete.
- The phone UI includes 1-second fade in/out behavior for bed and loop toggles.
- The phone UI uses a stem-plus-custom-layer playback model to reduce mobile decoder load.
- The phone UI source currently includes per-bed reset plus a global slider reset.
- Delivery audio assets are normalized to `48 kHz`.
- Stereo stems exist for Ambient, Rainy Day, Seaside, Drone, Forest, and Summer Field.
- Future asset standards are documented in `SOUND_LIBRARY_RULEBOOK.txt`.

## Recent History

- `90b0463` ignores the local 44.1 kHz backup folder so it stays out of Git.
- `f20ed2b` contains the main optimization and pocket-mixing work.
- `3ac1cef` is the initial import baseline.

## Known Constraints

- Running or installing the Android build still requires either a connected device or a configured emulator.
- Release Android builds require a populated `android-wrapper\keystore.properties` file.
- The raw editor transcript should be treated as an archive only; this repo should rely on distilled notes rather than long chat history.

## Likely Next Work

- Install and launch the latest Android wrapper on a phone again when a device is connected.
- Continue mobile audio optimization if stutter returns under Bluetooth or heavier mixes.
- Improve stem generation from fully synchronized source sessions instead of approximate long renders.
- Expand the sound library using the existing rulebook and 48 kHz delivery standards.

## Starter Prompt For A New Chat

```text
This chat is for the Soundscape project.

Current state:
- Desktop JUCE app runs from the current CMake/VS2022 build.
- Android wrapper package is uk.fivereasons.soundscape.
- Android version is 0.3.0 / versionCode 4.
- Phone UI already has presets, fades, stem/custom playback budgeting, and slider reset controls.
- Delivery assets are 48 kHz and stereo stems exist for the six ambient beds.

Please continue from the current repo state rather than re-deriving the project history.
```