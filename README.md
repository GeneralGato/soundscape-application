# Soundscape Application

A JUCE-based relaxation and soundscape application.

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

Screenshots:

<img width="1080" height="2460" alt="Audio_Live" src="https://github.com/user-attachments/assets/e9b0785c-f2d1-46b5-81ee-ea8fb2c27b11" />
<img width="1080" height="2460" alt="Ambient_Beds" src="https://github.com/user-attachments/assets/724f6b4b-ee7a-4576-9662-bdff412f67e4" />
<img width="1080" height="2460" alt="Foreground_Loops" src="https://github.com/user-attachments/assets/2a7e63d3-2096-4f07-bca4-1eaad67e2593" />
