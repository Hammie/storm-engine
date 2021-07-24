#include "utf8.h"

#include <array>
#include <stdexcept>

namespace utf8
{

namespace
{

std::string ConvertToUtf8(const unsigned code_page, const std::string &str)
{
    int count = MultiByteToWideChar(code_page, 0, str.c_str(), static_cast<int>(str.length()), NULL, 0);
    if (count == 0)
    {
        throw std::runtime_error("Failed to convert to utf8");
    }
    std::wstring wstr(count, 0);
    int written = MultiByteToWideChar(code_page, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], count);
    if (written == 0)
    {
        throw std::runtime_error("Failed to convert to utf8");
    }
    return utf8::ConvertWideToUtf8(wstr);
}

static constexpr std::array<unsigned int, 2> CodePagesToTry{
    1251,
    1252,
};

} // namespace

bool EnsureUtf8(std::string &str)
{
    if (IsValidUtf8(str))
    {
        return true;
    }
    else
    {
        for (const auto code_page : CodePagesToTry)
        {
            std::string result = ConvertToUtf8(code_page, str);
            if (IsValidUtf8(result))
            {
                const auto *test = reinterpret_cast<const char8_t *>(result.c_str());
                str = result;
                return true;
            }
        }
    }
    return false;
}

} // namespace utf8
