# Building BIM Version Column

## Requirements

- Windows 10 or 11 (x64)
- Visual Studio 2022 or Build Tools 2022
- Workload: **Desktop development with C++**

## Quick start

Open an **x64 Native Tools Command Prompt for VS 2022** at the repository root.

Build the Shell extension DLL:

```cmd
scripts\build_dll.bat

Build the Console Test utility:
scripts\build_console.bat

Both outputs are in bin.

Testing (console):
bin\BimVersionTest.exe "C:\Models\Test.rvt"
Expected outpuut:
[C:\Models\Test.rvt] BIM version: rvt2024


Manual registration:
C:\Windows\System32\regsvr32.exe "bin\BimPropertyHandler.dll"
copy resources\BimVersion.propdesc %SystemRoot%\System32\
:: then call PSRegisterPropertySchema on the copied file
reg add "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers\.rvt" /ve /d "{12345678-1234-1234-1234-1234567890AB}" /f
:: repeat for .rfa and .ifc