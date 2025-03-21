@ECHO OFF 

SET LIBZT_COMMITHASH=daf70601c9e784d0bd6e5b672d4d4bf44d92d351
SET LIBZT_VERSION=3.0.3

SET ZTCPP_COMMITHASH=830cfd291270dbf0c02a04e6c3781fb68f544f1e
SET ZTCPP_VERSION=3.0.3

SET RMLUI_COMMITHASH=33dc5517c883edf9bf54f9339bf223333e8f79c4
SET RMLUI_VERSION=5.1

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

ECHO "Getting RmlUi..."
git clone https://github.com/jbatnozic/rmlui-conan
CD rmlui-conan
git checkout %RMLUI_COMMITHASH%
conan export . --version %RMLUI_VERSION% --user jbatnozic --channel stable
IF %ERRORLEVEL% EQU 0 GOTO exportedrmlui
CD ..
EXIT /b 1
:exportedrmlui
CD ..

ECHO "All required Conan recipes exported!"

REM Clean up
CD ..
DEL /f /s /q Build.Temp 1>NUL
RMDIR /s /q Build.Temp

PAUSE
