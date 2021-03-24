#ifndef STORM_ENGINE_ATTRIBUTE_HPP
#define STORM_ENGINE_ATTRIBUTE_HPP
#pragma once

#include "string_codec.h"

#include "../../Common_h/storm_assert.h"

#include <string>
#include <vector>

class Attribute {
  public:
    explicit Attribute (AbstractStringCodec& string_codec, const std::string_view& name)
        : m_StringCodec(string_codec)
    {
        m_NameCode = m_StringCodec.Convert(name);
    }

    [[nodiscard]] const char* getName() const {
        return m_StringCodec.Convert(m_NameCode);
    }

    [[nodiscard]] StringReference getNameCode() const {
        return m_NameCode;
    }

    [[nodiscard]] Attribute& getChildAttribute(const std::string_view& name) {
        Assert(m_Value.empty());
        auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&name, this] (const Attribute& child) {
            return child.getName() == name;
        });
        if (found == m_Children.end()) {

        }
        return *found;
    }

    Attribute& createChildAttribute(const std::string_view& name) {
        return m_Children.emplace_back(m_StringCodec, name);
    }

    template<typename ValueType = std::string_view>
    ValueType getProperty(const std::string_view& property_name);

    Attribute& setProperty(const std::string_view& property_name, const std::string_view& value) {
        return setPropertyImpl<const std::string_view&>(property_name, value);
    }

    Attribute& setProperty(const std::string_view& property_name, uint32_t value) {
        return setPropertyImpl(property_name, value);
    }

    Attribute& setProperty(const std::string_view& property_name, float value) {
        return setPropertyImpl(property_name, value);
    }

  protected:
    template<typename T = std::string_view>
    [[nodiscard]] T getValue() const;

    template<typename ValueType>
    Attribute& setValue(ValueType value) {
        m_Value = value;
        return *this;
    }

    template<typename ValueType = const std::string_view&>
    Attribute& setPropertyImpl(const std::string_view& property_name, ValueType value);

  private:
    AbstractStringCodec& m_StringCodec;
    std::vector<Attribute> m_Children;
    std::string m_Value;
    Attribute* m_Parent = nullptr;
    StringReference m_NameCode{};
};


template<>
std::string_view Attribute::getValue<std::string_view>() const {
    Assert(m_Children.empty());
    return m_Value;
}

template<>
uint32_t Attribute::getValue<uint32_t>() const {
    Assert(m_Children.empty());
    return std::stoul(m_Value);
}

template<>
float Attribute::getValue<float>() const {
    Assert(m_Children.empty());
    return std::stof(m_Value);
}

template<>
Attribute& Attribute::setValue<uint32_t>(uint32_t value) {
    m_Value = std::to_string(value);
    return *this;
}

template<>
Attribute& Attribute::setValue<float>(float value) {
    m_Value = std::to_string(value);
    return *this;
}

template<typename ValueType>
inline ValueType Attribute::getProperty(const std::string_view& property_name) {
    auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
      return child.getName() == property_name;
    });
    if (found != m_Children.end()) {
        return found->getValue<ValueType>();
    }
    else {
        return ValueType{};
    }
}

template<typename ValueType>
inline Attribute& Attribute::setPropertyImpl(const std::string_view& property_name, ValueType value) {
    auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
      return child.getName() == property_name;
    });
    if (found != m_Children.end()) {
        found->setValue(value);
    }
    else {
        Attribute& property = m_Children.emplace_back(m_StringCodec, property_name);
        property.setValue(value);
    }
    return *this;
}

#endif // STORM_ENGINE_ATTRIBUTE_HPP
