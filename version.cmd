SET VERSION=0.0.2

SET VPATH=vs2005\ui\version.rc

rem @echo off

mkdir tmp
echo SET REVN=$WCREV$>tmp\1
echo SET REVDATE=$WCDATE$>>tmp\1
"C:\Program Files\TortoiseSVN\bin\SubWCRev.exe" . tmp\1 tmp\2
sed s/[\/,:]/-/g tmp\2>tmp\2.bat

call tmp\2.bat

echo IDS_VERSION  "%VERSION%.%REVN%" >%VPATH%
echo IDS_REVDATE  "%REVDATE%" >>%VPATH%   
