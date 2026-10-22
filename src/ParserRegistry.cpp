#include "ParserRegistry.h"
#include "RevitParser.h"
#include "IfcParser.h"
#include <algorithm>
#include <cctype>

ParserRegistry& ParserRegistry::Instance() {
    static ParserRegistry instance;
    return instance;
}

ParserRegistry::ParserRegistry() {
    // === THE ONLY PLACE TO EDIT WHEN ADDING A NEW FORMAT ===
    Register(std::make_unique<RevitParser>());
    Register(std::make_unique<IfcParser>());
    // Register(std::make_unique<NavisworksParser>());  // once ready
}

void ParserRegistry::Register(std::unique_ptr<IBimParser> parser) {
    if (parser) {
        m_parsers.push_back(std::move(parser));
    }
}

IBimParser* ParserRegistry::FindByExtension(const std::wstring& extension) const {
    for (const auto& parser : m_parsers) {
        auto exts = parser->GetSupportedExtensions();
        if (std::find(exts.begin(), exts.end(), extension) != exts.end()) {
            return parser.get();
        }
    }
    return nullptr;
}

std::vector<std::wstring> ParserRegistry::GetAllExtensions() const {
    std::vector<std::wstring> result;
    for (const auto& parser : m_parsers) {
        auto exts = parser->GetSupportedExtensions();
        result.insert(result.end(), exts.begin(), exts.end());
    }
    return result;
}