Building LeechCraft on Windows
==============================

Prerequisites
-------------

1. Visual Studio 2015 with C++ support (Community edition is enough)
2. Qt 5.7.1 (x86) for Visual Studio
3. QtWebkit 5.7.1
4. [vcpkg][]

Build
-----

### QtWebkit

1. Download [qtwebkit source package][qtwebkit].
2. Follow instructions from [webkitguide][webkit-guide]. The following will try
   to direct you through the building process.
3. Install [ActivePerl][activeperl].
4. Make sure to install [Chocolatey][chocolatey], it will be a source of some
   of the packages.
5. Install the dependencies:

   ```console
   $ choco install python2
   $ choco install ruby
   ```
6. Extract the `qtwebkit-opensource-<version>` somewhere
7. Download [WebKit Support Library][webkit-support-library] and place the ZIP
   file to the qtwebkit source root.
8. Manually fix `Tools\Scripts\update-webkit-support-libs` script: replace
   `WEXITSTATUS($?)` with `$?`
9. Execute the following inside of "Qt 5.7 32-bit for Desktop (MSVC 2015)"
   shell:

   ```console
   $ set %PATH%=%PATH%;<path to qt>\Src\gnuwin32\bin
   $ copy <path to qt>\Src\gnuwin32\bin\flex.exe <path to qt>\Src\gnuwin32\bin\win_flex.exe
   $ set SQLITE3SRCDIR=<path to qt>\Src\qtbase\src\3rdparty\sqlite
   $ set CL= /MP
   $ perl Tools\Scripts\build-webkit --qt
   ```

#### ⚠ TODO: Currently I have problems with ICU; it seems to be the last issue here

### LeechCraft

Execute `build.ps1` with PowerShell to build the project with default settings:

```console
$ ./tools/win32/build.ps1
```

Check the help section to change the build parameters:

```console
$ Get-Help ./tools/win32/build.ps1
```

[activeperl]: http://www.activestate.com/Products/ActivePerl/
[chocolatey]: https://chocolatey.org/
[qtwebkit]: https://download.qt.io/community_releases/5.7/5.7.1/qtwebkit-opensource-src-5.7.1.zip
[vcpkg]: https://github.com/Microsoft/vcpkg
[webkit-guide]: https://trac.webkit.org/wiki/BuildingQtOnWindows
[webkit-support-library]: https://developer.apple.com/opensource/internet/webkit_sptlib_agree.html
