#include "locator.h"
#include "../../Shared/messages.h"
#include "Entity.h"
#include "core.h"

LOCATOR::LOCATOR()
{
    gs = nullptr;
    geo = nullptr;
    groupID = -1;
    stringIndex = -1;
}

LOCATOR::~LOCATOR()
{
    delete geo;
    geo = nullptr;
}

bool LOCATOR::Init()
{
    rs = static_cast<VDX9RENDER *>(core.CreateService("dx9render"));
    gs = static_cast<VGEOMETRY *>(core.CreateService("geometry"));
    if (!gs)
        return false;

    return true;
}

bool LOCATOR::VerifyParticles()
{
    ParticlesID = EntityManager::GetEntityId("particles");
    if (!ParticlesID)
        ParticlesID = EntityManager::CreateEntity("particles");

    return static_cast<bool>(ParticlesID);
}

void LOCATOR::LocateForI_L2(Attribute *pA, GEOS *g, GEOS::LABEL &label)
{
    char name[16];
    GEOS::LABEL label2;

    const auto groupID = g->FindName(label.name);
    if (groupID < 0)
    {
        core.Trace("?void LOCATOR::LocateForI_L2(...)");
        return;
    }

    Attribute& aShips = pA->getProperty("ships");

    long n = 0;
    for (long stringIndex = 0; (stringIndex = g->FindLabelG(stringIndex, groupID)) >= 0; stringIndex++)
    {
        g->GetLabel(stringIndex, label2);
        sprintf_s(name, "l%d", n);
        Attribute& attr = aShips[name];
        attr["x"] = label2.m[3][0];
        attr["y"] = label2.m[3][1];
        attr["z"] = label2.m[3][2];
        attr["ay"] = atan2f(label2.m[2][0], label2.m[2][2]);
        n++;
    }
}

void LOCATOR::LocateForI(VDATA *pData)
{
    Attribute *pA;
    Attribute *pAA;
    GEOS *g;
    GEOS::LABEL label;
    long i, n;

    if (pData == nullptr)
    {
        core.Trace("?void LOCATOR::LocateForI(VDATA * pData)");
        return;
    }
    pA = pData->GetAClass();
    if (pA == nullptr)
    {
        core.Trace("?void LOCATOR::LocateForI(VDATA * pData)");
        return;
    }
    if (!pA->hasProperty("locators"))
    {
        core.Trace("?void LOCATOR::LocateForI(VDATA * pData)");
        return;
    }
    char sFileLocators[256];

    const Attribute& aModels = pA->getProperty("filespath")["models"];
    sprintf_s(sFileLocators, "%s\\%s", aModels.get<const char*>(""), pA->getProperty("locators").get<const char*>());
    rs->SetLoadTextureEnable(false);
    g = gs->CreateGeometry(sFileLocators, "", 0);
    rs->SetLoadTextureEnable(true);
    if (!g)
    {
        core.Trace("?void LOCATOR::LocateForI(VDATA * pData)");
        return;
    }

    Attribute& aReload = pA->getProperty("reload");

    auto groupID = g->FindName("reload");
    if (groupID >= 0)
    {
        for (long i = 0; (i = g->FindLabelG(i, groupID)) >= 0; i++)
        {
            g->GetLabel(i, label);
            if (!aReload.empty()) {
                for (Attribute& attr : aReload) {
                    if (!attr.hasProperty("name"))
                    {
                        core.Trace("LOCATOR: no name");
                        continue;
                    }
                    if (_stricmp(attr["name"].get<const char*>(), label.name) == 0)
                    {
                        attr["x"] = label.m[3][0];
                        attr["y"] = label.m[3][1];
                        attr["z"] = label.m[3][2];
                        attr["ay"] = atan2f(label.m[2][0], label.m[2][2]);
                        LocateForI_L2(&attr, g, label);
                    }
                }
            }
        }
    }

    // check for unfind reloads
    if (!aReload.empty()) {
        for (Attribute& attr : aReload) {
            if (!attr.hasProperty("x"))
            {
                core.Trace("LOCATOR: Can't find locator with name: %s, geo: %s", attr["name"].get<const char*>(),
                           pA->getProperty("locators").get<const char*>());
                continue;
            }
            if (_stricmp(attr["name"].get<const char*>(), label.name) == 0)
            {
                attr["x"] = label.m[3][0];
                attr["y"] = label.m[3][1];
                attr["z"] = label.m[3][2];
                attr["ay"] = atan2f(label.m[2][0], label.m[2][2]);
                LocateForI_L2(&attr, g, label);
            }
        }
    }

    groupID = g->FindName("quest_ships");
    if (groupID >= 0)
    {
        Attribute& aQuestShips = pA->getProperty("Quest_ships");
        LocateForI_Locators(&aQuestShips, g, groupID, _XYZ_ | _AY_);
    }

    groupID = g->FindName("net_deathmatch");
    if (groupID >= 0)
    {
        Attribute& aNetDeatchMatch = pA->getProperty("net_deathmatch");
        LocateForI_Locators(&aNetDeatchMatch, g, groupID, _XYZ_ | _AY_);
    }

    groupID = g->FindName("net_team");
    if (groupID >= 0)
    {
        Attribute &aNetTeam = pA->getProperty("net_team");
        LocateForI_Locators(&aNetTeam, g, groupID, _XYZ_ | _AY_);
    }

    groupID = g->FindName("net_convoy");
    if (groupID >= 0)
    {
        Attribute &aNetConvoy = pA->getProperty("net_convoy");
        LocateForI_Locators(&aNetConvoy, g, groupID, _XYZ_ | _AY_);
    }

    groupID = g->FindName("net_fort");
    if (groupID >= 0)
    {
        Attribute &aNetFort = pA->getProperty("net_fort");
        LocateForI_Locators(&aNetFort, g, groupID, _XYZ_ | _AY_);
    }

    groupID = g->FindName("fire");
    if (groupID >= 0)
    {
        Attribute &aFire = pA->getProperty("fire");
        LocateForI_Locators(&aFire, g, groupID, _XYZ_);
    }

    Attribute &aLoadGroup = pA->getProperty("LoadGroup");
    if (!aLoadGroup.empty())
        for (Attribute &aGroupName : aLoadGroup) {
            const char* pLoadGroupName = aGroupName.get<const char*>(nullptr);
            if (!pLoadGroupName)
                continue;

            groupID = g->FindName(pLoadGroupName);
            if (groupID < 0)
                continue;

            Attribute& aGroup = pA->getProperty(pLoadGroupName);
            LocateForI_Locators(&aGroup, g, groupID, _XYZ_ | _AY_);
        }

    delete g;
}

void LOCATOR::LocateForI_Locators(Attribute *pA, GEOS *geo, long iGroupID, uint32_t dwFlags)
{
    long i;
    GEOS::LABEL label;
    Attribute *pAA;

    for (i = 0; (i = geo->FindLabelG(i, iGroupID)) >= 0; i++)
    {
        geo->GetLabel(i, label);
        Attribute& aLabel = pA->getProperty(label.name);
        if (dwFlags & _X_)
            aLabel["x"] = label.m[3][0];
        if (dwFlags & _Y_)
            aLabel["y"] = label.m[3][1];
        if (dwFlags & _Z_)
            aLabel["z"] = label.m[3][2];
        if (dwFlags & _AY_)
            aLabel["ay"] = atan2f(label.m[2][0], label.m[2][2]);
    }
}

uint64_t LOCATOR::ProcessMessage(MESSAGE &message)
{
    long message_code;
    char name[MAX_PATH];
    GEOS::LABEL label;
    Attribute *pA;
    char buffer[MAX_PATH];

    message_code = message.Long();
    switch (message_code)
    {
    case LM_LOCATE_I:
        LocateForI(message.ScriptVariablePointer());
        break;
    case LM_LOCATE_FIRST:
        message.String(sizeof(buffer), buffer);
        pA = message.AttributePointer();
        groupID = geo->FindName(buffer);
        if (groupID >= 0)
        {
            VerifyParticles();
            stringIndex = geo->FindLabelG(0, groupID);
            if (stringIndex < 0)
                break;
            geo->GetLabel(stringIndex, label);

            if (pA)
            {
                Attribute& attr = *pA;
                attr["x"] = label.m[3][0];
                attr["y"] = label.m[3][1];
                attr["z"] = label.m[3][2];
                attr["ay"] = atan2f(label.m[2][0], label.m[2][2]);
                attr["vx"] = label.m[2][0];
                attr["vy"] = label.m[2][1];
                attr["vz"] = label.m[2][2];
            }
            stringIndex++;
            return 1;
        }
        return 0;
    case LM_LOCATE_NEXT:
        pA = message.AttributePointer();
        if (groupID >= 0)
        {
            VerifyParticles();
            stringIndex = geo->FindLabelG(stringIndex, groupID);
            if (stringIndex < 0)
                return 0;
            geo->GetLabel(stringIndex, label);
            if (pA)
            {
                Attribute& attr = *pA;
                attr["x"] = label.m[3][0];
                attr["y"] = label.m[3][1];
                attr["z"] = label.m[3][2];
                attr["ay"] = atan2f(label.m[2][0], label.m[2][2]);
                attr["vx"] = label.m[2][0];
                attr["vy"] = label.m[2][1];
                attr["vz"] = label.m[2][2];
            }
            stringIndex++;
            return 1;
        }
        return 0;

    case LM_SET_GEOMETRY:
        message.String(sizeof(name), name);
        delete geo;
        geo = nullptr;
        rs->SetLoadTextureEnable(false);
        geo = gs->CreateGeometry(name, "", 0);
        rs->SetLoadTextureEnable(true);
        break;
        /*case LM_LOCATE:
          groupID = geo->FindName("smoke");
          if(groupID >= 0)
          {
            VerifyParticles();
            for(stringIndex = 0; (stringIndex = geo->FindLabelG(stringIndex, groupID)) >= 0; stringIndex++)
            {
              geo->GetLabel(stringIndex, label);
              core.Send_Message(ParticlesID,"lsffffffl",
                PS_CREATE,"smoke",label.m[3][0],label.m[3][1],label.m[3][2],-1.57,0,0,0);
            }
          }

          groupID = geo->FindName("fire");
          if(groupID >= 0)
          {
            if(!core.FindClass(&ParticlesID,"particles",0))
            {
              if(!EntityManager::CreateEntity(&ParticlesID,"particles")) return 0;
            }
            for(stringIndex = 0; (stringIndex = geo->FindLabelG(stringIndex, groupID)) >= 0; stringIndex++)
            {
              geo->GetLabel(stringIndex, label);
              core.Send_Message(ParticlesID,"lsffffffl",
                PS_CREATE,"fire",label.m[3][0],label.m[3][1],label.m[3][2],-1.57,0,0,0);
            }
          }

          groupID = geo->FindName("water");
          if(groupID >= 0)
          {
            if(!core.FindClass(&ParticlesID,"particles",0))
            {
              if(!EntityManager::CreateEntity(&ParticlesID,"particles")) return 0;
            }
            for(stringIndex = 0; (stringIndex = geo->FindLabelG(stringIndex, groupID)) >= 0; stringIndex++)
            {

              geo->GetLabel(stringIndex, label);
              core.Send_Message(ParticlesID,"lsffffffl",
                PS_CREATEX,"waterfall",label.m[3][0],label.m[3][1],label.m[3][2],label.m[2][0],label.m[2][1],-label.m[2][2],0);
            }
          }
        break;*/
    }
    return 0;
}

uint32_t LOCATOR::AttributeChanged(Attribute &pA)
{
    return 0;
}
