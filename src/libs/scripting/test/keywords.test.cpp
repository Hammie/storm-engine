#include "storm/scripting/keywords.hpp"

#include <catch2/catch.hpp>

using namespace storm::scripting;

TEST_CASE("Parse tokens", "[compiler]")
{
    KeywordManager keyword_manager;

    SECTION("'#hold'")
    {
        CHECK(keyword_manager.getTokenForKeyword("#hold") == TokenType::HOLD_COMPILATION);
    }

    SECTION("'#include'")
    {
        CHECK(keyword_manager.getTokenForKeyword("#include") == TokenType::INCLIDE_FILE);
    }
}

TEST_CASE("Tokens are case insensitive", "[compiler]")
{
    KeywordManager keyword_manager;

    CHECK(keyword_manager.getTokenForKeyword("return") == TokenType::FUNCTION_RETURN);
    CHECK(keyword_manager.getTokenForKeyword("Return") == TokenType::FUNCTION_RETURN);
    CHECK(keyword_manager.getTokenForKeyword("RETURN") == TokenType::FUNCTION_RETURN);
    CHECK(keyword_manager.getTokenForKeyword("rEtUrN") == TokenType::FUNCTION_RETURN);
}