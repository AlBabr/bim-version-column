#include "BimVersionCache.h"
#include <windows.h>

namespace {
const wchar_t* kStreamName = L":BimVersion";
}

std::wstring BimVersionCache::Read(const std::wstring& filePath) {
    std::wstring adsPath = filePath + kStreamName;
    HANDLE h = CreateFileW(adsPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
                           nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return L"";

    char buffer[64] = {0};
    DWORD read = 0;
    ReadFile(h, buffer, sizeof(buffer) - 1, &read, nullptr);
    CloseHandle(h);

    if (read > 0) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, buffer,
                                       static_cast<int>(read), nullptr, 0);
        std::wstring result(wlen, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, buffer,
                            static_cast<int>(read), &result[0], wlen);
        return result;
    }
    return L"";
}

void BimVersionCache::Write(const std::wstring& filePath, const std::wstring& version) {
    if (version.empty()) return;

    std::wstring adsPath = filePath + kStreamName;
    HANDLE h = CreateFileW(adsPath.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;

    int len = WideCharToMultiByte(CP_UTF8, 0, version.c_str(), -1,
                                  nullptr, 0, nullptr, nullptr);
    std::string utf8(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, version.c_str(), -1, &utf8[0], len,
                        nullptr, nullptr);

    DWORD written = 0;
    WriteFile(h, utf8.c_str(), static_cast<DWORD>(utf8.size()), &written, nullptr);
    CloseHandle(h);
}