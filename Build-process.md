# Build process

## Requirements

 * A working Qt4 >= 4.8  or QT5 dev environment (need to specify WITH_QT5=OFF in cmake arguments for Qt4)
 * A working cmake environment
 * OpenSSL development libraries and headers (optional)
 * Python 2.7 or 3 (or both) dev environment (optional)
 * Distorm development libraries and headers (optional)


This project is based on the Qt library which is multi-platform, so the building process should roughly be the same once you have a working dev environment.

You also need cmake for an easier build.
See http://www.cmake.org/

## Generic compilation options

> -DALL=ON

Configure and compile everything.
In this configuration non-basic modules can be deactivated individually by setting them to OFF

> -DBASIC=ON

Configure and compile only the bare necessary parts:
* libtransform
* The Pip3line GUI
* The command line tool
* The default transformation plugin

Other modules can be activated individually.

> -DWITH_QT5=ON

Configure and compile the binary for QT5. (default)
This is now the default configuration, if you want to compile against Qt4 set this option to OFF

## Linux build & Install

The Linux build and installation process is pretty much painless

> shell$ mkdir build

> shell$ cd build

> shell$ cmake [options] ../ (Or whatever is the path for the sources)

> shell$ make

If you want to speed up the compilation process on an eight cores machine you can replace the last step with:

> shell$ make -j8

Then installation as follow:

> shell$ sudo make install

Be careful that the the base install path is "/usr/local" by default.
If your library path is not configured to check /usr/local/lib this may cause the issue:
```
pip3line: error while loading shared libraries: libtransform.so: cannot open shared object file: No such file or directory
```

If you want/need a different path, for example "/usr" you may run cmake with the following option

> shell$ cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ../ 

There is also an uninstall _make_ command available

> shell$ sudo make uninstall

### Ubuntu

Make sure to install the following package:

python2.7-dev
python3-dev
libqt5svg5-dev
qt5-default
libqt5xmlpatterns5-dev
cmake
git
libssl-dev
libdistorm3-dev

## Windows specific
First brace yourself, the build process on a Windows platform can be quite tricky.

Use the preconfigured Qt command lines shortcuts to start with a proper pre-configured environment.

Additionally on windows you need to specify a generator for CMake.

If you are using the MinGW version of the QT5 binaries

> C:\path> cmake -G  "MinGW Makefiles" [path_to_sources_root + options]

> C:\path> mingw32-make -j8

If you are using the Visual Studio 2010/2012 version, you need use nmake instead, and you have to run the script vsvars.bat located in the Visual Studio directory first.

For an x64 compilation with VS 2013:

> C:\path> "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64

For an x86 compilation with VS 2013:

> C:\path> "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86

Then simply:
> C:\path> cmake -G  "NMake Makefiles" ..\ [ + options]

for example:
> C:\path> cmake -G  "NMake Makefiles" ..\  -DALL=ON -DWITH_QT5=ON

Following with the compilation phase
> C:\path> nmake

A list of supported generators is given with:

> shell$ cmake -h

On windows you need to copy the relevant dll into the program/system folder, otherwise you end up with some missing libraries errors. There is an old-school DOS script file win_package.bat that might help you (or maybe not ..)

**Beware**: whenever developing on Windows platforms do not to mix debug and release binaries (for example the debug version of the gui using the release version of libtransform), this will lead to random memory corruptions and crashes.

## Apple OS specific

Unfortunately I only have a sporadic access to Apple hardware, so I can't test it regularly, and they may be some compilation issues.
Most of them should be around missing headers (Windows and Linux systems are kind of more flexible with headers definitions, if not lenient)
 
## OpenSSL plugin

> -DWITH_OPENSSL=ON

Defaults path for the libraries and headers have been hardcoded in the cmake configuration file for easier use. (i.e. C:\OpenSSL-Win32 and C:\OpenSSL-Win64)

Note: the OpenSSL plugin is less relevant with QT5, as built-in QT5 hashes replace most of the hashes offered by OpenSSL.

## Python27 and Python3 plugins

> shell$ cmake ../ -DWITH_PYTHON27=ON -DWITH_PYTHON3=ON

The cmake configuration does not check for the presence of the different libraries files needed, as it  get confused when there is a dual installation  (Python 2.7 and Python 3.x).

Defaults libraries path locations are hardcoded to make things easier for the default installation environment (i.e. C:\Python27\ and C:\Python34\).

One can specify the path by adding the following options for cmake:

Python 3 plugin
> -DPYTHON3_INCLUDE_DIRS=[PATH] -DPYTHON3_LIBRARIES=[PATH]

Python 2.7 plugin
> -DPYTHON27_INCLUDE_DIRS=[PATH] -DPYTHON27_LIBRARIES=[PATH]

For Windows download Python installation packages from:

https://www.python.org/downloads/

Be aware that you cannot mix x86 and x64 executables and DLLs, therefore be sure to choose the right architecture.
Unfortunately the official Python page is not clear on what version you are downloading (32bits). If you are not sure, you are probably downloading the x86 version, as the x64 version is clearly indicate in the name (amd64).

**Beware**: see bug https://github.com/nccgroup/pip3line/issues/4

## Distorm plugin

> -DWITH_DISTORM=ON

As with the other plugins, some defaults libraries path have been hard-coded, on the basis that the Distorm source tree is at the same level as the Pip3line source directory.

Distorm can be found on GitHub and it is recommended to use the latest git version:

git clone https://github.com/gdabah/distorm.git

It is relatively easy to compile (compare to OpenSSL for instance), even on Windows platforms (hint: Have a look into the 'make' directory)
