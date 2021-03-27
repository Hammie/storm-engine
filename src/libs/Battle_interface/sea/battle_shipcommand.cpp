#include "battle_shipcommand.h"

#include "core.h"

#include "../shared/battle_interface/msg_control.h"
#include "island_descr.h"
#include "ships_list.h"

BIShipCommandList::BIShipCommandList(entid_t eid, Attribute &pA, VDX9RENDER *rs) : BICommandList(eid, pA, rs)
{
    Init();
}

BIShipCommandList::~BIShipCommandList()
{
    Release();
}

void BIShipCommandList::FillIcons()
{
    long nIconsQuantity = 0;

    if (m_nCurrentCommandMode & BI_COMMODE_MY_SHIP_SELECT)
        nIconsQuantity +=
            ShipAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, true, false, false, false);
    if (m_nCurrentCommandMode & BI_COMMODE_NEUTRAL_SHIP_SELECT)
        nIconsQuantity +=
            ShipAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, false, false, true, false);
    if (m_nCurrentCommandMode & BI_COMMODE_FRIEND_SHIP_SELECT)
        nIconsQuantity +=
            ShipAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, false, false, false, true);
    if (m_nCurrentCommandMode & BI_COMMODE_ENEMY_SHIP_SELECT)
        nIconsQuantity +=
            ShipAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, false, true, false, false);

    if (m_nCurrentCommandMode & BI_COMMODE_FRIEND_FORT_SELECT)
        nIconsQuantity += FortAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, true, false, false);
    if (m_nCurrentCommandMode & BI_COMMODE_NEUTRAL_FORT_SELECT)
        nIconsQuantity += FortAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, false, true, false);
    if (m_nCurrentCommandMode & BI_COMMODE_ENEMY_FORT_SELECT)
        nIconsQuantity += FortAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, false, false, true);
    if (m_nCurrentCommandMode & BI_COMMODE_LAND_SELECT)
        nIconsQuantity += LandAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0);

    if (m_nCurrentCommandMode & BI_COMMODE_COMMAND_SELECT)
        nIconsQuantity += CommandAdding();

    if (m_nCurrentCommandMode & BI_COMMODE_CANNON_CHARGE)
        nIconsQuantity += ChargeAdding();

    if (m_nCurrentCommandMode & BI_COMMODE_USER_ICONS)
        nIconsQuantity += UserIconsAdding();

    if (m_nCurrentCommandMode & BI_COMMODE_ENEMY_TOWN)
        nIconsQuantity +=
            TownAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, true, true, true, false, false);

    if (m_nCurrentCommandMode & BI_COMMODE_DISEASED_TOWN)
        nIconsQuantity +=
            TownAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, true, false, true, true, true);

    if (m_nCurrentCommandMode & BI_COMMODE_NOTDISEASED_TOWN)
        nIconsQuantity +=
            TownAdding((m_nCurrentCommandMode & BI_COMMODE_ALLLOCATOR_SELECT) != 0, false, true, true, true, true);

    nIconsQuantity += AddCancelIcon();
}

void BIShipCommandList::Init()
{
    BICommandList::Init();

    m_nChargeTextureNum = -1;
    m_nCommandTextureNum = -1;
    m_nIconShowMaxQuantity = 8;

    if (m_pARoot.hasProperty("CommandTexture")) {
        const Attribute& command_textures = m_pARoot["CommandTexture"];

        m_nChargeTextureNum = command_textures["ChargeTexNum"].get<uint32_t>(m_nChargeTextureNum);
        m_nCommandTextureNum = command_textures["CommandTexNum"].get<uint32_t>(m_nCommandTextureNum);
    }

    if (m_pARoot.hasProperty("CommandList")) {
        const Attribute& command_list = m_pARoot["CommandList"];

        m_nIconShowMaxQuantity = command_list["CommandMaxIconQuantity"].get<uint32_t>(m_nIconShowMaxQuantity);
    }
}

void BIShipCommandList::Release()
{
}

long BIShipCommandList::ShipAdding(bool allLabel, bool bMyShip, bool bEnemy, bool bNeutral, bool bFriend)
{
    long n;
    long retVal = 0;

    // list of ships
    auto *sd = g_ShipList.GetShipRoot();
    if (sd == nullptr)
        return 0;

    // distance for removing the ship from the list
    auto *pA = GetCurrentCommandAttribute();
    auto sqrRadius = -1.f;
    if (pA)
        sqrRadius = pA->getProperty("EffectRadius").get<float>(sqrRadius);
    if (sqrRadius < 0.f)
        allLabel = true;
    sqrRadius *= sqrRadius;

    auto *selShip = g_ShipList.FindShip(m_nCurrentCommandCharacterIndex);
    float selX, selZ;
    if (selShip == nullptr)
    {
        allLabel = true;
        selX = 0;
        selZ = 0;
    }
    else
    {
        selX = selShip->pShip->GetPos().x;
        selZ = selShip->pShip->GetPos().z;
    }

    auto *main_sd = g_ShipList.GetMainCharacterShip();
    if (bMyShip && main_sd != selShip)
    {
        n = AddToIconList(main_sd->textureNum, main_sd->pictureNum, main_sd->selectPictureNum, -1,
                          main_sd->characterIndex, nullptr, -1, nullptr,
                          main_sd->pAttr->getProperty("name").get<std::string_view>().data());
        if (n > 0)
        {
            retVal += n;
            AddFlagPictureToIcon(main_sd->characterIndex);
        }
    }

    for (; sd != nullptr; sd = sd->next)
    {
        if (sd != selShip && sd != main_sd)
        {
            if (bMyShip && sd->isMyShip || !bMyShip && bEnemy && !sd->isMyShip && sd->relation == BI_RELATION_ENEMY ||
                !bMyShip && bFriend && !sd->isMyShip && sd->relation == BI_RELATION_FRIEND ||
                !bMyShip && bNeutral && !sd->isMyShip && sd->relation == BI_RELATION_NEUTRAL)
            {
                // fits the distance?
                if (!allLabel)
                {
                    const auto cv = sd->pShip->GetPos();
                    if (SQR(selX - cv.x) + SQR(selZ - cv.z) > sqrRadius)
                        continue;
                }
                // check for validity of the ship from the script
                if (!m_sCurrentCommandName.empty())
                {
                    auto *pvdat =
                        core.Event("evntCheckEnableShip", "sl", m_sCurrentCommandName.c_str(), sd->characterIndex);
                    if (pvdat != nullptr && pvdat->GetLong() == 0)
                        continue;
                }
                n = AddToIconList(sd->textureNum, sd->pictureNum, sd->selectPictureNum, -1, sd->characterIndex, nullptr,
                                  -1, nullptr, sd->pAttr->getProperty("name").get<std::string_view>().data());
                if (n > 0)
                {
                    retVal += n;
                    AddFlagPictureToIcon(sd->characterIndex);
                }
            }
        }
    }

    return retVal;
}

long BIShipCommandList::FortAdding(bool allLabel, bool bFriend, bool bNeutral, bool bEnemy)
{
    auto *pL = g_IslandDescr.GetFirstFort();
    if (pL == nullptr)
        return 0;
    long retVal = 0;

    auto *pA = GetCurrentCommandAttribute();
    auto sqrRadius = pL->r;
    if (pA)
        sqrRadius = pA->getProperty("EffectRadius").get<float>(sqrRadius);
    if (sqrRadius < 0.f)
        allLabel = true;
    sqrRadius *= sqrRadius;

    // Determine the coordinates of the ship receiving the commands
    const auto selectedCharacter = m_nCurrentCommandCharacterIndex;
    auto *selShip = g_ShipList.FindShip(selectedCharacter);
    float selX, selZ;
    if (selShip == nullptr)
    {
        allLabel = true;
        selX = 0;
        selZ = 0;
    }
    else
    {
        selX = selShip->pShip->GetPos().x;
        selZ = selShip->pShip->GetPos().z;
    }

    do
    {
        if (bFriend && pL->relation == BI_RELATION_FRIEND || bNeutral && pL->relation == BI_RELATION_NEUTRAL ||
            bEnemy && pL->relation == BI_RELATION_ENEMY)
        {
            if (!allLabel)
                if (SQR(pL->x - selX) + SQR(pL->z - selZ) > sqrRadius)
                    continue;
            auto *pvdat = core.Event("evntCheckEnableLocator", "sa", m_sCurrentCommandName.c_str(), pL->pA);
            if (pvdat != nullptr && pvdat->GetLong() == 0)
                continue;
            std::string_view pLocName;
            if (pL->pA != nullptr)
                pLocName = pL->pA->getProperty("name").get<std::string_view>();
            retVal += AddToIconList(pL->texIdx, pL->picIdx, pL->selPicIdx, -1, pL->characterIndex, nullptr, -1,
                                    pLocName.data(), pL->pchr_note.data());
        }
    } while ((pL = g_IslandDescr.GetNext()) != nullptr);

    return retVal;

    return 0;
}

long BIShipCommandList::LandAdding(bool allLabel)
{
    auto *pL = g_IslandDescr.GetFirstLand();
    if (pL == nullptr)
        return 0;
    long retVal = 0;

    auto *pA = GetCurrentCommandAttribute();
    auto sqrRadius = pL->r;
    if (pA)
        sqrRadius = pA->getProperty("EffectRadius").get<float>(sqrRadius);
    if (sqrRadius < 0.f)
        allLabel = true;
    sqrRadius *= sqrRadius;

    // Determine the coordinates of the ship receiving the commands
    const auto selectedCharacter = m_nCurrentCommandCharacterIndex;
    auto *selShip = g_ShipList.FindShip(selectedCharacter);
    float selX, selZ;
    if (selShip == nullptr)
    {
        allLabel = true;
        selX = 0;
        selZ = 0;
    }
    else
    {
        selX = selShip->pShip->GetPos().x;
        selZ = selShip->pShip->GetPos().z;
    }

    do
    {
        if (!allLabel)
            if (SQR(pL->x - selX) + SQR(pL->z - selZ) > sqrRadius)
                continue;
        auto *pvdat = core.Event("evntCheckEnableLocator", "sa", m_sCurrentCommandName.c_str(), pL->pA);
        if (pvdat != nullptr && pvdat->GetLong() == 0)
            continue;
        std::string_view pLocName;
        if (pL->pA != nullptr)
            pLocName = pL->pA->getProperty("name").get<std::string_view>();
        retVal += AddToIconList(pL->texIdx, pL->picIdx, pL->selPicIdx, -1, pL->characterIndex, nullptr, -1, pLocName.data(),
                                pL->pchr_note.data());
    } while ((pL = g_IslandDescr.GetNext()) != nullptr);
    return retVal;
}

long BIShipCommandList::CommandAdding()
{
    core.Event("BI_SetPossibleCommands", "l", m_nCurrentCommandCharacterIndex);
    long retVal = 0;

    if (!m_pARoot.hasProperty("Commands")) {
        return 0;
    }
    const Attribute& commands = m_pARoot["Commands"];

    for (const Attribute& command : commands) {
        if (command["enable"].get<bool>()) {
            const long pictureNum = command["picNum"].get<uint32_t>(0);
            const long selPictureNum = command["selPicNum"].get<uint32_t>(0);
            const long cooldownPictureNum = command["cooldownPicNum"].get<uint32_t>(-1);
            const long texNum = command["texNum"].get<uint32_t>(m_nCommandTextureNum);
            const std::string_view eventName = command["event"].get<std::string_view>();
            retVal += AddToIconList(texNum, pictureNum, selPictureNum, cooldownPictureNum, -1, eventName.data(), -1, nullptr, command["note"].get<std::string_view>().data());
        }
    }

    return retVal;
}

long BIShipCommandList::ChargeAdding()
{
    // Determine the amount of each charge on board
    auto *tmpDat = core.Event("BI_GetChargeQuantity", "l", m_nCurrentCommandCharacterIndex);
    if (tmpDat == nullptr)
        return 0;
    long lIdx = 0; // number of charge types
    tmpDat->Get(lIdx, 0);
    if (lIdx <= 0)
        return 0;

    const Attribute& charge = m_pARoot["charge"];
    long retVal = 0;
    m_aChargeQuantity.clear();
    for (auto i = 0; i < lIdx; i++)
    {
        m_aChargeQuantity.push_back(0);
        tmpDat->Get(m_aChargeQuantity[i], i + 1);

        std::string param = fmt::format("charge{}", i + 1);
        if (charge.hasProperty(param)) {
            const Attribute& param_attr = charge[param];
            const long nNormalPicIndex = param_attr["picNum"].get<uint32_t>(-1);
            const long nSelectPicIndex = param_attr["selPicNum"].get<uint32_t>(-1);
            retVal += AddToIconList(m_nChargeTextureNum, nNormalPicIndex, nSelectPicIndex, -1, -1, nullptr, i + 1, nullptr,
                                    nullptr);
        }
    }
    return retVal;
}

long BIShipCommandList::UserIconsAdding()
{
    long retVal = 0;

    if (!m_pARoot.hasProperty("UserIcons")) {
        return 0;
    }
    const Attribute& user_icons = m_pARoot["UserIcons"];

    size_t i = 0;
    for (const Attribute& icon : user_icons) {
        if (icon["enable"].get<bool>()) {
            const long pictureNum = icon["pic"].get<uint32_t>(0);
            const long selPictureNum = icon["selpic"].get<uint32_t>(0);
            const long textureNum = icon["tex"].get<uint32_t>(-1);
            const std::string_view name = icon["name"].get<std::string_view>();
            const std::string_view note = icon["note"].get<std::string_view>();
            retVal += AddToIconList(textureNum, pictureNum, selPictureNum, -1, -1, nullptr, i + 1, name.data(),
                                    note.data());
        }
        ++i;
    }

    return retVal;
}

long BIShipCommandList::AbilityAdding()
{
    core.Event("evntSetUsingAbility", "l", m_nCurrentCommandCharacterIndex);
    long retVal = 0;

    if (!m_pARoot.hasProperty("AbilityIcons")) {
        return 0;
    }
    const Attribute& ability_icons = m_pARoot["AbilityIcons"];

    for (const Attribute& icon : ability_icons) {
        if (icon["enable"].get<bool>()) {
            const long pictureNum = icon["picNum"].get<uint32_t>(0);
            const long selPictureNum = icon["selPicNum"].get<uint32_t>(0);
            const long textureNum = icon["texNum"].get<uint32_t>(-1);
            const long cooldownPictureNum = icon["cooldownPicNum"].get<uint32_t>(-1);
            const std::string_view event_name = icon["event"].get<std::string_view>();
            const std::string_view note = icon["note"].get<std::string_view>();
            retVal += AddToIconList(textureNum, pictureNum, selPictureNum, cooldownPictureNum, -1, event_name.data(), -1, nullptr,
                                    note.data());
        }
    }

    return retVal;
}

long BIShipCommandList::AddCancelIcon()
{
    if (!m_pARoot.hasProperty("Commands")) {
        return 0;
    }
    const Attribute& commands = m_pARoot["Commands"];

    if (!commands.hasProperty("Cancel")) {
        return 0;
    }
    const Attribute& cancel = m_pARoot["Cancel"];

    const long pictureNum = cancel["picNum"].get<uint32_t>(0);
    const long selPictureNum = cancel["selPicNum"].get<uint32_t>(0);
    const long textureNum = cancel["texNum"].get<uint32_t>(-1);
    const std::string_view event_name = cancel["event"].get<std::string_view>();
    const std::string_view note = cancel["note"].get<std::string_view>();
    return AddToIconList(textureNum, pictureNum, selPictureNum, -1, -1, event_name.data(), -1, nullptr, note.data());
}

long BIShipCommandList::TownAdding(bool allLabel, bool bDiseased, bool bNotDiseased, bool bEnemy, bool bNeutral,
                                   bool bFriend)
{
    auto *pL = g_IslandDescr.GetFirstLocator();
    if (pL == nullptr)
        return 0;
    long retVal = 0;

    // determine the radius of the command (everything that is not included in it is not shown)
    auto *pA = GetCurrentCommandAttribute();
    auto sqrRadius = pL->r;
    if (pA)
        sqrRadius = pA->getProperty("EffectRadius").get<float>(sqrRadius);
    if (sqrRadius < 0.f)
        allLabel = true;
    sqrRadius *= sqrRadius;

    // Determine the coordinates of the ship receiving the commands
    const auto selectedCharacter = m_nCurrentCommandCharacterIndex;
    auto *selShip = g_ShipList.FindShip(selectedCharacter);
    float selX, selZ;
    if (selShip == nullptr)
    {
        allLabel = true;
        selX = 0;
        selZ = 0;
    }
    else
    {
        selX = selShip->pShip->GetPos().x;
        selZ = selShip->pShip->GetPos().z;
    }

    do
    {
        if (pL->locatorType != ISLAND_LOCATOR_TOWN)
            continue; // check by type - must be city
        // check against (enemy, neutral, friend)
        if (pL->relation == BI_RELATION_ENEMY && !bEnemy)
            continue;
        if (pL->relation == BI_RELATION_NEUTRAL && !bNeutral)
            continue;
        if (pL->relation == BI_RELATION_FRIEND && !bFriend)
            continue;
        if (pL->bDiseased && !bDiseased)
            continue;
        if (!pL->bDiseased && !bNotDiseased)
            continue;
        if (!allLabel)
            if (SQR(pL->x - selX) + SQR(pL->z - selZ) > sqrRadius)
                continue;
        auto *pvdat = core.Event("evntCheckEnableLocator", "sa", m_sCurrentCommandName.c_str(), pL->pA);
        if (pvdat != nullptr && pvdat->GetLong() == 0)
            continue;
        std::string_view pLocName;
        if (pL->pA != nullptr)
            pLocName = pL->pA->getProperty("name").get<std::string_view>();
        retVal += AddToIconList(pL->texIdx, pL->picIdx, pL->selPicIdx, -1, pL->characterIndex, nullptr, -1, pLocName.data(),
                                pL->pchr_note.data());
    } while ((pL = g_IslandDescr.GetNext()) != nullptr);
    return retVal;
}

void BIShipCommandList::AddFlagPictureToIcon(long nCharIdx)
{
    auto *pvdat = core.Event("evntGetSmallFlagData", "l", nCharIdx);
    if (!pvdat)
        return;
    long nTex, nPic, nBackPic;
    pvdat->Get(nTex, 0);
    pvdat->Get(nPic, 1);
    pvdat->Get(nBackPic, 2);
    AddAdditiveToIconList(nTex, nBackPic, 2.f, 32.f, 32.f);
    AddAdditiveToIconList(nTex, nPic, -32.f, 32.f, 32.f);
}
