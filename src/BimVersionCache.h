#pragma once
#include <string>

// Simple version cache backed by NTFS Alternate Data Streams.
// On FAT32 or network drives the operations silently no-op,
// returning an empty string so the parser runs again.
class BimVersionCache {
public:
    static std::wstring Read(const std::wstring& filePath);
    static void Write(const std::wstring& filePath, const std::wstring& version);
};