rem == General variables setup script ==
rem == (c) Eugene Mamin <thedzhon@gmail.com>

@echo off

rem == Build variables ==

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" x86

set QTDIR=C:\Project\Qt

set BOOST_ROOT=C:\Project\boost_1_49_0
set TORRENT_DIR=C:\Project\libtorrent-trunk
set QXMPP_DIR=C:\\Project\\installed\\qxmpp
set QJSON_DIR=C:\Project\qjson
set SPEEX_DIR=C:\Project\speex-1.2rc1
rem Go to http://wiki.videolan.org/GenerateLibFromDll first!
rem Generate libs into this folder
set VLC_DIR=C:\Project\vlc-2.1.0-git-20120204-0003\
set TAGLIB_DIR=C:\Project\installed\taglib
set QWT_DIR=C:\Project\qwt-6.0
rem �������� ���� �� ��������
rem set POPPLER_QT4_DIR=C:\Project\poppler-qt4
set OXYGENICONS_DIR=C:\Project\oxygen-icons-4.8.0
set HUNSPELL_DIR=C:\Project\hunspell-1.3.2
set QCA_DIR=C:\\Project\\installed\\QCA

rem 7za.exe, gunzip.exe, vcredist_x86.exe, myspell dicts
set TOOLS_DIR=C:\Project\tools

set BUILD_RELEASE_AND_DEBUG=1
set CMAKE_GENERATOR="Visual Studio 10"

rem == Collect variables ==

set BUILD_TYPE=RelWithDebInfo
rem This is the directory where LeechCraft will live
set TARGET_DIR="LeechCraft"
set SIGNCOMMAND=signtool sign /a /t http://time.certum.pl
set SIGNCODE=1

rem Set these variables to proper paths of your system:

set BUILD_DIR_PREFIX="Release"
if "%BUILD_TYPE%" == "Debug" (
	set BUILD_DIR_PREFIX="Debug"
)

set BOOST_BIN_DIR="%BOOST_ROOT%\stage\lib"
set BOOST_VERSION="1_49"
set LIBTORRENT_BIN_DIR="%TORRENT_DIR%\bin\msvc-10.0\%BUILD_DIR_PREFIX%\boost-link-shared\boost-source\threading-multi"
set OPENSSL_BIN_DIR="C:\Project\OpenSSL-Win32\"
set QT_BIN_DIR="%QTDIR%\bin"
set QJSON_BIN_DIR="%QJSON_DIR%\build\lib\%BUILD_DIR_PREFIX%\"
set TAGLIB_BIN_DIR="%TAGLIB_DIR%\bin\"
set POPPLER_QT4_BIN_DIR="%POPPLER_QT4_DIR%\bin"
set HUNSPELL_BIN_DIR="%HUNSPELL_DIR%\src\win_api\%BUILD_DIR_PREFIX%_dll\libhunspell"
set QCA_BIN_DIR="%QCA_DIR%\bin"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"

