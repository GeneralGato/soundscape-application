# Soundscape Pocket Android Wrapper

This is a thin Android shell for the phone-first Soundscape UI in [../phone-pwa](../phone-pwa) plus the shared audio assets in [../Resources](../Resources).

What it does:

- packages the local web UI and the existing MP3 library into one offline APK
- keeps the screen awake during playback
- locks the launcher activity to portrait for quick one-hand use on a phone
- avoids cloud backup and network dependencies

## First-time setup

1. Copy [local.properties.example](local.properties.example) to `local.properties`
2. Update `sdk.dir` to your local Android SDK path
3. Optional for release builds: copy [keystore.properties.example](keystore.properties.example) to `keystore.properties`

## Build

- Debug APK: `build-debug.cmd`
- Release APK: `build-release.cmd`

You can also open this folder in Android Studio and run the `app` configuration directly.

## Install

- Debug install: `install-debug.cmd`
- Release install: `install-release.cmd`

## Notes

- The wrapper syncs assets from [../phone-pwa](../phone-pwa) and [../Resources](../Resources) into the Android build output before merge-assets runs.
- If `adb` is not on PATH, the install scripts will fail even if the APK builds successfully.