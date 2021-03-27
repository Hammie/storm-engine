#include "battle_command.h"
#include "../../Shared/battle_interface/msg_control.h"
#include "Utils.h"
#include "core.h"
#include "image/imgrender.h"
#include "sea/ships_list.h"

BICommandList::BICommandList(entid_t eid, Attribute &pA, VDX9RENDER *rs)
    : m_pARoot(pA)
{
    m_idHostObj = eid;
    m_pRS = rs;

    m_pImgRender = new BIImageRender(rs);
    Assert(m_pImgRender);

    m_nStartUsedCommandIndex = 0;
    m_nSelectedCommandIndex = 0;
    m_nIconShowMaxQuantity = 5;

    m_NoteFontID = -1;
    m_NotePos.x = m_NotePos.y = 0;
    m_NoteOffset.x = m_NoteOffset.y = 0;

    m_bUpArrow = m_bDownArrow = false;

    m_bActive = false;

    Init();
}

BICommandList::~BICommandList()
{
    Release();
}

void BICommandList::Draw()
{
    if (m_aCooldownUpdate.size() > 0)
    {
        const auto fDT = core.GetDeltaTime() * .001f;
        for (long n = 0; n < m_aCooldownUpdate.size(); n++)
        {
            m_aCooldownUpdate[n].fTime -= fDT;
            if (m_aCooldownUpdate[n].fTime < 0)
            {
                m_aCooldownUpdate[n].fTime = m_aCooldownUpdate[n].fUpdateTime;
                auto *pDat = core.Event("neGetCooldownFactor", "s",
                                        m_aUsedCommand[m_aCooldownUpdate[n].nIconNum].sCommandName.c_str());
                if (pDat)
                    m_aUsedCommand[m_aCooldownUpdate[n].nIconNum].fCooldownFactor = pDat->GetFloat();
                UpdateShowIcon();
            }
        }
    }
    if (m_pImgRender)
        m_pImgRender->Render();

    if (!m_NoteText.empty())
        m_pRS->ExtPrint(m_NoteFontID, m_NoteFontColor, 0, PR_ALIGN_CENTER, true, m_NoteFontScale, 0, 0, m_NotePos.x,
                        m_NotePos.y, "%s", m_NoteText.c_str());
}

void BICommandList::Update(long nTopLine, long nCharacterIndex, long nCommandMode)
{
    long nOldSelIndex = 0;
    if (nTopLine == m_LeftTopPoint.y && nCharacterIndex == m_nCurrentCommandCharacterIndex &&
        m_nCurrentCommandMode == nCommandMode)
        nOldSelIndex = m_nSelectedCommandIndex;

    m_LeftTopPoint.y = nTopLine;
    if (nCharacterIndex != m_nCurrentCommandCharacterIndex)
        m_sCurrentCommandName = "";
    m_nCurrentCommandCharacterIndex = nCharacterIndex;
    m_nCurrentCommandMode = nCommandMode;

    m_nStartUsedCommandIndex = 0;
    m_nSelectedCommandIndex = 0;
    m_aUsedCommand.clear();
    m_aCooldownUpdate.clear();

    m_NoteText.clear();

    FillIcons();

    if (nOldSelIndex > 0 && nOldSelIndex < m_aUsedCommand.size())
        m_nSelectedCommandIndex = nOldSelIndex;

    UpdateShowIcon();
}

size_t BICommandList::AddTexture(const char *pcTextureName, uint32_t nCols, uint32_t nRows)
{
    m_aTexture.push_back(TextureDescr{pcTextureName, nCols, nRows});
    return m_aTexture.size() - 1;
}

long BICommandList::ExecuteConfirm()
{
    if (m_nSelectedCommandIndex >= m_aUsedCommand.size())
        return 0; // error!

    long endCode = 0;
    long nTargIndex = 0;
    std::string sLocName;

    if (m_aUsedCommand[m_nSelectedCommandIndex].nCooldownPictureIndex >= 0 &&
        m_aUsedCommand[m_nSelectedCommandIndex].fCooldownFactor < 1.f)
        return -1;

    if (!m_aUsedCommand[m_nSelectedCommandIndex].sCommandName.empty())
    {
        m_sCurrentCommandName = m_aUsedCommand[m_nSelectedCommandIndex].sCommandName;
        auto *pVD = core.Event("BI_CommandEndChecking", "s", m_sCurrentCommandName.c_str());
        if (pVD != nullptr)
            pVD->Get(endCode);
    }
    else
    {
        sLocName = m_aUsedCommand[m_nSelectedCommandIndex].sLocName;
        nTargIndex = m_aUsedCommand[m_nSelectedCommandIndex].nTargetIndex;
        if (sLocName.empty() && m_aUsedCommand[m_nSelectedCommandIndex].nCharIndex >= 0)
            nTargIndex = m_aUsedCommand[m_nSelectedCommandIndex].nCharIndex;
    }
    core.Event("evntBattleCommandSound", "s", "activate"); // boal 22.08.06
    switch (endCode)
    {
    case -1:
    case 0:
        core.Event("BI_LaunchCommand", "lsls", m_nCurrentCommandCharacterIndex, m_sCurrentCommandName.c_str(),
                   nTargIndex, sLocName.c_str());
        m_sCurrentCommandName = "";
        break;
    default:
        Update(m_LeftTopPoint.y, m_nCurrentCommandCharacterIndex, endCode);
    }
    return endCode;
}

long BICommandList::ExecuteLeft()
{
    if (m_nSelectedCommandIndex > 0)
    {
        m_nSelectedCommandIndex--;
        if (m_nSelectedCommandIndex < m_nStartUsedCommandIndex)
        {
            m_nStartUsedCommandIndex = m_nSelectedCommandIndex;
        }
        core.Event("evntBattleCommandSound", "s", "left"); // boal 22.08.06
        UpdateShowIcon();
    }
    return 0;
}

long BICommandList::ExecuteRight()
{
    if (m_nSelectedCommandIndex < static_cast<long>(m_aUsedCommand.size()) - 1)
    {
        m_nSelectedCommandIndex++;
        if (m_nSelectedCommandIndex >= m_nStartUsedCommandIndex + m_nIconShowMaxQuantity)
        {
            m_nStartUsedCommandIndex = m_nSelectedCommandIndex - m_nIconShowMaxQuantity + 1;
        }
        core.Event("evntBattleCommandSound", "s", "right"); // boal 22.08.06
        UpdateShowIcon();
    }
    return 0;
}

long BICommandList::ExecuteCancel()
{
    m_nSelectedCommandIndex = 0;
    m_nStartUsedCommandIndex = 0;
    if (m_sCurrentCommandName.empty())
        return 0;
    m_sCurrentCommandName = "";
    if (m_nCurrentCommandMode == BI_COMMODE_COMMAND_SELECT)
        return 0;
    return BI_COMMODE_COMMAND_SELECT;
}

void BICommandList::SetActive(bool bActive)
{
    if (!m_pARoot.empty())
    {
        m_pARoot["ComState"] = bActive;
    }

    if (m_bActive == bActive)
        return;
    m_bActive = bActive;
    UpdateShowIcon();
}

void BICommandList::SetUpDown(bool bUp, bool bDown)
{
    if (bUp == m_bUpArrow && bDown == m_bDownArrow)
        return;
    m_bUpArrow = bUp;
    m_bDownArrow = bDown;
    UpdateShowIcon();
}

void BICommandList::Init()
{
    Assert(m_pImgRender);

    m_LeftTopPoint.x = 120;
    m_LeftTopPoint.y = 100;
    m_IconSize.x = 64;
    m_IconSize.y = 64;
    m_nIconSpace = 8;

    FONT_RELEASE(m_pRS, m_NoteFontID);
    m_NoteFontColor = ARGB(255, 255, 255, 255);
    m_NoteFontScale = 1.f;
    m_NoteOffset.x = m_NoteOffset.y = 0;

    // default arrow data value
    m_sUpDownArrowTexture = "";
    m_pntUpDownArrowSize.x = m_pntUpDownArrowSize.y = 32;
    FULLRECT(m_frUpArrowUV);
    FULLRECT(m_frDownArrowUV);
    m_pntUpArrowOffset.x = 32;
    m_pntUpArrowOffset.y = -34;
    m_pntDownArrowOffset.x = 32;
    m_pntDownArrowOffset.y = 66;

    // default data for active icon
    m_sActiveIconTexture = "";
    m_pntActiveIconOffset.x = -33;
    m_pntActiveIconOffset.y = 0;
    m_pntActiveIconSize.x = m_pntActiveIconSize.y = 64;
    FULLRECT(m_frActiveIconUV1);
    FULLRECT(m_frActiveIconUV2);
    m_sActiveIconNote = "";

    const Attribute& command_list = m_pARoot["CommandList"];
    if (!command_list.empty())
    {
        // get icon parameters
        command_list["CommandIconSpace"].get_to(m_nIconSpace, 0l);
        command_list["CommandIconLeft"].get_to(m_LeftTopPoint.x, 100l);
        command_list["CommandIconWidth"].get_to(m_IconSize.x);
        command_list["CommandIconHeight"].get_to(m_IconSize.y);

        // get note font parameters
        m_NoteFontID = BIUtils::GetFontIDFromAttr(command_list, "CommandNoteFont", m_pRS).value_or(-1);

        command_list["CommandNoteColor"].get_to(m_NoteFontColor);
        command_list["CommandNoteScale"].get_to(m_NoteFontScale);
        command_list["CommandNoteOffset"].get_to(m_NoteOffset);

        command_list["UDArrow_Texture"].get_to(m_sUpDownArrowTexture);
        command_list["UDArrow_UV_Up"].get_to(m_frUpArrowUV);
        command_list["UDArrow_UV_Down"].get_to(m_frDownArrowUV);
        command_list["UDArrow_Size"].get_to(m_pntUpDownArrowSize);
        command_list["UDArrow_Offset_Up"].get_to(m_pntUpArrowOffset);
        command_list["UDArrow_Offset_Down"].get_to(m_pntDownArrowOffset);

        command_list["ActiveIcon_Texture"].get_to(m_sActiveIconTexture);
        command_list["ActiveIcon_Offset"].get_to(m_pntActiveIconOffset);
        command_list["ActiveIcon_Size"].get_to(m_pntActiveIconSize);
        command_list["ActiveIcon_UV1"].get_to(m_frActiveIconUV1);
        command_list["ActiveIcon_UV2"].get_to(m_frActiveIconUV2);
        command_list["ActiveIcon_Note"].get_to(m_sActiveIconNote);
    }

    const Attribute& command_texture_list = m_pARoot["CommandTextures"]["list"];
    if (!command_list.empty()) {
        for (const Attribute& texture : command_texture_list) {
            if (!texture.empty()) {
                TextureDescr td =
                { texture["name"].get<std::string>(),
                  texture["xsize"].get<uint32_t>(1),
                  texture["ysize"].get<uint32_t>(1),
                };

                if (td.nCols < 1)
                    td.nCols = 1;
                if (td.nRows < 1)
                    td.nRows = 1;

                m_pImgRender->CreateMaterial(td.sFileName.c_str());

                m_aTexture.push_back(td);
            }
        }
    }
    m_nIconShowMaxQuantity = 5;
}

long BICommandList::AddToIconList(long nTextureNum, long nNormPictureNum, long nSelPictureNum, long nCooldownPictureNum,
                                  long nCharacterIndex, const char *pcCommandName, long nTargetIndex,
                                  const char *pcLocName, const char *pcNoteName)
{
    long n;
    // filtering out already used objects
    for (n = 0; n < m_aUsedCommand.size(); n++)
    {
        if (nCharacterIndex != -1 && m_aUsedCommand[n].nCharIndex == nCharacterIndex)
            return 0;
        if (pcCommandName && m_aUsedCommand[n].sCommandName == pcCommandName)
            return 0;
        if (pcLocName && m_aUsedCommand[n].sLocName == pcLocName)
            return 0;
        if (nTargetIndex != -1 && m_aUsedCommand[n].nTargetIndex == nTargetIndex)
            return 0;
    }

    UsedCommand uc;
    uc.fCooldownFactor = 1.f;
    uc.nCharIndex = nCharacterIndex;
    uc.nCooldownPictureIndex = nCooldownPictureNum;
    uc.nNormPictureIndex = nNormPictureNum;
    uc.nSelPictureIndex = nSelPictureNum;
    uc.nTargetIndex = nTargetIndex;
    uc.nTextureIndex = nTextureNum;
    if (pcCommandName)
        uc.sCommandName = pcCommandName;
    if (pcLocName)
        uc.sLocName = pcLocName;
    if (pcNoteName)
        uc.sNote = pcNoteName;
    m_aUsedCommand.push_back(uc);

    if (nCooldownPictureNum >= 0)
    {
        auto *pDat = core.Event("neGetCooldownFactor", "s", pcCommandName);
        if (pDat)
            m_aUsedCommand[n].fCooldownFactor = pDat->GetFloat();
        CoolDownUpdateData data;
        data.fUpdateTime = data.fTime = 1.f;
        data.nIconNum = n;
        m_aCooldownUpdate.push_back(data);
    }
    return 1;
}

void BICommandList::AddAdditiveToIconList(long nTextureNum, long nPictureNum, float fDist, float fWidth, float fHeight)
{
    const size_t n = m_aUsedCommand.size() - 1;
    if (n < 0)
        return;

    UsedCommand::AdditiveIcon icon;
    icon.fDelta = fDist;
    icon.fpSize.x = fWidth;
    icon.fpSize.y = fHeight;
    icon.nPic = nPictureNum;
    icon.nTex = nTextureNum;

    m_aUsedCommand[n].aAddPicList.push_back(icon);
}

void BICommandList::Release()
{
    STORM_DELETE(m_pImgRender);
    FONT_RELEASE(m_pRS, m_NoteFontID);
}

long BICommandList::IconAdd(long nPictureNum, long nTextureNum, RECT &rpos)
{
    if (nTextureNum < 0 || nTextureNum >= m_aTexture.size() || nPictureNum < 0 ||
        nPictureNum >= m_aTexture[nTextureNum].nCols * m_aTexture[nTextureNum].nRows)
        return 0;

    FRECT uv;
    m_pImgRender->CreateImage(BIType_square, m_aTexture[nTextureNum].sFileName.c_str(), 0xFF808080,
                              GetPictureUV(nTextureNum, nPictureNum, uv), rpos);
    return 1;
}

long BICommandList::ClockIconAdd(long nForePictureNum, long nBackPictureNum, long nTextureNum, RECT &rpos,
                                 float fFactor)
{
    if (nTextureNum < 0 || nTextureNum >= m_aTexture.size() || nForePictureNum < 0 ||
        nForePictureNum >= m_aTexture[nTextureNum].nCols * m_aTexture[nTextureNum].nRows)
        return 0;

    FRECT uv;
    m_pImgRender->CreateImage(BIType_square, m_aTexture[nTextureNum].sFileName.c_str(), 0xFF808080,
                              GetPictureUV(nTextureNum, nBackPictureNum, uv), rpos);
    auto *pImg = m_pImgRender->CreateImage(BIType_clocksquare, m_aTexture[nTextureNum].sFileName.c_str(), 0xFF808080,
                                           GetPictureUV(nTextureNum, nForePictureNum, uv), rpos);
    if (pImg)
        pImg->CutClock(0.f, 1.f, fFactor);
    return 1;
}

void BICommandList::AdditiveIconAdd(float fX, float fY, std::vector<UsedCommand::AdditiveIcon> &aList)
{
    if (aList.size() <= 0)
        return;
    RECT rCur;
    for (long n = 0; n < aList.size(); n++)
    {
        rCur.top = static_cast<long>(fY + aList[n].fDelta);
        rCur.bottom = rCur.top + static_cast<long>(aList[n].fpSize.y);
        rCur.left = static_cast<long>(fX - .5f * aList[n].fpSize.x);
        rCur.right = rCur.left + static_cast<long>(aList[n].fpSize.x);
        if (IconAdd(aList[n].nPic, aList[n].nTex, rCur) > 0)
        {
            fY = static_cast<float>(rCur.bottom);
        }
    }
}

FRECT &BICommandList::GetPictureUV(long nTextureNum, long nPictureNum, FRECT &uv)
{
    if (nTextureNum < 0 || nTextureNum >= m_aTexture.size() || nPictureNum < 0 ||
        nPictureNum >= m_aTexture[nTextureNum].nCols * m_aTexture[nTextureNum].nRows)
    {
        uv.left = uv.top = 0.f;
        uv.right = uv.bottom = 1.f;
    }
    else
    {
        const auto fDU = 1.f / m_aTexture[nTextureNum].nCols;
        const auto fDV = 1.f / m_aTexture[nTextureNum].nRows;
        const auto ny = nPictureNum / m_aTexture[nTextureNum].nCols;
        const auto nx = nPictureNum - ny * m_aTexture[nTextureNum].nCols;
        uv.left = nx * fDU;
        uv.right = uv.left + fDU;
        uv.top = ny * fDV;
        uv.bottom = uv.top + fDV;
    }
    return uv;
}

RECT &BICommandList::GetCurrentPos(long num, RECT &rpos) const
{
    rpos.left = m_LeftTopPoint.x + num * (m_IconSize.x + m_nIconSpace);
    rpos.right = rpos.left + m_IconSize.x;
    rpos.top = m_LeftTopPoint.y;
    rpos.bottom = rpos.top + m_IconSize.y;
    return rpos;
}

void BICommandList::UpdateShowIcon()
{
    RECT rPos;

    m_pImgRender->ReleaseAllImages();

    if (!m_bActive)
    {
        rPos.left = m_LeftTopPoint.x + m_pntActiveIconOffset.x;
        rPos.top = m_LeftTopPoint.y + m_pntActiveIconOffset.y;
        rPos.right = rPos.left + m_pntActiveIconSize.x;
        rPos.bottom = rPos.top + m_pntActiveIconSize.y;
        m_pImgRender->CreateImage(BIType_square, m_sActiveIconTexture.c_str(), 0xFF808080, m_frActiveIconUV1, rPos);
        SetNote(m_sActiveIconNote.c_str(), (rPos.left + rPos.right) / 2, (rPos.top + rPos.bottom) / 2);
        return;
    }
    rPos.left = m_LeftTopPoint.x + m_pntActiveIconOffset.x;
    rPos.top = m_LeftTopPoint.y + m_pntActiveIconOffset.y;
    rPos.right = rPos.left + m_pntActiveIconSize.x;
    rPos.bottom = rPos.top + m_pntActiveIconSize.y;
    m_pImgRender->CreateImage(BIType_square, m_sActiveIconTexture.c_str(), 0xFF808080, m_frActiveIconUV2, rPos);

    m_bLeftArrow = m_nStartUsedCommandIndex > 0;
    m_bRightArrow = false;

    long i = 0;
    for (auto n = m_nStartUsedCommandIndex; n < m_aUsedCommand.size() && i < m_nIconShowMaxQuantity; n++)
    {
        GetCurrentPos(i, rPos);
        AdditiveIconAdd(.5f * (rPos.left + rPos.right), static_cast<float>(rPos.bottom), m_aUsedCommand[n].aAddPicList);
        if (n == m_nSelectedCommandIndex)
        {
            if (m_aUsedCommand[n].nCooldownPictureIndex < 0)
                i += IconAdd(m_aUsedCommand[n].nSelPictureIndex, m_aUsedCommand[n].nTextureIndex, rPos);
            else
                i += ClockIconAdd(m_aUsedCommand[n].nSelPictureIndex, m_aUsedCommand[n].nCooldownPictureIndex,
                                  m_aUsedCommand[n].nTextureIndex, rPos, m_aUsedCommand[n].fCooldownFactor);
            SetNote(m_aUsedCommand[n].sNote.c_str(), (rPos.left + rPos.right) / 2, (rPos.top + rPos.bottom) / 2);
            auto *const pSD = g_ShipList.FindShip(m_aUsedCommand[n].nCharIndex);
            if (pSD)
            {
                core.Event("evntBISelectShip", "ll", pSD->characterIndex, pSD->relation != BI_RELATION_ENEMY);
            }
            else
                core.Event("evntBISelectShip", "ll", -1, true);
        }
        else
        {
            if (m_aUsedCommand[n].nCooldownPictureIndex < 0)
                i += IconAdd(m_aUsedCommand[n].nNormPictureIndex, m_aUsedCommand[n].nTextureIndex, rPos);
            else
                i += ClockIconAdd(m_aUsedCommand[n].nNormPictureIndex, m_aUsedCommand[n].nCooldownPictureIndex,
                                  m_aUsedCommand[n].nTextureIndex, rPos, m_aUsedCommand[n].fCooldownFactor);
        }
    }

    // add bottom / top arrows for the command list
    if (m_bUpArrow)
    {
        rPos.left = m_LeftTopPoint.x + m_pntUpArrowOffset.x;
        rPos.top = m_LeftTopPoint.y + m_pntUpArrowOffset.y;
        rPos.right = rPos.left + m_pntUpDownArrowSize.x;
        rPos.bottom = rPos.top + m_pntUpDownArrowSize.y;
        m_pImgRender->CreateImage(BIType_square, m_sUpDownArrowTexture.c_str(), 0xFF808080, m_frUpArrowUV, rPos);
    }
    if (m_bDownArrow)
    {
        rPos.left = m_LeftTopPoint.x + m_pntDownArrowOffset.x;
        rPos.top = m_LeftTopPoint.y + m_pntDownArrowOffset.y;
        rPos.right = rPos.left + m_pntUpDownArrowSize.x;
        rPos.bottom = rPos.top + m_pntUpDownArrowSize.y;
        m_pImgRender->CreateImage(BIType_square, m_sUpDownArrowTexture.c_str(), 0xFF808080, m_frDownArrowUV, rPos);
    }
}

void BICommandList::SetNote(const char *pcNote, long nX, long nY)
{
    m_NoteText = pcNote;
    m_NotePos.x = nX + m_NoteOffset.x;
    m_NotePos.y = nY + m_NoteOffset.y;
}

Attribute *BICommandList::GetCurrentCommandAttribute() const
{
    if (m_sCurrentCommandName.empty())
        return nullptr;

    Attribute *pAR = nullptr;
    if (m_nCurrentCommandMode & BI_COMMODE_ABILITY_ICONS)
        pAR = &m_pARoot.getProperty("AbilityIcons");
    else
        pAR = &m_pARoot.getProperty("Commands");
    if (pAR->empty())
        return nullptr;

    for (Attribute& attr : *pAR) {
        if (!attr.empty()) {
            if (m_sCurrentCommandName == attr["event"].get<std::string_view>()) {
                return &attr;
            }
        }
    }

    return nullptr;
}
