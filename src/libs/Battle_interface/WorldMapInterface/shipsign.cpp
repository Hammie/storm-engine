#include "shipsign.h"
#include "../Utils.h"

WMShipIcon::WMShipIcon(entid_t BIEntityID, VDX9RENDER *pRS) : BISignIcon(BIEntityID, pRS)
{
}

WMShipIcon::~WMShipIcon()
{
}

void WMShipIcon::ExecuteCommand(CommandType command)
{
}

long WMShipIcon::CalculateSignQuantity()
{
    long n;
    char param[256];

    if (m_pAData)
    {
        for (n = 0; n < MAX_SIGN_QUANTITY; n++)
        {
            sprintf_s(param, sizeof(param), "sign%d", n + 1);
            const Attribute& param_attr = m_pAData->getProperty(param);
            if (!param_attr.empty())
            {
                m_Sign[n].bUse = true;
                param_attr["leftprogress"].get_to(m_Sign[n].fLeftState, 0.f);
                param_attr["rightprogress"].get_to(m_Sign[n].fRightState, 0.f);
                param_attr["starprogress"].get_to(m_Sign[n].fStarProgress, 0.f);
                FULLRECT(m_Sign[n].rFaceUV);
                param_attr["faceuv"].get_to(m_Sign[n].rFaceUV);
                const Attribute& attr_text = param_attr["text"];
                if (!attr_text.empty())
                    m_Sign[n].sText = attr_text.get<std::string_view>().data();
            }
            else
            {
                m_Sign[n].bUse = false;
            }
        }
    }

    return BISignIcon::CalculateSignQuantity();
}

void WMShipIcon::UpdateChildrens()
{
}
