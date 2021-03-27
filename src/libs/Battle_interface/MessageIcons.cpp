#include "MessageIcons.h"
#include "Utils.h"

MESSAGE_ICONS::MESSAGE_ICONS()
{
    rs = nullptr;

    m_bShowMsgIcon = false;
    m_idMsgIconsTexture = -1;
    m_vMsgIconBufID = -1;
    m_iMsgIconBufID = -1;

    PZERO(m_pIconsAttr, sizeof(m_pIconsAttr));
}

MESSAGE_ICONS::~MESSAGE_ICONS()
{
    VERTEX_BUFFER_RELEASE(rs, m_vMsgIconBufID);
    INDEX_BUFFER_RELEASE(rs, m_iMsgIconBufID);
    TEXTURE_RELEASE(rs, m_idMsgIconsTexture);
}

void MESSAGE_ICONS::Update(uint32_t deltaTime)
{
    int i, j, n;
    if (m_bShowMsgIcon)
    {
        auto *pVBuf = static_cast<BI_COLOR_VERTEX *>(rs->LockVertexBuffer(m_vMsgIconBufID));
        if (!pVBuf)
            return;

        // Calculating the blinking color
        auto blindColor = m_dwNormalColor;
        if (m_bBlindDirectUp)
            blindColor =
                BIUtils::GetIntervalColor(m_dwHighBlindColor, m_dwLowBlindColor, m_fCurBlindTime / m_fBlindTimeUp);
        else
            blindColor =
                BIUtils::GetIntervalColor(m_dwLowBlindColor, m_dwHighBlindColor, m_fCurBlindTime / m_fBlindTimeDown);

        auto fFallDist = m_fFallSpeed * deltaTime * .001f;

        m_nMsgIconQnt = 0;

        for (i = 0; i < MESSAGE_ICONS_COLUMN_QUANTITY; i++)
        {
            // act on icons
            for (j = 0; j < m_pMsgColumns[i].rowQ; j++)
            {
                // check if icons are removed
                if (m_pMsgColumns[i].pRow[j].bDoBlend)
                {
                    if ((m_pMsgColumns[i].pRow[j].curTime -= deltaTime * .001f) <= 0.f)
                    {
                        m_pMsgColumns[i].rowQ--;
                        if (j < m_pMsgColumns[i].rowQ)
                            memcpy(&m_pMsgColumns[i].pRow[j], &m_pMsgColumns[i].pRow[j + 1],
                                   sizeof(COLUMN_DESCRIBER::ROW_DESCR) * (m_pMsgColumns[i].rowQ - j));
                        j--;
                        continue;
                    }
                    m_pMsgColumns[i].pRow[j].color =
                        BIUtils::GetIntervalColor(ARGB(0, 128, 128, 128), ARGB(255, 128, 128, 128),
                                                  m_pMsgColumns[i].pRow[j].curTime / m_fBlendTime);
                }
                // move icons down
                auto fBottonLimit = static_cast<float>(m_nBottomY);
                if (j > 0)
                    fBottonLimit = m_pMsgColumns[i].pRow[j - 1].bottom - m_nMsgIconHeight - m_nMsgIconDist;
                if (m_pMsgColumns[i].pRow[j].bottom < fBottonLimit)
                {
                    m_pMsgColumns[i].pRow[j].bottom += fFallDist;
                    if (m_pMsgColumns[i].pRow[j].bottom > fBottonLimit)
                        m_pMsgColumns[i].pRow[j].bottom = fBottonLimit;
                }
            }

            // mark icons
            if (m_pIconsAttr[i])
            {
                // mark all icons as removable
                for (n = 0; n < m_pMsgColumns[i].rowQ; n++)
                {
                    if (!m_pMsgColumns[i].pRow[n].bDoBlend)
                    {
                        m_pMsgColumns[i].pRow[n].bDoBlend = true;
                        m_pMsgColumns[i].pRow[n].curTime = m_fBlendTime;
                    }
                }
                // loop through icons
                for (const Attribute& icon_attr : *m_pIconsAttr[i]) {
                    int picNum = icon_attr["pic"].get<int>();
                    int k;
                    for (k = 0; k < m_pMsgColumns[i].rowQ; k++)
                        if (picNum == m_pMsgColumns[i].pRow[k].pic)
                        {
                            m_pMsgColumns[i].pRow[k].bDoBlend = false;
                            m_pMsgColumns[i].pRow[k].color = ARGB(255, 128, 128, 128);
                            break;
                        }
                    if (k < m_pMsgColumns[i].rowQ)
                        continue;
                    m_pMsgColumns[i].pRow[m_pMsgColumns[i].rowQ].pic = picNum;
                    m_pMsgColumns[i].pRow[m_pMsgColumns[i].rowQ].bottom =
                        static_cast<float>(m_nBottomY) - m_pMsgColumns[i].rowQ * (m_nMsgIconHeight + m_nMsgIconDist);
                    m_pMsgColumns[i].pRow[k].color = ARGB(255, 128, 128, 128);
                    m_pMsgColumns[i].pRow[k].bDoBlend = false;
                    m_pMsgColumns[i].pRow[k].bDoBlind = false;
                    m_pMsgColumns[i].rowQ++;
                }
            }

            // output the icons in the buffer
            FRECT frectTmp;
            for (j = 0; j < m_pMsgColumns[i].rowQ; j++)
            {
                frectTmp.right = (frectTmp.left = static_cast<float>(m_pMsgColumns[i].leftPos)) + m_nMsgIconWidth;
                frectTmp.top = (frectTmp.bottom = m_pMsgColumns[i].pRow[j].bottom) - m_nMsgIconHeight;
                SetRectanglePos(&pVBuf[m_nMsgIconQnt * 4], frectTmp);
                SetRectangleTexture(&pVBuf[m_nMsgIconQnt * 4],
                                    GetTexRectFromPosition(frectTmp, m_pMsgColumns[i].pRow[j].pic, m_nHorzTextureSize,
                                                           m_nVertTextureSize));
                for (n = 0; n < 4; n++)
                    pVBuf[m_nMsgIconQnt * 4 + n].col = m_pMsgColumns[i].pRow[j].color;
                m_nMsgIconQnt++;
            }
        }

        rs->UnLockVertexBuffer(m_vMsgIconBufID);
    }
}

void MESSAGE_ICONS::Draw() const
{
    // show messages
    if (m_bShowMsgIcon && m_nMsgIconQnt > 0 && m_idMsgIconsTexture >= 0)
    {
        rs->TextureSet(0, m_idMsgIconsTexture);

        rs->DrawBuffer(m_vMsgIconBufID, sizeof(BI_COLOR_VERTEX), m_iMsgIconBufID, 0, m_nMsgIconQnt * 4, 0,
                       m_nMsgIconQnt * 2, "battle_msg");
    }
}

void MESSAGE_ICONS::StartData(Attribute *pAData[MESSAGE_ICONS_COLUMN_QUANTITY],
                              long pLeft[MESSAGE_ICONS_COLUMN_QUANTITY])
{
    if (pAData == nullptr)
        return;

    for (int i = 0; i < MESSAGE_ICONS_COLUMN_QUANTITY; i++)
    {
        m_pMsgColumns[i].rowQ = 0;
        m_pMsgColumns[i].leftPos = pLeft[i];
        m_pIconsAttr[i] = pAData[i];
        if (!pAData[i] || !m_pMsgColumns[i].pRow)
            continue;

//        if (q > m_nMsgIconRowQnt)
//            q = m_nMsgIconRowQnt;
        int j = 0;
        for (const Attribute& attr : *pAData[i]) {
            if (!attr.empty()) {
                attr.get_to(m_pMsgColumns[i].pRow[j].pic, -1l);
                m_pMsgColumns[i].pRow[j].bottom = static_cast<float>(m_nBottomY - (m_nMsgIconHeight + m_nMsgIconDist) * j);
            }
            ++j;
        }
    }
}

bool MESSAGE_ICONS::InitData(entid_t host_eid, VDX9RENDER *_rs, Attribute *pARoot)
{
    m_idHost = host_eid;
    rs = _rs;
    if (!_rs || !pARoot)
        return false;

    m_idMsgIconsTexture = -1;

    const Attribute& attr = *pARoot;
    attr["IconWidth"].get_to(m_nMsgIconWidth, 64l);
    attr["IconHeight"].get_to(m_nMsgIconHeight, 16l);
    attr["IconDist"].get_to(m_nMsgIconDist, 2l);
    attr["IconBottom"].get_to(m_nBottomY, 360l);
    attr["IconMaxQuantity"].get_to(m_nMsgIconRowQnt, 4l);
    attr["BlendTime"].get_to(m_fBlendTime, 3.f);
    attr["FallSpeed"].get_to(m_fFallSpeed, 1.f);
    attr["argbHighBlind"].get_to(m_dwHighBlindColor, 0xFF808080);
    attr["argbLowBlind"].get_to(m_dwLowBlindColor, 0xFF404040);
    attr["BlindUpTime"].get_to(m_fBlindTimeUp, .5f);
    attr["BlindDownTime"].get_to(m_fBlindTimeDown, 1.f);
    m_idMsgIconsTexture = BIUtils::GetTextureFromAttr(rs, attr, "texture");
    attr["TexHSize"].get_to(m_nHorzTextureSize, 1l);
    attr["TexVSize"].get_to(m_nVertTextureSize, 1l);

    m_vMsgIconBufID = rs->CreateVertexBuffer(
        BI_COLOR_VERTEX_FORMAT, m_nMsgIconRowQnt * MESSAGE_ICONS_COLUMN_QUANTITY * 4 * sizeof(BI_COLOR_VERTEX),
        D3DUSAGE_WRITEONLY);
    m_iMsgIconBufID = rs->CreateIndexBuffer(m_nMsgIconRowQnt * MESSAGE_ICONS_COLUMN_QUANTITY * 6 * 2);
    m_nMsgIconQnt = 0;
    m_bShowMsgIcon = false;

    int i;
    for (i = 0; i < MESSAGE_ICONS_COLUMN_QUANTITY; i++)
    {
        m_pMsgColumns[i].rowQ = 0;
        m_pMsgColumns[i].startVertex = i * m_nMsgIconRowQnt;
        m_pMsgColumns[i].pRow = new COLUMN_DESCRIBER::ROW_DESCR[m_nMsgIconRowQnt];
        if (m_pMsgColumns[i].pRow == nullptr)
        {
            throw std::exception("allocate memory error");
        }
    }

    auto *pVBuf = static_cast<BI_COLOR_VERTEX *>(rs->LockVertexBuffer(m_vMsgIconBufID));
    if (pVBuf != nullptr)
    {
        for (i = 0; i < m_nMsgIconRowQnt * MESSAGE_ICONS_COLUMN_QUANTITY * 4; i++)
        {
            pVBuf[i].w = 0.5f;
            pVBuf[i].pos.z = 1.f;
            pVBuf[i].col = ARGB(255, 128, 128, 128);
        }
        rs->UnLockVertexBuffer(m_vMsgIconBufID);
    }

    auto *pIBuf = static_cast<uint16_t *>(rs->LockIndexBuffer(m_iMsgIconBufID));
    if (pIBuf != nullptr)
    {
        for (i = 0; i < m_nMsgIconRowQnt * MESSAGE_ICONS_COLUMN_QUANTITY; i++)
        {
            pIBuf[i * 6 + 0] = i * 4 + 0;
            pIBuf[i * 6 + 1] = i * 4 + 1;
            pIBuf[i * 6 + 2] = i * 4 + 2;
            pIBuf[i * 6 + 3] = i * 4 + 2;
            pIBuf[i * 6 + 4] = i * 4 + 1;
            pIBuf[i * 6 + 5] = i * 4 + 3;
        }
        rs->UnLockIndexBuffer(m_iMsgIconBufID);
    }

    return true;
}
