#include "battle_command.h"
#include "../../Shared/battle_interface/msg_control.h"
#include "Utils.h"
#include "core.h"
#include "image/imgrender.h"
#include "sea/ships_list.h"

namespace {

CommandConfiguration loadConfig(ATTRIBUTES& attribute, VDX9RENDER *rs) {
    CommandConfiguration config{};

    // get icon parameters
    config.m_nIconSpace = attribute.GetAttributeAsDword("CommandIconSpace", 0);
    config.m_LeftTopPoint.x = attribute.GetAttributeAsDword("CommandIconLeft", 100);
    config.m_IconSize.x = attribute.GetAttributeAsDword("CommandIconWidth", config.m_IconSize.x);
    config.m_IconSize.y = attribute.GetAttributeAsDword("CommandIconHeight", config.m_IconSize.y);

    const char *attr = nullptr;

    // get note font parameters
    if (attribute.GetAttribute("CommandNoteFont"))
        config.m_NoteFontID = rs->LoadFont(attribute.GetAttribute("CommandNoteFont"));
    config.m_NoteFontColor = attribute.GetAttributeAsDword("CommandNoteColor", config.m_NoteFontColor);
    config.m_NoteFontScale = attribute.GetAttributeAsFloat("CommandNoteScale", config.m_NoteFontScale);
    if (attribute.GetAttribute("CommandNoteOffset"))
        sscanf(attribute.GetAttribute("CommandNoteOffset"), "%d,%d", &config.m_NoteOffset.x, &config.m_NoteOffset.y);

    // Setting values for arrows (up / down)
    if (attr = attribute.GetAttribute("UDArrow_Texture"))
        config.m_sUpDownArrowTexture = attr;
    BIUtils::ReadRectFromAttr(&attribute, "UDArrow_UV_Up", config.m_frUpArrowUV, config.m_frUpArrowUV);
    BIUtils::ReadRectFromAttr(&attribute, "UDArrow_UV_Down", config.m_frDownArrowUV, config.m_frDownArrowUV);
    BIUtils::ReadPosFromAttr(&attribute, "UDArrow_Size", config.m_pntUpDownArrowSize.x, config.m_pntUpDownArrowSize.y,
                             config.m_pntUpDownArrowSize.x, config.m_pntUpDownArrowSize.y);
    BIUtils::ReadPosFromAttr(&attribute, "UDArrow_Offset_Up", config.m_pntUpArrowOffset.x, config.m_pntUpArrowOffset.y,
                             config.m_pntUpArrowOffset.x, config.m_pntUpArrowOffset.y);
    BIUtils::ReadPosFromAttr(&attribute, "UDArrow_Offset_Down", config.m_pntDownArrowOffset.x, config.m_pntDownArrowOffset.y,
                             config.m_pntDownArrowOffset.x, config.m_pntDownArrowOffset.y);

    // set values for the menu activity icon
    if (attr = attribute.GetAttribute("ActiveIcon_Texture"))
        config.m_sActiveIconTexture = attr;
    BIUtils::ReadPosFromAttr(&attribute, "ActiveIcon_Offset", config.m_pntActiveIconOffset.x, config.m_pntActiveIconOffset.y,
                             config.m_pntActiveIconOffset.x, config.m_pntActiveIconOffset.y);
    BIUtils::ReadPosFromAttr(&attribute, "ActiveIcon_Size", config.m_pntActiveIconSize.x, config.m_pntActiveIconSize.y,
                             config.m_pntActiveIconSize.x, config.m_pntActiveIconSize.y);
    BIUtils::ReadRectFromAttr(&attribute, "ActiveIcon_UV1", config.m_frActiveIconUV1, config.m_frActiveIconUV1);
    BIUtils::ReadRectFromAttr(&attribute, "ActiveIcon_UV2", config.m_frActiveIconUV2, config.m_frActiveIconUV2);
    if (attr = attribute.GetAttribute("ActiveIcon_Note"))
        config.m_sActiveIconNote = attr;

    return config;
}

CommandConfiguration loadConfigPotc(ATTRIBUTES& attribute, VDX9RENDER *rs) {
    CommandConfiguration config{};

    // get icon parameters
    config.m_nIconSpace = attribute.GetAttributeAsDword("iconDistance", 4);
    config.m_LeftTopPoint.x = attribute.GetAttributeAsDword("leftIconsOffset", 16);
    config.m_LeftTopPoint.y = attribute.GetAttributeAsDword("downIconsOffset", 400);
    config.m_IconSize.x = attribute.GetAttributeAsDword("iconWidth", config.m_IconSize.x);
    config.m_IconSize.y = attribute.GetAttributeAsDword("iconHeight", config.m_IconSize.y);

    if (attribute.GetAttribute("commandNoteFont"))
        config.m_NoteFontID = rs->LoadFont(attribute.GetAttribute("commandNoteFont"));
    config.m_NoteOffset.x = attribute.GetAttributeAsDword("noteXOffset", 0);
    config.m_NoteOffset.y = attribute.GetAttributeAsDword("noteYOffset", 0);

    // Fix up some settings to match POTC behaviour
    // _________________________________________________________________________

    const float aspect_ratio = static_cast<float>(core.getScreenSize().height) / static_cast<float>(core.getScreenSize().width);

    // Notes should be left-aligned and anchored to the topleft
    config.m_NoteAlignment = PR_ALIGN_LEFT;
    config.m_NoteOffset.x -= config.m_IconSize.x / 2;
    config.m_NoteOffset.y -= static_cast<long>(static_cast<float>(config.m_IconSize.y / 2) / aspect_ratio);

    return config;
}

} // namespace

BICommandList::BICommandList(entid_t eid, ATTRIBUTES *pA, VDX9RENDER *rs)
{
    m_idHostObj = eid;
    m_pARoot = pA;
    m_pRS = rs;

    m_pImgRender = new BIImageRender(rs);
    Assert(m_pImgRender);

    m_nStartUsedCommandIndex = 0;
    m_nSelectedCommandIndex = 0;
    m_nIconShowMaxQuantity = 5;

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
        m_pRS->ExtPrint(m_Config.m_NoteFontID, m_Config.m_NoteFontColor, 0, m_Config.m_NoteAlignment, true, m_Config.m_NoteFontScale, 0, 0, m_NotePos.x,
                        m_NotePos.y, "%s", m_NoteText.c_str());
}

void BICommandList::Update(long nTopLine, long nCharacterIndex, long nCommandMode)
{
    long nOldSelIndex = 0;
    if (nTopLine == m_Config.m_LeftTopPoint.y && nCharacterIndex == m_nCurrentCommandCharacterIndex &&
        m_nCurrentCommandMode == nCommandMode)
        nOldSelIndex = m_nSelectedCommandIndex;

    if (core.getTargetVersion() != ENGINE_VERSION::PIRATES_OF_THE_CARIBBEAN) {
        m_Config.m_LeftTopPoint.y = nTopLine;
    }
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
        Update(m_Config.m_LeftTopPoint.y, m_nCurrentCommandCharacterIndex, endCode);
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
    if (m_pARoot)
    {
        m_pARoot->SetAttributeUseDword("ComState", (bActive ? 1 : 0));
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
    ATTRIBUTES *pAList{};
    ATTRIBUTES *pATextures{};

    FONT_RELEASE(m_pRS, m_Config.m_NoteFontID);

    if (m_pARoot) {
        if (pAList = m_pARoot->GetAttributeClass("CommandList"); pAList != nullptr)
        {
            m_Config = loadConfig(*pAList, m_pRS);
        }
        else if(pAList = m_pARoot->GetAttributeClass("CommandShowParam"); pAList != nullptr)
        {
            m_Config = loadConfigPotc(*pAList, m_pRS);
        }

        pAList = m_pARoot->GetAttributeClass("CommandTextures");

        pATextures = nullptr;
        if (pAList)
            pATextures = pAList->GetAttributeClass("list");
        if (pATextures)
        {
            size_t q = pATextures->GetAttributesNum();
            for (int n = 0; n < q; n++)
            {
                auto *pA = pATextures->GetAttributeClass(n);
                if (pA)
                {
                    TextureDescr td = {pA->GetAttribute("name") ? pA->GetAttribute("name") : std::string(),
                                       pA->GetAttributeAsDword("xsize", 1), pA->GetAttributeAsDword("ysize", 1)};

                    if (td.nCols < 1)
                        td.nCols = 1;
                    if (td.nRows < 1)
                        td.nRows = 1;

                    if (core.getTargetVersion() == ENGINE_VERSION::PIRATES_OF_THE_CARIBBEAN) {
                        td.nCols *= 2;
                    }

                    m_pImgRender->CreateMaterial(td.sFileName.c_str());

                    m_aTexture.push_back(td);
                }
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
    FONT_RELEASE(m_pRS, m_Config.m_NoteFontID);
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
    rpos.left = m_Config.m_LeftTopPoint.x + num * (m_Config.m_IconSize.x + m_Config.m_nIconSpace);
    rpos.right = rpos.left + m_Config.m_IconSize.x;
    rpos.top = m_Config.m_LeftTopPoint.y;
    rpos.bottom = rpos.top + m_Config.m_IconSize.y;
    return rpos;
}

void BICommandList::UpdateShowIcon()
{
    RECT rPos;

    m_pImgRender->ReleaseAllImages();

    if (!m_bActive)
    {
        rPos.left = m_Config.m_LeftTopPoint.x + m_Config.m_pntActiveIconOffset.x;
        rPos.top = m_Config.m_LeftTopPoint.y + m_Config.m_pntActiveIconOffset.y;
        rPos.right = rPos.left + m_Config.m_pntActiveIconSize.x;
        rPos.bottom = rPos.top + m_Config.m_pntActiveIconSize.y;
        m_pImgRender->CreateImage(BIType_square, m_Config.m_sActiveIconTexture.c_str(), 0xFF808080, m_Config.m_frActiveIconUV1, rPos);
        SetNote(m_Config.m_sActiveIconNote.c_str(), (rPos.left + rPos.right) / 2, (rPos.top + rPos.bottom) / 2);
        return;
    }
    rPos.left = m_Config.m_LeftTopPoint.x + m_Config.m_pntActiveIconOffset.x;
    rPos.top = m_Config.m_LeftTopPoint.y + m_Config.m_pntActiveIconOffset.y;
    rPos.right = rPos.left + m_Config.m_pntActiveIconSize.x;
    rPos.bottom = rPos.top + m_Config.m_pntActiveIconSize.y;
    m_pImgRender->CreateImage(BIType_square, m_Config.m_sActiveIconTexture.c_str(), 0xFF808080, m_Config.m_frActiveIconUV2, rPos);

    m_bLeftArrow = m_nStartUsedCommandIndex > 0;
    m_bRightArrow = false;

    long i = 0;
    for (auto n = m_nStartUsedCommandIndex; n < m_aUsedCommand.size() && i < m_nIconShowMaxQuantity; n++)
    {
        GetCurrentPos(i, rPos);
        AdditiveIconAdd(.5f * (rPos.left + rPos.right), static_cast<float>(rPos.bottom), m_aUsedCommand[n].aAddPicList);
        if (n == m_nSelectedCommandIndex)
        {
            if (m_aUsedCommand[n].nCooldownPictureIndex < 0) {
                if (core.getTargetVersion() == ENGINE_VERSION::PIRATES_OF_THE_CARIBBEAN) {
                    const long pictureIndex = m_aUsedCommand[n].nNormPictureIndex;
                    long textureId = m_aUsedCommand[n].nTextureIndex;
                    if (textureId == -1) {
                        textureId = 0;
                    }
                    if (m_aTexture.size() > textureId) {
                        const long textureColumns = m_aTexture[textureId].nCols;
                        const long potcTextureColumns = textureColumns / 2;
                        const long selectedRow = pictureIndex / (potcTextureColumns);
                        const long selectedPictureIndex = selectedRow * textureColumns + (pictureIndex % potcTextureColumns) + potcTextureColumns;
                        i += IconAdd(selectedPictureIndex, textureId, rPos);
                    }
                }
                else {
                    i += IconAdd(m_aUsedCommand[n].nSelPictureIndex, m_aUsedCommand[n].nTextureIndex, rPos);
                }
            }
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
        rPos.left = m_Config.m_LeftTopPoint.x + m_Config.m_pntUpArrowOffset.x;
        rPos.top = m_Config.m_LeftTopPoint.y + m_Config.m_pntUpArrowOffset.y;
        rPos.right = rPos.left + m_Config.m_pntUpDownArrowSize.x;
        rPos.bottom = rPos.top + m_Config.m_pntUpDownArrowSize.y;
        m_pImgRender->CreateImage(BIType_square, m_Config.m_sUpDownArrowTexture.c_str(), 0xFF808080, m_Config.m_frUpArrowUV, rPos);
    }
    if (m_bDownArrow)
    {
        rPos.left = m_Config.m_LeftTopPoint.x + m_Config.m_pntDownArrowOffset.x;
        rPos.top = m_Config.m_LeftTopPoint.y + m_Config.m_pntDownArrowOffset.y;
        rPos.right = rPos.left + m_Config.m_pntUpDownArrowSize.x;
        rPos.bottom = rPos.top + m_Config.m_pntUpDownArrowSize.y;
        m_pImgRender->CreateImage(BIType_square, m_Config.m_sUpDownArrowTexture.c_str(), 0xFF808080, m_Config.m_frDownArrowUV, rPos);
    }
}

void BICommandList::SetNote(const char *pcNote, long nX, long nY)
{
    m_NoteText = pcNote;
    m_NotePos.x = nX + m_Config.m_NoteOffset.x;
    m_NotePos.y = nY + m_Config.m_NoteOffset.y;
}

ATTRIBUTES *BICommandList::GetCurrentCommandAttribute() const
{
    if (m_sCurrentCommandName.empty())
        return nullptr;

    ATTRIBUTES *pAR = nullptr;
    if (m_nCurrentCommandMode & BI_COMMODE_ABILITY_ICONS)
        pAR = m_pARoot->GetAttributeClass("AbilityIcons");
    else
        pAR = m_pARoot->GetAttributeClass("Commands");
    if (!pAR)
        return nullptr;

    const size_t q = pAR->GetAttributesNum();
    for (long n = 0; n < q; n++)
    {
        auto *pA = pAR->GetAttributeClass(n);
        if (!pA)
            continue;
        auto *const pcCommName = pA->GetAttribute("event");
        if (m_sCurrentCommandName == pcCommName)
            return pA;
    }
    return nullptr;
}
