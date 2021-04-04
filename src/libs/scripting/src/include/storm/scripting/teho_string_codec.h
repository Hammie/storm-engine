#pragma once

#include "string_codec.h"

constexpr uint32_t HASH_TABLE_SIZE = 512;

struct HTSUBELEMENT
{
    char *pStr = nullptr;
    uint32_t dwHashCode{};
};

struct HTELEMENT
{
    HTSUBELEMENT *pElements = nullptr;
    uint32_t nStringsNum{};
};

class TEHO_STRING_CODEC final : public AbstractStringCodec
{
    uint32_t nHTIndex{};
    uint32_t nHTEIndex{};
    uint32_t nStringsNum{};

    HTELEMENT HTable[HASH_TABLE_SIZE]{};

  public:
    using AbstractStringCodec::Convert;

    TEHO_STRING_CODEC() : nHTIndex(0), nHTEIndex(0)
    {
        nStringsNum = 0;
    }

    ~TEHO_STRING_CODEC() noexcept override
    {
        Reset();
    };

    uint32_t GetNum() override
    {
        return nStringsNum;
    };

    uint32_t GetNum(uint32_t dwNum, uint32_t dwAlign = 8)
    {
        return (1 + dwNum / dwAlign) * dwAlign;
    }

    void Clear() override
    {
        for (uint32_t m = 0; m < HASH_TABLE_SIZE; m++)
        {
            if (HTable[m].pElements)
            {
                for (uint32_t n = 0; n < HTable[m].nStringsNum; n++)
                    delete HTable[m].pElements[n].pStr;
                free(HTable[m].pElements);
            }
            HTable[m].pElements = nullptr;
            HTable[m].nStringsNum = 0;
        }
    }

    uint32_t Convert(const char *pString, bool &bNew) override
    {
        uint32_t nStringCode;
        uint32_t n;
        if (pString == nullptr)
            return 0xffffffff;
        uint32_t nHash = MakeHashValue(pString);
        uint32_t nTableIndex = nHash & (HASH_TABLE_SIZE - 1);

        HTELEMENT *pE = &HTable[nTableIndex];

        for (n = 0; n < pE->nStringsNum; n++)
        {
            if (pE->pElements[n].dwHashCode == nHash && _stricmp(pString, pE->pElements[n].pStr) == 0)
            {
                nStringCode = (nTableIndex << 16) | (n & 0xffff);
                bNew = false;
                return nStringCode;
            }
        }

        n = pE->nStringsNum;
        pE->nStringsNum++;
        pE->pElements = (HTSUBELEMENT *)realloc(pE->pElements, GetNum(pE->nStringsNum) * sizeof(HTSUBELEMENT));

        const auto len = strlen(pString) + 1;
        pE->pElements[n].pStr = new char[len];
        memcpy(pE->pElements[n].pStr, pString, len);
        pE->pElements[n].dwHashCode = nHash;

        nStringCode = (nTableIndex << 16) | (n & 0xffff);
        nStringsNum++;
        bNew = true;
        return nStringCode;
    }

    const char *Convert(uint32_t code)
    {
        uint32_t nTableIndex = code >> 16;
        if (nTableIndex >= HASH_TABLE_SIZE)
        {
            return "ERROR: invalid SCCT index";
        }
        uint32_t n = code & 0xffff;
        if (n >= HTable[nTableIndex].nStringsNum)
        {
            return "INVALID SCC";
        }
        return HTable[nTableIndex].pElements[n].pStr;
    }

    static uint32_t MakeHashValue(const char *ps)
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

    const char *Get() override
    {
        nHTIndex = 0;
        for (nHTIndex = 0; nHTIndex < HASH_TABLE_SIZE; nHTIndex++)
        {
            if (HTable[nHTIndex].nStringsNum > 0)
            {
                nHTEIndex = 0;
                return HTable[nHTIndex].pElements[nHTEIndex].pStr;
            }
        }
        return nullptr;
    }

    const char *GetNext() override
    {
        nHTEIndex++;
        for (; nHTIndex < HASH_TABLE_SIZE; nHTIndex++)
        {
            if (HTable[nHTIndex].nStringsNum == 0)
                continue;
            if (nHTEIndex >= HTable[nHTIndex].nStringsNum)
            {
                nHTEIndex = 0;
                continue;
            }
            return HTable[nHTIndex].pElements[nHTEIndex].pStr;
        }
        return nullptr;
    }
    //*/
};

