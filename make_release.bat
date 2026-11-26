@echo off
setlocal
set "VERSION=V992.17"

platformio run -e esp12e -t buildprog || exit /b 1
platformio run -e esp12e -t buildfs || exit /b 1

set "FIRMWARE=.pio\build\esp12e\firmware.bin"
set "SPIFFS=.pio\build\esp12e\spiffs.bin"
if not exist "%FIRMWARE%" (
    echo Missing build output: %FIRMWARE% >&2
    exit /b 1
)
if not exist "%SPIFFS%" (
    echo Missing build output: %SPIFFS% >&2
    exit /b 1
)

copy /y "%FIRMWARE%" "RFDTxMod-v2_%VERSION%.bin" || exit /b 1
copy /y "%SPIFFS%" "RFDTxMod-v2_spiffs%VERSION%.bin" || exit /b 1
dir "RFDTxMod-v2_%VERSION%.bin" "RFDTxMod-v2_spiffs%VERSION%.bin"
pause
