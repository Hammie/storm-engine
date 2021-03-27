#include "ActivePerkShower.h"
#include "../../Shared/battle_interface/msg_control.h"
#include "bi_defines.h"
#include "core.h"
#include "vmodule_api.h"
#include <exception>

ActivePerkShower::ActivePerkShower()
{
    rs = nullptr;

    m_idVBuf = -1;
    m_idIBuf = -1;

    m_nTextureQ = 0;

    m_nShowPlaceQ = 0;
    m_pShowPlaces = nullptr;

    m_nIShowQ = 0;
}

ActivePerkShower::~ActivePerkShower()
{
    ReleaseAll();
}

bool ActivePerkShower::Init()
{
    if ((rs = static_cast<VDX9RENDER *>(core.CreateService("dx9render"))) == nullptr)
    {
        throw std::exception("Can`t create render service");
    }

    if (AttributesPointer == nullptr)
        return false;

    if (!CreateTextures(AttributesPointer->getProperty("Textures")))
        return false;

    if (!CreateShowPlaces(AttributesPointer->getProperty("ShowParam")))
    {
        ReleaseAll();
        return false;
    }
    if (!InitIconsList(AttributesPointer->getProperty("PerkList")["list"]))
    {
        ReleaseAll();
        return false;
    }
    return true;
}

void ActivePerkShower::Execute(uint32_t delta_time)
{
}

void ActivePerkShower::Realize(uint32_t delta_time) const
{
    if (m_pTexDescr.empty())
        return;
    rs->MakePostProcess();

    for (auto i = 0; i < m_nTextureQ; i++)
    {
        if (m_pTexDescr[i].m_nPicsQ == 0)
            continue;
        rs->TextureSet(0, m_pTexDescr[i].m_idTexture);
        rs->DrawBuffer(m_idVBuf, sizeof(BI_ONETEXTURE_VERTEX), m_idIBuf, m_pTexDescr[i].m_nVertStart,
                       m_pTexDescr[i].m_nPicsQ * 4, m_pTexDescr[i].m_nIndxStart, m_pTexDescr[i].m_nPicsQ * 2,
                       "battle_rectangle");
    }
}

uint64_t ActivePerkShower::ProcessMessage(MESSAGE &message)
{
    switch (message.Long())
    {
    case MSG_ACTIVE_PERK_LIST_REFRESH: {
        char param[256];
        message.String(sizeof(param), param);
        auto *const pA = message.AttributePointer();
        if (pA != nullptr) {
            if (_stricmp(param, "add") == 0)
                AddIconToList(*pA);
            else if (_stricmp(param, "del") == 0)
                DelIconFromList(*pA);
        }
    }
    break;
    case MSG_ACTIVE_PERK_ICON_REFRESH:
        RefreshShowPlaces(AttributesPointer->getProperty("ShowParam"));
        break;
    }
    return 0;
}

bool ActivePerkShower::CreateTextures(Attribute &pATextureRoot)
{
    if (pATextureRoot.empty())
        return false;

    std::transform(pATextureRoot.begin(), pATextureRoot.end(), std::back_inserter(m_pTexDescr), [this] (const Attribute& attr) {
      _TEXTURE_DESCR result{};

      if (!attr.empty()) {
          result.m_idTexture = rs->TextureCreate(attr["file"].get<std::string_view>().data());
          attr["horz"].get_to(result.m_nCol);
          attr["vert"].get_to(result.m_nRow);
      }

      return result;
    });

    m_nTextureQ = m_pTexDescr.size();
    return true;
}

bool ActivePerkShower::CreateShowPlaces(Attribute &pAPlacesRoot)
{
    if (pAPlacesRoot.empty())
        return false;

    RefreshShowPlaces(pAPlacesRoot);

    return InitCommonBuffers();
}

void ActivePerkShower::RefreshShowPlaces(Attribute &pAPlacesRoot)
{
    Attribute *pAttr;

    if (m_pShowPlaces)
        STORM_DELETE(m_pShowPlaces);

    m_nIconWidth = 64;
    m_nIconHeight = 64;
    const Attribute& icon_size = pAPlacesRoot["IconSize"];
    icon_size["horz"].get_to(m_nIconWidth);
    icon_size["vert"].get_to(m_nIconHeight);

    m_nSpaceHorz = 4;
    m_nSpaceVert = 4;
    const Attribute& icon_space = pAPlacesRoot["IconSpace"];
    icon_space["horz"].get_to(m_nSpaceHorz);
    icon_space["vert"].get_to(m_nSpaceVert);

    RECT rectBound;
    rectBound.left = 488;
    rectBound.top = 192;
    rectBound.right = 624;
    rectBound.bottom = 464;

    const Attribute& rect_attr = pAPlacesRoot["PosRect"];
    rect_attr["left"].get_to(rectBound.left);
    rect_attr["top"].get_to(rectBound.top);
    rect_attr["right"].get_to(rectBound.right);
    rect_attr["bottom"].get_to(rectBound.bottom);

    int nHorzQ = (rectBound.right - rectBound.left) / (m_nIconWidth + m_nSpaceHorz);
    int nVertQ = (rectBound.bottom - rectBound.top) / (m_nIconHeight + m_nSpaceVert);
    if (nHorzQ <= 0)
        nHorzQ = 1;
    if (nVertQ <= 0)
        nVertQ = 1;

    m_nShowPlaceQ = nHorzQ * nVertQ;
    m_pShowPlaces = new _SHOW_PLACE[m_nShowPlaceQ];
    if (m_pShowPlaces == nullptr)
    {
        throw std::exception("allocate memory error");
    }

    for (auto ih = 0; ih < nHorzQ; ih++)
    {
        for (auto iv = 0; iv < nVertQ; iv++)
        {
            const auto idx = iv + ih * nVertQ;
            m_pShowPlaces[idx].right = static_cast<float>(rectBound.right - ih * (m_nIconWidth + m_nSpaceHorz));
            m_pShowPlaces[idx].left = static_cast<float>(m_pShowPlaces[idx].right - m_nIconWidth);
            m_pShowPlaces[idx].top = static_cast<float>(rectBound.top + iv * (m_nIconHeight + m_nSpaceVert));
            m_pShowPlaces[idx].bottom = static_cast<float>(m_pShowPlaces[idx].top + m_nIconHeight);
        }
    }
}

bool ActivePerkShower::InitIconsList(Attribute &pAIconsRoot)
{
    if (pAIconsRoot.empty())
        return true;

    std::transform(pAIconsRoot.begin(), pAIconsRoot.end(), std::back_inserter(m_pIconsList), [this] (const Attribute& attr) {
      _PICTURE_DESCR result{};

      if (!attr.empty()) {
          attr["texture"].get_to(result.m_nPicNum);
          attr["pic_idx"].get_to(result.m_nPicTexIdx);
      }

      return result;
    });

    m_nIShowQ = m_pIconsList.size();

    FillVIBuffers();
    return true;
}

void ActivePerkShower::AddIconToList(Attribute &pAItemDescr)
{
    if (pAItemDescr.empty())
        return;
    const int picNum = pAItemDescr["pic_idx"].get<int>();
    const int texNum = pAItemDescr["texture"].get<int>();

    if (!m_pIconsList.empty())
    {
        for (auto i = 0; i < m_nIShowQ; i++)
        {
            if (texNum == m_pIconsList[i].m_nPicTexIdx && picNum == m_pIconsList[i].m_nPicNum)
                return; // there is already such an ability
        }
    }

    m_pIconsList.emplace_back(picNum, texNum);
    m_nIShowQ = m_pIconsList.size();

    FillVIBuffers();
}

void ActivePerkShower::DelIconFromList(Attribute &pAIconDescr)
{
    if (pAIconDescr.empty())
        return;
    const int picNum = pAIconDescr["pic_idx"].get<int>();
    const int texNum = pAIconDescr["texture"].get<int>();

    auto found = std::find_if(m_pIconsList.begin(), m_pIconsList.end(), [picNum, texNum](const _PICTURE_DESCR &descr) {
        return descr.m_nPicTexIdx == texNum && descr.m_nPicNum == picNum;
    });

    if (found != m_pIconsList.end()) {
        m_pIconsList.erase(found);
        m_nIShowQ = m_pIconsList.size();
        FillVIBuffers();
    }
}

void ActivePerkShower::FillVIBuffers()
{
    int pi, ti, start_idx;

    auto *pvb = static_cast<BI_ONETEXTURE_VERTEX *>(rs->LockVertexBuffer(m_idVBuf));
    if (pvb == nullptr)
        return;

    start_idx = 0;
    for (ti = 0; ti < m_nTextureQ; ti++)
    {
        m_pTexDescr[ti].m_nPicsQ = 0;
        m_pTexDescr[ti].m_nVertStart = start_idx * 4;
        for (pi = 0; pi < m_nIShowQ && start_idx < m_nShowPlaceQ; pi++)
        {
            if (m_pIconsList[pi].m_nPicTexIdx != ti)
                continue;
            m_pTexDescr[ti].m_nPicsQ++;
            FillRectData(&pvb[start_idx * 4], m_pShowPlaces[pi], GetTextureRect(ti, m_pIconsList[pi].m_nPicNum));
            start_idx++;
        }
    }

    rs->UnLockVertexBuffer(m_idVBuf);
}

void ActivePerkShower::FillRectData(void *vbuf, const FRECT &rectPos, const FRECT &rectTex)
{
    if (vbuf == nullptr)
        return;
    auto *ptmp = static_cast<BI_ONETEXTURE_VERTEX *>(vbuf);
    ptmp[0].pos.x = rectPos.left;
    ptmp[0].pos.y = rectPos.top;
    ptmp[1].pos.x = rectPos.left;
    ptmp[1].pos.y = rectPos.bottom;
    ptmp[2].pos.x = rectPos.right;
    ptmp[2].pos.y = rectPos.top;
    ptmp[3].pos.x = rectPos.right;
    ptmp[3].pos.y = rectPos.bottom;

    ptmp[0].tu = rectTex.left;
    ptmp[0].tv = rectTex.top;
    ptmp[1].tu = rectTex.left;
    ptmp[1].tv = rectTex.bottom;
    ptmp[2].tu = rectTex.right;
    ptmp[2].tv = rectTex.top;
    ptmp[3].tu = rectTex.right;
    ptmp[3].tv = rectTex.bottom;
}

FRECT ActivePerkShower::GetTextureRect(int textIdx, int picIdx) const
{
    FRECT retRect;

    const int vIdx = picIdx / m_pTexDescr[textIdx].m_nCol;
    const int hIdx = picIdx - vIdx * m_pTexDescr[textIdx].m_nCol;

    retRect.left = static_cast<float>(hIdx) / m_pTexDescr[textIdx].m_nCol;
    retRect.top = static_cast<float>(vIdx) / m_pTexDescr[textIdx].m_nRow;
    retRect.right = static_cast<float>(hIdx + 1.f) / m_pTexDescr[textIdx].m_nCol;
    retRect.bottom = static_cast<float>(vIdx + 1.f) / m_pTexDescr[textIdx].m_nRow;

    return retRect;
}

bool ActivePerkShower::InitCommonBuffers()
{
    m_idVBuf = rs->CreateVertexBuffer(BI_ONETEX_VERTEX_FORMAT, m_nShowPlaceQ * 4 * sizeof(BI_ONETEXTURE_VERTEX),
                                      D3DUSAGE_WRITEONLY);
    m_idIBuf = rs->CreateIndexBuffer(m_nShowPlaceQ * 6 * 2);
    if (m_idIBuf == -1 || m_idVBuf == -1)
        return false;

    int i;
    auto *pibuf = static_cast<uint16_t *>(rs->LockIndexBuffer(m_idIBuf));
    for (i = 0; i < m_nShowPlaceQ; i++)
    {
        pibuf[i * 6 + 0] = i * 4 + 0;
        pibuf[i * 6 + 1] = i * 4 + 1;
        pibuf[i * 6 + 2] = i * 4 + 2;
        pibuf[i * 6 + 3] = i * 4 + 2;
        pibuf[i * 6 + 4] = i * 4 + 1;
        pibuf[i * 6 + 5] = i * 4 + 3;
    }
    rs->UnLockIndexBuffer(m_idIBuf);

    auto *pvbuf = static_cast<BI_ONETEXTURE_VERTEX *>(rs->LockVertexBuffer(m_idVBuf));
    for (i = 0; i < m_nShowPlaceQ * 4; i++)
    {
        pvbuf[i].pos.z = 1.f;
        pvbuf[i].w = .5f;
    }
    rs->UnLockVertexBuffer(m_idVBuf);

    return true;
}

void ActivePerkShower::ReleaseAll()
{
    int i;

    VERTEX_BUFFER_RELEASE(rs, m_idVBuf);
    INDEX_BUFFER_RELEASE(rs, m_idIBuf);

    for (i = 0; i < m_nTextureQ; i++)
        TEXTURE_RELEASE(rs, m_pTexDescr[i].m_idTexture);
    m_pTexDescr.clear();
    m_nTextureQ = 0;

    STORM_DELETE(m_pShowPlaces);
    m_nShowPlaceQ = 0;

    m_pIconsList.clear();
    m_nIShowQ = 0;
}
