#include "RevitParser.h"
#include <windows.h>
#include <objbase.h>
#include <combaseapi.h>
#include <vector>
#include <cstdint>

std::vector<std::wstring> RevitParser::GetSupportedExtensions() const {
    return { L".rvt", L".rfa" };
}

std::vector<uint8_t> RevitParser::ReadBasicFileInfoStream(const std::wstring& path) {
    std::vector<uint8_t> result;

    HRESULT hrInit = CoInitialize(nullptr);

    IStorage* storage = nullptr;
    HRESULT hr = StgOpenStorage(path.c_str(), nullptr,
                                STGM_READ | STGM_SHARE_DENY_WRITE, nullptr, 0, &storage);

    if (SUCCEEDED(hr) && storage) {
        IStream* stream = nullptr;
        hr = storage->OpenStream(L"BasicFileInfo", nullptr,
                                 STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &stream);

        if (SUCCEEDED(hr) && stream) {
            STATSTG stat{};
            if (SUCCEEDED(stream->Stat(&stat, STATFLAG_NONAME))) {
                result.resize(static_cast<size_t>(stat.cbSize.QuadPart));
                ULONG read = 0;
                stream->Read(result.data(), static_cast<ULONG>(result.size()), &read);
                result.resize(read);
            }
            stream->Release();
        }
        storage->Release();
    }

    if (SUCCEEDED(hrInit)) {
        CoUninitialize();
    }

    return result;
}

std::wstring RevitParser::DecodeUtf16Le(const std::vector<uint8_t>& bytes, size_t startOffset) {
    std::wstring result;
    if (startOffset >= bytes.size()) return result;
    size_t count = (bytes.size() - startOffset) / 2;
    result.resize(count);
    for (size_t i = 0; i < count; ++i) {
        uint16_t ch = static_cast<uint16_t>(bytes[startOffset + i * 2]) |
                      (static_cast<uint16_t>(bytes[startOffset + i * 2 + 1]) << 8);
        result[i] = static_cast<wchar_t>(ch);
    }
    return result;
}

std::wstring RevitParser::ExtractVersionFromText(const std::wstring& text) {
    // Modern format (2019+): "Format: 2024"
    std::wstring::size_type fmtPos = text.find(L"Format:");
    if (fmtPos != std::wstring::npos) {
        size_t i = fmtPos + 7;
        while (i < text.size() && (text[i] == L' ' || text[i] == L'\t')) ++i;
        if (i + 4 <= text.size()) {
            std::wstring year = text.substr(i, 4);
            bool allDigits = true;
            for (wchar_t c : year) {
                if (c < L'0' || c > L'9') { allDigits = false; break; }
            }
            if (allDigits) return L"rvt" + year;
        }
    }

    // Legacy format (2016-2018): "Revit Build: Autodesk Revit 2018 (...)"
    std::wstring::size_type buildPos = text.find(L"Revit Build:");
    if (buildPos != std::wstring::npos) {
        for (size_t i = buildPos + 12; i + 4 <= text.size(); ++i) {
            bool allDigits = true;
            for (size_t j = 0; j < 4; ++j) {
                wchar_t c = text[i + j];
                if (c < L'0' || c > L'9') { allDigits = false; break; }
            }
            if (allDigits) return L"rvt" + text.substr(i, 4);
        }
    }
    return L"";
}

std::wstring RevitParser::GetVersion(const std::wstring& filePath) const {
    std::vector<uint8_t> streamBytes = ReadBasicFileInfoStream(filePath);
    if (streamBytes.empty()) return L"";

    // BasicFileInfo may be offset by one byte — try both alignments.
    for (size_t offset = 0; offset <= 1; ++offset) {
        std::wstring text = DecodeUtf16Le(streamBytes, offset);
        std::wstring version = ExtractVersionFromText(text);
        if (!version.empty()) return version;
    }
    return L"";
}