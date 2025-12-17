@echo off
echo Building COC project with CMake...
echo.

REM 检查 Cocos2d-x 路径是否配置
if "%COCOS2DX_ROOT%"=="" (
    echo Warning: COCOS2DX_ROOT environment variable is not set.
    echo Please set it or edit config.cmake file.
    echo.
)

if not exist build mkdir build
cd build

set VS_GENERATOR=Visual Studio 17 2022

echo Using generator: %VS_GENERATOR%
echo.

REM 如果设置了 COCOS2DX_ROOT 环境变量，传递给 CMake
if not "%COCOS2DX_ROOT%"=="" (
    cmake .. -G "%VS_GENERATOR%" -A Win32 -DCOCOS2DX_ROOT="%COCOS2DX_ROOT%" -Wno-deprecated
) else (
    cmake .. -G "%VS_GENERATOR%" -A Win32
)

if %errorlevel% neq 0 (
    cd ..
    echo.
    echo CMake configuration failed!
    echo Please check:
    echo   1. Cocos2d-x path is correctly set in environment variable
    echo   2. CMake is installed and in PATH
    echo   3. Visual Studio is installed
    pause
    exit /b %errorlevel%
)

echo Building cocos2d (Release)...
cmake --build . --config Release --target cocos2d
if %errorlevel% neq 0 (
    cd ..
    echo Build cocos2d library failed!
    pause
    exit /b %errorlevel%
)

echo Building COC (Release)...
cmake --build . --config Release --target COC
if %errorlevel% neq 0 (
    cd ..
    echo Build COC project failed!
    pause
    exit /b %errorlevel%
)

echo ================================================
echo Build completed successfully!

cd ..

