rem == General variables setup script ==
rem == (c) Eugene Mamin <thedzhon@gmail.com>

@echo off

rem == Build variables ==

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" x86

set QTDIR=C:\Project\Qt

set BOOST_ROOT=C:\Project\boost_1_48_0
set TORRENT_DIR=C:\Project\libtorrent
set QXMPP_LOCAL=C:\Project\qxmpp-dev
set QJSON_DIR=C:\Project\qjson
set SPEEX_DIR=C:\Project\speex-1.2rc1
rem Go to http://wiki.videolan.org/GenerateLibFromDll first!
rem Generate libs into this folder
set VLC_DIR=C:\Project\vlc-2.1.0-git-20120204-0003\
set QWT_DIR=C:\Project\qwt-6.0
set OXYGENICONS_DIR=C:\Project\oxygen-icons-4.8.0

rem 7za.exe, gunzip.exe, vcredist_x86.exe, myspell
set TOOLS_DIR=C:\Project\tools

set BUILD_RELEASE_AND_DEBUG=1
set CMAKE_GENERATOR="Visual Studio 10"

rem == Collect variables ==

set BUILD_TYPE=Debug

rem Set these variables to proper paths of your system:

set BOOST_BIN_DIR="%BOOST_ROOT%\stage\lib"
set BOOST_VERSION="1_48"
set LIBTORRENT_BIN_DIR="%TORRENT_DIR%\bin\msvc-10.0\Release\boost-link-shared\boost-source\threading-multi"
set OPENSSL_BIN_DIR="C:\Project\OpenSSL-Win32\"
set QT_BIN_DIR="%QTDIR%\bin"
set QJSON_BIN_DIR="%QJSON_DIR%\build\lib\MinSizeRel\"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"

