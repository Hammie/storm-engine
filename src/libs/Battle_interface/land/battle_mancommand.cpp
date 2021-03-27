#include "battle_mancommand.h"
#include "../shared/battle_interface/msg_control.h"
#include "core.h"
#include "vmodule_api.h"

BIManCommandList::BIManCommandList(entid_t eid, Attribute &pA, VDX9RENDER *rs) : BICommandList(eid, pA, rs)
{
    Init();
}

BIManCommandList::~BIManCommandList()
{
    Release();
}

void BIManCommandList::FillIcons()
{
    long nIconsQuantity = 0;

    if (m_nCurrentCommandMode & BI_COMMODE_COMMAND_SELECT)
        nIconsQuantity += CommandAdding();

    if (m_nCurrentCommandMode & BI_COMMODE_USER_ICONS)
        nIconsQuantity += UserIconsAdding();

    if (m_nCurrentCommandMode & BI_COMMODE_ABILITY_ICONS)
        nIconsQuantity += AbilityAdding();

    nIconsQuantity += AddCancelIcon();

    /*if( nIconsQuantity == 0 )
      AddCancelIcon();*/
}

void BIManCommandList::Init()
{
    BICommandList::Init();

    m_nIconShowMaxQuantity = 5;
    // boal -->
    const Attribute& pA = m_pARoot.getProperty("CommandList");
    if (!pA.empty())
    {
        const Attribute& icon_show_max_quantity = pA.getProperty("CommandMaxIconQuantity");
        if (!icon_show_max_quantity.empty()) {
             icon_show_max_quantity.get_to(m_nIconShowMaxQuantity);
        }
    }
    // boal <--
}

void BIManCommandList::Release()
{
}

long BIManCommandList::CommandAdding()
{
    core.Event("BI_SetPossibleCommands", "l", m_nCurrentCommandCharacterIndex);
    long retVal = 0;
    const Attribute& commands = m_pARoot.getProperty("Commands");
    if (commands.empty()) {
        return 0;
    }

    for (const Attribute& attr : commands) {
        if (attr.hasProperty("enable") && attr["enable"].get<bool>()) {
            const long pictureNum = attr["picNum"].get<uint32_t>(0);
            const long selPictureNum = attr["selPicNum"].get<uint32_t>(0);
            const long texNum = attr["texNum"].get<uint32_t>(-1);
            const std::string_view eventName = attr["event"].get<std::string_view>();
            retVal += AddToIconList(texNum, pictureNum, selPictureNum, -1, -1, eventName.data(), -1, nullptr,
                                    attr["note"].get<std::string_view>().data());
        }

    }

    return retVal;
}

long BIManCommandList::UserIconsAdding()
{
    const Attribute& user_icons = m_pARoot.getProperty("UserIcons");
    if (!user_icons.empty()) {
        long retVal = 0;
        int i = 0;
        for (const Attribute& attr : user_icons) {
            if (attr.hasProperty("enable") && attr["enable"].get<bool>()) {
                const long pictureNum = attr["pic"].get<uint32_t>(0);
                const long selPictureNum = attr["selpic"].get<uint32_t>(0);
                const long textureNum = attr["tex"].get<uint32_t>(-1);
                const std::string_view eventName = attr["event"].get<std::string_view>();
                retVal += AddToIconList(textureNum, pictureNum, selPictureNum, -1, -1, eventName.data(), i + 1, nullptr,
                                        attr["note"].get<std::string_view>().data());
            }
            ++i;
        }

        return retVal;
    }

    return 0;

}

long BIManCommandList::AbilityAdding()
{
    core.Event("evntSetUsingAbility", "l", m_nCurrentCommandCharacterIndex);
    const Attribute& ability_icons = m_pARoot.getProperty("AbilityIcons");
    if(ability_icons.empty()) {
        return 0;
    }

    long retVal = 0;
    for (const Attribute &attr : ability_icons)
    {
        if (attr.hasProperty("enable") && attr["enable"].get<bool>())
        {
            const long pictureNum = attr["picNum"].get<uint32_t>(0);
            const long selPictureNum = attr["selPicNum"].get<uint32_t>(0);
            const long textureNum = attr["texNum"].get<uint32_t>(-1);
            const std::string_view eventName = attr["event"].get<std::string_view>();
            retVal += AddToIconList(textureNum, pictureNum, selPictureNum, -1, -1, eventName.data(), -1, nullptr,
                                    attr["note"].get<std::string_view>().data());
        }
    }
    return retVal;
}

long BIManCommandList::AddCancelIcon()
{
    const Attribute& commands = m_pARoot.getProperty("Commands");
    const Attribute& cancel = commands["Cancel"];
    if (cancel.empty())
        return 0;
    const Attribute& attr = cancel;
    const long pictureNum = attr["picNum"].get<uint32_t>(0);
    const long selPictureNum = attr["selPicNum"].get<uint32_t>(0);
    const long textureNum = attr["texNum"].get<uint32_t>(-1);
    const std::string_view eventName = attr["event"].get<std::string_view>();
    return AddToIconList(textureNum, pictureNum, selPictureNum, -1, -1, eventName.data(), -1, nullptr,
                         attr["note"].get<std::string_view>().data());
}
