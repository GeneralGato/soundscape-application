# Soundscape Dev Notes

## Repo Layout

- `Source` contains the desktop JUCE application code.
- `android-wrapper` contains the Android WebView shell.
- `phone-pwa` contains the mobile-first static UI.
- `Resources` contains the live app audio assets.
- `Resources_backup_44k1_2026-05-02` is a local-only backup of older 44.1 kHz files and is intentionally gitignored.
- `SOUND_LIBRARY_RULEBOOK.txt` contains production guidance for future audio asset work.

## Desktop Build

Configure from the repo root:

```powershell
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -S . -B build\vs2022 -G "Visual Studio 17 2022" -A x64
```

Build a release executable:

```powershell
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build\vs2022 --config Release --target SoundscapeApplication
```

The release app binary is:

```text
build\vs2022\SoundscapeApplication_artefacts\Release\Soundscape Application.exe
```

The build copies `Resources` next to the executable so the artefacts folder can be launched directly.

## Phone UI Preview

The simplest local preview is to serve the repo root so `phone-pwa` can resolve `../Resources` correctly:

```powershell
Set-Location "C:\Users\andre\Soundscape Application"
python -m http.server 4173
```

Then open:

```text
http://127.0.0.1:4173/phone-pwa/
```

Previewing the generated Android asset copy is possible, but it is safer to treat the source `phone-pwa` preview as the primary layout check unless the wrapper assets have just been regenerated.

## Android Wrapper Build

- SDK path is configured through `android-wrapper\local.properties`.
- Current package id is `uk.fivereasons.soundscape`.
- Current version is `0.3.0` / `versionCode 4`.

Build a debug APK:

```powershell
Set-Location "C:\Users\andre\Soundscape Application\android-wrapper"
.\build-debug.cmd
```

Install to a connected device:

```powershell
Set-Location "C:\Users\andre\Soundscape Application\android-wrapper"
.\install-debug.cmd
```

Current debug APK output:

```text
android-wrapper\app\build\outputs\apk\debug\app-debug.apk
```

If no device or emulator is available, the Android version cannot be run directly.

## Android Wrapper Behavior

- The wrapper sync task copies `phone-pwa` plus `Resources` into the generated asset directory before Android asset merge steps.
- The wrapper relies on local asset packaging rather than network fetches.
- Release signing is optional for local debug work but required for release builds.

## Non-Obvious Phone Runtime Rules

- The phone runtime uses a 16-unit stream budget.
- One stem bed costs 1 unit.
- One custom-layer bed costs 6 units total.
- One solo loop costs 1 unit.
- If a bed has custom layer settings saved but the stream budget is full, it can continue playing as a stem while preserving the custom settings.
- This is the behavior previously surfaced as the saved-custom stem fallback.

## Audio Asset Notes

- Current delivery assets are `48 kHz`.
- The backup of original 44.1 kHz assets is intentionally not part of source control.
- The six current stem files live under `Resources\Soundscapes\Stems`.
- Future recording, editing, export, and mastering rules are intentionally centralized in `SOUND_LIBRARY_RULEBOOK.txt`.

## Session Hygiene

- Keep the full raw chat transcript outside the repo as a private archive.
- Use `SESSION_HANDOFF.md` as the short future-chat bootstrap.
- Add new technical facts here when they are operationally important and likely to be forgotten.