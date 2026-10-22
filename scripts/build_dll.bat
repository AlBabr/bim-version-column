
### scripts/build_dll.bat

```batch
@echo off
setlocal

set "ROOT=%~dp0.."
set "SRC=%ROOT%\src"
set "OUT=%ROOT%\bin"

if not exist "%OUT%" mkdir "%OUT%"

pushd "%SRC%"

cl /nologo /std:c++17 /EHsc /W4 /O2 /utf-8 /DUNICODE /D_UNICODE /LD ^
   BimParser.cpp ParserRegistry.cpp RevitParser.cpp IfcParser.cpp BimVersionCache.cpp BimPropertyHandler.cpp ^
   /Fe:"%OUT%\BimPropertyHandler.dll" ^
   /link /DEF:BimPropertyHandler.def ^
   /SUBSYSTEM:WINDOWS propsys.lib ole32.lib shlwapi.lib advapi32.lib

set "RC=%errorlevel%"
popd

if %RC% neq 0 (
    echo.
    echo [ERROR] Build failed.
    exit /b %RC%
)

echo.
echo [OK] Built %OUT%\BimPropertyHandler.dll
endlocal