rem == generate script for MSVS sln file == 

@echo off

call "%~dp0\winvars32.bat"

@echo on

cd ../..
for /f "tokens=*" %%a in ('git describe') do set LEECHCRAFT_VERSION=%%a
echo Version: %LEECHCRAFT_VERSION%
cd tools\win32

if %BUILD_RELEASE_AND_DEBUG% == 0 (
	if "%BUILD_TYPE%" == "Debug" (
		SET BTYPE=-DCMAKE_BUILD_TYPE=Debug
	)
	if "%BUILD_TYPE%" == "Release" (
		SET BTYPE=-DCMAKE_BUILD_TYPE=Release
	)
)	

rem Be sure that cmake executable is in your system %PATH%.
if exist build32 rmdir /s /q build32
if not exist build32 mkdir build32
cd build32
cmake ../../../src ^
	-G "MinGW Makefiles" ^
	%BTYPE% ^
	-DENABLE_ADVANCEDNOTIFICATIONS=False ^
	-DENABLE_AGGREGATOR=False ^
	-DENABLE_ANHERO=False ^
	-DENABLE_AZOTH=False ^
	-DENABLE_AUSCRIE=False ^
	-DENABLE_AZOTH=False ^
	-DENABLE_DBUSMANAGER=False ^
	-DENABLE_DEADLYRICS=False ^
	-DENABLE_DOLOZHEE=False ^
	-DENABLE_GLANCE=False ^
	-DENABLE_GMAILNOTIFIER=False ^
	-DENABLE_HISTORYHOLDER=False ^
	-DENABLE_HTTP=False ^
	-DENABLE_KINOTIFY=False ^
	-DENABLE_KNOWHOW=False ^
	-DENABLE_LACKMAN=False ^
	-DENABLE_LHTR=False ^
	-DENABLE_LMP=False ^
	-DENABLE_NETSTOREMANAGER=False ^
	-DENABLE_NETWORKMONITOR=False ^
	-DENABLE_NEWLIFE=False ^
	-DENABLE_POSHUKU=False ^
	-DENABLE_PINTAB=False ^
	-DENABLE_SECMAN=False ^
	-DENABLE_SEEKTHRU=False ^
	-DENABLE_SUMMARY=False ^
	-DENABLE_SYNCER=False ^
	-DENABLE_TABPP=False ^
	-DENABLE_TABSESSMANAGER=False ^
	-DENABLE_TABSLIST=False ^
	-DENABLE_TORRENT=False ^
	-DENABLE_VGRABBER=False ^
	-DENABLE_SHELLOPEN=False ^
	-DENABLE_FTP=False ^
	-DENABLE_SIDEBAR=False ^
	-DENABLE_LIZNOO=False ^
	-DENABLE_XPROXY=False ^
	-DQWT_DIR="d:\X-Files\Projects\_Lib\qwt"
pause