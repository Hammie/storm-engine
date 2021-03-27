#include "Utils.h"

#include "core.h"

#include "image/imgrender.h"
#include "vmodule_api.h"

// extern data
entid_t BIUtils::idBattleInterface;
uint32_t BIUtils::g_dwBlinkColor;

long BIUtils::GetLongFromAttr(const Attribute &pA, const char *name, long defVal)
{
    if (name == nullptr)
        return defVal;
    return pA[name].get<long>(defVal);
}

float BIUtils::GetFloatFromAttr(const Attribute &pA, const char *name, float defVal)
{
    if (name == nullptr)
        return defVal;
    return pA[name].get<float>(defVal);
}

bool BIUtils::ReadStringFromAttr(const Attribute &pA, const char *name, char *buf, long bufSize, const char *defVal)
{
    if (buf == nullptr || bufSize < 1)
        return false;
    buf[0] = 0;
    std::string_view strGet;
    auto bRet = true;
    if (!pA.hasProperty(name))
    {
        strGet = defVal;
        bRet = false;
    }
    pA.get_to(strGet);

    if (strGet != nullptr)
    {
        if (static_cast<int>(strGet.size()) > bufSize - 1)
        {
            strncpy_s(buf, bufSize, strGet.data(), bufSize - 1);
            buf[bufSize - 1] = 0;
        }
        else
            strcpy_s(buf, bufSize, strGet.data());
    }
    else
        bRet = false;

    return bRet;
}

const char *BIUtils::GetStringFromAttr(const Attribute &pA, const char *name, const char *defVal)
{
    if (name == nullptr)
        return (char *)defVal;

    if (const Attribute& property = pA.getProperty(name); !property.empty()) {
        return property.get<std::string_view>().data();
    }

    return defVal;
}

long BIUtils::GetTextureFromAttr(VDX9RENDER *rs, const Attribute &pA, const char *sAttrName)
{
    if (!rs)
        return -1;

    if (const Attribute& property = pA.getProperty(sAttrName); !property.empty()) {
        return rs->TextureCreate(property.get<std::string_view>().data());
    }

    return -1;
}

bool BIUtils::ReadRectFromAttr(const Attribute &pA, const std::string_view& name, FRECT &rOut)
{
    if (!name.empty() && pA.hasProperty(name))
    {
        auto pcStr = pA[name].get<std::string_view>();
        sscanf(pcStr.data(), "%f,%f,%f,%f", &rOut.left, &rOut.top, &rOut.right, &rOut.bottom);
        return true;
    }
    return false;
}

bool BIUtils::ReadRectFromAttr(const Attribute &pA, const std::string_view& name, FRECT &rOut, FRECT &rDefault)
{
    rOut = rDefault;
    if (!name.empty() && pA.hasProperty(name))
    {
        auto pcStr = pA[name].get<std::string_view>();
        sscanf(pcStr.data(), "%f,%f,%f,%f", &rOut.left, &rOut.top, &rOut.right, &rOut.bottom);
        return true;
    }
    return false;
}

bool BIUtils::ReadRectFromAttr(const Attribute &pA, const std::string_view& name, RECT &rOut, RECT &rDefault)
{
    rOut = rDefault;
    if (!name.empty() && pA.hasProperty(name))
    {
        auto pcStr = pA[name].get<std::string_view>();
        sscanf(pcStr.data(), "%d,%d,%d,%d", &rOut.left, &rOut.top, &rOut.right, &rOut.bottom);
        return true;
    }
    return false;
}

bool BIUtils::ReadPosFromAttr(const Attribute &pA, const std::string_view& name, BIFPOINT &fP)
{
    if (!name.empty() && pA.hasProperty(name))
    {
        auto pcStr = pA[name].get<std::string_view>();
        sscanf(pcStr.data(), "%f,%f", &fP.x, &fP.y);
        return true;
    }
    return false;
}

bool BIUtils::ReadPosFromAttr(const Attribute &pA, const std::string_view& name, float &fX, float &fY, float fXDef, float fYDef)
{
    fX = fXDef;
    fY = fYDef;
    if (!name.empty() && pA.hasProperty(name))
    {
        auto pcStr = pA[name].get<std::string_view>();
        sscanf(pcStr.data(), "%f,%f", &fX, &fY);
        return true;
    }
    return false;
}

bool BIUtils::ReadPosFromAttr(const Attribute &pA, const std::string_view& name, long &nX, long &nY, long nXDef, long nYDef)
{
    nX = nXDef;
    nY = nYDef;
    if (!name.empty() && pA.hasProperty(name))
    {
        auto pcStr = pA[name].get<std::string_view>();
        sscanf(pcStr.data(), "%d,%d", &nX, &nY);
        return true;
    }
    return false;
}

long BIUtils::GetAlignmentFromAttr(const Attribute &pA, const char *name, long nDefAlign)
{
    if (name) {
        if (const Attribute& property = pA.getProperty(name); !property.empty()) {
            const std::string_view value = property.get<std::string_view>();
            if (_stricmp(value.data(), "left") == 0)
                return PR_ALIGN_LEFT;
            if (_stricmp(value.data(), "center") == 0)
                return PR_ALIGN_CENTER;
            if (_stricmp(value.data(), "right") == 0)
                return PR_ALIGN_RIGHT;
        }
    }

    return nDefAlign;
}

long BIUtils::GetFontIDFromAttr(const Attribute &pA, const char *name, VDX9RENDER *rs, const char *pcDefFontName)
{
    if (rs) {
        if (name) {
	        if (const Attribute& font = pA.getProperty(name); !font.empty()) {
		        const std::string_view value = font.get<std::string_view>();
		        return rs->LoadFont(value.data());
	        }
        }
        if (rs && pcDefFontName)
            return rs->LoadFont((char *)pcDefFontName);
    }

    return -1;
}

std::optional<long> BIUtils::GetFontIDFromAttr(const Attribute &pA, const char *name, VDX9RENDER *rs)
{
    if (rs) {
        if (name) {
            if (const Attribute& font = pA.getProperty(name); !font.empty()) {
                const std::string_view value = font.get<std::string_view>();
                return rs->LoadFont(value.data());
            }
        }
    }

    return {};
}

bool BIUtils::ReadVectorFormAttr(const Attribute &pA, const char *name, CVECTOR &vOut, const CVECTOR &vDef)
{
    if (name)
    {
        if (const Attribute& property = pA.getProperty(name); !property.empty()) {
            vOut = property.get<CVECTOR>();
            return true;
        }
    }
    vOut = vDef;
    return false;
}

bool BIUtils::ComparePoint(POINT &p1, POINT &p2)
{
    return ((p1.x == p2.x) && (p1.y == p2.y));
}

Attribute *BIUtils::GetAttributesFromPath(Attribute *pA, ...)
{
    va_list arglist;

    char *sName;
    Attribute *pTmpAttr = pA;
    va_start(arglist, pA);
    while ((sName = va_arg(arglist, char *)) != nullptr)
    {
        if (pTmpAttr == nullptr)
            return nullptr;
        pTmpAttr = &(pTmpAttr->getProperty(sName));
    }
    va_end(arglist);

    return pTmpAttr;
}

uint32_t BIUtils::GetIntervalColor(uint32_t minV, uint32_t maxV, float fpar)
{
    long a = minV >> 24L;
    const long ad = static_cast<long>(maxV >> 24L) - a;
    long r = (minV & 0xFF0000) >> 16;
    const long rd = static_cast<long>((maxV & 0xFF0000) >> 16) - r;
    long g = (minV & 0xFF00) >> 8;
    const long gd = static_cast<long>((maxV & 0xFF00) >> 8) - g;
    long b = minV & 0xFF;
    const long bd = static_cast<long>(maxV & 0xFF) - b;

    a += static_cast<long>(ad * fpar);
    r += static_cast<long>(rd * fpar);
    g += static_cast<long>(gd * fpar);
    b += static_cast<long>(bd * fpar);

    return ARGB(a, r, g, b);
}

bool BIUtils::GetIntervalRect(float fk, const FRECT &r1, const FRECT &r2, FRECT &rOut)
{
    rOut.left = r1.left + fk * (r2.left - r1.left);
    rOut.top = r1.top + fk * (r2.top - r1.top);
    rOut.right = r1.right + fk * (r2.right - r1.right);
    rOut.bottom = r1.bottom + fk * (r2.bottom - r1.bottom);
    return true;
}

long BIUtils::GetMaxFromFourLong(long n1, long n2, long n3, long n4)
{
    if (n1 >= n2 && n1 >= n3 && n1 >= n4)
        return n1;
    if (n2 >= n3 && n2 >= n4)
        return n2;
    if (n3 >= n4)
        return n3;
    return n4;
}

float BIUtils::GetFromStr_Float(const char *&pcStr, float fDefault)
{
    if (!pcStr)
        return fDefault;
    long n;
    char ctmp[64];
    for (n = 0; pcStr[0] != ',' && pcStr[0]; pcStr++)
    {
        if (pcStr[0] <= 32)
            continue;
        if (n < sizeof(ctmp) - 1)
            ctmp[n++] = pcStr[0];
    }
    ctmp[n] = 0;
    while (pcStr[0] == ',')
        pcStr++;
    return static_cast<float>(atof(ctmp));
}

void BIUtils::FillTextInfoArray(VDX9RENDER *pRS, Attribute *pA, std::vector<BITextInfo> &tia)
{
    if (!pA)
        return;
    tia.clear();

    for (const Attribute& attr : *pA) {
        tia.emplace_back().Init(pRS, attr);
    }
}

void BIUtils::PrintTextInfoArray(std::vector<BITextInfo> &tia)
{
    for (long n = 0; n < tia.size(); n++)
        tia[n].Print();
}

BITextInfo::BITextInfo(BITextInfo &&text_info) noexcept
{
    pRS = text_info.pRS;
    sText = std::move(text_info.sText);
    pos = std::move(text_info.pos);
    fScale = text_info.fScale;
    nFont = text_info.nFont;
    dwColor = text_info.dwColor;
    bShadow = text_info.bShadow;
    pARefresh = text_info.pARefresh;

    text_info.nFont = -1;
}

BITextInfo::BITextInfo(const BITextInfo &text_info)
{
    pRS = text_info.pRS;
    sText = text_info.sText;
    pos = text_info.pos;
    fScale = text_info.fScale;
    nFont = text_info.nFont;
    dwColor = text_info.dwColor;
    bShadow = text_info.bShadow;
    pARefresh = text_info.pARefresh;

    pRS->IncRefCounter(nFont);
}

BITextInfo::BITextInfo()
{
    pRS = nullptr;
    nFont = -1;
}

BITextInfo::~BITextInfo()
{
    Release();
}

void BITextInfo::Release()
{
    sText = "";
    FONT_RELEASE(pRS, nFont);
}

void BITextInfo::Init(VDX9RENDER *rs, const Attribute &attr)
{
    FONT_RELEASE(pRS, nFont);
    pRS = rs;
    if (!pRS || attr.empty())
        return;

    nFont = pRS->LoadFont(attr["font"].get<std::string_view>().data());
    attr["scale"].get_to(fScale, 1.0f);
    attr["color"].get_to(dwColor, 0xFFFFFFFF);
    attr["shadow"].get_to(bShadow, true);
    attr["pos"].get_to(pos, {});
    attr["text"].get_to(sText);

    pARefresh = nullptr;
    if (attr["refreshable"].get<bool>(false)) {
        pARefresh = &attr;
    }
}

void BITextInfo::Print()
{
    if (nFont != -1)
    {
        if (pARefresh) {
            pARefresh->getProperty("text").get_to(sText);
        }
        if (!sText.empty())
            pRS->ExtPrint(nFont, dwColor, 0, PR_ALIGN_CENTER, bShadow, fScale, 0, 0, pos.x, pos.y, "%s", sText.c_str());
    }
}

BILinesInfo::BILinesInfo()
{
    pRS = nullptr;
}

BILinesInfo::~BILinesInfo()
{
    Release();
}

void BILinesInfo::Release()
{
    lines.clear();
}

void BILinesInfo::Init(VDX9RENDER *rs, Attribute *pA)
{
    pRS = rs;
    if (!pA)
        return;

    for (const Attribute& attr : *pA) {
        RS_LINE2D& begin_line = lines.emplace_back();
        RS_LINE2D& end_line = lines.emplace_back();

        attr["color"].get_to(begin_line.dwColor, 0xFFFFFFFF);
        end_line.dwColor = begin_line.dwColor;

        attr["begin"]["x"].get_to(begin_line.vPos.x);
        attr["begin"]["y"].get_to(begin_line.vPos.y);

        attr["end"]["x"].get_to(end_line.vPos.x);
        attr["end"]["y"].get_to(end_line.vPos.y);
    }
}

void BILinesInfo::Draw()
{
    if (!lines.empty()) {
        pRS->DrawLines2D(&lines[0], lines.size() / 2, "Line");
    }
}

BIImagesInfo::BIImagesInfo()
{
    pRS = nullptr;
    pImgRender = nullptr;
}

BIImagesInfo::~BIImagesInfo()
{
    Release();
}

void BIImagesInfo::Release()
{
    // images.DelAllWithPointers();
    for (const auto &image : images)
        delete image;
    STORM_DELETE(pImgRender);
}

void BIImagesInfo::Init(VDX9RENDER *rs, Attribute *pA)
{
    if (!pA || !rs)
        return;

    pRS = rs;
    pImgRender = new BIImageRender(rs);
    if (!pImgRender)
        return;

    for (const Attribute& attr : *pA) {
        if (!attr.empty()) {
            const auto rUV = attr["uv"].get<FRECT>({0, 0, 1, 1});
            const auto rPos = attr["pos"].get<RECT>({0, 0});

            IBIImage *pCurImg =
                pImgRender->CreateImage(BIType_square, attr["texture"].get<const char*>(),
                                        attr["color"].get<uint32_t>(ARGB(255, 128, 128, 128)), rUV, rPos);
            if (pCurImg)
                images.push_back(pCurImg);
        }
    }
}

void BIImagesInfo::Draw() const
{
    if (pImgRender)
        pImgRender->Render();
}

BIBorderInfo::BIBorderInfo() : dwColor1(0), dwColor2(0), fCur(0), fSpeed(0)
{
    pRS = nullptr;
    nVBuf = -1;
    nTexID = -1;
    bUp = true;
    bUsed = false;
}

BIBorderInfo::~BIBorderInfo()
{
    Release();
}

void BIBorderInfo::Release()
{
    VERTEX_BUFFER_RELEASE(pRS, nVBuf);
    TEXTURE_RELEASE(pRS, nTexID);
}

void BIBorderInfo::Init(VDX9RENDER *rs, Attribute *pA)
{
    pRS = rs;
    nVBuf = rs->CreateVertexBuffer(BI_COLOR_VERTEX_FORMAT, 2 * 5 * sizeof(BI_COLOR_VERTEX), D3DUSAGE_WRITEONLY);
    dwColor1 = dwColor2 = ARGB(255, 255, 255, 255);
    ext_pos.left = 0.f;
    ext_pos.top = 0.f;
    ext_pos.right = 1024.f;
    ext_pos.bottom = 768.f;
    int_pos1.left = 20.f;
    int_pos1.top = 20.f;
    int_pos1.right = 1004.f;
    int_pos1.bottom = 748.f;
    int_pos2 = int_pos1;
    fCur = 0.f;
    fSpeed = 1.f;
    nTexID = -1;
    bUp = true;
    bUsed = true;
    if (!pA)
        return;

    const Attribute& attr = *pA;

    attr["color1"].get_to(dwColor1);
    attr["color2"].get_to(dwColor2);
    attr["extpos"].get_to(ext_pos);
    attr["intpos1"].get_to(int_pos1);
    attr["intpos2"].get_to(int_pos2);
    const Attribute& speed = attr["speed"];
    if (!speed.empty()) {
        fSpeed = speed.get<float>() * 0.001f;
    }
    nTexID = pRS->TextureCreate(attr["texture"].get<const char*>());
    attr["used"].get_to(bUsed, false);
}

void BIBorderInfo::Draw()
{
    if (!bUsed || nVBuf < 0)
        return;
    auto *pV = static_cast<BI_COLOR_VERTEX *>(pRS->LockVertexBuffer(nVBuf));
    if (!pV)
        return;

    if (bUp)
    {
        fCur += fSpeed * core.GetDeltaTime();
        if (fCur > 1.f)
        {
            fCur = 1.f;
            bUp = false;
        }
    }
    else
    {
        fCur -= fSpeed * core.GetDeltaTime();
        if (fCur < 0.f)
        {
            fCur = 0.f;
            bUp = true;
        }
    }
    const uint32_t dwColor = BIUtils::GetIntervalColor(dwColor1, dwColor2, fCur);
    FRECT int_pos;
    BIUtils::GetIntervalRect(fCur, int_pos1, int_pos2, int_pos);

    for (long n = 0; n < 10; n++)
    {
        pV[n].col = dwColor;
        pV[n].w = 0.5f;
        pV[n].pos.z = 1.f;
    }
    //
    pV[0].tu = pV[1].tu = pV[4].tu = pV[5].tu = pV[8].tu = pV[9].tu = 0.f;
    pV[2].tu = pV[3].tu = pV[6].tu = pV[7].tu = 0.96f;
    pV[0].tv = pV[2].tv = pV[4].tv = pV[6].tv = pV[8].tv = 0.f;
    pV[1].tv = pV[3].tv = pV[5].tv = pV[7].tv = pV[9].tv = 0.96f;
    //
    pV[0].pos.x = ext_pos.left;
    pV[0].pos.y = ext_pos.top;
    pV[1].pos.x = int_pos.left;
    pV[1].pos.y = int_pos.top;
    pV[2].pos.x = ext_pos.right;
    pV[2].pos.y = ext_pos.top;
    pV[3].pos.x = int_pos.right;
    pV[3].pos.y = int_pos.top;
    pV[4].pos.x = ext_pos.right;
    pV[4].pos.y = ext_pos.bottom;
    pV[5].pos.x = int_pos.right;
    pV[5].pos.y = int_pos.bottom;
    pV[6].pos.x = ext_pos.left;
    pV[6].pos.y = ext_pos.bottom;
    pV[7].pos.x = int_pos.left;
    pV[7].pos.y = int_pos.bottom;
    pV[8].pos.x = ext_pos.left;
    pV[8].pos.y = ext_pos.top;
    pV[9].pos.x = int_pos.left;
    pV[9].pos.y = int_pos.top;
    pRS->UnLockVertexBuffer(nVBuf);

    pRS->TextureSet(0, nTexID);
    pRS->DrawPrimitive(D3DPT_TRIANGLESTRIP, nVBuf, sizeof(BI_COLOR_VERTEX), 0, 8, "battle_msg");
}
