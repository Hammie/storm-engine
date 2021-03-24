#include "attribute.hpp"

#include <catch2/catch.hpp>

using namespace std::string_literals;

TEST_CASE("Create new attribute", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attr{codec, "MyAttribute"};

    CHECK(attr.getName() == "MyAttribute"s);
}

TEST_CASE("Add child attribute", "[attribute]")
{
    STRING_CODEC codec;

    Attribute parent{codec, "MyAttribute"};
    parent.createChildAttribute("Child");

    CHECK(parent.getProperty("Child") == "");
}

TEST_CASE("Get property as string", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attribute{codec, "MyAttribute"};
    attribute.setProperty("StringProperty", "5.25a");
    attribute.setProperty("IntegerProperty", 5u);
    attribute.setProperty("FloatProperty", 5.25f);

    CHECK(attribute.getProperty("StringProperty") == "5.25a");
    CHECK(attribute.getProperty("IntegerProperty") == "5");
    CHECK(attribute.getProperty("FloatProperty") == "5.250000");
}

TEST_CASE("Get property as uint32_t", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attribute{codec, "MyAttribute"};
    attribute.setProperty("StringProperty", "5.25a");
    attribute.setProperty("IntegerProperty", 5u);
    attribute.setProperty("FloatProperty", 5.25f);

    CHECK(attribute.getProperty<uint32_t>("StringProperty") == 5);
    CHECK(attribute.getProperty<uint32_t>("IntegerProperty") == 5);
    CHECK(attribute.getProperty<uint32_t>("FloatProperty") == 5);
}

TEST_CASE("Get property as float", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attribute{codec, "MyAttribute"};
    attribute.setProperty("StringProperty", "5.25a");
    attribute.setProperty("IntegerProperty", 5u);
    attribute.setProperty("FloatProperty", 5.25f);

    CHECK(attribute.getProperty<float>("StringProperty") == 5.25f);
    CHECK(attribute.getProperty<float>("IntegerProperty") == 5.f);
    CHECK(attribute.getProperty<float>("FloatProperty") == 5.25f);
}
