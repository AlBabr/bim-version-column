#pragma once
#include "IBimParser.h"

class IfcParser : public IBimParser {
public:
    std::vector<std::wstring> GetSupportedExtensions() const override;
    std::wstring GetVersion(const std::wstring& filePath) const override;
    std::wstring GetName() const override { return L"IfcParser"; }

private:
    static std::wstring NormalizeSchemaIdentifier(const std::wstring& identifier);
};