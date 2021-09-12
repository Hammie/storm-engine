#include "storm/string_replace.hpp"

namespace storm
{

size_t ReplaceAll(std::string &str, std::string_view search, std::string_view replacement)
{
    size_t count{};
    for (size_t pos{}; str.npos != (pos = str.find(search.data(), pos, search.length()));
         pos += replacement.length(), ++count)
    {
        str.replace(pos, search.length(), replacement.data(), replacement.length());
    }
    return count;
}

} // namespace storm
