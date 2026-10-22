#pragma once
#include "IBimParser.h"

class RevitParser : public IBimParser {
public:
    std::vector<std::wstring> GetSupportedExtensions() const override;
    std::wstring GetVersion(const std::wstring& filePath) const override;
    std::wstring GetName() const override { return L"RevitParser"; }

private:
    // Revit-specific helpers.
    static std::vector<uint8_t> ReadBasicFileInfoStream(const std::wstring& path);
    static std::wstring DecodeUtf16Le(const std::vector<uint8_t>& bytes, size_t startOffset);
    static std::wstring ExtractVersionFromText(const std::wstring& text);
};