REM this script must set QTDIR32 path to the root of the Qt folder. Example:
REM set QTDIR32=k:\Qt\5\5.4\msvc2013_opengl\

call set_qt_paths.bat

SETLOCAL

RMDIR /S /Q binaries\

if exist "%ProgramW6432%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
    call "%ProgramW6432%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
) else (
    if exist "%ProgramW6432%\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" (
        call "%ProgramW6432%\Microsoft Visual Studio\2022\Preview\VC\Auxiliary\Build\vcvarsall.bat" amd64
    ) else (
        call "%ProgramW6432%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" amd64
    )
)

pushd ..\
del .qmake.stash
%QTDIR64%\bin\qmake.exe -tp vc -r
popd

call "%VS_TOOLS_DIR%VsDevCmd.bat" x86
msbuild ..\PrimitiveCameraViewer.sln /t:Build /p:Configuration=Release;Platform="x64";PlatformToolset=v143

xcopy /R /Y ..\bin\release\x64\PrimitiveCameraViewer.exe binaries\
::xcopy /R /Y "3rdparty binaries"\32\* binaries\

SETLOCAL
SET PATH=%QTDIR64%\bin\
%QTDIR64%\bin\windeployqt.exe --dir binaries\Qt --force --release --release --force --no-system-d3d-compiler --no-compiler-runtime --no-translations binaries\PrimitiveCameraViewer.exe
ENDLOCAL

if exist "%systemroot%\Sysnative\" (
    set SYS64=%systemroot%\Sysnative
) else (
    set SYS64=%systemroot%\System32
)

echo %SYS64%

%SYS64%\cmd.exe /c "xcopy /R /Y %SystemRoot%\System32\msvcp140.dll binaries\64\msvcr\"
if not %errorlevel% == 0 goto dll_not_found
%SYS64%\cmd.exe /c "xcopy /R /Y %SystemRoot%\System32\msvcp140_1.dll binaries\64\msvcr\"
if not %errorlevel% == 0 goto dll_not_found
%SYS64%\cmd.exe /c "xcopy /R /Y %SystemRoot%\System32\vcruntime140.dll binaries\64\msvcr\"
if not %errorlevel% == 0 goto dll_not_found
%SYS64%\cmd.exe /c "xcopy /R /Y %SystemRoot%\System32\vcruntime140_1.dll binaries\64\msvcr\"
if not %errorlevel% == 0 goto dll_not_found

if not defined WIN_SDK (
    if exist "%programfiles(x86)%\Windows Kits\10\Redist\10.0.19041.0" (
        set WIN_SDK=10.0.19041.0
    ) else (
        set WIN_SDK=10.0.18362.0
    )
)

xcopy /R /Y "%programfiles(x86)%\Windows Kits\10\Redist\%WIN_SDK%\ucrt\DLLs\x64\*" binaries\64\msvcr\
if %ERRORLEVEL% GEQ 1 goto windows_sdk_not_found

del binaries\Qt\opengl*.*
del binaries\Qt\lib*gl*.*

ENDLOCAL

"%programfiles(x86)%\Inno Setup 6\iscc" setup.iss
exit /B 0

:build_fail
ENDLOCAL
echo Build failed
pause
exit /b 1

:dll_not_found
ENDLOCAL
echo VC++ Redistributable DLL not found
pause
exit /b 1

:windows_sdk_not_found
ENDLOCAL
echo Windows SDK not found (required for CRT DLLs)
pause
exit /b 1