rem @echo off

call "%~dp0\winvars32.bat"

if %SIGNCODE% == 1 (
rem - Main files -

%SIGNCOMMAND% %TARGET_DIR%\leechcraft.exe
%SIGNCOMMAND% %TARGET_DIR%\lcutil.dll
%SIGNCOMMAND% %TARGET_DIR%\xmlsettingsdialog.dll

rem - Plugins -
for /r %TARGET_DIR%\plugins\bin %%f in (leechcraft_*.dll) do %SIGNCOMMAND% %%f
)