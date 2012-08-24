rem == generate script for MSVS sln file == 

@echo off

call "%~dp0\winvars32.bat"
set INSTALL_DIR="%~d0%~p0LeechCraft"

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
cmake "../../../src"  ^
	-G %CMAKE_GENERATOR% ^
	-DCMAKE_INSTALL_PREFIX:PATH=%INSTALL_DIR% ^
	-DENABLE_FTP=False %BTYPE% -DENABLE_DBUSMANAGER=False -DENABLE_ANHERO=False ^
	-DENABLE_LACKMAN=True -DENABLE_SECMAN=True -DENABLE_AZOTH=True ^
	-DENABLE_SHELLOPEN=True -DENABLE_GLANCE=True -DENABLE_TABSLIST=True ^
	-DENABLE_GMAILNOTIFIER=True ^
	-DENABLE_ADVANCEDNOTIFICATIONS=True ^
	-DENABLE_KNOWHOW=True ^
	-DENABLE_LAURE=True ^
	-DENABLE_LIZNOO=True ^
	-DENABLE_SIDEBAR=True ^
	-DENABLE_AZOTH_VADER=False ^
	-DENABLE_KBSWITCH=True ^
	-DENABLE_DOLOZHEE=True ^
	-DENABLE_OTLOZHU=True ^
	-DENABLE_VROOBY_UDISKS=False ^
	-DENABLE_MONOCLE=False ^
	-DRBTorrent_DIR=%TORRENT_DIR% ^
	-DQXmpp_DIR=%QXMPP_DIR% ^
	-DQJSON_DIR=%QJSON_DIR% ^
	-DSPEEX_DIR=%SPEEX_DIR% ^
	-DVLC_DIR=%VLC_DIR% ^
	-DQWT_DIR=%QWT_DIR% ^
	-DTAGLIB_DIR=%TAGLIB_DIR% ^
	-DHUNSPELL_DIR=%HUNSPELL_DIR% ^
	-DQCA_DIR=%QCA_DIR%
cd ..
pause