#include "BimParser.h"
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

namespace {

void printUsage() {
    std::wcerr << L"Usage: BimVersionTest.exe <file1> [<file2> ...]\n"
               << L"Supported: .rvt, .rfa, .ifc\n";
}

// Возвращает true, если файл существует
bool fileExists(const std::wstring& path) {
    DWORD attrs = GetFileAttributesW(path.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

// Обрабатывает один файл. Возвращает 0 при успехе, ненулевой код при ошибке.
int processFile(const std::wstring& path) {
    if (!fileExists(path)) {
        std::wcerr << L"[" << path << L"] File not found or is a directory.\n";
        return 2;
    }

    std::wstring version = GetBimVersion(path);
    if (version.empty()) {
        std::wcerr << L"[" << path << L"] Version not detected.\n";
        return 3;
    }

    std::wcout << L"[" << path << L"] BIM version: " << version << L"\n";
    return 0;
}

} // namespace

int wmain(int argc, wchar_t** argv) {
    // Unicode (UTF-16 via WriteConsoleW)
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
    if (argc < 2) {
        printUsage();
        return 1;
    }

    int exitCode = 0;
    for (int i = 1; i < argc; ++i) {
        int rc = processFile(argv[i]);
        if (rc != 0) exitCode = rc;
    }
    return exitCode;
}