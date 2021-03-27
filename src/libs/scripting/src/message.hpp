#pragma once

#include <any>
#include <array>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

class Attribute;
class VDATA;

namespace scripting {

using entid_t = uint64_t;

namespace detail {

template<char c>
struct MessageFormatTraits;

template<> struct MessageFormatTraits<'a'> { using type = Attribute*; };
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
    static const size_t Length = N - 1;

    constexpr MessageFormat(const char(&in)[N]) : arr_{}
    {
        std::copy(in, in + Length, arr_.begin());
    }

    [[nodiscard]]
    constexpr std::string_view str() const noexcept
    {
        return std::string_view(arr_.begin(), arr_.end());
    }

    [[nodiscard]]
    constexpr size_t size() const noexcept
    {
        return arr_.size();
    }

    [[nodiscard]]
    constexpr char at(size_t index) const noexcept
    {
        return arr_.at(index);
    }

    std::array<char, Length> arr_;
};

} // namespace detail

class Message {
public:
    Message() noexcept = default;

    explicit Message(std::string format, std::vector<std::any> params)
        : format_(std::move(format))
        , data_(std::move(params))
    { }

    template<detail::MessageFormat format, typename... Params>
    [[nodiscard]]
    static Message Create(Params ...params)
    {
        Message result;
        result.format_ = format.str();
        return result;
    }

    template<detail::MessageFormat format>
    [[nodiscard]]
    auto GetParams() const
    {
        return GetParams_impl<format>(std::make_index_sequence<format.size()>{});
    }

    template<detail::MessageFormat format, typename... Params>
    auto SetParams(Params ...params)
    {
        return SetParams_impl<format>(std::make_index_sequence<format.size()>{}, params...);
    }

    [[nodiscard]]
    std::string_view Format() const
    {
        return format_;
    }

    [[nodiscard]]
    bool CheckFormat(const std::string_view& format) const
    {
        return format_ == format;
    }

  private:
    template<detail::MessageFormat format, typename T, T... Index>
    auto GetParams_impl (std::integer_sequence<size_t, Index...> seq) const
    {
        return detail::MessageParams<format.at(Index)...>{
            std::any_cast<typename detail::MessageFormatTraits<format.at(Index)>::type>(data_.at(Index))...
        };
    }

    template<detail::MessageFormat format, typename T, T... Index, typename... Params>
    auto SetParams_impl(std::integer_sequence<size_t, Index...> seq, Params ...params)
    {
        data_ = std::vector<std::any>{
            static_cast<typename detail::MessageFormatTraits<format.at(Index)>::type>(params)...
        };
    }

    std::string format_;
    std::vector<std::any> data_;
};

} // namespace scripting
