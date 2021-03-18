#pragma once

#include <any>
#include <array>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace scripting {

struct ATTRIBUTES {};
struct VDATA {};
struct entid_t {};

template<char c>
struct MessageFormatTraits;

template<> struct MessageFormatTraits<'a'> { using type = ATTRIBUTES*; };
template<> struct MessageFormatTraits<'e'> { using type = VDATA*; };
template<> struct MessageFormatTraits<'f'> { using type = float; };
template<> struct MessageFormatTraits<'i'> { using type = entid_t; };
template<> struct MessageFormatTraits<'l'> { using type = long; };
template<> struct MessageFormatTraits<'p'> { using type = uintptr_t; };
template<> struct MessageFormatTraits<'s'> { using type = std::string; };

template<char... Format>
using MessageParams = std::tuple<typename MessageFormatTraits<Format>::type...>;

template<size_t N>
struct MessageFormat
{
    std::array<char, N> arr_;

    constexpr MessageFormat(const char(&in)[N]) : arr_{}
    {
        std::copy(in, in + N, arr_.begin());
    }

    [[nodiscard]]
    constexpr size_t size() const noexcept {
        return arr_.size();
    }

    [[nodiscard]]
    constexpr char at(size_t index) const noexcept {
        return arr_.at(index);
    }
};

class Message {
public:
    template<MessageFormat format>
    auto GetParams() {
        return GetParams_impl<format>(std::make_index_sequence<format.size() - 1>{});
    }

    template<MessageFormat format, typename... Params>
    auto SetParams(Params ...params) {
        return SetParams_impl<format>(std::make_index_sequence<format.size() - 1>{}, params...);
    }

private:
    template<MessageFormat format, typename T, T... Index>
    auto GetParams_impl (std::integer_sequence<size_t, Index...> seq) {
        return MessageParams<format.at(Index)...>{
            std::any_cast<typename MessageFormatTraits<format.at(Index)>::type>(data_.at(Index))...
        };
    }

    template<MessageFormat format, typename T, T... Index, typename... Params>
    auto SetParams_impl(std::integer_sequence<size_t, Index...> seq, Params ...params) {
        data_ = std::vector<std::any>{
            static_cast<typename MessageFormatTraits<format.at(Index)>::type>(params)...
        };
    }

    std::vector<std::any> data_;
};

} // namespace scripting
