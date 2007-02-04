call vcvars32.bat
vcbuild.exe /build vs2005\sockets\zlib\zlib.vcproj Release
if ERRORLEVEL 1 goto :EOF
vcbuild.exe /rebuild vs2005\ui\ui.vcproj Release
if ERRORLEVEL 1 goto :EOF
del /s /q dist\
mkdir dist
copy "vs2005\ui\Pocket PC 2003 (ARMV4)\Release\Bombus.exe" dist\Bombus.exe
copy resources\*.* dist\
pkzip dist\bombus.zip dist\*.*