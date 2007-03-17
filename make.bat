call vcvars32.bat
call version.cmd
vcbuild.exe /build vs2005\sockets\zlib\zlib.vcproj Release
if ERRORLEVEL 1 goto :EOF
vcbuild.exe /rebuild vs2005\ui\ui.vcproj Release
if ERRORLEVEL 1 goto :EOF
del /s /q dist\
mkdir dist
xcopy /y "vs2005\ui\Pocket PC 2003 (ARMV4)\Release\Bombus.exe" dist\
xcopy /s /y resources\*.* dist\
pkzipc -add -rec -path=relative "dist\bombus-%REVN% [%REVDATE%].zip" dist\*.*
xcopy "dist\bombus-%REVN% [%REVDATE%].zip" "dist\bombus-ng.zip"