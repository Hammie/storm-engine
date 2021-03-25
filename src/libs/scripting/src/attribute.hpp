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

    [[nodiscard]] std::string_view getName() const {
        return m_StringCodec.Convert(m_NameCode);
    }

    [[nodiscard]] StringReference getNameCode() const {
        return m_NameCode;
    }

    template<typename T>
    [[nodiscard]] auto as() const {
        return getValue<T>();
    }

    template<typename T>
    Attribute& set(T value) {
        setValue<T>(value);
        return *this;
    }

    template<typename T>
    Attribute& operator = (T value) {
        set(value);
        return *this;
    }

    Attribute& createChildAttribute(const std::string_view& name) {
        return m_Children.emplace_back(m_StringCodec, name);
    }

    [[nodiscard]] bool hasProperty(const std::string_view& property_name) const noexcept {
        auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
          return child.getName() == property_name;
        });
        return found != m_Children.end();
    }

    [[nodiscard]] Attribute& getProperty(const std::string_view& property_name) {
        auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
          return child.getName() == property_name;
        });
        if (found != m_Children.end()) {
            return *found;
        }
        else {
            return createChildAttribute(property_name);
        }
    }

    [[nodiscard]] const Attribute& getProperty(const std::string_view& property_name) const {
        auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
          return child.getName() == property_name;
        });
        Assert(found != m_Children.end());
        return *found;
    }

    [[nodiscard]] Attribute& operator [] (const std::string_view& property_name) {
        return getProperty(property_name);
    }

    [[nodiscard]] const Attribute& operator [] (const std::string_view& property_name) const {
        return getProperty(property_name);
    }

  protected:
    template<typename T = std::string_view>
    [[nodiscard]] T getValue() const;

    template<typename ValueType>
    Attribute& setValue(ValueType value) {
        m_Value = value;
        return *this;
    }

  private:
    AbstractStringCodec& m_StringCodec;
    std::vector<Attribute> m_Children;
    std::string m_Value;
    Attribute* m_Parent = nullptr;
    StringReference m_NameCode{};
};

template<>
inline std::string_view Attribute::getValue<std::string_view>() const {
    Assert(m_Children.empty());
    return m_Value;
}

template<>
inline uint32_t Attribute::getValue<uint32_t>() const {
    Assert(m_Children.empty());
    return std::stoul(m_Value);
}

template<>
inline float Attribute::getValue<float>() const {
    Assert(m_Children.empty());
    return std::stof(m_Value);
}

template<>
inline Attribute& Attribute::setValue<uint32_t>(uint32_t value) {
    m_Value = std::to_string(value);
    return *this;
}

template<>
inline Attribute& Attribute::setValue<float>(float value) {
    m_Value = std::to_string(value);
    return *this;
}

#endif // STORM_ENGINE_ATTRIBUTE_HPP
