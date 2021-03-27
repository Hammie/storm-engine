#ifndef __SHIPS_LIST_
#define __SHIPS_LIST_

#include "../bi_defines.h"
#include "Sd2_h/VAI_ObjBase.h"

class SHIP_DESCRIBE_LIST
{
  public:
    struct SHIP_DESCR
    {
        bool isMyShip;
        bool isDead;
        long relation;
        long characterIndex;
        Attribute *pAttr; // ship attributes
        long maxHP;
        long maxSP;
        long maxCrew;
        long pictureNum;
        long selectPictureNum;
        long textureNum;
        VAI_OBJBASE *pShip;
        uint32_t dwShipColor;

        SHIP_DESCR *next;
    };

  public:
    SHIP_DESCRIBE_LIST();
    ~SHIP_DESCRIBE_LIST();

    void Add(long mainChrIndex, long chIdx, Attribute &pChAttr, Attribute &pShipAttr, bool bMyShip, long relation,
             uint32_t dwShipColor);
    SHIP_DESCR *GetMainCharacterShip() const
    {
        return mainCharacter;
    }
    Attribute *GetMainCharacterShipAttr() const
    {
        return pMainShipAttr;
    }
    void Release(long charIdx);
    void ShipSink(long charIdx);
    void ReleaseAll();
    SHIP_DESCR *FindShip(long idxCharacter) const;
    SHIP_DESCR *GetShipRoot() const
    {
        return root;
    }
    void Refresh();

  protected:
    SHIP_DESCR *root;
    SHIP_DESCR *mainCharacter;
    Attribute *pMainShipAttr;
};

extern SHIP_DESCRIBE_LIST g_ShipList;

#endif
