#include "storm/scripting/compiler.hpp"

#include <catch2/catch.hpp>

using namespace storm::scripting;

TEST_CASE("Get next token length", "[compiler]")
{
    SECTION("Empty input")
    {
        CHECK(Compiler::getNextTokenLength("") == 0);
    }

    SECTION("Semicolon ';'")
    {
        CHECK(Compiler::getNextTokenLength(";some more code") == 0);
    }

    SECTION("Code delimiters")
    {
        auto symbol = GENERATE(as<std::string>{}, "{", "}", "(", ")", "[", "]", ":", ",");
        CHECK(Compiler::getNextTokenLength(symbol + "some more code") == 1);
    }

    SECTION("Single-character operators")
    {
        const std::string &symbol = GENERATE("*", "/", "+", "-", "%", "^", "&", "!", "<", ">", "=", ".");
        CHECK(Compiler::getNextTokenLength(symbol + "some more code") == 1);
    }

    SECTION("Double-character operators")
    {
        const std::string &symbol = GENERATE("*=", "/=", "+=", "++", "-=", "--", "&&",  "!=", "<=", ">=", "==");
        CHECK(Compiler::getNextTokenLength(symbol + "some more code") == 2);
    }
}
