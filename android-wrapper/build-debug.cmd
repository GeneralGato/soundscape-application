@echo off
setlocal
call "%~dp0gradlew.bat" assembleDebug
if errorlevel 1 exit /b %errorlevel%
echo.
echo Soundscape debug APK ready:
echo %~dp0app\build\outputs\apk\debug\app-debug.apk