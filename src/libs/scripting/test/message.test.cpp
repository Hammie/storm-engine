#include "storm/scripting/message.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Create new message", "[message]")
{
    const auto msg = scripting::Message::Create<"li">(1, 2);

    CHECK(msg.Format() == "li");
    CHECK(msg.CheckFormat("li"));
    CHECK_FALSE(msg.CheckFormat("ll"));
}

TEST_CASE("Store parameters in message", "[message]")
{
    scripting::Message message{};
    message.SetParams<"lfs">(2, 5, "Hello");

    const auto [number, floatNumber, str] = message.GetParams<"lfs">();

    CHECK(number == 2);
    CHECK(floatNumber == 5);
    CHECK(str == "Hello");
}
