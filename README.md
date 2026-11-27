# Soundscape Application

An interactive audio relaxation application originally developed as my final-year BSc (Hons) Creative Music Technology project at UWE Bristol in 2021, subsequently recovered and modernised for Windows and Android.

The application allows users to create personalised soundscapes by combining ambient sound beds, adjusting individual audio layers and mixing foreground sounds.

## Features

- Interactive multilayer audio mixing with individual volume and panning controls.
- Native C++/JUCE desktop application.
- Mobile-first, offline-capable Android application.
- Named user presets and configurable soundscape mixes.
- Smooth playback transitions and parameter changes.
- Shared, JSON-based audio asset definitions.

## Improvements since the original 2021 version

- Modernised the legacy JUCE desktop project using C++17, CMake and Visual Studio 2022.
- Introduced a redesigned, resizable desktop dashboard and a dedicated mobile interface.
- Developed an Android WebView wrapper with locally packaged audio assets.
- Implemented a hybrid playback architecture combining pre-mixed stereo stems with individually adjustable audio layers.
- Introduced a 16-unit mobile audio stream budget to manage concurrent playback and decoder workload.
- Implemented fallback behaviour that preserves custom mixing settings when playback resources are limited.
- Replaced fixed audio layer definitions with a shared JSON asset manifest used by both platforms.
- Standardised delivery audio assets at 48 kHz and established documented sound library production guidelines.

## Technology stack

- **Desktop:** C++17, JUCE, CMake, Visual Studio 2022
- **Mobile:** Android, WebView, HTML, CSS, JavaScript, Web Audio
- **Audio:** Multilayer mixing, stereo stems, gain smoothing, panning and playback resource management
- **Development:** Git, JSON, AI-assisted development workflows

## Project documentation

- [Development notes](DEV_NOTES.md) - Architecture, build workflows and runtime behaviour.
- [Sound library rulebook](SOUND_LIBRARY_RULEBOOK.txt) - Audio production standards and asset quality requirements.
- [Session handoff](SESSION_HANDOFF.md) - Development status, known limitations and planned improvements.

## Screenshots:

<img width="1080" height="2460" alt="Audio_Live" src="https://github.com/user-attachments/assets/e9b0785c-f2d1-46b5-81ee-ea8fb2c27b11" />
<img width="1080" height="2460" alt="Ambient_Beds" src="https://github.com/user-attachments/assets/724f6b4b-ee7a-4576-9662-bdff412f67e4" />
<img width="1080" height="2460" alt="Foreground_Loops" src="https://github.com/user-attachments/assets/2a7e63d3-2096-4f07-bca4-1eaad67e2593" />

## Demo:

https://github.com/user-attachments/assets/dd902db3-d163-4dec-a109-8ad89b011ea5

[Original 2021 university project demonstration](https://www.youtube.com/watch?v=RfKW-H2I6ew)

## Clone

This repository uses the JUCE framework as a git submodule.

After cloning, initialise it with:

```powershell
git submodule update --init --recursive
```

## Windows build

This project can be rebuilt locally with CMake and Visual Studio 2022.

Configure:

```powershell
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -S . -B build\vs2022 -G "Visual Studio 17 2022" -A x64
```

Build:

```powershell
"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build\vs2022 --config Debug --target SoundscapeApplication
```

Output executable:

`build\vs2022\SoundscapeApplication_artefacts\Debug\Soundscape Application.exe`

The `Resources` folder is copied next to the executable as part of the build so the app can be launched directly from the generated artefacts directory.

## Android wrapper

The project also includes a phone-first Android wrapper in `android-wrapper/`, backed by the offline web UI in `phone-pwa/`.

Debug APK build:

```powershell
cd android-wrapper
.\build-debug.cmd
```

Install to a connected Android device with USB debugging enabled:

```powershell
cd android-wrapper
.\install-debug.cmd
```






