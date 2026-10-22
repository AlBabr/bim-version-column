# BIM Version Column for Windows Explorer

A lightweight Windows Shell extension that adds a **BIM Version** column to
File Explorer, showing the actual version of Revit (`.rvt`, `.rfa`) and IFC
(`.ifc`) files directly in the details view.

## Why

Sorting and filtering BIM files in Explorer is painful when all you see is
"file.rvt". Which version was this saved in? Revit 2021? 2024? An IFC2X3 or
an IFC4? This extension answers that at a glance, without opening any
authoring tool.

## Features

- **Revit** (`.rvt`, `.rfa`): reads the `BasicFileInfo` OLE stream and parses
  the `Format:` field (Revit 2019+) or `Revit Build:` field (Revit 2016–2018).
- **IFC** (`.ifc`): reads the STEP header and extracts the `FILE_SCHEMA`
  identifier (`IFC2X3`, `IFC4`, `IFC4X3`, ...).
- **Fast**: results are cached in an NTFS Alternate Data Stream
  (`:BimVersion`) next to each file. On FAT32/network shares the cache is
  silently skipped — the parser just runs again.
- **Native C++**: no .NET, no external dependencies, no background processes.
- **Open source**: build the DLL yourself, inspect the code, and verify the
  binary matches.

## What it does NOT do

- It does not open files in Revit, IFC viewers, or any other application.
- It does not modify your BIM files. Only a hidden NTFS metadata stream is
  written (the same mechanism Windows itself uses for thumbnails and tags).

## Building from source

Requirements:
- Windows 10 / 11
- Visual Studio 2022 with the **Desktop development with C++** workload
- An **x64 Native Tools Command Prompt for VS 2022**

Then:

```cmd
cd scripts
build_dll.bat

Optionally build the console test utility:
build_console.bat
bin\BimVersionTest.exe "D:\Models\sample.rvt"