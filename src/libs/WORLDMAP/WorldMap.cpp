//============================================================================================
//    Spirenkov Maxim, 2001
//--------------------------------------------------------------------------------------------
//    Sea Dogs II
//--------------------------------------------------------------------------------------------
//    WorldMap
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "WorldMap.h"
#include "../../Shared/messages.h"

#include "core.h"

#include "Entity.h"
#include "WdmCameraStdCtrl.h"
#include "WdmClouds.h"
#include "WdmFollowShip.h"
#include "WdmIcon.h"
#include "WdmIslands.h"
#include "WdmMerchantShip.h"
#include "WdmPlayerShip.h"
#include "WdmSea.h"
#include "WdmStorm.h"
#include "WdmWarringShip.h"
#include "WdmWindUI.h"
#include "defines.h"

#include "message.hpp"

//============================================================================================

//#define EVENTS_OFF
//#define ENCS_OFF

#define WDM_MAX_STORMS 4

long WorldMap::month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// ============================================================================================
// Construction, destruction
// ============================================================================================

WorldMap::WorldMap()
{
    Assert(!wdmObjects);
    new WdmObjects();
    firstFreeObject = 0;
    firstObject = -1;
    firstPrObject = -1;
    firstMrObject = -1;
    firstLrObject = -1;
    for (long i = 0; i < WDMAP_MAXOBJECTS; i++)
        object[i].next = i + 1;
    object[WDMAP_MAXOBJECTS - 1].next = -1;
    wdmObjects->wm = this;
    camera = nullptr;
    srand(GetTickCount());
    encTime = 0.0f;
    aStorm = nullptr;
    aEncounter = nullptr;
    aInfo = nullptr;
    saveData = nullptr;
    timeScale = 1.0f;
    hour = 11.0f;
    day = 14;
    mon = 6;
    year = 1655;
    encCounter = 0;
}

WorldMap::~WorldMap()
{
    Assert(AttributesPointer != nullptr);
    Attribute& attr = *AttributesPointer;

    if (AttributesPointer)
    {
        attr["WindData"] = wdmObjects->GetWindSaveString(bufForSave);
    }
    // leave the encounter parameters intact
    for (long i = 0; i < wdmObjects->ships.size(); i++)
    {
        if (wdmObjects->ships[i] == wdmObjects->playerShip)
            continue;
        static_cast<WdmEnemyShip *>(wdmObjects->ships[i])->SetSaveAttribute(nullptr);
    }
    for (long i = 0; i < wdmObjects->storms.size(); i++)
    {
        wdmObjects->storms[i]->SetSaveAttribute(nullptr);
    }
    // Player's ship
    if (wdmObjects->playerShip)
    {
        float x, z, ay;
        wdmObjects->playerShip->GetPosition(x, z, ay);
        attr["playerShipX"] = x;
        attr["playerShipZ"] = z;
        attr["playerShipAY"] = ay;
    }
    // Camera
    if (wdmObjects->camera)
    {
       attr["wdmCameraY"] = wdmObjects->camera->pos.y;
       attr["wdmCameraAY"] = wdmObjects->camera->ang.y;
    }
    ResetScriptInterfaces();
    /*for(; firstObject >= 0; firstObject = object[firstObject].next)
    {
      delete object[firstObject].ro;
    }*/
    delete camera;
    WdmRenderObject::DeleteAllObjects();
    wdmObjects->Clear();

    delete wdmObjects;
}

//============================================================================================
// Entity
//============================================================================================

// Initialization
bool WorldMap::Init()
{
    Assert(AttributesPointer != nullptr);
    Attribute& attr = *AttributesPointer;

    // GUARD(LocationCamera::Init())
    // Layers
    // core.LayerCreate("execute", true, false);
    EntityManager::SetLayerType(EXECUTE, EntityManager::Layer::Type::execute);
    // core.LayerCreate("realize", true, false);
    EntityManager::SetLayerType(REALIZE, EntityManager::Layer::Type::realize);
    EntityManager::AddToLayer(EXECUTE, GetId(), 10000);
    EntityManager::AddToLayer(REALIZE, GetId(), 10000);

    // DX9 render
    rs = static_cast<VDX9RENDER *>(core.CreateService("dx9render"));
    if (!rs)
        throw std::exception("No service: dx9render");
    rs->SetPerspective((1.57f + 1.0f) / 2);
    wdmObjects->rs = rs;
    // GS
    wdmObjects->gs = static_cast<VGEOMETRY *>(core.CreateService("geometry"));
    // Create map objects
    WdmRenderObject *ro;
    // Create islands
    ro = AddObject(new WdmIslands());
    AddObject(ro, -100000);
    AddLObject(ro, 500);
    //
    rs->ProgressView();
    // Create the sea
    auto *sea = new WdmSea();
    AddObject(sea);
    AddPObject(sea, 10);
    AddLObject(sea, -1);
    // create clouds
    AddLObject(AddObject(new WdmClouds()), 10000);
    rs->ProgressView();
    // Create a camera
    camera = new WdmCameraStdCtrl();
    auto camAy = 0.0f;
    auto camH = -1.0f;
    auto camLock = false;
    if (AttributesPointer)
    {
        attr["wdmCameraAY"].get_to(camAy);
        attr["wdmCameraY"].get_to(camH);
        attr["wdmCameraRotLock"].get_to(camLock, false);
        wdmObjects->SetWindSaveString(attr["WindData"].get<const char*>());
        attr["shipSpeedOppositeWind"].get_to(wdmObjects->shipSpeedOppositeWind);
        attr["shipSpeedOverWind"].get_to(wdmObjects->shipSpeedOverWind);
    }
    else
    {
        wdmObjects->SetWindSaveString(nullptr);
    }
    camera->Init(camAy, camH);
    camera->lock = camLock;
    // create the player's ship
    ro = CreateModel(new WdmPlayerShip(), "Ship");
    Assert(ro);
    AddLObject(ro, 100);
    auto psX = 50.0f;
    auto psZ = 0.0f;
    auto psAy = 0.0f;
    auto psRad = 16.0f;
    if (AttributesPointer)
    {
        attr["playerShipX"].get_to(psX);
        attr["playerShipZ"].get_to(psZ);
        attr["playerShipAY"].get_to(psAy);
        attr["playerShipActionRadius"].get_to(psRad);
        attr["enemyshipViewDistMin"].get_to(wdmObjects->enemyshipViewDistMin);
        attr["enemyshipViewDistMax"].get_to(wdmObjects->enemyshipViewDistMax);
        attr["enemyshipDistKill"].get_to(wdmObjects->enemyshipDistKill);
        attr["enemyshipBrnDistMin"].get_to(wdmObjects->enemyshipBrnDistMin);
        attr["enemyshipBrnDistMax"].get_to(wdmObjects->enemyshipBrnDistMax);
        attr["stormViewDistMin"].get_to(wdmObjects->stormViewDistMin);
        attr["stormViewDistMax"].get_to(wdmObjects->stormViewDistMax);
        attr["stormDistKill"].get_to(wdmObjects->stormDistKill);
        attr["stormBrnDistMin"].get_to(wdmObjects->stormBrnDistMin);
        attr["stormBrnDistMax"].get_to(wdmObjects->stormBrnDistMax);
        attr["stormZone"].get_to(wdmObjects->stormZone);
        const char* sDebug = attr["debug"].get<const char*>();
        wdmObjects->isDebug = (_stricmp(sDebug, "true") == 0);
        saveData = &attr["encounters"];
        attr["resizeRatio"].get_to(wdmObjects->resizeRatio);
    }
    static_cast<WdmShip *>(ro)->Teleport(psX, psZ, psAy);
    static_cast<WdmPlayerShip *>(ro)->SetActionRadius(psRad);
    rs->ProgressView();
    // Create a location descriptor
    wdmObjects->islands->SetIslandsData(AttributesPointer, false);
    // Script interface attributes
    if (AttributesPointer)
    {
        // Storms interface
        aStorm = &attr["storm"];
        // Ship encounters inderface
        aEncounter = &attr["encounter"];
        // Info
        aInfo = &attr["info"];
        // Date
        aDate = &attr["date"];
    }
    if (aDate)
    {
        const long sec = (*aDate)["sec"].get<long>(1);
        const long min = (*aDate)["min"].get<long>(1);
        (*aDate)["min"].get_to(hour);
        hour += (min + sec / 60.0f) / 60.0f;
        (*aDate)["day"].get_to(day);
        (*aDate)["month"].get_to(mon);
        (*aDate)["year"].get_to(year);
        (*aDate)["hourPerSec"].get_to(timeScale);
    }
    ResetScriptInterfaces();
    rs->ProgressView();
    // Creating interface elements

    // Date
    auto *windUI = new WdmWindUI();
    windUI->SetAttributes(AttributesPointer);
    AddLObject(AddObject(windUI, 1001), 10100);

    // Compass
    // ro = CreateModel(new WdmWindRose(), "WindRose");
    // AddLObject(ro, 10099);
    // The calendar
    // WdmCounter * cnt = new WdmCounter();
    // if(!cnt->Init()) core.Trace("Counter not created");

    // AddLObject(cnt, 10099);
    // Icon
    AddLObject(AddObject(new WdmIcon(), 1000), 10099);

    // load encounters, if any
    if (saveData)
    {
        Assert(saveData != nullptr);
        Attribute& aSaveData = *saveData;

        for (Attribute& aEntry : aSaveData) {
            if (aEntry.empty())
                continue;
            const char *type = aEntry["type"].get<const char*>();
            const char *modelName = aEntry["modelName"].get<const char*>();
            if (!type || !type[0])
            {
                aEntry.clear();
                continue;
            }
            if (_stricmp(type, "Merchant") == 0 && modelName && modelName[0])
            {
                if (!CreateMerchantShip(modelName, nullptr, nullptr, 1.0f, -1.0f, &aEntry))
                {
                    core.Trace("WoldMap: not loaded merchant encounter.");
                }
                continue;
            }
            if (_stricmp(type, "Follow") == 0 && modelName && modelName[0])
            {
                if (!CreateFollowShip(modelName, 1.0f, -1.0f, &aEntry))
                {
                    core.Trace("WoldMap: not loaded follow encounter.");
                }
                continue;
            }
            if (_stricmp(type, "Warring") == 0 && modelName && modelName[0])
            {
                const char* attacked = aEntry["attacked"].get<const char*>(nullptr);
                if (attacked)
                {
                    Attribute& aSaveAttacked = aSaveData[attacked];
                    if (!aSaveAttacked.empty())
                    {
                        auto *const modelName1 = aSaveAttacked["modelName"].get<const char*>();
                        if (modelName1 && modelName1[0])
                        {
                            if (!CreateWarringShips(modelName, modelName1, -1.0f, &aEntry, &aSaveAttacked))
                            {
                                core.Trace("WoldMap: not loaded warring encounter.");
                            }
                        }
                        else
                        {
                            core.Trace("WoldMap: not loaded warring encounter.");
                            aEntry.clear();
                            aSaveAttacked.clear();
                        }
                    }
                }
                continue;
            }
            if (_stricmp(type, "Attacked") == 0)
            {
                continue;
            }
            if (_stricmp(type, "Storm") == 0)
            {
                const auto isTornado = (aEntry.getProperty("isTornado").get<bool>(false));
                if (!CreateStorm(isTornado, -1.0f,  &aEntry))
                {
                    core.Trace("WoldMap: not loaded storm encounter.");
                }
                continue;
            }
            aEntry.clear();
        }
    }

    rs->ProgressView();

    // Adjusting the player's ship
    auto *playerShip = static_cast<WdmPlayerShip *>(wdmObjects->playerShip);
    playerShip->PushOutFromIsland();

    const Attribute& aIsland = attr["island"];
    if (!aIsland.empty())
    {
        float x, z, ay;
        playerShip->GetPosition(x, z, ay);
        if (!wdmObjects->islands->CheckIslandArea(aIsland.get<const char*>(), x, z))
        {
            wdmObjects->islands->GetNearPointToArea(aIsland.get<const char*>(), x, z);
            playerShip->Teleport(x, z, ay);
        }
    }
    return true;
    // UNGUARD
}

// Execution
void WorldMap::Execute(uint32_t delta_time)
{
}

void WorldMap::Realize(uint32_t delta_time)
{
    Assert(AttributesPointer != nullptr);
    Attribute& attr = *AttributesPointer;

    if (AttributesPointer && wdmObjects->playerShip)
    {
        CVECTOR wind(0.0f);
        float x, z, ay;
        wdmObjects->playerShip->GetPosition(x, z, ay);
        const auto force = wdmObjects->GetWind(x, z, wind);
        attr["WindX"] = wind.x;
        attr["WindZ"] = wind.z;
        attr["WindF"] = force;
    }
    if (!wdmObjects->isPause)
    {
        CONTROL_STATE cs;
        core.Controls->GetControlState("WMapCancel", cs);
        if (cs.state == CST_ACTIVATED)
        {
            if (wdmObjects->playerShip)
            {
                if (!static_cast<WdmPlayerShip *>(wdmObjects->playerShip)->ExitFromMap())
                    core.Event("ExitFromWorldMap");
            }
            else
                core.Event("ExitFromWorldMap");
        }
    }
    //---------------------------------------------------------
    const auto dltTime = 0.001f * delta_time;
    // Updating the date
    if (hour < 0.0f)
        hour = 0.0f;
    hour += dltTime * timeScale;
    auto days = static_cast<long>(hour / 24.0f);
    hour = (hour / 24.0f - days) * 24.0f;
    const auto dtHour = static_cast<long>(hour);
    const auto dtMin = static_cast<long>((hour - dtHour) * 60.0f);
    const auto dtSec = static_cast<long>(((hour - dtHour) * 60.0f - dtMin) * 60.0f);
    aDate->getProperty("sec") = dtSec;
    aDate->getProperty("min") = dtMin;
    aDate->getProperty("hour") = dtHour;
    if (days)
    {
        for (; days > 0; days--)
        {
            day++;
            if (day > month[mon])
            {
                day -= month[mon++];
                if (mon > 12)
                {
                    mon = 1;
                    year++;
                    aDate->getProperty("year") = year;
                }
                aDate->getProperty("month") = mon;
            }
            aDate->getProperty("day") = day;

#ifndef EVENTS_OFF
            core.Event("WorldMap_UpdateDate", "f", hour);
            wdmObjects->isNextDayUpdate = true;
            core.Event("NextDay");
#endif
        }
    }
    else
    {
#ifndef EVENTS_OFF
        core.Event("WorldMap_UpdateDate", "f", hour);
#endif
    }
    //
    auto *tmp = aDate->getProperty("sec").get<const char*>(nullptr);
    if (tmp)
        strcpy_s(wdmObjects->attrSec, tmp);
    tmp = aDate->getProperty("min").get<const char*>(nullptr);
    if (tmp)
        strcpy_s(wdmObjects->attrMin, tmp);
    tmp = aDate->getProperty("hour").get<const char*>(nullptr);
    if (tmp)
        strcpy_s(wdmObjects->attrHour, tmp);
    tmp = aDate->getProperty("day").get<const char*>(nullptr);
    if (tmp)
        strcpy_s(wdmObjects->attrDay, tmp);
    tmp = aDate->getProperty("month").get<const char*>(nullptr);
    if (tmp)
        strcpy_s(wdmObjects->attrMonth, tmp);
    tmp = aDate->getProperty("year").get<const char*>(nullptr);
    if (tmp)
        strcpy_s(wdmObjects->attrYear, tmp);
    //---------------------------------------------------------
    if (camera && !wdmObjects->isPause)
        camera->Move(dltTime, rs);
    auto isKill = false;
    // execute all objects
    for (auto i = firstObject; i >= 0; i = object[i].next)
    {
        if (!object[i].ro->killMe)
            object[i].ro->Update(object[i].ro->isEnablePause && wdmObjects->isPause ? 0.0f : dltTime);
        isKill |= object[i].ro->killMe;
    }
    // Remove objects if necessary
    if (isKill)
    {
        for (auto i = firstObject; i >= 0;)
            if (object[i].ro->killMe)
            {
                DeleteObject(object[i].ro);
                i = firstObject;
            }
            else
                i = object[i].next;
    }
    // Current number of events
    if (aStorm)
        aStorm->getProperty("num") = wdmObjects->storms.size();
    if (aEncounter)
        aEncounter->getProperty("num") = wdmObjects->ships.size() - (wdmObjects->playerShip != nullptr);
    // Events
    encTime += dltTime;
    if (encTime >= 1.0f && wdmObjects->playerShip && !wdmObjects->isPause)
    {
        auto psx = 0.0f, psz = 0.0f, psay = 0.0f;
        wdmObjects->playerShip->GetPosition(psx, psz, psay);
#ifndef ENCS_OFF
        core.Event("WorldMap_EncounterCreate", "ffff", encTime, psx, psz, psay);
#endif
        encTime = 0.0f;
    }
    rs->SetRenderState(D3DRS_FOGENABLE, FALSE);
    rs->SetRenderState(D3DRS_LIGHTING, FALSE);
    for (auto i = firstPrObject; i >= 0; i = object[i].next)
    {
        if (!object[i].ro->killMe)
            object[i].ro->PRender(rs);
    }
    for (auto i = firstMrObject; i >= 0; i = object[i].next)
    {
        if (!object[i].ro->killMe)
            object[i].ro->MRender(rs);
    }
    for (auto i = firstLrObject; i >= 0; i = object[i].next)
    {
        if (!object[i].ro->killMe)
            object[i].ro->LRender(rs);
    }
    // renew the wind
    wdmObjects->UpdateWind(dltTime);
    wdmObjects->isNextDayUpdate = false;
    // Checking the update attribute of the encounter
    if (AttributesPointer)
    {
        const char *upd = attr["addQuestEncounters"].get<const char*>(nullptr);
        if (upd && upd[0] != 0)
        {
            core.Event("WorldMap_AddQuestEncounters", nullptr);
        }
    }
}

// Messages
uint64_t WorldMap::ProcessMessage(MESSAGE &message)
{
    scripting::Message msg = message.Convert();

    switch (std::get<0>(msg.GetParams<"l">()))
    {
    case MSG_WORLDMAP_CREATESTORM: {
        if (msg.CheckFormat("ll")) {
            auto [_, isTornado] = msg.GetParams<"ll">();
            CreateStorm(isTornado != 0);
        }
        else {
            CreateStorm(false);
        }
    }
    break;
    case MSG_WORLDMAP_CREATEENC_MER: {
        if (msg.CheckFormat("lsssff")) {
            auto [_, modelName, startLocator, endLocator, speed, timeOut] = msg.GetParams<"lsssff">();
            return CreateMerchantShip(modelName.data(), startLocator.data(), endLocator.data(), speed, timeOut);
        }
        else if (msg.CheckFormat("llssf")) {
            auto [_, type, modelName, startLocator, speed] = msg.GetParams<"llssf">();
            return CreateMerchantShip(modelName.data(), startLocator.data(), nullptr, speed);
        }
    }
    break;
        // boal 04/01/06 -->
    case MSG_WORLDMAP_CREATEENC_MER_XZ: {
        auto [_, name, fx1, fz1, fx2, fz2, speed, timeOut] = msg.GetParams<"lsffffff">();
        return CreateMerchantShipXZ(name.data(), fx1, fz1, fx2, fz2, speed, timeOut);
    }
    break;
        // boal <--
    case MSG_WORLDMAP_CREATEENC_FLW: {
        if (msg.CheckFormat("lsff") ) {
            auto [_, name, speed, timeOut] = msg.GetParams<"lsff">();
            return CreateFollowShip(name.data(), speed, timeOut);
        }
        else if (msg.CheckFormat("llsf") ) {
            auto [_, type, name, speed] = msg.GetParams<"llsf">();
            return CreateFollowShip(name.data(), speed);
        }
    }
    break;
    case MSG_WORLDMAP_CREATEENC_WAR: {
        if (msg.CheckFormat("lssf") ) {
            auto [_, name1, name2, timeOut] = msg.GetParams<"lssf">();
            return CreateWarringShips(name1.data(), name2.data(), timeOut);
        }
        else if (msg.CheckFormat("llsls") ) {
            auto [_, type1, name1, type2, name2] = msg.GetParams<"llsls">();
            return CreateWarringShips(name1.data(), name2.data());
        }
    }
    break;
    case MSG_WORLDMAP_CREATEENC_RELEASE:
        ReleaseEncounters();
        break;
    case MSG_WORLDMAP_LAUNCH_EXIT_TO_SEA:
        if (wdmObjects->playerShip)
        {
            if (!static_cast<WdmPlayerShip *>(wdmObjects->playerShip)->ExitFromMap())
                core.Event("ExitFromWorldMap");
        }
        else
            core.Event("ExitFromWorldMap");
        break;
    case MSG_WORLDMAP_SET_NATION_FLAG:
        std::tie(std::ignore, wdmObjects->nationFlagIndex) = msg.GetParams<"ll">();
        break;
    case MSG_WORLDMAP_SET_COORDINATES: {
        auto [_, coordinate] = msg.GetParams<"ls">();
        strcpy_s(wdmObjects->coordinate, coordinate.data());
    } break;
    }
    return 0;
}

// Changing an attribute
uint32_t WorldMap::AttributeChanged(Attribute &apnt)
{
    float x, z, ay;
    if (!AttributesPointer)
        return 0;

    Assert(AttributesPointer != nullptr);
    Attribute& attr = *AttributesPointer;

    Attribute *pa = const_cast<Attribute*>(apnt.getParent());

    if (_stricmp(apnt.getName().data(), "deleteUpdate") == 0)
    {
        for (long i = 0; i < wdmObjects->ships.size(); i++)
        {
            if (wdmObjects->ships[i] == wdmObjects->playerShip)
                continue;
            static_cast<WdmEnemyShip *>(wdmObjects->ships[i])->DeleteUpdate();
        }
        for (long i = 0; i < wdmObjects->storms.size(); i++)
        {
            wdmObjects->storms[i]->DeleteUpdate();
        }
    }
    else if (_stricmp(apnt.getName().data(), "playerShipUpdate") == 0)
    {
        if (wdmObjects->playerShip)
        {
            float x, z, ay;
            wdmObjects->playerShip->GetPosition(x, z, ay);
            attr["playerShipX"] = x;
            attr["playerShipZ"] = z;
            attr["playerShipAY"] = ay;
        }
    }
    else if (_stricmp(apnt.getName().data(), "cur") == 0)
    {
        if (pa == aStorm)
        {
            const auto cur = pa->getProperty("cur").get<long>();
            if (cur >= 0 && cur < wdmObjects->storms.size())
            {
                Assert(wdmObjects->storms[cur]);
                wdmObjects->storms[cur]->GetPosition(x, z);
                pa->getProperty("x") = x;
                pa->getProperty("z") = z;
                pa->getProperty("time") = wdmObjects->storms[cur]->GetLiveTime();
            }
            else
            {
                pa->getProperty("cur") = -1;
            }
        }
        else if (pa == aEncounter)
        {
            const auto cur = pa->getProperty("cur").get<long>();
            // Determine the encounter index
            long i = 0;
            for (long enc = 0; i < wdmObjects->ships.size(); i++)
            {
                if (wdmObjects->ships[i] == wdmObjects->playerShip)
                    continue;
                if (enc == cur)
                    break;
                enc++;
            }
            if (i < wdmObjects->ships.size())
            {
                Assert(wdmObjects->ships[i]);
                wdmObjects->ships[i]->GetPosition(x, z, ay);
                pa->getProperty("x") = x;
                pa->getProperty("z") = z;
                pa->getProperty("ay") = ay;
                auto *const es = static_cast<WdmEnemyShip *>(wdmObjects->ships[i]);
                pa->getProperty("time") = es->GetLiveTime();
                char buf[32];
                sprintf_s(buf, "%i", es->type);
                pa->getProperty("type") = buf;
                pa->getProperty("select") = es->isSelect;
                pa->getProperty("id") = (char *)static_cast<WdmEnemyShip *>(wdmObjects->ships[i])->GetAttributeName();
                // If there is an attacker, get his index
                if (es->attack)
                {
                    Assert(es->attack != es);
                    long i, j = 0;
                    for (i = 0; i < wdmObjects->ships.size(); i++)
                    {
                        if (wdmObjects->ships[i] == wdmObjects->playerShip)
                            continue;
                        if (wdmObjects->ships[i] == es->attack)
                            break;
                        j++;
                    }
                    if (i >= wdmObjects->ships.size())
                        j = -1;
                    pa->getProperty("attack") = j;
                }
                else
                {
                    pa->getProperty("attack") = -1;
                }
            }
            else
            {
                pa->getProperty("cur") = -1;
            }
        }
    }
    else if (_stricmp(apnt.getName().data(), "updateinfo") == 0)
    {
        if (pa == aInfo)
        {
            pa->getProperty("playerInStorm") = wdmObjects->playarInStorm;
        }
    }
    else
    {
        for (const Attribute *pa = &apnt; pa != nullptr; pa = pa->getParent())
        {
            if (_stricmp(pa->getName().data(), "labels") == 0)
            {
                wdmObjects->islands->SetIslandsData(AttributesPointer, true);
                return 0;
            }
        }
    }
    return 0;
}

// ============================================================================================
// Objects management
// ============================================================================================

// Add object
WdmRenderObject *WorldMap::AddObject(WdmRenderObject *obj, long level)
{
    if (!obj)
        return nullptr;
    const auto i = GetObject(firstObject, level);
    object[i].ro = obj;
    return obj;
}

// Add object to render list before reflection
void WorldMap::AddPObject(WdmRenderObject *obj, long level)
{
    if (!obj)
        return;
    const auto i = GetObject(firstPrObject, level);
    object[i].ro = obj;
}

// Add object to reflection render list
void WorldMap::AddMObject(WdmRenderObject *obj, long level)
{
    if (!obj)
        return;
    const long i = GetObject(firstMrObject, level);
    object[i].ro = obj;
}

// Add object to render list after reflection
void WorldMap::AddLObject(WdmRenderObject *obj, long level)
{
    if (!obj)
        return;
    const long i = GetObject(firstLrObject, level);
    object[i].ro = obj;
}

// Delete object
void WorldMap::DeleteObject(WdmRenderObject *obj)
{
    if (!obj)
        return;
    // go through all the lists, deleting the entry about the object
    for (long i = firstObject, j; i >= 0;)
    {
        j = i;
        i = object[i].next;
        if (object[j].ro == obj)
            FreeObject(firstObject, j);
    }
    for (long i = firstPrObject, j; i >= 0;)
    {
        j = i;
        i = object[i].next;
        if (object[j].ro == obj)
            FreeObject(firstPrObject, j);
    }

    for (long i = firstMrObject, j; i >= 0;)
    {
        j = i;
        i = object[i].next;
        if (object[j].ro == obj)
            FreeObject(firstMrObject, j);
    }
    for (long i = firstLrObject, j; i >= 0;)
    {
        j = i;
        i = object[i].next;
        if (object[j].ro == obj)
            FreeObject(firstLrObject, j);
    }
    delete obj;
}

// ============================================================================================
// Encapsulation
// ============================================================================================

// Objects management

// Include a record about an object in the list with the required level
long WorldMap::GetObject(long &first, long level)
{
    Assert(firstFreeObject >= 0);
    const long i = firstFreeObject;
    firstFreeObject = object[firstFreeObject].next;
    object[i].ro = nullptr;
    object[i].level = level;
    object[i].prev = -1;
    object[i].next = -1;
    if (first >= 0)
    {
        if (level >= object[first].level)
        {
            long j;
            for (j = first; object[j].next >= 0 && level >= object[object[j].next].level; j = object[j].next)
                ;
            object[i].prev = j;
            object[i].next = object[j].next;
            object[j].next = i;
            if (object[i].next >= 0)
                object[object[i].next].prev = i;
        }
        else
        {
            object[i].next = first;
            first = i;
        }
    }
    else
        first = i;
    return i;
}

// Exclude entry from the list
void WorldMap::FreeObject(long &first, long i)
{
    Assert(i >= 0.0f && i < WDMAP_MAXOBJECTS);
    object[i].ro = nullptr;
    object[i].level = 0;
    if (object[i].next >= 0)
        object[object[i].next].prev = object[i].prev;
    if (object[i].prev >= 0)
        object[object[i].prev].next = object[i].next;
    else
        first = object[i].next;
    object[i].prev = -1;
    object[i].next = firstFreeObject;
    firstFreeObject = i;
}

// Utilities

// Initialize the model and add it to the required render lists
WdmRenderObject *WorldMap::CreateModel(WdmRenderModel *rm, const char *modelName, bool pr, bool mr, bool lr,
                                       long objectLevel, long drawLevel)
{
    if (!modelName || !modelName[0])
    {
        delete rm; // boal fix needs to be deleted
        return nullptr;
    }
    if (!rm->Load(modelName))
    {
        delete rm;
        return nullptr;
    }
    AddObject(rm, objectLevel);
    if (pr)
        AddPObject(rm, drawLevel);
    if (mr)
        AddMObject(rm, drawLevel);
    if (lr)
        AddLObject(rm, drawLevel);
    return rm;
}

// Create a storm if possible and set a lifetime
bool WorldMap::CreateStorm(bool isTornado, float time, Attribute *save)
{
    if (wdmObjects->storms.size() >= WDM_MAX_STORMS)
        return false;
    auto *s = new WdmStorm();
    AddLObject(s, 800);
    if (!AddObject(s))
        return false;
    if (time > 0.0f)
    {
        if (time < 1.0f)
            time = 1.0f;
        s->SetLiveTime(time);
    }
    if (!save)
        save = GetEncSaveData("Storm", "EncounterID1");
    s->SetSaveAttribute(save);
    s->isTornado = isTornado;
    return true;
}

// Create a merchant's ship
bool WorldMap::CreateMerchantShip(const char *modelName, const char *locNameStart, const char *locNameEnd, float kSpeed,
                                  float time, Attribute *save)
{
    if (kSpeed < 0.1f)
        kSpeed = 0.1f;
    WdmShip *ship = new WdmMerchantShip();
    if (ship->killMe)
    {
        delete ship;
        return false;
    }
    if (!CreateModel(ship, modelName))
        return false;
    AddLObject(ship, 100);
    // Looking for a place to sail
    if (!wdmObjects->islands)
    {
        core.Trace("World map: Islands not found");
    }
    CVECTOR gpos;
    if (!locNameEnd || !locNameEnd[0])
    {
        if (wdmObjects->islands)
        {
            if (!wdmObjects->islands->GetRandomMerchantPoint(gpos))
            {
                core.Trace("World map: Locators <Merchants:...> not found");
            }
        }
    }
    else
    {
        if (wdmObjects->islands)
        {
            if (!wdmObjects->islands->GetQuestLocator(locNameEnd, gpos))
            {
                core.Trace("World map: Quest locator <Quests:%s> for merchant not found", locNameEnd);
            }
        }
    }
    static_cast<WdmMerchantShip *>(ship)->Goto(gpos.x, gpos.z, 2.0f);
    // If necessary, change the current position
    if (locNameStart && locNameStart[0])
    {
        if (wdmObjects->islands)
        {
            if (wdmObjects->islands->GetQuestLocator(locNameStart, gpos))
            {
                ship->Teleport(gpos.x, gpos.z, rand() * (PI * 2.0f / RAND_MAX));
            }
            else
            {
                core.Trace("World map: Quest locator <Quests:%s> for merchant not found", locNameStart); // boal fix
            }
        }
    }
    // Speed
    ship->SetMaxSpeed(kSpeed);
    // Lifetime
    if (time >= 0.0f)
    {
        if (time < 3.0f)
            time = 3.0f;
        static_cast<WdmEnemyShip *>(ship)->SetLiveTime(time);
    }
    if (!save)
        save = GetEncSaveData("Merchant", "EncounterID1");
    if (save)
    {
        save->getProperty("modelName") = (char *)modelName;
    }
    static_cast<WdmEnemyShip *>(ship)->SetSaveAttribute(save);
    return true;
}

// boal Create a merchant's ship in coordinates
bool WorldMap::CreateMerchantShipXZ(const char *modelName, float x1, float z1, float x2, float z2, float kSpeed,
                                    float time, Attribute *save)
{
    if (kSpeed < 0.1f)
        kSpeed = 0.1f;
    WdmShip *ship = new WdmMerchantShip();
    if (ship->killMe)
    {
        delete ship;
        return false;
    }
    if (!CreateModel(ship, modelName))
        return false;
    AddLObject(ship, 100);
    // Looking for a place to sail
    if (!wdmObjects->islands)
    {
        core.Trace("World map: Islands not found");
    }

    static_cast<WdmMerchantShip *>(ship)->Goto(x2, z2, 2.0f); // where
    // If necessary, change the current position
    ship->Teleport(x1, z1, rand() * (PI * 2.0f / RAND_MAX)); // from where
    // Speed
    ship->SetMaxSpeed(kSpeed);
    // Lifetime
    if (time >= 0.0f)
    {
        if (time < 3.0f)
            time = 3.0f;
        static_cast<WdmEnemyShip *>(ship)->SetLiveTime(time);
    }
    if (!save)
        save = GetEncSaveData("Merchant", "EncounterID1");
    if (save)
    {
        save->getProperty("modelName") = (char *)modelName;
    }
    static_cast<WdmEnemyShip *>(ship)->SetSaveAttribute(save);
    return true;
}

// Create a ship that follows us
bool WorldMap::CreateFollowShip(const char *modelName, float kSpeed, float time, Attribute *save)
{
    if (kSpeed < 0.1f)
        kSpeed = 0.1f;
    WdmShip *ship = new WdmFollowShip();
    if (ship->killMe)
    {
        delete ship;
        return false;
    }
    if (!CreateModel(ship, modelName))
        return false;
    AddLObject(ship, 100);
    // Speed
    ship->SetMaxSpeed(kSpeed);
    // Lifetime
    if (time >= 0.0f)
    {
        if (time < 1.0f)
            time = 1.0f;
        static_cast<WdmEnemyShip *>(ship)->SetLiveTime(time);
    }
    VDATA *isSkipEnable = core.Event("WorldMap_IsSkipEnable");
    if (isSkipEnable)
    {
        long skipEnable = 0;
        if (isSkipEnable->Get(skipEnable))
        {
            static_cast<WdmEnemyShip *>(ship)->canSkip = skipEnable != 0;
        }
    }
    if (!save)
        save = GetEncSaveData("Follow", "EncounterID1");
    if (save)
    {
        save->getProperty("modelName") = (char *)modelName;
    }
    static_cast<WdmEnemyShip *>(ship)->SetSaveAttribute(save);
    return true;
}

bool WorldMap::CreateWarringShips(const char *modelName1, const char *modelName2, float time, Attribute *save1,
                                  Attribute *save2)
{
    static const float pi = 3.14159265359f;
    // Create ships
    auto *ship1 = new WdmWarringShip();
    if (ship1->killMe)
    {
        delete ship1;
        return false;
    }
    if (!CreateModel(ship1, modelName1))
        return false;
    auto *ship2 = new WdmWarringShip();
    if (ship2->killMe)
    {
        delete ship2;
        return false;
    }
    if (!CreateModel(ship2, modelName2))
        return false;
    const float moveRadius = (ship1->modelRadius + ship2->modelRadius) * (0.4f + (rand() & 3) * (0.1f / 3.0f));
    const float fullRadius = 0.6f * (moveRadius + 2.0f * std::max(ship1->modelRadius, ship2->modelRadius));
    // General position
    float x, z;
    if (!WdmEnemyShip::GeneratePosition(fullRadius, 1.5f, x, z))
        return false;
    // General angle
    const float angl = rand() * 2.0f * pi / (RAND_MAX + 1);
    // Center offset
    const float dx = moveRadius * cosf(angl);
    const float dz = -moveRadius * sinf(angl);
    AddLObject(ship1, 100);
    ship1->Teleport(x + dx, z + dz, angl + pi * (rand() & 1));
    AddLObject(ship2, 100);
    ship2->Teleport(x - dx, z - dz, angl + pi * (rand() & 1));
    ship2->SetLiveTime(ship1->GetLiveTime());
    // Setting characteristics
    ship1->attack = ship2;
    ship2->attack = ship1;
    // Lifetime
    if (time >= 0.0f)
    {
        if (time < 1.0f)
            time = 1.0f;
        ship1->SetLiveTime(time);
        ship2->SetLiveTime(time);
    }
    if (!save2)
        save2 = GetEncSaveData("Attacked", "EncounterID1");
    if (!save1)
        save1 = GetEncSaveData("Warring", "EncounterID2");
    if (save1 && save2)
    {
        save1->getProperty("attacked") = save2->getName();
    }
    if (save1)
    {
        save1->getProperty("modelName") = (char *)modelName1;
    }
    if (save2)
    {
        save2->getProperty("modelName") = (char *)modelName2;
    }
    ship1->SetSaveAttribute(save1);
    ship2->SetSaveAttribute(save2);
    return true;
}

// Find coordinates and radius by destination
bool WorldMap::FindIslandPosition(const char *name, float &x, float &z, float &r)
{
    return false;
}

void WorldMap::ResetScriptInterfaces() const
{
    if (aStorm)
    {
        aStorm->getProperty("num") = 0;
        aStorm->getProperty("cur") = 0;
        aStorm->getProperty("x") = 0;
        aStorm->getProperty("z") = 0;
        aStorm->getProperty("time") = 0;
    }
    if (aEncounter)
    {
        aEncounter->getProperty("num") = 0;
        aEncounter->getProperty("cur") = 0;
        aEncounter->getProperty("x") = 0;
        aEncounter->getProperty("z") = 0;
        aEncounter->getProperty("ay") = 0;
        aEncounter->getProperty("time") = 0;
        aEncounter->getProperty("type") = "-1";
        aEncounter->getProperty("attack") = -1;
        aEncounter->getProperty("nation") = -1;
        aEncounter->getProperty("id") = "";
    }
    if (aInfo)
    {
        aInfo->getProperty("playerInStorm") = 0;
    }
}

// Delete all encounters
void WorldMap::ReleaseEncounters()
{
    // leave the encounter parameters intact
    for (long i = 0; i < wdmObjects->ships.size(); i++)
    {
        if (wdmObjects->ships[i] == wdmObjects->playerShip)
            continue;
        static_cast<WdmEnemyShip *>(wdmObjects->ships[i])->SetSaveAttribute(nullptr);
        wdmObjects->ships[i]->killMe = true;
    }
    for (long i = 0; i < wdmObjects->storms.size(); i++)
    {
        wdmObjects->storms[i]->SetSaveAttribute(nullptr);
        wdmObjects->storms[i]->killMe = true;
    }
}

// Create an attribute to save the encounter parameters
Attribute *WorldMap::GetEncSaveData(const char *type, const char *retName)
{
    if (!saveData)
        return nullptr;
    // Generating the name of the attribute
    encCounter++;
    char atrName[64];
    long i;
    for (i = 0; i < 1000000; i++, encCounter++)
    {
        sprintf_s(atrName, "enc_%u", encCounter);
        Attribute& attr = saveData->getProperty(atrName);
        if (attr.empty())
            break;
        if (attr.hasProperty("needDelete"))
        {
            attr.clear();
            break;
        }
    }
    if (i == 1000000)
        return nullptr;
    // Create a branch
    Attribute& newAttr = saveData->getProperty(atrName);
    // Set the type
    newAttr["type"] = type;
    // Save the name
    if (AttributesPointer)
    {
        AttributesPointer->getProperty(retName) = atrName;
    }
    return &newAttr;
}
