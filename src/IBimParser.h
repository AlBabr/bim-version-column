#pragma once
#include <string>
#include <vector>

// Interface for a BIM file parser.
// Each parser is responsible for one or more file formats.
class IBimParser {
public:
    virtual ~IBimParser() = default;

    // Returns the list of extensions (lowercase, with leading dot)
    // this parser handles, e.g. { L".rvt", L".rfa" } or { L".ifc" }.
    virtual std::vector<std::wstring> GetSupportedExtensions() const = 0;

    // Extracts the version string from the file.
    // Returns e.g. "rvt2024", "ifc2x3", or an empty string on failure.
    virtual std::wstring GetVersion(const std::wstring& filePath) const = 0;

    // Human-readable parser name (for logs and diagnostics).
    virtual std::wstring GetName() const = 0;
};