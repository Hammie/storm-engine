#include "storm/string_replace.hpp"

#include <catch2/catch.hpp>

TEST_CASE("String replacement", "[utils]")
{
    using namespace storm;
    using namespace std::string_literals;

    auto value = "test\\\\string\\"s;

    ReplaceAll(value, "\\\\", "\\");

    CHECK(value == "test\\string\\");
}
