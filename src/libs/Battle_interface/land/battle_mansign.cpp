#include "battle_mansign.h"
#include "../Utils.h"
#include "../shared/battle_interface/msg_control.h"
#include "battle_mancommand.h"
#include "core.h"
#include "vmodule_api.h"

BIManSign::BIManSign(entid_t BIEntityID, VDX9RENDER *pRS)
{
    Assert(pRS);

    m_idHostEntity = BIEntityID;
    m_pRS = pRS;
    m_pCommandList = nullptr;
    m_nManQuantity = 0;

    m_nVBufID = -1;
    m_nIBufID = -1;
    m_nBackTextureID = -1;
    m_nManStateTextureID = -1;
    m_nGunChargeTextureID = -1;
    m_nBackSquareQ = 0;
    m_nManStateSquareQ = 0;
    m_nGunChargeSquareQ = 0;

    m_nMaxSquareQ = 0;
    m_nSquareQ = 0;

    m_bMakeUpdate = true;
    m_bMakeVertexFill = false;
    m_nCommandMode = 0;

    m_nCommandListVerticalOffset = -48;

    m_nCurrentManIndex = 0;

    for (long n = 0; n < MAX_MAN_QUANTITY; n++)
        m_Man[n].nTexture = -1;

    m_bIsAlarmOn = false;
    m_nAlarmTextureID = -1;
    m_fAlarmTime = 0.f;
    m_fAlarmUpSpeed = 1.f;
    m_fAlarmDownSpeed = 1.f;
}

BIManSign::~BIManSign()
{
    Release();
}

void BIManSign::Draw()
{
    // update
    if (m_bMakeUpdate)
    {
        Recollect();
        CheckDataChange();
        m_bMakeUpdate = false;

        UpdateCommandList();
        m_bMakeVertexFill = true;
    }
    else
    {
        // checking for changes in the data
        CheckDataChange();
    }

    // buffer overflow
    if (m_bMakeVertexFill)
    {
        FillVertexBuffer();
        m_bMakeVertexFill = false;
    }

    if (m_nVBufID != -1 && m_nIBufID != -1)
    {
        long nStartV = 0;
        const long nStartI = 0;

        for (long n = 0; n < m_nManQuantity; n++)
        {
            m_pRS->TextureSet(0, m_Man[n].nTexture);
            m_pRS->DrawBuffer(m_nVBufID, sizeof(BI_COLOR_VERTEX), m_nIBufID, nStartV, 4, nStartI, 2,
                              "battle_colorRectangle");
            nStartV += 4;
        }

        //
        if (m_nBackSquareQ > 0)
        {
            m_pRS->TextureSet(0, m_nBackTextureID);
            m_pRS->DrawBuffer(m_nVBufID, sizeof(BI_COLOR_VERTEX), m_nIBufID, nStartV, m_nBackSquareQ * 4, nStartI,
                              m_nBackSquareQ * 2, "battle_colorRectangle");
        }
        nStartV += m_nBackSquareQ * 4;

        if (m_bIsAlarmOn && m_nManQuantity > 0)
        {
            m_pRS->TextureSet(0, m_nAlarmTextureID);
            m_pRS->DrawBuffer(m_nVBufID, sizeof(BI_COLOR_VERTEX), m_nIBufID, nStartV, 4, nStartI, 2,
                              "battle_tex_col_Rectangle");
            nStartV += 4;
        }

        //
        if (m_nManStateSquareQ > 0)
        {
            m_pRS->TextureSet(0, m_nManStateTextureID);
            m_pRS->DrawBuffer(m_nVBufID, sizeof(BI_COLOR_VERTEX), m_nIBufID, nStartV, m_nManStateSquareQ * 4, nStartI,
                              m_nManStateSquareQ * 2, "battle_colorRectangle");
        }
        nStartV += m_nManStateSquareQ * 4;

        //
        if (m_nGunChargeSquareQ > 0)
        {
            m_pRS->TextureSet(0, m_nGunChargeTextureID);
            m_pRS->DrawBuffer(m_nVBufID, sizeof(BI_COLOR_VERTEX), m_nIBufID, nStartV, m_nGunChargeSquareQ * 4, nStartI,
                              m_nGunChargeSquareQ * 2, "battle_alphacutcolor");
        }
        nStartV += m_nGunChargeSquareQ * 4;
    }

    if (m_pCommandList)
        m_pCommandList->Draw();
}

void BIManSign::Init(Attribute *pRoot, Attribute *pA)
{
    long n;

    m_pARoot = pRoot;

    m_pCommandList = new BIManCommandList(m_idHostEntity, *pRoot, m_pRS);

    // default value
    m_nBackTextureID = -1;
    m_dwBackColor = ARGB(255, 128, 128, 128);
    FULLRECT(m_rBackUV);
    ZERROPOINT(m_pntBackOffset);
    FILLPOINT(m_pntBackIconSize, 128, 128);

    m_bIsAlarmOn = false;
    m_nAlarmTextureID = -1;
    m_dwAlarmHighColor = ARGB(255, 168, 28, 28);
    m_dwAlarmLowColor = ARGB(64, 168, 28, 28);
    FULLRECT(m_rAlarmUV);
    ZERROPOINT(m_pntAlarmOffset);
    FILLPOINT(m_pntAlarmIconSize, 128, 128);
    m_fAlarmTime = 0.f;
    m_fAlarmUpSpeed = 2.f;
    m_fAlarmDownSpeed = 0.666f;

    m_nManStateTextureID = -1;
    m_dwManStateColor = ARGB(255, 128, 128, 128);
    FULLRECT(m_rManHPUV);
    ZERROPOINT(m_pntManHPOffset);
    FILLPOINT(m_pntManHPIconSize, 128, 128);
    FULLRECT(m_rManEnergyUV);
    ZERROPOINT(m_pntManEnergyOffset);
    FILLPOINT(m_pntManEnergyIconSize, 128, 128);

    m_nGunChargeTextureID = -1;
    m_dwGunChargeColor = ARGB(255, 128, 128, 128);
    m_dwGunChargeBackColor = ARGB(255, 128, 64, 64);
    FULLRECT(m_rGunChargeUV);
    ZERROPOINT(m_pntGunChargeOffset);
    FILLPOINT(m_pntGunChargeIconSize, 128, 128);

    m_dwManFaceColor = ARGB(255, 128, 128, 128);
    ZERROPOINT(m_pntManPicOffset);
    FILLPOINT(m_pntManPicIconSize, 64, 64);

    m_nCommandListVerticalOffset = -48;

    for (n = 0; n < MAX_MAN_QUANTITY; n++)
    {
        m_Man[n].pntPos.x = 20.f;
        m_Man[n].pntPos.y = 20.f + (m_pntBackIconSize.y + 10.f) * n;
    }

    if (pA)
    {
        Attribute& attr = *pA;

        const std::string_view pcTmp = attr["backtexturename"].get<std::string_view>();
        if (!pcTmp.empty())
            m_nBackTextureID = m_pRS->TextureCreate(pcTmp.data());

        m_dwBackColor = attr["backcolor"].get<uint32_t>(m_dwBackColor);

        BIUtils::ReadRectFromAttr(attr, "backuv", m_rBackUV);

        BIUtils::ReadPosFromAttr(attr, "backoffset", m_pntBackOffset);
        BIUtils::ReadPosFromAttr(attr, "backiconsize", m_pntBackIconSize);

        const std::string_view alarmtexturename = attr["alarmtexturename"].get<std::string_view>();
        if (!alarmtexturename.empty())
            m_nAlarmTextureID = m_pRS->TextureCreate(alarmtexturename.data());

        m_dwAlarmHighColor = attr["alarmhighcolor"].get<uint32_t>(m_dwAlarmHighColor);
        m_dwAlarmLowColor = attr["alarmlowcolor"].get<uint32_t>(m_dwAlarmLowColor);

        BIUtils::ReadRectFromAttr(attr, "alarmuv", m_rAlarmUV);

        BIUtils::ReadPosFromAttr(attr, "alarmoffset", m_pntAlarmOffset);
        BIUtils::ReadPosFromAttr(attr, "alarmiconsize", m_pntAlarmIconSize);

        if (attr.hasProperty("alarmuptime")) {
            const float alarm_up_time = attr["alarmuptime"].get<float>();
            if (alarm_up_time > 0) {
                m_fAlarmUpSpeed = 1.f / alarm_up_time;
            }
        }
        if (attr.hasProperty("alarmdowntime")) {
            const float alarm_down_time = attr["alarmdowntime"].get<float>();
            if (alarm_down_time > 0) {
                m_fAlarmDownSpeed = 1.f / alarm_down_time;
            }
        }

        const std::string_view manstatetexturename = attr["manstatetexturename"].get<std::string_view>();
        if (!manstatetexturename.empty())
            m_nManStateTextureID = m_pRS->TextureCreate(manstatetexturename.data());
        m_dwManStateColor = attr["manstatecolor"].get<uint32_t>(m_dwManStateColor);
        BIUtils::ReadRectFromAttr(attr, "manhpuv", m_rManHPUV);
        BIUtils::ReadPosFromAttr(attr, "manhpoffset", m_pntManHPOffset);
        BIUtils::ReadPosFromAttr(attr, "manhpiconsize", m_pntManHPIconSize);
        BIUtils::ReadRectFromAttr(attr, "manenegryuv", m_rManEnergyUV);
        BIUtils::ReadPosFromAttr(attr, "manenegryoffset", m_pntManEnergyOffset);
        BIUtils::ReadPosFromAttr(attr, "manenergyiconsize", m_pntManEnergyIconSize);

        const std::string_view gunchargetexturename = attr["gunchargetexturename"].get<std::string_view>();
        if (!gunchargetexturename.empty())
            m_nGunChargeTextureID = m_pRS->TextureCreate(gunchargetexturename.data());
        m_dwGunChargeColor = attr["gunchargecolor"].get<uint32_t>(m_dwGunChargeColor);
        m_dwGunChargeBackColor = attr["gunchargebackcolor"].get<uint32_t>(m_dwGunChargeBackColor);
        BIUtils::ReadRectFromAttr(attr, "gunchargeuv", m_rGunChargeUV);
        BIUtils::ReadPosFromAttr(attr, "gunchargeoffset", m_pntGunChargeOffset);
        BIUtils::ReadPosFromAttr(attr, "gunchargeiconsize", m_pntGunChargeIconSize);

        if (attr.hasProperty("gunchargeprogress")) {
            const std::string_view charge_progress_str = attr["gunchargeprogress"].get<std::string_view>();
            size_t offset = 0;
            for (size_t i = 0; i < charge_progress_str.size(); ++i)
            {
                if (charge_progress_str[i] == ',')
                {
                    const std::string_view &charge_string = charge_progress_str.substr(offset, i - offset);
                    m_aChargeProgress.push_back(std::stof(charge_string.data()));
                }
            }
        }

        m_dwManFaceColor = attr["manfacecolor"].get<uint32_t>(m_dwManFaceColor);
        BIUtils::ReadPosFromAttr(attr, "manfaceoffset", m_pntManPicOffset);
        BIUtils::ReadPosFromAttr(attr, "manfaceiconsize", m_pntManPicIconSize);

        m_nCommandListVerticalOffset = attr["commandlistverticaloffset"].get<uint32_t>();

        for (n = 0; n < MAX_MAN_QUANTITY; n++)
        {
            std::string_view param_pos = attr[fmt::format("iconoffset{}", n + 1)].get<std::string_view>();
            if (!param_pos.empty())
            {
                sscanf(param_pos.data(), "%f,%f", &m_Man[n].pntPos.x, &m_Man[n].pntPos.y);
            }
        }
    }

    m_bMakeUpdate = true;
}

size_t BIManSign::AddTexture(const char *pcTextureName, long nCols, long nRows) const
{
    if (m_pCommandList)
        return m_pCommandList->AddTexture(pcTextureName, nCols, nRows);
    return -1;
}

void BIManSign::Recollect()
{
    const auto n = CalculateManQuantity();
    UpdateBuffers(n);
}

bool BIManSign::IsActive() const
{
    if (!m_pCommandList)
        return false;
    return m_pCommandList->GetActive();
}

void BIManSign::SetActive(bool bActive)
{
    const auto bSameActive = (IsActive() == bActive);
    if (m_pCommandList)
        m_pCommandList->SetActive(bActive);
    if (bSameActive)
        return;
    m_nCommandMode = BI_COMMODE_COMMAND_SELECT;
    m_bMakeUpdate = true;
}

void BIManSign::MakeControl()
{
    CONTROL_STATE cs;

    core.Controls->GetControlState(BI_COMMANDS_CONFIRM, cs);
    if (cs.state == CST_ACTIVATED)
        ExecuteCommand(BI_MSG_COMMAND_ACTIVATE);

    core.Controls->GetControlState(BI_COMMANDS_LEFTSTEP, cs);
    if (cs.state == CST_ACTIVATED)
        ExecuteCommand(BI_MSG_COMMAND_LEFT);

    core.Controls->GetControlState(BI_COMMANDS_RIGHTSTEP, cs);
    if (cs.state == CST_ACTIVATED)
        ExecuteCommand(BI_MSG_COMMAND_RIGHT);

    core.Controls->GetControlState(BI_COMMANDS_UPSTEP, cs);
    if (cs.state == CST_ACTIVATED)
        ExecuteCommand(BI_MSG_COMMAND_UP);

    core.Controls->GetControlState(BI_COMMANDS_DOWNSTEP, cs);
    if (cs.state == CST_ACTIVATED)
        ExecuteCommand(BI_MSG_COMMAND_DOWN);

    core.Controls->GetControlState(BI_COMMANDS_CANCEL, cs);
    if (cs.state == CST_ACTIVATED)
        ExecuteCommand(BI_MSG_COMMAND_DEACTIVATE);
}

void BIManSign::ExecuteCommand(long command)
{
    switch (command)
    {
    case BI_MSG_COMMAND_ACTIVATE:
        if (m_pCommandList)
        {
            const auto nTmp = m_pCommandList->ExecuteConfirm();
            if (nTmp != -1)
                m_nCommandMode = nTmp;
        }
        if (m_nCommandMode == 0)
            SetActive(false);
        break;

    case BI_MSG_COMMAND_LEFT:
        if (m_pCommandList)
            m_pCommandList->ExecuteLeft();
        break;

    case BI_MSG_COMMAND_RIGHT:
        if (m_pCommandList)
            m_pCommandList->ExecuteRight();
        break;

    case BI_MSG_COMMAND_UP:
        break;

    case BI_MSG_COMMAND_DOWN:
        break;

    case BI_MSG_COMMAND_DEACTIVATE:
        m_nCommandMode = m_pCommandList->ExecuteCancel();
        if (m_nCommandMode == 0)
            SetActive(false);
        else
            UpdateCommandList();
        break;

    default:
        core.Trace("Warning! Unknown executing command: %d", command);
    }
}

void BIManSign::Release()
{
    STORM_DELETE(m_pCommandList);
    TEXTURE_RELEASE(m_pRS, m_nBackTextureID);
    // TEXTURE_RELEASE( m_pRS, m_nShipTextureID );
    TEXTURE_RELEASE(m_pRS, m_nManStateTextureID);
    TEXTURE_RELEASE(m_pRS, m_nGunChargeTextureID);
    VERTEX_BUFFER_RELEASE(m_pRS, m_nVBufID);
    INDEX_BUFFER_RELEASE(m_pRS, m_nIBufID);

    m_nMaxSquareQ = 0;
    m_nSquareQ = 0;
    m_nBackSquareQ = 0;
    m_nManStateSquareQ = 0;
    m_nGunChargeSquareQ = 0;
}

long BIManSign::CalculateManQuantity()
{
    // reset all
    m_nManQuantity = 0;
    for (long n = 0; n < MAX_MAN_QUANTITY; n++)
    {
        m_Man[n].nCharacterIndex = -1;
        m_Man[n].nSlotIndex = -1;
        m_Man[n].fHealth = 0.f;
        m_Man[n].fEnergy = 0.f;
        m_Man[n].bAlarm = false;
        m_Man[n].nShootMax = 4;
        m_Man[n].nShootCurrent = 0;
    }

    if (m_pARoot && m_pARoot->hasProperty("data")) {
        const Attribute& data = (*m_pARoot)["data"];
        if (data.hasProperty("icons")) {
            const Attribute& icons = data["icons"];
            for (long n = 0; n < MAX_MAN_QUANTITY; n++)
            {
                if (icons.hasProperty(fmt::format("id{}", n))) {
                    m_Man[m_nManQuantity].nCharacterIndex = -1;
                    m_Man[m_nManQuantity].nSlotIndex = n;
                    m_nManQuantity++;
                }
            }
        }
    }

    return m_nManQuantity;
}

void BIManSign::UpdateBuffers(long nShipQ)
{
    m_nBackSquareQ = nShipQ;
    m_nManStateSquareQ = nShipQ * 2;
    m_nGunChargeSquareQ = nShipQ * 2;
    const auto nManSquareQ = nShipQ;
    const long nAlarmSquareQ = (m_bIsAlarmOn && (nShipQ > 0)) ? 1 : 0;

    const auto nMaxSquareQ =
        BIUtils::GetMaxFromFourLong(m_nBackSquareQ, m_nManStateSquareQ, m_nGunChargeSquareQ, nManSquareQ);
    if (m_nMaxSquareQ != nMaxSquareQ)
    {
        m_nMaxSquareQ = nMaxSquareQ;
        INDEX_BUFFER_RELEASE(m_pRS, m_nIBufID);
        if (m_nMaxSquareQ > 0)
            m_nIBufID = m_pRS->CreateIndexBuffer(m_nMaxSquareQ * 6 * sizeof(uint16_t));
        FillIndexBuffer();
    }

    if ((m_nBackSquareQ + m_nManStateSquareQ + m_nGunChargeSquareQ + nManSquareQ + nAlarmSquareQ) != m_nSquareQ)
    {
        m_nSquareQ = m_nBackSquareQ + m_nManStateSquareQ + m_nGunChargeSquareQ + nManSquareQ + nAlarmSquareQ;
        const long n = 6 * 4 + 1;
        VERTEX_BUFFER_RELEASE(m_pRS, m_nVBufID);
        m_nVBufID =
            m_pRS->CreateVertexBuffer(BI_COLOR_VERTEX_FORMAT, n * 4 * sizeof(BI_COLOR_VERTEX), D3DUSAGE_WRITEONLY);
    }
    // FillVertexBuffer();
}

void BIManSign::FillIndexBuffer() const
{
    if (m_nIBufID < 0)
        return;
    auto *pI = static_cast<uint16_t *>(m_pRS->LockIndexBuffer(m_nIBufID));
    if (pI)
    {
        for (long n = 0; n < m_nMaxSquareQ; n++)
        {
            pI[n * 6 + 0] = static_cast<uint16_t>(n * 4 + 0);
            pI[n * 6 + 1] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 2] = static_cast<uint16_t>(n * 4 + 2);

            pI[n * 6 + 3] = static_cast<uint16_t>(n * 4 + 2);
            pI[n * 6 + 4] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 5] = static_cast<uint16_t>(n * 4 + 3);
        }
        m_pRS->UnLockIndexBuffer(m_nIBufID);
    }
}

void BIManSign::FillVertexBuffer()
{
    long n;
    if (m_nVBufID < 0)
        return;
    auto *pV = static_cast<BI_COLOR_VERTEX *>(m_pRS->LockVertexBuffer(m_nVBufID));
    if (pV)
    {
        long vn = 0;

        // face
        for (n = 0; n < m_nManQuantity; n++)
            vn += WriteSquareToVBuff(&pV[vn], m_Man[n].rUV, m_dwManFaceColor, m_Man[n].pntPos + m_pntManPicOffset,
                                     static_cast<FPOINT>(m_pntManPicIconSize));

        // back icon
        for (n = 0; n < m_nManQuantity; n++)
            vn += WriteSquareToVBuff(&pV[vn], m_rBackUV, m_dwBackColor, m_Man[n].pntPos + m_pntBackOffset,
                                     static_cast<FPOINT>(m_pntBackIconSize));

        // alarm icon
        if (m_bIsAlarmOn && m_nManQuantity > 0)
        {
            vn += WriteSquareToVBuff(&pV[vn], m_rAlarmUV,
                                     GetColorByFactor(m_dwAlarmLowColor, m_dwAlarmHighColor, m_fAlarmTime),
                                     m_Man[0].pntPos + m_pntAlarmOffset, static_cast<FPOINT>(m_pntAlarmIconSize));
        }

        // state (HP & Energy)
        for (n = 0; n < m_nManQuantity; n++)
        {
            vn += WriteSquareToVBuffWithProgress(&pV[vn], m_rManHPUV, m_dwManStateColor,
                                                 m_Man[n].pntPos + m_pntManHPOffset, static_cast<FPOINT>(m_pntManHPIconSize),
                                                 GetProgressManHP(n), 0.f, 0.f, 0.f);
            vn += WriteSquareToVBuffWithProgress(&pV[vn], m_rManEnergyUV, m_dwManStateColor,
                                                 m_Man[n].pntPos + m_pntManEnergyOffset, static_cast<FPOINT>(m_pntManEnergyIconSize),
                                                 GetProgressManEnergy(n), 0.f, 0.f, 0.f);
        }

        // Gun charge status
        for (n = 0; n < m_nManQuantity; n++)
        {
            vn += WriteSquareToVBuffWithProgress(&pV[vn], m_rGunChargeUV, m_dwGunChargeBackColor,
                                                 m_Man[n].pntPos + m_pntGunChargeOffset, static_cast<FPOINT>(m_pntGunChargeIconSize), 0.f,
                                                 0.f, 0.f, GetProgressGunChargeMax(n));
            vn += WriteSquareToVBuffWithProgress(&pV[vn], m_rGunChargeUV, m_dwGunChargeColor,
                                                 m_Man[n].pntPos + m_pntGunChargeOffset, static_cast<FPOINT>(m_pntGunChargeIconSize), 0.f,
                                                 0.f, 0.f, GetProgressGunCharge(n));
        }

        m_pRS->UnLockVertexBuffer(m_nVBufID);
    }
}

long BIManSign::WriteSquareToVBuff(BI_COLOR_VERTEX *pv, const FRECT &uv, uint32_t color, const BIFPOINT &center,
                                   const FPOINT &size)
{
    if (!pv)
        return 0;

    const auto fLeft = static_cast<float>(center.x - size.x / 2);
    const auto fTop = static_cast<float>(center.y - size.y / 2);
    const auto fRight = fLeft + size.x;
    const auto fBottom = fTop + size.y;

    pv[0].pos.x = fLeft;
    pv[0].pos.y = fTop;
    pv[0].pos.z = 1.f;
    pv[0].w = 0.5f;
    pv[0].col = color;
    pv[0].tu = uv.left;
    pv[0].tv = uv.top;

    pv[1].pos.x = fLeft;
    pv[1].pos.y = fBottom;
    pv[1].pos.z = 1.f;
    pv[1].w = 0.5f;
    pv[1].col = color;
    pv[1].tu = uv.left;
    pv[1].tv = uv.bottom;

    pv[2].pos.x = fRight;
    pv[2].pos.y = fTop;
    pv[2].pos.z = 1.f;
    pv[2].w = 0.5f;
    pv[2].col = color;
    pv[2].tu = uv.right;
    pv[2].tv = uv.top;

    pv[3].pos.x = fRight;
    pv[3].pos.y = fBottom;
    pv[3].pos.z = 1.f;
    pv[3].w = 0.5f;
    pv[3].col = color;
    pv[3].tu = uv.right;
    pv[3].tv = uv.bottom;

    return 4;
}

long BIManSign::WriteSquareToVBuffWithProgress(BI_COLOR_VERTEX *pv, const FRECT &uv, uint32_t color,
                                               const BIFPOINT &center, const FPOINT &size, float fClampUp,
                                               float fClampDown, float fClampLeft, float fClampRight)
{
    if (!pv)
        return 0;

    auto fLeft = static_cast<float>(center.x - size.x / 2);
    auto fTop = static_cast<float>(center.y - size.y / 2);
    auto fRight = fLeft + size.x;
    auto fBottom = fTop + size.y;

    fLeft += size.x * fClampLeft;
    fRight -= size.x * fClampRight;
    fTop += size.y * fClampUp;
    fBottom += size.y * fClampDown;

    const auto fLeftUV = uv.left + (uv.right - uv.left) * fClampLeft;
    const auto fRightUV = uv.right - (uv.right - uv.left) * fClampRight;
    const auto fTopUV = uv.top + (uv.bottom - uv.top) * fClampUp;
    const auto fBottomUV = uv.bottom - (uv.bottom - uv.top) * fClampDown;

    pv[0].pos.x = fLeft;
    pv[0].pos.y = fTop;
    pv[0].pos.z = 1.f;
    pv[0].w = 0.5f;
    pv[0].col = color;
    pv[0].tu = fLeftUV;
    pv[0].tv = fTopUV;

    pv[1].pos.x = fLeft;
    pv[1].pos.y = fBottom;
    pv[1].pos.z = 1.f;
    pv[1].w = 0.5f;
    pv[1].col = color;
    pv[1].tu = fLeftUV;
    pv[1].tv = fBottomUV;

    pv[2].pos.x = fRight;
    pv[2].pos.y = fTop;
    pv[2].pos.z = 1.f;
    pv[2].w = 0.5f;
    pv[2].col = color;
    pv[2].tu = fRightUV;
    pv[2].tv = fTopUV;

    pv[3].pos.x = fRight;
    pv[3].pos.y = fBottom;
    pv[3].pos.z = 1.f;
    pv[3].w = 0.5f;
    pv[3].col = color;
    pv[3].tu = fRightUV;
    pv[3].tv = fBottomUV;

    return 4;
}

void BIManSign::UpdateCommandList() const
{
    if (m_pCommandList)
        m_pCommandList->Update(GetCurrentCommandTopLine(), GetCurrentCommandCharacterIndex(), GetCurrentCommandMode());
}

long BIManSign::GetCurrentCommandTopLine() const
{
    auto n = m_nCurrentManIndex;
    if (n < 0 || n >= m_nManQuantity)
        n = 0;
    return static_cast<long>(m_Man[n].pntPos.y) + m_nCommandListVerticalOffset;
}

long BIManSign::GetCurrentCommandCharacterIndex() const
{
    auto n = m_nCurrentManIndex;
    if (n < 0 || n >= m_nManQuantity)
        n = 0;
    return static_cast<long>(m_Man[n].nCharacterIndex);
}

long BIManSign::GetCurrentCommandMode() const
{
    return m_nCommandMode;
}

float BIManSign::GetProgressManHP(long nIdx) const
{
    if (nIdx < 0 || nIdx >= m_nManQuantity)
        return 1.f;
    return 1.f - m_Man[nIdx].fHealth;
}

float BIManSign::GetProgressManEnergy(long nIdx) const
{
    if (nIdx < 0 || nIdx >= m_nManQuantity)
        return 1.f;
    return 1.f - m_Man[nIdx].fEnergy;
}

float BIManSign::GetProgressGunChargeMax(long nIdx)
{
    if (nIdx < 0 || nIdx >= m_nManQuantity)
        return 1.f;
    return 1.f - GetGunProgressByIndex(m_Man[nIdx].nShootMax);
}

float BIManSign::GetProgressGunCharge(long nIdx)
{
    if (nIdx < 0 || nIdx >= m_nManQuantity)
        return 1.f;
    return 1.f - GetGunProgressByIndex(m_Man[nIdx].nShootCurrent);
}

float BIManSign::GetGunProgressByIndex(long nIdx)
{
    if (nIdx < 0 || nIdx >= m_aChargeProgress.size())
        return 0.f;
    return m_aChargeProgress[nIdx];
}

void BIManSign::CheckDataChange()
{
    if (!(m_pARoot && m_pARoot->hasProperty("data"))) {
        return;
    }
    const Attribute& data = (*m_pARoot)["data"];

    if (BoolACompare(data, "alarm", m_bIsAlarmOn))
        m_bMakeVertexFill = true;

    if (!data.hasProperty("icons")) {
        return;
    }
    const Attribute& icons = data["icons"];

    for (long n = 0; n < MAX_MAN_QUANTITY; n++)
    {
        if (m_Man[n].nSlotIndex < 0)
            continue;
        const std::string attrname = fmt::format("id{}", m_Man[n].nSlotIndex);
        if (!icons.hasProperty(attrname)) {
            continue;
        }
        const Attribute& slot = icons[attrname];
        if (LongACompare(slot, "chrindex", m_Man[n].nCharacterIndex))
            m_bMakeVertexFill = true;
        if (FloatACompare(slot, "health", m_Man[n].fHealth))
            m_bMakeVertexFill = true;
        if (FloatACompare(slot, "energy", m_Man[n].fEnergy))
            m_bMakeVertexFill = true;
        if (LongACompare(slot, "shootMax", m_Man[n].nShootMax))
            m_bMakeVertexFill = true;
        if (LongACompare(slot, "shootCur", m_Man[n].nShootCurrent))
            m_bMakeVertexFill = true;
        if (FRectACompare(slot, "uv", m_Man[n].rUV))
            m_bMakeVertexFill = true;
        if (StringACompare(slot, "texture", m_Man[n].sTexture))
        {
            TEXTURE_RELEASE(m_pRS, m_Man[n].nTexture);
            m_Man[n].nTexture = m_pRS->TextureCreate(m_Man[n].sTexture.c_str());
            m_bMakeVertexFill = true;
        }
    }

    if (m_bIsAlarmOn)
    {
        m_bMakeVertexFill = true;
        if (m_bAlarmUpDirection)
        {
            m_fAlarmTime += core.GetDeltaTime() * .001f * m_fAlarmUpSpeed;
            if (m_fAlarmTime >= 1.f)
            {
                m_fAlarmTime = 1.f;
                m_bAlarmUpDirection = false;
            }
        }
        else
        {
            m_fAlarmTime -= core.GetDeltaTime() * .001f * m_fAlarmDownSpeed;
            if (m_fAlarmTime <= 0.f)
            {
                m_fAlarmTime = 0.f;
                m_bAlarmUpDirection = true;
            }
        }
    }
}
