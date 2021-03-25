#include "attribute.hpp"

#include <catch2/catch.hpp>

using namespace std::string_literals;

TEST_CASE("Create new attribute", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attr{codec, "MyAttribute"};

    CHECK(attr.getName() == "MyAttribute");
}

TEST_CASE("Add child attribute", "[attribute]")
{
    STRING_CODEC codec;

    Attribute parent{codec, "MyAttribute"};
    parent.createChildAttribute("Child");

    CHECK(parent.hasProperty("Child"));
}

TEST_CASE("Get property as string", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attribute{codec, "MyAttribute"};
    attribute["StringProperty"] = "5.25a";
    attribute["IntegerProperty"] = 5u;
    attribute["FloatProperty"] = 5.25f;

    CHECK(attribute.getProperty("StringProperty").as<std::string_view>() == "5.25a");
    CHECK(attribute.getProperty("IntegerProperty").as<std::string_view>() == "5");
    CHECK(attribute.getProperty("FloatProperty").as<std::string_view>() == "5.250000");
}

TEST_CASE("Get property as uint32_t", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attribute{codec, "MyAttribute"};
    attribute["StringProperty"] = "5.25a";
    attribute["IntegerProperty"] = 5u;
    attribute["FloatProperty"] = 5.25f;

    CHECK(attribute.getProperty("StringProperty").as<uint32_t>() == 5);
    CHECK(attribute.getProperty("IntegerProperty").as<uint32_t>() == 5);
    CHECK(attribute.getProperty("FloatProperty").as<uint32_t>() == 5);
}

TEST_CASE("Get property as float", "[attribute]")
{
    STRING_CODEC codec;

    Attribute attribute{codec, "MyAttribute"};
    attribute["StringProperty"] = "5.25a";
    attribute["IntegerProperty"] = 5u;
    attribute["FloatProperty"] = 5.25f;

    CHECK(attribute.getProperty("StringProperty").as<float>() == 5.25f);
    CHECK(attribute.getProperty("IntegerProperty").as<float>() == 5.f);
    CHECK(attribute.getProperty("FloatProperty").as<float>() == 5.25f);
}

TEST_CASE("Nested properties", "[attribute]")
{
    STRING_CODEC codec;

    Attribute root{codec, "MyAttribute"};
    Attribute& child = root.createChildAttribute("Child");
    Attribute& grand_child = child.createChildAttribute("GrandChild");

    CHECK(root.hasProperty("Child"));
    CHECK(root["Child"].hasProperty("GrandChild"));
}
