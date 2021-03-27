#ifndef AIHELPER_HPP
#define AIHELPER_HPP

#include "Character.h"
#include "Island_Base.h"
#include "Sd2_h/SaveLoad.h"
#include "collide.h"
#include "dx9render.h"
#include <vector>

class AIAttributesHolder
{
  protected:
    Attribute *pACharacter;

  public:
    virtual void SetACharacter(Attribute *pAP)
    {
        pACharacter = pAP;
    };
    virtual Attribute *GetACharacter() const
    {
        return pACharacter;
    };
};

class VAI_INNEROBJ;

class AIHelper
{
  public:
    AIHelper();
    ~AIHelper();

    static Attribute *pASeaCameras;
    static ISLAND_BASE *pIsland;
    static VDX9RENDER *pRS;
    static COLLIDE *pCollide;

    static float fGravity;

    bool SetDevice();
    bool Init() const;
    bool Uninit();
    void AddCharacter(Attribute *pACharacter, Attribute *pAMainCharacter);
    void CalculateRelations();

    bool isFriend(Attribute *pA1, Attribute *pA2) const;
    bool isEnemy(Attribute *pA1, Attribute *pA2) const;
    bool isNeutral(Attribute *pA1, Attribute *pA2) const;

    Attribute *GetMainCharacter(Attribute *pACharacter);

    static VAI_INNEROBJ *FindAIInnerObj(Attribute *pACharacter);

    uint32_t GetRelation(Attribute *pA1, Attribute *pA2) const;
    uint32_t GetRelationSafe(Attribute *pA1, Attribute *pA2) const;

    static void Print(float x, float y, float fScale, const char *pFormat, ...);
    static void Print3D(CVECTOR vPos, float dy, float fScale, const char *pFormat, ...);

    void Save(CSaveLoad *pSL);
    void Load(CSaveLoad *pSL);

  private:
    uint32_t *pRelations, dwRelationSize;
    std::vector<Attribute *> aCharacters, aMainCharacters;

    uint32_t *GetRelation(uint32_t x, uint32_t y) const;
    uint32_t FindIndex(Attribute *pACharacter) const;
};

extern AIHelper Helper;

#endif
