@echo off
setlocal

set "ROOT=%~dp0.."
set "DLL=%ROOT%\bin\BimPropertyHandler.dll"
set "PROPDESC=%ROOT%\resources\BimVersion.propdesc"
set "HANDLER_CLSID={6a5be61c-19b6-4653-8fea-fb7a37a551ef}"

if not exist "%DLL%" (
    echo [ERROR] DLL not found: %DLL%
    echo Build it first with scripts\build_dll.bat
    goto :end
)

echo [1/4] Registering COM server...
C:\Windows\System32\regsvr32.exe /s "%DLL%"
if %errorlevel% neq 0 (
    echo [ERROR] regsvr32 failed.
    goto :end
)
echo       OK

echo [2/4] Copying propdesc to System32...
copy /Y "%PROPDESC%" "%SystemRoot%\System32\BimVersion.propdesc" >nul
if %errorlevel% neq 0 (
    echo [ERROR] Failed to copy propdesc.
    goto :end
)
echo       OK

echo [3/4] Registering property schema...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$path = '%SystemRoot%\System32\BimVersion.propdesc';" ^
  "Add-Type -TypeDefinition 'using System; using System.Runtime.InteropServices; public class PropSys { [DllImport(\"propsys.dll\", CharSet=CharSet.Unicode)] public static extern int PSRegisterPropertySchema(string pszPath); }';" ^
  "$hr = [PropSys]::PSRegisterPropertySchema($path); if ($hr -ne 0) { Write-Warning ('HRESULT 0x{0:X8}' -f $hr) } else { Write-Host 'Schema registered.' }"

echo [4/4] Associating handler with .rvt, .rfa, .ifc...
for %%E in (.rvt .rfa .ifc) do (
    reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\%%E" /ve /d "%HANDLER_CLSID%" /f >nul
)

echo.
echo [OK] Registered. Restart Explorer to see the column.
echo   taskkill /f /im explorer.exe ^&^& start explorer.exe

:end
echo.
pause
endlocal