#ifndef CHARACTER_WORK_HPP
#define CHARACTER_WORK_HPP

#include "Attributes.h"
#include "storm_assert.h"

inline const char *GetFirstName(Attribute *pACharacter)
{
    Assert(pACharacter);
    const char *pName = pACharacter->getProperty("name").get<const char*>();
    Assert(pName);
    return pName;
}

inline const char *GetLastName(Attribute *pACharacter)
{
    Assert(pACharacter);
    const char *pName = pACharacter->getProperty("name").get<const char*>();
    Assert(pName);
    return pName;
}

inline long GetIndex(Attribute *pACharacter)
{
    Assert(pACharacter);
    Assert(pACharacter->hasProperty("index"));
    return pACharacter->getProperty("index").get<long>();
}

inline long GetNation(Attribute *pACharacter)
{
    Assert(pACharacter);
    Assert(pACharacter->hasProperty("nation"));
    return pACharacter->getProperty("nation").get<long>();
}

inline long GetRank(Attribute *pACharacter)
{
    Assert(pACharacter);
    Assert(pACharacter->hasProperty("rank"));
    return pACharacter->getProperty("rank").get<long>();
}

inline Attribute *GetAShip(Attribute *pACharacter)
{
    Assert(pACharacter);
    Assert(pACharacter->hasProperty("ship"));
    return &pACharacter->getProperty("ship");
}

#endif
