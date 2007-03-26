@echo off
rem This script generates version.rc and sets env variables REVN and REVDATE

SET VERSION=0.0.5

SET VPATH=vs2005\ui\version.rc

svn up

mkdir tmp
echo SET REVN=$WCREV$>tmp\1
echo SET REVDATE=$WCDATE$>>tmp\1
"C:\Program Files\TortoiseSVN\bin\SubWCRev.exe" . tmp\1 tmp\2
sed s/[\/,:]/-/g tmp\2>tmp\2.bat

call tmp\2.bat

echo IDS_VERSION  "%VERSION%.%REVN%" >%VPATH%
echo IDS_REVDATE  "%REVDATE%" >>%VPATH%   
