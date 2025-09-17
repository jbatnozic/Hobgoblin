:: Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
:: See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

@ECHO OFF

:::::::::::::::::::
:: SET VARIABLES ::
:::::::::::::::::::

SET PROFILE=hobgoblin-msvc-%2

SET PROFL_ARGS=--profile:host=%PROFILE% ^
               --profile:build=%PROFILE%

SET BUILD_ARGS=--build=missing --build=outdated --build=cascade

:: libzt won't build with C++20 for some reason
SET EXTRA_ARGS=--settings:host libzt*:compiler.cppstd=17 ^
               --settings:build libzt*:compiler.cppstd=17

::::::::::::::::::::
:: SELECT COMMAND ::
::::::::::::::::::::

IF "%1"=="clean"   GOTO clean
IF "%1"=="install" GOTO install
IF "%1"=="build"   GOTO build

ECHO Error: unknown command '%1' provided; exiting.
EXIT /B 1

::::::::::::::::::::
:: COMMAND BODIES ::
::::::::::::::::::::

:clean
IF EXIST _Build (
    RMDIR /S /Q _Build
)
IF EXIST CMakeUserPresets.json (
    DEL      /Q CMakeUserPresets.json
)
EXIT /B 0

:install
if "%3"=="" (
    conan install . -of _Build/Hobgoblin-x64/ ^
        %PROFL_ARGS% ^
        %BUILD_ARGS% ^
        %EXTRA_ARGS% ^
        -s:h build_type=Debug ^
        -s:b build_type=Debug
        
    conan install . -of _Build/Hobgoblin-x64/ ^
        %PROFL_ARGS% ^
        %BUILD_ARGS% ^
        %EXTRA_ARGS% ^
        -s:h build_type=Release ^
        -s:b build_type=Release

    REM To install RelWithDebInfo profile edit Conanw.bat.

    :: conan install . -of _Build/Hobgoblin-x64/ ^
    ::    %PROFL_ARGS% ^
    ::    %BUILD_ARGS% ^
    ::	  %EXTRA_ARGS% ^
    ::    -s:h build_type=RelWithDebInfo
    ::    -s:b build_type=RelWithDebInfo
) ELSE (
    conan install . -of _Build/Hobgoblin-x64/ ^
        %PROFL_ARGS% ^
        %BUILD_ARGS% ^
        %EXTRA_ARGS% ^
        -s:h build_type=%3 ^
        -s:b build_type=%3
)
EXIT /B 0

:build
conan build . -of _Build/Hobgoblin-x64/ ^
    %PROFL_ARGS% ^
    %BUILD_ARGS% ^
    -s:h build_type=%3 ^
    -s:b build_type=%3
EXIT /B 0
