#include "shipcommand.h"
#include "../shared/battle_interface/msg_control.h"
#include "core.h"
#include "vmodule_api.h"

WMShipCommandList::WMShipCommandList(entid_t eid, Attribute &pA, VDX9RENDER *rs) : BICommandList(eid, pA, rs)
{
    Init();
}

WMShipCommandList::~WMShipCommandList()
{
    Release();
}

void WMShipCommandList::FillIcons()
{
    long nIconsQuantity = 0;

    if (m_nCurrentCommandMode & BI_COMMODE_COMMAND_SELECT)
        nIconsQuantity += CommandAdding();
}

void WMShipCommandList::Init()
{
    BICommandList::Init();
}

void WMShipCommandList::Release()
{
}

long WMShipCommandList::CommandAdding()
{
    core.Event("WM_SetPossibleCommands", "l", m_nCurrentCommandCharacterIndex);
    long retVal = 0;

    if (const Attribute& commands = m_pARoot["Commands"]; !commands.empty()) {
        for (const Attribute& command : commands) {
            if (command["enable"].get<bool>(false)) {
                const auto pictureNum = command["picNum"].get<long>(0l);
                const auto selPictureNum = command["selPicNum"].get<long>(0l);
                const auto texNum = command["texNum"].get<long>(-1l);
                const auto eventName = command["event"].get<std::string_view>();
                const auto note = command["note"].get<std::string_view>();
                retVal += AddToIconList(texNum, pictureNum, selPictureNum, -1, -1, eventName.data(), -1, nullptr, note.data());
            }
        }
    }

    return retVal;
}
