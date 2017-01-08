Building LeechCraft on Windows
==============================

Prerequisites
-------------

1. Visual Studio 2015 with C++ support (Community edition is enough)
2. [vcpkg][]

Make sure that the path to vcpkg installation is short enough. See
[vcpkg#426][vcpkg-426] for details.

Build
-----

Execute `build.ps1` with PowerShell to build the project with default settings:

```console
$ ./tools/win32/build.ps1
```

Check the help section to change the build parameters:

```console
$ Get-Help ./tools/win32/build.ps1
```

[vcpkg]: https://github.com/Microsoft/vcpkg
[vcpkg-426]: https://github.com/Microsoft/vcpkg/issues/426
