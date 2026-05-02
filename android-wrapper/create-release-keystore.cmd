@echo off
setlocal

set "KEYSTORE_DIR=%~dp0keystore"
set "KEYSTORE_FILE=%KEYSTORE_DIR%\soundscape-pocket-release.jks"
set "KEY_ALIAS=soundscapepocket"

if exist "%KEYSTORE_FILE%" (
  echo Release keystore already exists:
  echo %KEYSTORE_FILE%
  exit /b 1
)

if not exist "%KEYSTORE_DIR%" mkdir "%KEYSTORE_DIR%"

keytool -genkeypair -v ^
  -keystore "%KEYSTORE_FILE%" ^
  -alias %KEY_ALIAS% ^
  -storetype PKCS12 ^
  -keyalg RSA ^
  -keysize 2048 ^
  -validity 9125

if errorlevel 1 exit /b %errorlevel%

echo.
echo Keystore created:
echo %KEYSTORE_FILE%
echo.
echo Next steps:
echo 1. Copy keystore.properties.example to keystore.properties
echo 2. Set storeFile=keystore/soundscape-pocket-release.jks
echo 3. Fill in storePassword, keyAlias, and keyPassword with the values you chose