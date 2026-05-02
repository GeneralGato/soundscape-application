@echo off
setlocal

if not exist "%~dp0keystore.properties" (
  echo Missing keystore.properties.
  echo Copy keystore.properties.example to keystore.properties and fill in the signing values first.
  exit /b 1
)

call "%~dp0gradlew.bat" assembleRelease
if errorlevel 1 exit /b %errorlevel%

echo.
echo Signed Soundscape release APK ready:
echo %~dp0app\build\outputs\apk\release\app-release.apk