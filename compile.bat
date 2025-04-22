@echo off
set "VCPATH=C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC"
set "WKITPATH=C:\Program Files (x86)\Windows Kits\10"
set "UCRTPATH=%WKITPATH%\Include\10.0.22621.0\ucrt"
set "UMPATH=%WKITPATH%\Include\10.0.22621.0\um"
set "SHAREDPATH=%WKITPATH%\Include\10.0.22621.0\shared"

set "PATH=%VCPATH%\Tools\MSVC\14.43.34808\bin\Hostx64\x64;%PATH%"
set "INCLUDE=%VCPATH%\Tools\MSVC\14.43.34808\include;%UCRTPATH%;%UMPATH%;%SHAREDPATH%"
set "LIB=%VCPATH%\Tools\MSVC\14.43.34808\lib\x64;%WKITPATH%\Lib\10.0.22621.0\ucrt\x64;%WKITPATH%\Lib\10.0.22621.0\um\x64"

set "OUTDIR=%CD%\x64\Release"

echo Cleaning old files...
if exist *.obj del *.obj
if exist ImageSearch.exe del ImageSearch.exe
if not exist "%OUTDIR%" mkdir "%OUTDIR%"

echo Building test image generator...
"%VCPATH%\Tools\MSVC\14.43.34808\bin\Hostx64\x64\cl.exe" /nologo /EHsc /O2 /fp:fast /I. generate_test_images.cpp /Fe:generate_test_images.exe /link FreeImage.lib

if errorlevel 1 (
    echo Failed to build test image generator
    exit /b 1
)

echo Generating test images...
generate_test_images.exe

echo Building performance tests...
"%VCPATH%\Tools\MSVC\14.43.34808\bin\Hostx64\x64\cl.exe" /nologo /EHsc /O2 /openmp /fp:fast /arch:AVX2 /I. /c /DWIN32 /D_WINDOWS performance_tests.cpp

echo Building main program...
"%VCPATH%\Tools\MSVC\14.43.34808\bin\Hostx64\x64\cl.exe" /nologo /EHsc /O2 /openmp /fp:fast /arch:AVX2 /I. /c /DWIN32 /D_WINDOWS main.cpp

echo Linking...
"%VCPATH%\Tools\MSVC\14.43.34808\bin\Hostx64\x64\link.exe" /nologo /out:"%OUTDIR%\ImageSearch.exe" main.obj performance_tests.obj FreeImage.lib gdi32.lib user32.lib kernel32.lib /LIBPATH:"%VCPATH%\Tools\MSVC\14.43.34808\lib\x64" /LIBPATH:"%WKITPATH%\Lib\10.0.22621.0\ucrt\x64" /LIBPATH:"%WKITPATH%\Lib\10.0.22621.0\um\x64" /SUBSYSTEM:CONSOLE

if errorlevel 1 (
    echo Compilation failed
    exit /b 1
)

echo Copying DLL and test images...
copy FreeImage.dll "%OUTDIR%\" >nul
copy test_image_*.png "%OUTDIR%\" >nul

echo Running performance tests...
"%OUTDIR%\ImageSearch.exe" --perf-test
set TEST_RESULT=%ERRORLEVEL%

if %TEST_RESULT% NEQ 0 (
    echo Performance tests failed with error code %TEST_RESULT%
    exit /b %TEST_RESULT%
)

echo All tests completed successfully
exit /b 0
