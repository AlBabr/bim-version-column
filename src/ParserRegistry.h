#pragma once
#include "IBimParser.h"
#include <memory>
#include <vector>
#include <string>

class ParserRegistry {
public:
    static ParserRegistry& Instance();

    // Registers a parser. Called once during registry initialization.
    void Register(std::unique_ptr<IBimParser> parser);

    // Returns the parser that supports the given extension (lowercase,
    // with leading dot, e.g. L".rvt"), or nullptr if none matches.
    IBimParser* FindByExtension(const std::wstring& extension) const;

    // For diagnostics and logging.
    std::vector<std::wstring> GetAllExtensions() const;

private:
    ParserRegistry();
    ParserRegistry(const ParserRegistry&) = delete;
    ParserRegistry& operator=(const ParserRegistry&) = delete;

    std::vector<std::unique_ptr<IBimParser>> m_parsers;
};