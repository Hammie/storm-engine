#include "message.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Test", "[message]")
{
    scripting::Message message{};
    message.SetParams<"lfs">(2, 5, "Hello");

    const auto [number, floatNumber, str] = message.GetParams<"lfs">();

    CHECK(number == 2);
    CHECK(floatNumber == 5);
    CHECK(str == "Hello");
}
