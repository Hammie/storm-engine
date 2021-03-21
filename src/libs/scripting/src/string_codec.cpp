#include "string_codec.h"

namespace {

constexpr uint32_t MakeHashValue(const char* ps)
{
    uint32_t hval = 0;
    while (*ps != 0)
    {
        char v = *ps++;
        if ('A' <= v && v <= 'Z')
            v += 'a' - 'A'; // case independent
        hval = (hval << 4) + (unsigned long int)v;
        uint32_t g = hval & ((unsigned long int)0xf << (32 - 4));
        if (g != 0)
        {
            hval ^= g >> (32 - 8);
            hval ^= g;
        }
    }
    return hval;
}

} // namespace

uint32_t AbstractStringCodec::Convert(const char *pString, long iLen)
{
    if (pString == nullptr)
        return 0xffffffff;

    char cTemp[1024];
    strncpy_s(cTemp, pString, iLen);
    cTemp[iLen] = 0;

    bool bNew;
    return Convert(cTemp, bNew);
}

uint32_t AbstractStringCodec::Convert(const char *pString)
{
    if (pString == nullptr)
        return 0xffffffff;
    bool bNew;
    return Convert(pString, bNew);
}

const char* STRING_CODEC::Convert(const uint32_t code) const
{
    auto it = table.find(code);
    if (it == table.end()) {
        return nullptr;
    }
    else {
        return it->second.c_str();
    }
}

uint32_t STRING_CODEC::Convert(const char *pString, bool &bNew)
{
    if (pString == nullptr)
        return 0xffffffff;

    uint32_t hash = MakeHashValue(pString);

    // Avoid hash collision
    for(;;) {
        auto existingValue = table.find(hash);
        if (existingValue == table.end()) {
            table.emplace(hash, pString);
            bNew = true;
            return hash;
        }
        if (_stricmp(existingValue->second.c_str(), pString) == 0) {
            bNew = false;
            return hash;
        }
        ++hash;
    }

}

const char* STRING_CODEC::Get()
{
    iterator = table.begin();

    if (iterator == table.end()) {
        return nullptr;
    }

    return (*iterator).second.c_str();
}

const char* STRING_CODEC::GetNext()
{
    if (iterator == table.end()) {
        return nullptr;
    }

    ++iterator;
    return (*iterator).second.c_str();
}
