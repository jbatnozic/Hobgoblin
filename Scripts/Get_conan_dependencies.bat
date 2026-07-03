@ECHO OFF 

SET LIBZT_COMMITHASH=19bc10691b99c89c760c24cfb27f5256ae18ec7c
SET LIBZT_VERSION=3.0.4

SET ZTCPP_COMMITHASH=f1e853ad1ee4fcaa378bf920e46b9448c873234e
SET ZTCPP_VERSION=3.0.4

ECHO "Looking for git..."
where git
IF %ERRORLEVEL% EQU 0 GOTO foundgit
EXIT /b 1
:foundgit

ECHO "Looking for conan..."
where conan
IF %ERRORLEVEL% EQU 0 GOTO foundconan
EXIT /b 1
:foundconan

ECHO "All required tools found!"

MKDIR Build.Temp
CD Build.Temp

ECHO "Getting libzt..."
git clone https://github.com/jbatnozic/libzt-conan
CD libzt-conan\Conan2.x
git checkout %LIBZT_COMMITHASH%
conan export . --version %LIBZT_VERSION% --user jbatnozic --channel stable
IF %ERRORLEVEL% EQU 0 GOTO exportedlibzt
CD ..\..
EXIT /b 1
:exportedlibzt
CD ..\..

ECHO "Getting ZTCpp..."
git clone https://github.com/jbatnozic/ztcpp
CD ztcpp
git checkout %ZTCPP_COMMITHASH%
conan export . --version %ZTCPP_VERSION% --user jbatnozic --channel stable
IF %ERRORLEVEL% EQU 0 GOTO exportedztcpp
CD ..
EXIT /b 1
:exportedztcpp
CD ..

ECHO "All required Conan recipes exported!"

REM Clean up
CD ..
DEL /f /s /q Build.Temp 1>NUL
RMDIR /s /q Build.Temp

PAUSE
