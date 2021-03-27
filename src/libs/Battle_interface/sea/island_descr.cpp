#include "island_descr.h"

#include "core.h"

#include "../shared/battle_interface/msg_control.h"
#include "model.h"

ISLAND_DESCRIBER::ISLAND_DESCRIBER() : m_lastFindIdx(0), m_nFindType(0)
{
    m_bYesIsland = false;
    m_pIslandAttributes = nullptr;
}

ISLAND_DESCRIBER::~ISLAND_DESCRIBER()
{
    ReleaseAll();
}

void ISLAND_DESCRIBER::ReleaseAll()
{
    m_pIslandAttributes = nullptr;
    m_bYesIsland = false;
}

void ISLAND_DESCRIBER::SetIsland(Attribute *pAIsland)
{
    if (pAIsland == nullptr)
        return;
    m_bYesIsland = true;
    m_pIslandAttributes = pAIsland;
    // create a list of locators
    if (const Attribute& reload = pAIsland->getProperty("reload"); !reload.empty()) {
        std::transform(reload.begin(), reload.end(), std::back_inserter(m_pLocators), [](const Attribute &attr) {
            LOCATOR_DESCR descr{};

            descr.pchr_note = attr["labelLoc"].get<std::string_view>();
            descr.x = attr["x"].get<float>();
            descr.z = attr["z"].get<float>();
            descr.r = attr["radius"].get<float>();
            descr.locatorType = ISLAND_LOCATOR_LAND;
            descr.relation = BI_RELATION_NEUTRAL;
            descr.picIdx = -1;
            descr.texIdx = -1;
            descr.characterIndex = -1;
            descr.bDiseased = false;
            auto *pvdat = core.Event("evntGetLandData", "a", &attr);
            if (pvdat)
            {
                long lTmp;
                if (pvdat->Get(lTmp, 0))
                    if (lTmp == 0)
                        descr.locatorType = ISLAND_LOCATOR_LAND;
                    else if (lTmp == 1)
                        descr.locatorType = ISLAND_LOCATOR_FORT;
                    else
                        descr.locatorType = ISLAND_LOCATOR_TOWN;
                if (pvdat->Get(lTmp, 1))
                    descr.relation = lTmp;
                if (pvdat->Get(lTmp, 2))
                    descr.texIdx = lTmp;
                if (pvdat->Get(lTmp, 3))
                    descr.picIdx = lTmp;
                if (pvdat->Get(lTmp, 4))
                    descr.selPicIdx = lTmp;
                if (pvdat->Get(lTmp, 5))
                    descr.bDiseased = (lTmp != 0);
                descr.pchr_note = attr["labelLoc"].get<std::string_view>().data();
            }
            return descr;
        });
    }
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetFirstLocator()
{
    m_nFindType = ISLAND_FIND_LOCATOR;
    m_lastFindIdx = 0;
    if (m_pLocators.empty())
        return nullptr;
    return FindLocator(m_pLocators.data(), m_pLocators.size());
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetFirstLand()
{
    m_nFindType = ISLAND_FIND_LAND;
    m_lastFindIdx = 0;
    if (m_pLocators.empty())
        return nullptr;
    return FindLocator(m_pLocators.data(), m_pLocators.size());
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetFirstFort()
{
    m_nFindType = ISLAND_FIND_FORT;
    m_lastFindIdx = 0;
    if (m_pLocators.empty())
        return nullptr;
    return FindLocator(m_pLocators.data(), m_pLocators.size());
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetFirstFriendFort()
{
    m_nFindType = ISLAND_FIND_FRIENDFORT;
    m_lastFindIdx = 0;
    if (m_pLocators.empty())
        return nullptr;
    return FindLocator(m_pLocators.data(), m_pLocators.size());
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetFirstNeutralFort()
{
    m_nFindType = ISLAND_FIND_NEUTRALFORT;
    m_lastFindIdx = 0;
    if (m_pLocators.empty())
        return nullptr;
    return FindLocator(m_pLocators.data(), m_pLocators.size());
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetFirstEnemyFort()
{
    m_nFindType = ISLAND_FIND_ENEMYFORT;
    m_lastFindIdx = 0;
    if (m_pLocators.empty())
        return nullptr;
    return FindLocator(m_pLocators.data(), m_pLocators.size());
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetFirstBrokenFort()
{
    m_nFindType = ISLAND_FIND_BROKENFORT;
    m_lastFindIdx = 0;
    if (m_pLocators.empty())
        return nullptr;
    return FindLocator(m_pLocators.data(), m_pLocators.size());
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::GetNext()
{
    if (m_lastFindIdx < 0 || m_lastFindIdx >= m_pLocators.size())
        return nullptr;
    assert(!m_pLocators.empty());
    return FindLocator(&m_pLocators[m_lastFindIdx], m_pLocators.size() - m_lastFindIdx);
}

ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::FindLocator(LOCATOR_DESCR *p, size_t nMax)
{
    assert(p != NULL);
    if (nMax < 1)
        return nullptr;
    int i;

    switch (m_nFindType)
    {
    case ISLAND_FIND_LOCATOR:
        m_lastFindIdx++;
        return &p[0];
    case ISLAND_FIND_LAND:
        for (i = 0; i < nMax; i++)
            if (p[i].locatorType == ISLAND_LOCATOR_LAND)
            {
                m_lastFindIdx += i + 1;
                return &p[i];
            }
        break;
    default:
        for (i = 0; i < nMax; i++)
            if (p[i].locatorType == ISLAND_LOCATOR_FORT)
                switch (m_nFindType)
                {
                case ISLAND_FIND_FORT:
                    m_lastFindIdx += i + 1;
                    return &p[i];
                case ISLAND_FIND_FRIENDFORT:
                    if (p[i].relation == BI_RELATION_FRIEND)
                    {
                        m_lastFindIdx += i + 1;
                        return &p[i];
                    }
                    break;
                case ISLAND_FIND_NEUTRALFORT:
                    if (p[i].relation == BI_RELATION_NEUTRAL)
                    {
                        m_lastFindIdx += i + 1;
                        return &p[i];
                    }
                    break;
                case ISLAND_FIND_ENEMYFORT:
                    if (p[i].relation == BI_RELATION_ENEMY)
                    {
                        m_lastFindIdx += i + 1;
                        return &p[i];
                    }
                    break;
                case ISLAND_FIND_BROKENFORT:
                    m_lastFindIdx += i + 1;
                    return &p[i];
                }
        break;
    }
    // found nothing
    m_lastFindIdx += nMax;
    return nullptr;
}

const ISLAND_DESCRIBER::LOCATOR_DESCR *ISLAND_DESCRIBER::FindLocatorByName(char *name) const
{
    if (name == nullptr)
        return nullptr;
    for (auto i = 0; i < m_pLocators.size(); i++)
    {
        if (m_pLocators[i].pA == nullptr)
            continue;
        const std::string_view curName = m_pLocators[i].pA->getProperty("name").get<std::string_view>();
        if (curName != nullptr && _stricmp(name, curName.data()) == 0)
            return &m_pLocators[i];
    }
    return nullptr;
}

void ISLAND_DESCRIBER::Refresh() const
{
    if (m_pLocators.empty())
        return;
    for (auto i = 0; i < m_pLocators.size(); i++)
    {
        if (m_pLocators[i].locatorType == ISLAND_LOCATOR_FORT)
        {
            m_pLocators[i].relation =
                GetVDATALong(core.Event(BI_EVENT_GET_FORT_RELATION, "a", m_pLocators[i].pA), BI_RELATION_NEUTRAL);
        }
    }
}
