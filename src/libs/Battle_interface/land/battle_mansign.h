#ifndef _BATTLE_MANSIGN_H
#define _BATTLE_MANSIGN_H

#include "../bi_defines.h"
#include <string>
#include <vector>

#define MAX_MAN_QUANTITY 4

class BIManCommandList;

class BIManSign
{
  public:
    BIManSign(entid_t BIEntityID, VDX9RENDER *pRS);
    ~BIManSign();

    void Draw();
    void Init(Attribute *pRoot, Attribute *pA);

    size_t AddTexture(const char *pcTextureName, long nCols, long nRows) const;

    void Recollect();
    void SetUpdate()
    {
        m_bMakeUpdate = true;
    }
    bool IsActive() const;
    void SetActive(bool bActive);
    void MakeControl();
    void ExecuteCommand(long command);

  protected:
    void Release();
    long CalculateManQuantity();
    void UpdateBuffers(long nShipQ);
    void FillIndexBuffer() const;
    void FillVertexBuffer();
    long WriteSquareToVBuff(BI_COLOR_VERTEX *pv, const FRECT &uv, uint32_t color, const BIFPOINT &center,
                            const FPOINT &size);
    long WriteSquareToVBuffWithProgress(BI_COLOR_VERTEX *pv, const FRECT &uv, uint32_t color, const BIFPOINT &center,
                                        const FPOINT &size, float fClampUp, float fClampDown, float fClampLeft,
                                        float fClampRight);
    void UpdateCommandList() const;

    long GetCurrentCommandTopLine() const;
    long GetCurrentCommandCharacterIndex() const;
    long GetCurrentCommandMode() const;

    float GetProgressManHP(long nIdx) const;
    float GetProgressManEnergy(long nIdx) const;
    float GetProgressGunChargeMax(long nIdx);
    float GetProgressGunCharge(long nIdx);

    float GetGunProgressByIndex(long nIdx);

    void CheckDataChange();

    bool LongACompare(const Attribute &pA, const char *attrName, long &nCompareVal);
    bool FloatACompare(const Attribute &pA, const char *attrName, float &fCompareVal);
    bool StringACompare(const Attribute &pA, const char *attrName, std::string &sCompareVal);
    bool FRectACompare(const Attribute &pA, const char *attrName, FRECT &rCompareVal);
    bool BoolACompare(const Attribute &pA, const char *attrName, bool &bCompareVal);
    uint32_t GetColorByFactor(uint32_t dwLowColor, uint32_t dwHighColor, float fFactor);

    VDX9RENDER *m_pRS;
    Attribute *m_pARoot = nullptr;
    BIManCommandList *m_pCommandList;
    entid_t m_idHostEntity;
    long m_nCommandMode;

    long m_nVBufID;
    long m_nIBufID;
    long m_nSquareQ;

    long m_nMaxSquareQ;

    long m_nBackTextureID;
    long m_nBackSquareQ;
    uint32_t m_dwBackColor;
    FRECT m_rBackUV;
    BIFPOINT m_pntBackOffset;
    BIFPOINT m_pntBackIconSize;

    bool m_bIsAlarmOn;
    long m_nAlarmSquareQ;
    long m_nAlarmTextureID;
    uint32_t m_dwAlarmHighColor;
    uint32_t m_dwAlarmLowColor;
    FRECT m_rAlarmUV;
    BIFPOINT m_pntAlarmOffset;
    BIFPOINT m_pntAlarmIconSize;
    bool m_bAlarmUpDirection;
    float m_fAlarmTime;
    float m_fAlarmUpSpeed;
    float m_fAlarmDownSpeed;

    long m_nManStateTextureID;
    long m_nManStateSquareQ;
    uint32_t m_dwManStateColor;
    FRECT m_rManHPUV;
    BIFPOINT m_pntManHPOffset;
    BIFPOINT m_pntManHPIconSize;
    FRECT m_rManEnergyUV;
    BIFPOINT m_pntManEnergyOffset;
    BIFPOINT m_pntManEnergyIconSize;

    long m_nGunChargeTextureID;
    long m_nGunChargeSquareQ;
    uint32_t m_dwGunChargeColor;
    uint32_t m_dwGunChargeBackColor;
    FRECT m_rGunChargeUV;
    BIFPOINT m_pntGunChargeOffset;
    BIFPOINT m_pntGunChargeIconSize;

    std::vector<float> m_aChargeProgress;

    FRECT m_rManPicUV;
    BIFPOINT m_pntManPicOffset;
    BIFPOINT m_pntManPicIconSize;
    uint32_t m_dwManFaceColor;

    struct ManDescr
    {
        FPOINT pntPos; // center
        std::string sTexture;
        long nTexture;
        FRECT rUV;

        long nSlotIndex;
        long nCharacterIndex;
        float fHealth;
        float fEnergy;
        bool bAlarm;
        long nShootMax;
        long nShootCurrent;
    } m_Man[MAX_MAN_QUANTITY];

    long m_nManQuantity;
    long m_nCurrentManIndex;
    long m_nCommandListVerticalOffset;

    bool m_bMakeUpdate;
    bool m_bMakeVertexFill;
    bool m_bActive;
};

inline bool BIManSign::LongACompare(const Attribute &pA, const char *attrName, long &nCompareVal)
{
    const auto tmp = nCompareVal;
    nCompareVal = pA[attrName].get<uint32_t>();
    return (nCompareVal != tmp);
}

inline bool BIManSign::FloatACompare(const Attribute &pA, const char *attrName, float &fCompareVal)
{
    const auto tmp = fCompareVal;
    fCompareVal = pA[attrName].get<float>();
    return (fCompareVal != tmp);
}

inline bool BIManSign::StringACompare(const Attribute &pA, const char *attrName, std::string &sCompareVal)
{
    if (!pA.hasProperty("attrName")) {
        return !sCompareVal.empty();
    }
    const auto attr_value = pA[attrName].get<std::string_view>();
    if (sCompareVal == attr_value) {
        return false;
    }
    sCompareVal = attr_value;
    return true;
}

inline bool BIManSign::FRectACompare(const Attribute &pA, const char *attrName, FRECT &rCompareVal)
{
    if (!pA.hasProperty(attrName)) {
        return false;
    }
    const auto pVal = pA[attrName].get<std::string_view>();
    FRECT rTmp;
    rTmp.left = rCompareVal.left;
    rTmp.top = rCompareVal.top;
    rTmp.right = rCompareVal.right;
    rTmp.bottom = rCompareVal.bottom;
    sscanf(pVal.data(), "%f,%f,%f,%f", &rCompareVal.left, &rCompareVal.top, &rCompareVal.right, &rCompareVal.bottom);
    if (rCompareVal.left == rTmp.left && rCompareVal.top == rTmp.top && rCompareVal.right == rTmp.right &&
        rCompareVal.bottom == rTmp.bottom)
        return false;
    return true;
}

inline bool BIManSign::BoolACompare(const Attribute &pA, const char *attrName, bool &bCompareVal)
{
    const auto tmp = bCompareVal;
    if (pA.hasProperty(attrName)) {
        bCompareVal = pA[attrName].get<uint32_t>() != 0;
    }
    return (bCompareVal != tmp);
}

inline uint32_t BIManSign::GetColorByFactor(uint32_t dwLowColor, uint32_t dwHighColor, float fFactor)
{
    long asrc = (dwLowColor >> 24) & 0xFF;
    long rsrc = (dwLowColor >> 16) & 0xFF;
    long gsrc = (dwLowColor >> 8) & 0xFF;
    long bsrc = dwLowColor & 0xFF;
    //
    const long adst = (dwHighColor >> 24) & 0xFF;
    const long rdst = (dwHighColor >> 16) & 0xFF;
    const long gdst = (dwHighColor >> 8) & 0xFF;
    const long bdst = dwHighColor & 0xFF;
    //
    asrc += static_cast<long>((adst - asrc) * fFactor) & 0xFF;
    rsrc += static_cast<long>((rdst - rsrc) * fFactor) & 0xFF;
    gsrc += static_cast<long>((gdst - gsrc) * fFactor) & 0xFF;
    bsrc += static_cast<long>((bdst - bsrc) * fFactor) & 0xFF;
    //
    return ARGB(asrc, rsrc, gsrc, bsrc);
}

#endif
