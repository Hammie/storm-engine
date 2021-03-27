#ifndef _BI_UTILS_H_
#define _BI_UTILS_H_

#include "bi_defines.h"
#include <optional>
#include <string>
#include <vector>

class BITextInfo final
{
  public:
    BITextInfo(BITextInfo &&text_info) noexcept;

    BITextInfo(const BITextInfo &text_info);

    BITextInfo();
    ~BITextInfo();
    void Release();
    void Init(VDX9RENDER *rs, const Attribute &attr);
    void Print();

    VDX9RENDER *pRS;
    std::string sText;
    POINT pos;
    float fScale;
    long nFont;
    uint32_t dwColor;
    bool bShadow;

    const Attribute *pARefresh = nullptr;
};

class BILinesInfo
{
  public:
    BILinesInfo();
    ~BILinesInfo();
    void Release();
    void Init(VDX9RENDER *rs, Attribute *pA);
    void Draw();

    VDX9RENDER *pRS;
    std::vector<RS_LINE2D> lines;
};

class IBIImage;
class BIImageRender;

class BIImagesInfo
{
  public:
    BIImagesInfo();
    ~BIImagesInfo();
    void Release();
    void Init(VDX9RENDER *rs, Attribute *pA);
    void Draw() const;

    VDX9RENDER *pRS;
    BIImageRender *pImgRender;
    std::vector<IBIImage *> images;
};

class BIBorderInfo
{
  public:
    BIBorderInfo();
    ~BIBorderInfo();
    void Release();
    void Init(VDX9RENDER *rs, Attribute *pA);
    void Draw();

    VDX9RENDER *pRS;
    long nVBuf;
    long nTexID;
    FRECT ext_pos;
    FRECT int_pos1;
    FRECT int_pos2;
    uint32_t dwColor1;
    uint32_t dwColor2;
    float fCur;
    float fSpeed;
    bool bUp;
    bool bUsed;
};

class BIUtils
{
    //---------------------------------------
  public: // functions
    static long GetLongFromAttr(const Attribute &attribute, const char *name, long defVal);
    static float GetFloatFromAttr(const Attribute &attribute, const char *name, float defVal);
    static bool ReadStringFromAttr(const Attribute &attribute, const char *name, char *buf, long bufSize, const char *defVal);
    static const char *GetStringFromAttr(const Attribute &attribute, const char *name, const char *defVal);
    static long GetTextureFromAttr(VDX9RENDER *rs, const Attribute &attribute, const char *sAttrName);
    static bool ReadRectFromAttr(const Attribute &attribute, const std::string_view& name, FRECT &rOut);
    static bool ReadRectFromAttr(const Attribute &attribute, const std::string_view& name, FRECT &rOut, FRECT &rDefault);
    static bool ReadRectFromAttr(const Attribute &attribute, const std::string_view& name, RECT &rOut, RECT &rDefault);
    static bool ReadPosFromAttr(const Attribute &attribute, const std::string_view& name, BIFPOINT &fP);
    static bool ReadPosFromAttr(const Attribute &attribute, const std::string_view& name, float &fX, float &fY, float fXDef, float fYDef);
    static bool ReadPosFromAttr(const Attribute &attribute, const std::string_view& name, long &nX, long &nY, long nXDef, long nYDef);
    static long GetAlignmentFromAttr(const Attribute &attribute, const char *name, long nDefAlign);
    static long GetFontIDFromAttr(const Attribute &attribute, const char *name, VDX9RENDER *rs, const char *pcDefFontName);
    static std::optional<long> GetFontIDFromAttr(const Attribute &attribute, const char *name, VDX9RENDER *rs);
    static bool ReadVectorFormAttr(const Attribute &attribute, const char *name, CVECTOR &vOut, const CVECTOR &vDef);

    static bool ComparePoint(POINT &p1, POINT &p2);

    static Attribute *GetAttributesFromPath(Attribute *pA, ...);

    static uint32_t GetIntervalColor(uint32_t minV, uint32_t maxV, float fpar);
    static bool GetIntervalRect(float fk, const FRECT &r1, const FRECT &r2, FRECT &rOut);

    static long GetMaxFromFourLong(long n1, long n2, long n3, long n4);

    static float GetFromStr_Float(const char *&pcStr, float fDefault);

    static void FillTextInfoArray(VDX9RENDER *pRS, Attribute *pA, std::vector<BITextInfo> &tia);
    static void PrintTextInfoArray(std::vector<BITextInfo> &tia);
    //---------------------------------------
    //---------------------------------------
  public: // data
    static entid_t idBattleInterface;
    static uint32_t g_dwBlinkColor;
    //---------------------------------------
};

#endif
