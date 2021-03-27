#ifndef STORM_ENGINE_ATTRIBUTE_HPP
#define STORM_ENGINE_ATTRIBUTE_HPP
#pragma once

#include "string_codec.h"

#include "../../Common_h/storm_assert.h"

#include <string>
#include <vector>

class Attribute {
  public:
    Attribute (AbstractStringCodec& string_codec, const std::string_view& name)
        : m_StringCodec(&string_codec)
    {
        m_NameCode = m_StringCodec->Convert(name);
    }

    Attribute (const Attribute& parent, const std::string_view& name)
        : m_StringCodec(parent.m_StringCodec)
        , m_Parent(&parent)
    {
        m_NameCode = m_StringCodec->Convert(name);
    }

    [[nodiscard]] std::string_view getName() const {
        return m_StringCodec->Convert(m_NameCode);
    }

    [[nodiscard]] StringReference getNameCode() const {
        return m_NameCode;
    }

    [[nodiscard]] const Attribute* getParent() const {
        return m_Parent;
    }

    template<typename T>
    [[nodiscard]] auto get() const {
        Assert(m_HasValue);
        return getValue<T>();
    }

    template<typename T>
    [[nodiscard]] auto get(const T& def) const {
        if (!m_HasValue) {
            return def;
        }
        return getValue<T>();
    }

    template<typename T>
    const Attribute& get_to(T& var) const {
        if (m_HasValue) {
            var = getValue<T>();
        }
        return *this;
    }

    template<typename T>
    const Attribute& get_to(T& var, const T& def) const {
        if (!m_HasValue) {
            var = def;
        }
        else {
            var = getValue<T>();
        }
        return *this;
    }

//    template<typename T> requires (std::is_arithmetic_v<T>)
//    operator T () const {
//        return as<T>();
//    }

    template<typename T>
    Attribute& set(T value) {
        setValue<T>(value);
        m_HasValue = true;
        return *this;
    }

    bool empty() const noexcept {
        return !m_HasValue && std::all_of(m_Children.begin(), m_Children.end(), [] (const Attribute& child) {
            return child.empty();
        });
    }

    template<typename T>
    Attribute& operator = (T value) {
        set(value);
        return *this;
    }

    Attribute& createChildAttribute(const std::string_view& name) {
        Assert(!m_HasValue);
        return m_Children.emplace_back(*this, name);
    }

    [[nodiscard]] bool hasProperty(const std::string_view& property_name) const noexcept {
        auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
          return _stricmp(child.getName().data(),  property_name.data()) == 0;
        });
        return found != m_Children.end() && !found->empty();
    }

    [[nodiscard]] Attribute& getProperty(const std::string_view& property_name) {
        auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
          return _stricmp(child.getName().data(),  property_name.data()) == 0;
        });
        if (found != m_Children.end()) {
            return *found;
        }
        else {
            return m_Children.emplace_back(*this, property_name);
        }
    }

    [[nodiscard]] const Attribute& getProperty(const std::string_view& property_name) const {
        auto found = std::find_if(std::begin(m_Children), std::end(m_Children), [&property_name, this] (const Attribute& child) {
          return _stricmp(child.getName().data(),  property_name.data()) == 0;
        });
        if (found != m_Children.end()) {
            return *found;
        }
        else {
            return m_Children.emplace_back(*this, property_name);
        }
    }

    [[nodiscard]] Attribute& getProperty(const StringReference& name_code) {
        return getProperty(m_StringCodec->Convert(name_code));
    }

    [[nodiscard]] const Attribute& getProperty(const StringReference& name_code) const {
        return getProperty(m_StringCodec->Convert(name_code));
    }

    Attribute& clear() {
        m_HasValue = false;
        m_Value.clear();
        m_Children.clear();
        return *this;
    }

    [[nodiscard]] Attribute& operator [] (const std::string_view& property_name) {
        return getProperty(property_name);
    }

    [[nodiscard]] const Attribute& operator [] (const std::string_view& property_name) const {
        return getProperty(property_name);
    }

    [[nodiscard]] auto begin() {
        return std::begin(m_Children);
    }

    [[nodiscard]] auto begin() const {
        return std::begin(m_Children);
    }

    [[nodiscard]] auto end() {
        return std::begin(m_Children);
    }

    [[nodiscard]] auto end() const {
        return std::begin(m_Children);
    }

    [[nodiscard]] bool operator == (const std::string_view& name) const {
        return _stricmp(getName().data(), name.data()) == 0;
    }

    [[nodiscard]] AbstractStringCodec& getStringCodec() {
        return *m_StringCodec;
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
    AbstractStringCodec* m_StringCodec = nullptr;
    mutable std::vector<Attribute> m_Children;
    std::string m_Value;
    const Attribute* m_Parent = nullptr;
    StringReference m_NameCode{};
    bool m_HasValue = false;
};

template<>
inline std::string_view Attribute::getValue<std::string_view>() const {
    return m_Value;
}

template<>
inline std::string Attribute::getValue<std::string>() const {
    return m_Value;
}

template<>
inline const char* Attribute::getValue<const char*>() const {
    return m_Value.c_str();
}

template<>
inline long Attribute::getValue<long>() const {
    return std::stol(m_Value);
}

template<>
inline int32_t Attribute::getValue<int32_t>() const {
    return std::stol(m_Value);
}

template<>
inline uint32_t Attribute::getValue<uint32_t>() const {
    return std::stoul(m_Value);
}

template<>
inline int64_t Attribute::getValue<int64_t>() const {
    return std::stoll(m_Value);
}

template<>
inline uint64_t Attribute::getValue<uint64_t>() const {
    return std::stoull(m_Value);
}

template<>
inline bool Attribute::getValue<bool>() const {
    return std::stoul(m_Value) != 0;
}

template<>
inline float Attribute::getValue<float>() const {
    return std::stof(m_Value);
}

template<>
inline double Attribute::getValue<double>() const {
    return std::stod(m_Value);
}

template<>
inline Attribute& Attribute::setValue<uint32_t>(uint32_t value) {
    m_Value = std::to_string(value);
    return *this;
}

template<>
inline Attribute& Attribute::setValue<bool>(bool value) {
    m_Value = std::to_string(value ? 1 : 0);
    return *this;
}

template<>
inline Attribute& Attribute::setValue<float>(float value) {
    m_Value = std::to_string(value);
    return *this;
}

#endif // STORM_ENGINE_ATTRIBUTE_HPP
