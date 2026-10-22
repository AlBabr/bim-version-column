#include "IfcParser.h"
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

namespace {
std::wstring ToLowerW(const std::wstring& s) {
    std::wstring result = s;
    for (auto& c : result) {
        if (c >= L'A' && c <= L'Z') c = static_cast<wchar_t>(c - L'A' + L'a');
    }
    return result;
}

std::wstring TrimW(const std::wstring& s) {
    size_t start = 0;
    while (start < s.size() && (s[start] == L' ' || s[start] == L'\t' ||
                                 s[start] == L'\r' || s[start] == L'\n')) ++start;
    size_t end = s.size();
    while (end > start && (s[end - 1] == L' ' || s[end - 1] == L'\t' ||
                            s[end - 1] == L'\r' || s[end - 1] == L'\n')) --end;
    return s.substr(start, end - start);
}
} // namespace

std::vector<std::wstring> IfcParser::GetSupportedExtensions() const {
    return { L".ifc" };
}

std::wstring IfcParser::NormalizeSchemaIdentifier(const std::wstring& identifier) {
    std::wstring token = ToLowerW(TrimW(identifier));
    if (token.rfind(L"ifc4x3", 0) == 0) return L"ifc4x3";
    if (token.rfind(L"ifc4x1", 0) == 0) return L"ifc4x1";
    if (token.rfind(L"ifc4", 0) == 0)   return L"ifc4";
    if (token.rfind(L"ifc2x3", 0) == 0) return L"ifc2x3";
    if (token.rfind(L"ifc2x2", 0) == 0) return L"ifc2x2";
    if (token.rfind(L"ifc2x", 0) == 0)  return L"ifc2x";
    return L"";
}

std::wstring IfcParser::GetVersion(const std::wstring& filePath) const {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return L"";

    constexpr size_t HEADER_WINDOW = 64 * 1024;
    std::vector<char> buffer(HEADER_WINDOW);
    file.read(buffer.data(), static_cast<std::streamsize>(HEADER_WINDOW));
    std::streamsize bytesRead = file.gcount();
    if (bytesRead <= 0) return L"";
    buffer.resize(static_cast<size_t>(bytesRead));

    std::string data(buffer.begin(), buffer.end());
    std::string upper = data;
    for (auto& c : upper) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    }

    size_t schemaPos = upper.find("FILE_SCHEMA");
    if (schemaPos == std::string::npos) return L"";

    size_t pos = schemaPos + 11;
    while (pos < upper.size() && upper[pos] != '(') ++pos;
    if (pos >= upper.size()) return L"";
    ++pos;

    while (pos < upper.size() &&
           (upper[pos] == '(' || upper[pos] == ' ' || upper[pos] == '\t')) ++pos;
    if (pos >= upper.size() || upper[pos] != '\'') return L"";

    size_t quoteStart = pos + 1;
    size_t quoteEnd = data.find('\'', quoteStart);
    if (quoteEnd == std::string::npos || quoteEnd == quoteStart) return L"";

    std::string identifier = data.substr(quoteStart, quoteEnd - quoteStart);
    std::wstring wid(identifier.begin(), identifier.end());
    return NormalizeSchemaIdentifier(wid);
}