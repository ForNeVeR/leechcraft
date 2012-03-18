rem == General variables setup script ==
rem == (c) Eugene Mamin <thedzhon@gmail.com>

@echo off

rem == Build variables ==

rem call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" x86

set QTDIR=c:\Programs\QtSDK\Desktop\Qt\4.7.4\mingw

set BOOST_ROOT=d:\X-Files\Projects\_Lib\boost_1_48_0
rem set TORRENT_DIR=C:\DEVLIBS\libtorrent-rasterbar-0.15.9
rem set QXMPP_LOCAL=C:\DEVLIBS\qxmpp-dev
rem set QJSON_DIR=C:\DEVLIBS\qjson-0.7.1
rem set SPEEX_DIR=C:\DEVLIBS\speex-1.2rc1
rem Go to http://wiki.videolan.org/GenerateLibFromDll first!
rem Generate libs into this folder
rem set VLC_DIR=C:\DEVLIBS\vlc-1.1.11-win32\vlc-1.1.11
set QWT_DIR=d:\X-Files\Projects\_Lib\qwt

set BUILD_RELEASE_AND_DEBUG=1

rem == Collect variables ==

set BUILD_TYPE=Release

rem Set these variables to proper paths of your system:

set BOOST_BIN_DIR="%BOOST_ROOT%\stage\lib"
set BOOST_VERSION="1_48"
rem set LIBTORRENT_BIN_DIR="%TORRENT_DIR%\bin\msvc-10.0\Release\boost-link-shared\boost-source\threading-multi"
rem set OPENSSL_BIN_DIR="C:\DEVLIBS\OpenSSL-Win32"
rem set QT_BIN_DIR="%QTDIR%\bin"
rem set QJSON_BIN_DIR="%QJSON_DIR%\build\lib\MinSizeRel\"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"

