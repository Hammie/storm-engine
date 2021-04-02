#ifndef STORM_ENGINE_KEYWORDS_HPP
#define STORM_ENGINE_KEYWORDS_HPP
#pragma once

#include "tokens.hpp"

#include <string>

namespace storm::scripting {

class KeywordManager {
  public:
    TokenType getTokenForKeyword (const std::string_view& keyword);
};

} // namespace storm::scripting

#endif // STORM_ENGINE_KEYWORDS_HPP
