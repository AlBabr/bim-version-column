#include "BimParser.h"
#include "ParserRegistry.h"
#include "BimVersionCache.h"

namespace {
std::wstring ToLowerW(const std::wstring& s) {
    std::wstring result = s;
    for (auto& c : result) {
        if (c >= L'A' && c <= L'Z') c = static_cast<wchar_t>(c - L'A' + L'a');
    }
    return result;
}

std::wstring GetExtension(const std::wstring& filePath) {
    size_t dotPos = filePath.find_last_of(L'.');
    if (dotPos == std::wstring::npos) return L"";
    return ToLowerW(filePath.substr(dotPos));
}
} // namespace

std::wstring GetBimVersion(const std::wstring& filePath) {
    std::wstring ext = GetExtension(filePath);
    if (ext.empty()) return L"";

    IBimParser* parser = ParserRegistry::Instance().FindByExtension(ext);
    if (!parser) return L"";  // unsupported format — silently return empty

    // 1. Cache
    std::wstring cached = BimVersionCache::Read(filePath);
    if (!cached.empty()) return cached;

    // 2. Parser
    std::wstring version = parser->GetVersion(filePath);

    // 3. Save to cache
    if (!version.empty()) {
        BimVersionCache::Write(filePath, version);
    }

    return version;
}

// Kept for backward compatibility; the real logic lives in IfcParser.
std::wstring GetIfcVersion(const std::wstring& filePath) {
    IBimParser* parser = ParserRegistry::Instance().FindByExtension(L".ifc");
    if (!parser) return L"";
    return parser->GetVersion(filePath);
}