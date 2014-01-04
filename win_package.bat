@echo off

IF "%1"=="" goto Usage
SET GOTVERSION=false
IF NOT "%1"=="2012" SET GOTVERSION=true 
IF NOT "%1"=="2010" SET GOTVERSION=true 
IF NOT "%1"=="2012OGL" SET GOTVERSION=true 
IF NOT "%1"=="2010OGL" SET GOTVERSION=true 

IF "%GOTVERSION%"=="false" GOTO Usage
SET ANGLEUSE=false
IF "%1"=="2012" SET ANGLEUSE=true 
IF "%1"=="2010" SET ANGLEUSE=true 

SET VSVERSION=%1
REM Default Qt dir
SET "BASE_DIR_QT=C:\Qt\Qt5.2.0\5.2.0"
REM this variable is set for the MSVC2010/OpenGL version of the Qt binaries
REM to be changed if necessary

IF "%VSVERSION%"=="2010OGL" set QT_LIBS="%BASE_DIR_QT%\msvc2010_opengl"
IF "%VSVERSION%"=="2010" set QT_LIBS=%BASE_DIR_QT%"\msvc2010"
IF "%VSVERSION%"=="2012OGL" set QT_LIBS=%BASE_DIR_QT%"\msvc2012_64_opengl"
IF "%VSVERSION%"=="2012" set QT_LIBS=%BASE_DIR_QT%"\msvc2012_64"



IF NOT EXIST %QT_LIBS% GOTO Nonexistentdir

REM 
REM copying binaries to the package directory
REM
@setlocal enableextensions enabledelayedexpansion

echo [1] Copying pip3line binaries 
md package > nul 2> nul
cd package
copy "..\lib\transform.dll"
copy "..\bin\pip3line.exe"
copy "..\bin\pip3linecmd.exe"
md plugins > nul 2> nul
copy "..\bin\plugins\*.dll" ".\plugins"


REM QT libraries
echo [2] Copying QT libraries
copy %QT_LIBS%"\bin\Qt5Core.dll"
copy %QT_LIBS%"\bin\Qt5Gui.dll"
copy %QT_LIBS%"\bin\Qt5Network.dll"
copy %QT_LIBS%"\bin\Qt5Svg.dll"
copy %QT_LIBS%"\bin\Qt5Widgets.dll"
copy %QT_LIBS%"\bin\Qt5XmlPatterns.dll"
copy %QT_LIBS%"\bin\icudt51.dll"
copy %QT_LIBS%"\bin\icuin51.dll"
copy %QT_LIBS%"\bin\icuuc51.dll"

echo [3] Copying QT mandatory plugins
REM QT mandatory plug-ins, otherwise the application won't even start
md platforms
copy %QT_LIBS%"\plugins\platforms\qminimal.dll" ".\platforms"
copy %QT_LIBS%"\plugins\platforms\qwindows.dll" ".\platforms"

echo [4] Copying distorm lib (if present)
REM Distorm3 lib
SET "DISTORM_PATH=..\..\distorm"
IF NOT EXIST %DISTORM_PATH% GOTO Nonexistentdistorm
copy %DISTORM_PATH%"\distorm3.dll"

echo [5] Copying OpenSSL lib (if present)
REM Openssl lib (default is 32 bit, change for anything else if needed)
SET "OPENSSL_PATH=C:\OpenSSL-Win32"
IF "%PROCESSOR_ARCHITECTURE%"=="AMD64" SET "OPENSSL_PATH=C:\OpenSSL-Win64" 
IF NOT EXIST %OPENSSL_PATH% GOTO NonexistentOpenssl
copy %OPENSSL_PATH%"\bin\ssleay32.dll"
copy %OPENSSL_PATH%"\bin\libeay32.dll"

REM if you are using the ANGLE version of the Qt windows binaries
REM you need to copy a couple of more files
IF "%ANGLEUSE%"=="false" GOTO EndInstall
echo [6] Copying ANGLE Libraries
copy %QT_LIBS%"\bin\d3dcompiler_46.dll"
copy %QT_LIBS%"\bin\libEGL.dll"
copy %QT_LIBS%"\bin\libGLESv2.dll"



:EndInstall
cd ..
GOTO End

:Usage
ECHO.
ECHO Need Qt Visual Studio version, i.e.
ECHO %~n0.bat 2010
ECHO 
ECHO Available versions : 
ECHO     2012OGL : VS 2012 with OpenGL
ECHO     2012    : VS 2012 (using ANGLE)
ECHO     2010OGL : VS 2010 with OpenGL
ECHO     2010    : VS 2010 (using ANGLE)
GOTO End

:Nonexistentdir
ECHO.
ECHO The directory %QT_LIBS% does not exist
GOTO End

:Nonexistentdistorm
ECHO The directory %DISTORM_PATH% not found, skipping.
GOTO End

:NonexistentOpenssl
ECHO The directory %OPENSSL_PATH% not found, skipping.
GOTO End

:End
ENDLOCAL
