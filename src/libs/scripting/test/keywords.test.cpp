#include "storm/scripting/keywords.hpp"

#include <catch2/catch.hpp>

using namespace storm::scripting;

TEST_CASE("Parse tokens", "[compiler]")
{
    KeywordManager keyword_manager;

    SECTION("'#hold'")
    {
        CHECK(keyword_manager.getTokenForKeyword("#hold") == S_TOKEN_TYPE::HOLD_COMPILATION);
    }

    SECTION("'#include'")
    {
        CHECK(keyword_manager.getTokenForKeyword("#include") == S_TOKEN_TYPE::INCLIDE_FILE);
    }
}

