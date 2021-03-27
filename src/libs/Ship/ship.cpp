#include "ship.h"
#include "../../Shared/messages.h"
#include "../../Shared/sea_ai/Script_defines.h"
#include "../../Shared/sound.h"
#include "../../shared/mast_msg.h"
#include "../../shared/sail_msg.h"
#include "../Sea_ai/AIFlowGraph.h"
#include "Track.h"
#include "inlines.h"

#include "../apps/ENGINE/compatibility.hpp"

VDX9RENDER *SHIP::pRS = nullptr;
SEA_BASE *SHIP::pSea = nullptr;
ISLAND_BASE *SHIP::pIsland = nullptr;
COLLIDE *SHIP::pCollide = nullptr;
VGEOMETRY *SHIP::pGS = nullptr;

SHIP::SHIP()
{
    pShipsLights = nullptr;
    vSpeedAccel = 0.0f;
    uniIDX = 0;
    bUse = false;
    bDead = false;
    fSailState = 0.0f;
    bShip2Strand = false;
    bSetLightAndFog = false;
    iNumMasts = 0;
    iNumHulls = 0;

    bSetFixed = false;
    fFixedSpeed = 0.0f;

    bMassaShow = false;

    dtMastTrace.Setup(FRAND(0.5f), 0.5f);
    dtUpdateParameters.Setup(FRAND(1.0f), 1.0f);

    vCurDeadDir = 0.0f;

    State.vPos = 0.0f;
    fXOffset = fZOffset = 0.f;

    vSpeed = 0.0f;
    vSpeedsA = 0.0f;
    fMinusVolume = 0.0f;

    mRoot.SetIdentity();

    bVisible = true;
    bMounted = false;
    bKeelContour = false;
    bPerkTurnActive = false;

    fRockingY = 1.0f;
    fRockingAZ = 1.0f;

    bModelUpperShip = false;
    pModelUpperShip = nullptr;
}

SHIP::~SHIP()
{
    EntityManager::EraseEntity(GetModelEID());
    core.Send_Message(sail_id, "li", MSG_SAIL_DEL_GROUP, GetId());
    core.Send_Message(rope_id, "li", MSG_ROPE_DEL_GROUP, GetModelEID());
    core.Send_Message(flag_id, "li", MSG_FLAG_DEL_GROUP, GetModelEID());
    core.Send_Message(vant_id, "li", MSG_VANT_DEL_GROUP, GetModelEID());
    core.Send_Message(vantl_id, "li", MSG_VANT_DEL_GROUP, GetModelEID());
    core.Send_Message(vantz_id, "li", MSG_VANT_DEL_GROUP, GetModelEID());
    EntityManager::EraseEntity(blots_id);

    if (const auto eidTmp = EntityManager::GetEntityId("ShipTracks"))
    {
        auto *pST = static_cast<ShipTracks *>(EntityManager::GetEntityPointer(eidTmp));
        if (pST)
            pST->DelShip(this);
    }

    // aFirePlaces.clear();
}

//##################################################################
bool SHIP::Init()
{
    ZERO3(State, SP, vPos);
    ZERO3(vAng, ShipPoints, Strength);
    fXOffset = fZOffset = 0.f;

    Strength[STRENGTH_MAIN].bInertia = true;
    Strength[STRENGTH_MAIN].vSpeed = 0.0f;
    Strength[STRENGTH_MAIN].vRotate = 0.0f;

    srand(GetTickCount());

    LoadServices();

    const auto priorityExecutePtr = static_cast<VDATA *>(core.GetScriptVariable("iShipPriorityExecute"));
    if (priorityExecutePtr) {
        iShipPriorityExecute = priorityExecutePtr->GetLong();
    }
    else {
        iShipPriorityExecute = 2;
    }

    const auto priorityRealizePtr = static_cast<VDATA *>(core.GetScriptVariable("iShipPriorityRealize"));
    if (priorityRealizePtr) {
        iShipPriorityRealize = priorityRealizePtr->GetLong();
    }
    else {
        iShipPriorityRealize = 31;
    }

    return true;
}

void SHIP::LoadServices()
{
    pIsland = nullptr;
    pSea = nullptr;
    pRS = nullptr;
    pGS = nullptr;

    pGS = static_cast<VGEOMETRY *>(core.CreateService("geometry"));
    Assert(pGS);
    pRS = static_cast<VDX9RENDER *>(core.CreateService("dx9render"));
    Assert(pRS);
    pCollide = static_cast<COLLIDE *>(core.CreateService("coll"));
    Assert(pCollide);

    touch_id = EntityManager::GetEntityId("touch");

    pIsland = static_cast<ISLAND_BASE *>(EntityManager::GetEntityPointer(EntityManager::GetEntityId("island")));
    if (sea_id = EntityManager::GetEntityId("sea"))
        pSea = static_cast<SEA_BASE *>(EntityManager::GetEntityPointer(sea_id));

    FirePlace::eidSound = EntityManager::GetEntityId("sound");
}

CVECTOR SHIP::ShipRocking(float fDeltaTime)
{
    CMatrix mat;
    mat = *GetMatrix();

    fDeltaTime = Min(fDeltaTime, 0.1f);
    auto fDelta = (fDeltaTime / 0.025f);

    if (!pSea)
        return vAng;
    // core.Send_Message(model,"ll",2,(long)&mat);

    auto vAng2 = State.vAng;

    CVECTOR point, fang;
    fang.x = 0.0f;
    fang.y = 0.0f;
    fang.z = 0.0f;
    auto fFullY = 0.0f, fup = 0.0f;

    long ix, iz;

    auto fCos = cosf(State.vAng.y);
    auto fSin = sinf(State.vAng.y);

    for (ix = 0; ix < 6; ix++)
    {
        auto x = (static_cast<float>(ix) * State.vBoxSize.x * 0.2f - 0.5f * State.vBoxSize.x);
        for (iz = 0; iz < 6; iz++)
        {
            auto *pspt = &ShipPoints[ix][iz];
            auto z = (static_cast<float>(iz) * State.vBoxSize.z * 0.2f - 0.5f * State.vBoxSize.z);

            auto xx = x, zz = z;
            RotateAroundY(xx, zz, fCos, fSin);
            point.x = xx + State.vPos.x + fXOffset;
            point.y = mat.m[0][1] * x + mat.m[2][1] * z + State.vPos.y;
            point.z = zz + State.vPos.z + fZOffset;

            auto wave_y = pSea->WaveXZ(point.x, point.z);
            auto f = (wave_y - point.y);

            pspt->fY = wave_y;

            fFullY += pspt->fY;
        }
    }

    auto fNewPos = fFullY / 36.0f;
    //(ShipPoints[2][2].fY + ShipPoints[3][2].fY + ShipPoints[2][3].fY + ShipPoints[3][3].fY) / 4.0f;
    State.vPos.y = State.vPos.y + Min(fDelta * fRockingY, 1.0f) * (fNewPos - State.vPos.y);

    // calculate ang.x and ang.z
    auto fAng = 0.0f;
    for (iz = 0; iz < 3; iz++)
    {
        auto fHeight = 0.0f, fHeight1 = 0.0f;
        for (ix = 0; ix < 6; ix++)
        {
            fHeight += ShipPoints[ix][iz].fY;
            fHeight1 += ShipPoints[ix][5 - iz].fY;
        }
        fHeight /= 6.0f;
        fHeight1 /= 6.0f;

        auto z = 1.0f / (0.5f * State.vBoxSize.z - static_cast<float>(iz) * State.vBoxSize.z * 0.2f);
        fAng += static_cast<float>(atan((fHeight - fHeight1) / 2.0f * z));
    }
    vAng2.x = fAng / 3.0f;

    fAng = 0.0f;
    for (ix = 0; ix < 3; ix++)
    {
        auto fHeight = 0.0f, fHeight1 = 0.0f;
        for (iz = 0; iz < 6; iz++)
        {
            fHeight += ShipPoints[ix][iz].fY;
            fHeight1 += ShipPoints[5 - ix][iz].fY;
        }
        fHeight /= 6.0f;
        fHeight1 /= 6.0f;

        auto x = 1.0f / (0.5f * State.vBoxSize.x - static_cast<float>(ix) * State.vBoxSize.x * 0.2f);
        fAng += static_cast<float>(atan((fHeight - fHeight1) / 2.0f * x));
    }
    vAng2.z = -fAng / 3.0f;

    return vAng + Min(fRockingAZ * fDelta, 1.0f) * (vAng2 - vAng);
}

BOOL SHIP::CalculateNewSpeedVector(CVECTOR *Speed, CVECTOR *Rotate)
{
    long i;
    CVECTOR result(0.0f, 0.0f, 0.0f);
    for (i = 0; i < RESERVED_STRENGTH; i++)
    {
        *Speed += Strength[i].vSpeed;
        *Rotate += Strength[i].vRotate;
    }

    Speed->z *= GetMaxSpeedZ();
    Rotate->y *= GetMaxSpeedY();

    return true;
}

BOOL SHIP::ApplyStrength(float dtime, BOOL bCollision)
{
    float sign, fK;
    long i;

    // apply impulse strength
    for (i = RESERVED_STRENGTH; i < MAX_STRENGTH; i++)
        if (Strength[i].bUse && !Strength[i].bInertia)
        {
            // State.vRotate.x += Strength[i].vSpeed.z * 1.3f;
            State.vSpeed += Strength[i].vSpeed;
            State.vRotate += Strength[i].vRotate;
            if (!bCollision)
                Strength[i].bUse = false;
        }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // !                   Limitation  Section                      !
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // must use calculated parameters!!!!!
    /*if (State.vSpeed.x>5.0f) State.vSpeed.x = 5.0f;
    if (State.vSpeed.x<-5.0f) State.vSpeed.x = -5.0f;
    if (State.vSpeed.z>10.0f) State.vSpeed.z = 10.0f;
    if (State.vSpeed.z<-10.0f) State.vSpeed.z = -10.0f;
    if (State.vRotate.y>0.1f) State.vRotate.y = 0.1f;
    if (State.vRotate.y<-0.1f) State.vRotate.y = -0.1f; */

    // calculate new speed,rotate vector
    auto new_speed = CVECTOR(0.0f, 0.0f, 0.0f), new_rotate = CVECTOR(0.0f, 0.0f, 0.0f);

    CalculateNewSpeedVector(&new_speed, &new_rotate);
    if (bPerkTurnActive)
    {
        new_rotate.y = SIGN(fInitialPerkAngle) * GetMaxSpeedY();
    }

#define SMALL_DELTA(x) (fabsf((x)-0.00001f) > 0.00001f)
    for (i = 0; i < 3; i++)
    {
        fK = dtime * State.fMassInertia;
        if (i == 0) // x moving
        {
            const auto force = new_speed.v[i] - State.vSpeed.v[i];
            if (SMALL_DELTA(new_speed.v[i]))
                State.vSpeed.v[i] += fK * force * State.vInertiaAccel.x;
            else
                State.vSpeed.v[i] += fK * -(State.vSpeed.v[i]) * State.vInertiaBrake.x;
        }
        if (i == 2) // z moving
        {
            vSpeedAccel.z = State.vSpeed.v[i];

            const auto force = new_speed.v[i] - State.vSpeed.v[i];
            if (SMALL_DELTA(new_speed.v[i]))
                State.vSpeed.v[i] += fK * force * State.vInertiaAccel.z;
            else
            {
                State.vSpeed.v[i] += fK * -(State.vSpeed.v[i]) * State.vInertiaBrake.z;
            }
            // apply water resistance

            /*State.vSpeed.v[i] += dtime * State.fMassInertia * State.vWaterResis.x * force;*/
            vSpeedAccel.z -= State.vSpeed.v[i];
        }

        sign = SIGNZ(new_rotate.v[i]);
        if (i == 1) // y rotating
        {
            auto prev_rot = State.vRotate.v[i];
            const auto force = new_rotate.v[i] - State.vRotate.v[i];

            if (SMALL_DELTA(new_rotate.v[i]))
                State.vRotate.v[i] += fK * force * State.vInertiaAccel.y;
            else
                State.vRotate.v[i] += fK * -State.vRotate.v[i] * State.vInertiaBrake.y;
        }
    }

    return true;
}

BOOL SHIP::TouchMove(uint32_t DeltaTime, TOUCH_PARAMS *pTPOld, TOUCH_PARAMS *pTPNew)
{
    if (!pTPOld && !pTPNew)
        return false;
    const auto old_state = State;

    if (pTPOld)
    {
        pTPOld->vSpeed = State.vSpeed;
        pTPOld->vRotate = State.vRotate;
        pTPOld->vPos = State.vPos;
        pTPOld->vAng = State.vAng;
        pTPOld->vPos.x += fXOffset;
        pTPOld->vPos.z += fZOffset;
    }

    Move(DeltaTime, true);

    if (pTPNew)
    {
        pTPNew->vSpeed = State.vSpeed;
        pTPNew->vRotate = State.vRotate;
        pTPNew->vPos = State.vPos;
        pTPNew->vAng = State.vAng;
        pTPNew->vPos.x += fXOffset;
        pTPNew->vPos.z += fZOffset;
    }

    State = old_state;

    return true;
}

BOOL SHIP::Move(uint32_t DeltaTime, BOOL bCollision)
{
    const auto dtime = DELTA_TIME(DeltaTime);
    ApplyStrength(dtime, bCollision);
    if (!bCollision && !isDead())
    {
        State.vAng = ShipRocking(dtime);
    }

    // Update position
    // if (!isDead())
    {
        auto k = KNOTS2METERS(((bSetFixed) ? fFixedSpeed : State.vSpeed.z)) * dtime;
        State.vPos.z += k * cosf(State.vAng.y);
        State.vPos.x += k * sinf(State.vAng.y);

        k = KNOTS2METERS(State.vSpeed.x) * dtime;
        State.vPos.z += k * cosf(State.vAng.y + PId2);
        State.vPos.x += k * sinf(State.vAng.y + PId2);

        State.vAng.y += State.vRotate.y * dtime;
    }

    return true;
}

#define SHIP_TEST_Z_TIME 1.0f
#define SHIP_TEST_Y_TIME 1.0f

float SHIP::GetRotationAngle(float *pfTime)
{
    if (pfTime)
        *pfTime = 0.0f;
    if (fabsf(State.vRotate.y) < 0.001f)
        return 0.0f;
    auto fAngle = 0.0f;

    auto fDist = 0.0f, fTime = SHIP_TEST_Y_TIME;
    auto fSpeedY = State.vRotate.y;
    const auto fK = SHIP_TEST_Y_TIME * State.fMassInertia * State.vInertiaBrake.y;

    while (true) // not funny!!! IT'S SLOW!!!!
    {
        fSpeedY += fK * -(fSpeedY);
        fAngle += fSpeedY;
        if (fabsf(fSpeedY) < 0.001f)
            break;
        fTime += SHIP_TEST_Y_TIME;
    }
    if (pfTime)
        *pfTime = fTime;
    return fabsf(fAngle);
}

float SHIP::GetBrakingDistance(float *pfTime)
{
    if (pfTime)
        *pfTime = 0.0f;
    if (State.vSpeed.z < 0.01f)
        return 0.0f;
    auto fDist = 0.0f, fTime = SHIP_TEST_Z_TIME;
    auto fSpeedZ = State.vSpeed.z;
    const auto fK = SHIP_TEST_Z_TIME * State.fMassInertia * State.vInertiaBrake.z;

    while (true) // not funny!!! IT'S SLOW!!!!
    {
        fSpeedZ += fK * -(fSpeedZ);
        fDist += KNOTS2METERS(fSpeedZ);
        if (fSpeedZ < 0.001f)
            break;
        fTime += SHIP_TEST_Z_TIME;
    }
    if (pfTime)
        *pfTime = fTime;
    return fDist;
}

// calculate ship immersion
void SHIP::CalculateImmersion()
{
    GetACharacter()->getProperty("Ship")["Immersion"].get_to(State.fShipImmersion, 0.0f);
    // return State.fShipImmersion;
}

void SHIP::CheckShip2Strand(float fDeltaTime)
{
    if (!pIsland)
        return;
    auto *pModel = GetModel();
    Assert(pModel);
    auto bNewShip2Strand = false;
    for (long i = 0; i < MAX_KEEL_POINTS; i++)
    {
        float fRes;
        const auto vTmp = pModel->mtx * vKeelContour[i];
        if (!pIsland->GetDepth(vTmp.x, vTmp.z, &fRes))
            continue;
        if (fRes < -10.0f && (fRes + 10.0f) >= vTmp.y)
        {
            bNewShip2Strand = true;
            break;
        }
    }
    if (bShip2Strand != bNewShip2Strand)
    {
        bShip2Strand = bNewShip2Strand;
        core.Event(SHIP_TO_STRAND, "ll", GetIndex(GetACharacter()), bShip2Strand);
    }
}

void SHIP::SetDead()
{
    if (!isDead())
    {
        State.vAng = vAng;

        vOldAng = State.vAng;
        vOldPos = State.vPos;

        Attribute& aShipSinkSpeed = GetACharacter()->getProperty("ship")["sink"]["speed"];

        // aref aSink; makearef(aSink, rDead.Ship.Sink);
        if (aShipSinkSpeed.empty())
        {
            aShipSinkSpeed["y"] = 0.35f;
            aShipSinkSpeed["x"] = 0.017f * (FRAND(1.0f) * 2.0f - 1.0f);
            aShipSinkSpeed["z"] = 0.017f * (FRAND(1.0f) * 2.0f - 1.0f);
        }

        // Assert(pASinkSpeed);

        aShipSinkSpeed.get_to(vDeadDir);
        vCurDeadDir = 0.0f;

        bDead = true;
        EntityManager::RemoveFromLayer(SEA_REFLECTION2, GetId());

        if (pShipsLights)
            pShipsLights->SetDead(this);
    }
}

void SHIP::Execute(uint32_t DeltaTime)
{
    Attribute &aPerks = GetACharacter()->getProperty("TmpPerks");
    const Attribute &aRocking = GetAShip()->getProperty("Rocking");

    if (!aRocking.empty())
    {
        aRocking["y"].get_to(fRockingY, 1.0f);
        aRocking["az"].get_to(fRockingAZ, 1.0f);
    }

    // check fast perk turn
    if (!aPerks.empty() && !bPerkTurnActive)
    {
        aPerks.get_to(bPerkTurnActive);
        if (bPerkTurnActive)
        {
            aPerks["turn"].get_to(fInitialPerkAngle);
            fResultPerkAngle = State.vAng.y + fInitialPerkAngle;
        }
    }

    // check for end of fast turn perk
    if (bPerkTurnActive)
    {
        auto vCurDir = CVECTOR(sinf(State.vAng.y), 0.0f, cosf(State.vAng.y));
        auto vResDir = CVECTOR(sinf(fResultPerkAngle), 0.0f, cosf(fResultPerkAngle));
        if ((vResDir | vCurDir) >= 0.9f)
        {
            bPerkTurnActive = false;
            aPerks["turn"] = -1;
        }
    }

    // temp
    uniIDX = GetACharacter()->getProperty("index").get<long>();
    if (uniIDX >= 900)
        uniIDX = uniIDX - 900 + 2;

    auto fDeltaTime = Min(0.1f, static_cast<float>(DeltaTime) * 0.001f);

    if (!bMounted)
        return;

    Attribute &aShip = GetACharacter()->getProperty("ship");
    const Attribute &aShipStopped = aShip["stopped"];
    const auto bMainCharacter = GetACharacter()->getProperty("MainCharacter").get<bool>(false);

    if (dtUpdateParameters.Update(fDeltaTime))
    {
        if (!aShipStopped.get<bool>(false))
        {
            core.Event(SHIP_UPDATE_PARAMETERS, "lf", GetIndex(GetACharacter()), fSailState);
        }
    }
    // check impulse
    Attribute &aImpulse = aShip["Impulse"];
    if (!aImpulse.empty() && !isDead())
    {
        CVECTOR vRotate = 0.0f, vXSpeed = 0.0f;
        aImpulse["Rotate"].get_to(vRotate, {0.f, 0.f, 0.f});
        aImpulse["Speed"].get_to(vXSpeed, {0.f, 0.f, 0.f});
        aImpulse.clear();

        STRENGTH st;
        st.bInertia = false;
        st.bUse = true;
        st.vRotate = vRotate * fDeltaTime;
        st.vSpeed = vXSpeed * fDeltaTime;
        AddStrength(&st);
    }

    // if (DeltaTime==0) _asm int 3
    CalculateImmersion(); //

    core.Send_Message(sail_id, "lipf", MSG_SAIL_GET_SPEED, GetId(), &Strength[STRENGTH_MAIN].vSpeed.z, fSailState);
    if (isDead())
        Strength[STRENGTH_MAIN].vSpeed.z = 0.0f;

    vPos = State.vPos;
    vAng = State.vAng;

    Move(DeltaTime, false);

    if (!isDead())
    {
        auto fRotate = State.vRotate.y;
        if (fRotate > 0.25f)
            fRotate = 0.25f;
        if (fRotate < -0.25f)
            fRotate = -0.25f;
        vAng.z += fRotate;
    }

    Attribute &aSpeed = aShip["speed"];
    aSpeed["x"] = State.vSpeed.x;
    aSpeed["y"] = State.vRotate.y;
    aSpeed["z"] = State.vSpeed.z;
    /*if (isDead())
    {
      CVECTOR vAng2 = ShipRocking(fDeltaTime);

      float fScale = Bring2Range(8.0f, 22.0f, -5.0f, 0.0f, vPos.y);
      fMinusVolume += fScale * fDeltaTime * 0.5f;
      State.vAng = vAng2;

      vCurDeadDir += vDeadDir * fDeltaTime;

      State.vSpeed.z -= State.vSpeed.z * 0.3f * fDeltaTime;

      if (isVisible() && vPos.y < -80.0f)
      {
        bVisible = false;

        // stop all fireplaces
        for (uint32_t i=0; i<aFirePlaces.size(); i++) aFirePlaces[i].Stop();

        // del vant,flags,sail and ropes
        core.Send_Message(sail_id, "li", MSG_SAIL_DEL_GROUP, GetId());
        core.Send_Message(rope_id, "li", MSG_ROPE_DEL_GROUP, GetModelEID());
        core.Send_Message(flag_id, "li", MSG_FLAG_DEL_GROUP, GetModelEID());
        core.Send_Message(vant_id, "li", MSG_VANT_DEL_GROUP, GetModelEID());

        core.LayerDel((char*)sRealizeLayer.c_str(), GetId());
        core.LayerDel("ship_cannon_trace", GetId());
        core.Event(SHIP_DELETE, "li", GetIndex(GetACharacter()), GetId());
      }

      if (vPos.y < -100.0f) vPos.y = -1000.0f;
    }*/

    if (isDead())
    {
        auto fScale = Bring2Range(0.0f, 1.0f, -15.0f, 0.0f, vPos.y);

        // State.vPos.y = 0.0f;
        // State.vAng.x = 0.0f;
        // State.vAng.z = 0.0f;

        State.vAng = vOldAng;
        State.vPos = vOldPos;

        State.vAng = ShipRocking(fDeltaTime);

        vOldAng = State.vAng;
        vOldPos = State.vPos;

        vCurDeadDir.x += vDeadDir.x * fDeltaTime;
        vCurDeadDir.y += vDeadDir.y * fDeltaTime;
        vCurDeadDir.z += vDeadDir.z * fDeltaTime;

        /*State.vPos.y *= fScale;        State.vPos.y -= vCurDeadDir.y;
        State.vAng.x *= fScale;        State.vAng.x += vCurDeadDir.x;
        State.vAng.z *= fScale;        State.vAng.z += vCurDeadDir.z;*/

        State.vPos.y -= vCurDeadDir.y;
        // State.vAng.x += vCurDeadDir.x;
        // State.vAng.z += vCurDeadDir.z;

        vAng.z = State.vAng.z;
        vAng.x = State.vAng.x;
        vPos.y = State.vPos.y;

        if (isVisible() && vPos.y < -50.0f)
        {
            bVisible = false;

            // stop all fireplaces
            for (uint32_t i = 0; i < aFirePlaces.size(); i++)
                aFirePlaces[i].Stop();

            // del vant,flags,sail and ropes
            core.Send_Message(sail_id, "li", MSG_SAIL_DEL_GROUP, GetId());
            core.Send_Message(rope_id, "li", MSG_ROPE_DEL_GROUP, GetModelEID());
            core.Send_Message(flag_id, "li", MSG_FLAG_DEL_GROUP, GetModelEID());
            core.Send_Message(vant_id, "li", MSG_VANT_DEL_GROUP, GetModelEID());
            core.Send_Message(vantl_id, "li", MSG_VANT_DEL_GROUP, GetModelEID());
            core.Send_Message(vantz_id, "li", MSG_VANT_DEL_GROUP, GetModelEID());

            EntityManager::RemoveFromLayer(RealizeLayer, GetId());
            EntityManager::RemoveFromLayer(SHIP_CANNON_TRACE, GetId());
            core.Event(SHIP_DELETE, "li", GetIndex(GetACharacter()), GetId());
        }
    }
    else
    {
    }

    // set attributes for script
    aShip["pos"].get_to(State.vPos);
    State.vPos.x += fXOffset;
    State.vPos.z += fZOffset;
    aShip["ang"].get_to(State.vAng);

    // check strand
    if (!bDead && bKeelContour)
        CheckShip2Strand(fDeltaTime);

    RecalculateWorldOffset();
    mRoot = CMatrix(vAng.x + vCurDeadDir.x, vAng.y, vAng.z + vCurDeadDir.z, vPos.x + fXOffset,
                    vPos.y - State.fShipImmersion - SP.fWaterLine, vPos.z + fZOffset);
    SetMatrix(mRoot);

    auto matrix = UpdateModelMatrix();

    // activate mast tracer
    if (dtMastTrace.Update(fDeltaTime))
    {
        for (long i = 0; i < iNumMasts; i++)
            if (!pMasts[i].bBroken)
            {
                CVECTOR v1, v2;
                VDATA *pV;
                auto fShipRes = 2.0f, fIslRes = 2.0f;

                auto *pM = &pMasts[i];
                v1 = matrix * pM->vSrc;
                v2 = matrix * pM->vDst;

                auto id = GetId();
                fShipRes = pCollide->Trace(EntityManager::GetEntityIdIterators(MAST_SHIP_TRACE), v1, v2, &id, 1);
                if (fShipRes <= 1.0f)
                {
                    auto *pACollideCharacter = GetACharacter();
                    auto *pShip = static_cast<SHIP *>(EntityManager::GetEntityPointer(pCollide->GetObjectID()));
                    if (pShip)
                        pACollideCharacter = pShip->GetACharacter();
                    pV = core.Event(SHIP_MAST_DAMAGE, "llffffaa", SHIP_MAST_TOUCH_SHIP, pM->iMastNum, v1.x, v1.y, v1.z,
                                    pM->fDamage, GetACharacter(), pACollideCharacter);
                    pM->fDamage = Clamp(pV->GetFloat());
                }

                id = GetModelEID();
                fIslRes = pCollide->Trace(EntityManager::GetEntityIdIterators(MAST_ISLAND_TRACE), v1, v2, &id, 1);
                if (fIslRes <= 1.0f)
                {
                    pV = core.Event(SHIP_MAST_DAMAGE, "llffffa", SHIP_MAST_TOUCH_ISLAND, pM->iMastNum, v1.x, v1.y, v1.z,
                                    pM->fDamage, GetACharacter());
                    pM->fDamage = Clamp(pV->GetFloat());
                }

                MastFall(pM);
            }
    }

    // key states
    Strength[STRENGTH_MAIN].vRotate.y = 0.0f;

    if (bMainCharacter && aShipStopped.get<bool>(false))
    {
        CONTROL_STATE cs, cs1;
        core.Controls->GetControlState("Ship_TurnLeft", cs);
        core.Controls->GetControlState("Ship_TurnLeft1", cs1);
        if (cs.state == CST_ACTIVE || cs1.state == CST_ACTIVE)
            Strength[STRENGTH_MAIN].vRotate.y = -1.0f;
        core.Controls->GetControlState("Ship_TurnRight", cs);
        core.Controls->GetControlState("Ship_TurnRight1", cs1);
        if (cs.state == CST_ACTIVE || cs1.state == CST_ACTIVE)
            Strength[STRENGTH_MAIN].vRotate.y = 1.0f;
        core.Controls->GetControlState("Ship_SailUp", cs);
        core.Controls->GetControlState("Ship_SailUp1", cs1);
        if (cs.state == CST_ACTIVATED || cs1.state == CST_ACTIVATED)
            fSailState += 0.5f;
        core.Controls->GetControlState("Ship_SailDown", cs);
        core.Controls->GetControlState("Ship_SailDown1", cs1);
        if (cs.state == CST_ACTIVATED || cs1.state == CST_ACTIVATED)
            fSailState -= 0.5f;

        if (fSailState < 0.0f)
            fSailState = 0.0f;
        if (fSailState > 1.0f)
            fSailState = 1.0f;
    }

    /*if (isDead())
    {
      fSailState = 0.0f;
    }*/

    // execute fire places
    for (uint32_t i = 0; i < aFirePlaces.size(); i++)
        aFirePlaces[i].Execute(fDeltaTime);

    // water sound: set position and volume
    Attribute &aSounds = aShip["Sounds"];

    if (!aSounds.empty())
    {
        const Attribute &aWaterID = aSounds["WaterID"];
        const Attribute &aTurnID = aSounds["TurnID"];
        const Attribute &aSailsID = aSounds["SailsID"];

        CMatrix mRotate;
        mRoot.Get3X3(mRotate);
        auto vBoxSize = State.vBoxSize / 2.0f;

        auto fWaterSpeed = aSounds["WaterSpeed"].get<float>(40.0f);
        auto iWaterSound = aSounds["WaterID"].get<long>(-1l);;
        if (iWaterSound > 0)
        {
            const auto waterBox = aWaterID.get<CVECTOR>({0.f, 0.f, 0.f});
            auto vPos = State.vPos + (mRotate * CVECTOR(vBoxSize.x * waterBox.x, vBoxSize.y * waterBox.y, vBoxSize.z * waterBox.z));
            auto fWaterSoundVolume = Min(KNOTS2METERS(fabsf(State.vSpeed.z)), fWaterSpeed) / fWaterSpeed;
            core.Send_Message(FirePlace::eidSound, "llf", MSG_SOUND_SET_VOLUME, iWaterSound, fWaterSoundVolume);
            core.Send_Message(FirePlace::eidSound, "lllfff", MSG_SOUND_SET_3D_PARAM, iWaterSound, SOUND_PARAM_POSITION,
                              vPos.x + fXOffset, vPos.y, vPos.z + fZOffset);
        }

        auto fTurnSpeed = aSounds["TurnSpeed"].get<float>(20.0f);
        auto iTurnSound = aSounds["TurnID"].get<long>(-1l);
        if (iTurnSound > 0)
        {
            const auto turnBox = aTurnID.get<CVECTOR>({0.f, 0.f, 0.f});
            auto vPos = State.vPos + (mRotate * CVECTOR(vBoxSize.x * turnBox.x, vBoxSize.y * turnBox.y, vBoxSize.z * turnBox.z));
            auto fTurnSoundVolume = Min(fabsf(State.vRotate.y), fTurnSpeed) / fTurnSpeed;
            core.Send_Message(FirePlace::eidSound, "llf", MSG_SOUND_SET_VOLUME, iTurnSound, fTurnSoundVolume);
            core.Send_Message(FirePlace::eidSound, "lllfff", MSG_SOUND_SET_3D_PARAM, iTurnSound, SOUND_PARAM_POSITION,
                              vPos.x + fXOffset, vPos.y, vPos.z + fZOffset);
        }

        auto iSailSound = aSounds["SailsID"].get<long>(-1l);
        if (iSailSound > 0)
        {
            const auto sailsBox = aSailsID.get<CVECTOR>({0.f, 0.f, 0.f});
            auto vPos = State.vPos + (mRotate * CVECTOR(vBoxSize.x * sailsBox.x, vBoxSize.y * sailsBox.y, vBoxSize.z * sailsBox.z));
            core.Send_Message(FirePlace::eidSound, "lllfff", MSG_SOUND_SET_3D_PARAM, iSailSound, SOUND_PARAM_POSITION,
                              vPos.x + fXOffset, vPos.y, vPos.z + fZOffset);
        }
    }
}
/*
void SHIP::MastFall(mast_t* pM) {
  if (pM && pM->pNode && pM->fDamage >= 1.0f) {
    entid_t ent;
    ent = EntityManager::CreateEntity("mast");
    core.Send_Message(ent, "lpii", MSG_MAST_SETGEOMETRY, pM->pNode, GetId(), GetModelEID());
    EntityManager::AddToLayer(ExecuteLayer, ent, iShipPriorityExecute + 1);
    EntityManager::AddToLayer(RealizeLayer, ent, iShipPriorityRealize + 1);

    pShipsLights->KillMast(this, pM->pNode, false);

    char str[256];
    sprintf_s(str, "%s", pM->pNode->GetName());
    auto* pAMasts = GetACharacter()->FindAClass(GetACharacter(), "Ship.Masts");
    if (!pAMasts)
      pAMasts = GetACharacter()->CreateSubAClass(GetACharacter(), "Ship.Masts");
    Assert(pAMasts);
    pAMasts->getProperty(str) = 1.0f;
    pM->bBroken = true;
  }
}
*/

void SHIP::HullFall(hull_t *pM)
{
    if (pM && pM->pNode && pM->fDamage >= 1.0f)
    {
        entid_t ent;
        ent = EntityManager::CreateEntity("hull");
        core.Send_Message(ent, "lpii", MSG_HULL_SETGEOMETRY, pM->pNode, GetId(), GetModelEID());
        EntityManager::AddToLayer(ExecuteLayer, ent, iShipPriorityExecute + 1);
        EntityManager::AddToLayer(RealizeLayer, ent, iShipPriorityRealize + 1);

        char str[256];
        sprintf_s(str, "%s", pM->pNode->GetName());

        Assert(GetACharacter() != nullptr);
        Attribute& attr = *GetACharacter();
        Attribute& aHulls = attr["Ship"]["Hulls"];
        aHulls[str] = 1.0f;
        pM->bBroken = true;
    }
}

void SHIP::MastFall(mast_t *pM)
{
    if (pM && pM->pNode && pM->fDamage >= 1.0f)
    {
        long iNum, iBase;
        char cMastNodeName[256];
        sprintf_s(cMastNodeName, "%s", pM->pNode->GetName());
        sscanf((char *)&cMastNodeName[_countof(MAST_IDENTIFY) - 1], "%d", &iNum);
        iBase = iNum / TOPMAST_BEGIN;
        //        core.Trace("SHIP::MastFall : nodeName %s  iNum = %d base = %d iNumMasts = %d", cMastNodeName, iNum,
        // iBase, iNumMasts );
        for (long i = 0; i < iNumMasts; i++)
        {
            mast_t *pMast = &pMasts[i];
            char str[256];
            long iMastNum;
            if (pMast && pMast->pNode && !pMasts[i].bBroken)
            {
                sprintf_s(str, "%s", pMast->pNode->GetName());
                sscanf((char *)&str[_countof(MAST_IDENTIFY) - 1], "%d", &iMastNum);
                bool bOk = false;
                if (iNum < TOPMAST_BEGIN) // mast, bring down all the topmills
                {
                    if ((iMastNum > iNum * TOPMAST_BEGIN && iMastNum < ((iNum + 1) * TOPMAST_BEGIN)) ||
                        iMastNum == iNum)
                        bOk = true;
                }
                else // topmast, bring down everything above
                {
                    if (((iMastNum > iNum) && iMastNum < ((iBase + 1) * TOPMAST_BEGIN)) || iMastNum == iNum)
                        bOk = true;
                }
                //                core.Trace("SHIP::MastFall : i = %d nodeName %s  iMastNum = %d bOk = %d", i, str,
                // iMastNum, bOk );
                if (bOk)
                {
                    entid_t ent;
                    ent = EntityManager::CreateEntity("mast");
                    core.Send_Message(ent, "lpii", MSG_MAST_SETGEOMETRY, pM->pNode, GetId(), GetModelEID());
                    EntityManager::AddToLayer(ExecuteLayer, ent, iShipPriorityExecute + 1);
                    EntityManager::AddToLayer(RealizeLayer, ent, iShipPriorityRealize + 1);
                    if (pShipsLights) {
                        pShipsLights->KillMast(this, pMast->pNode, false);
                    }

                    Assert(GetACharacter() != nullptr);
                    Attribute& attr = *GetACharacter();
                    Attribute& aMasts = attr["Ship"]["Masts"];
                    aMasts[str] = 1.0f;
                    pMast->bBroken = true;
                    pMast->fDamage = 1.0f;
                }
            }
        }
    }
}

CMatrix SHIP::UpdateModelMatrix()
{
    auto *pModel = GetModel();
    Assert(pModel);
    pModel->Update();
    return pModel->mtx;
}

void SHIP::RecalculateWorldOffset()
{
    // calculate X offset
    /*long nTmp = (long)((State.vPos.x + fXOffset) * 0.001f) * 1000;
    float fTmp = fXOffset - (float)nTmp;
    if( fTmp != 0.f ) {
      fXOffset = (float)nTmp;
      State.vPos.x += fTmp;
      vPos.x = State.vPos.x;
    }
    // calculate Z offset
    nTmp = (long)((State.vPos.z + fZOffset) * 0.001f) * 1000;
    fTmp = fZOffset - (float)nTmp;
    if( fTmp != 0.f ) {
      fZOffset = (float)nTmp;
      State.vPos.z += fTmp;
      vPos.z = State.vPos.z;
    }*/
}

//##################################################################

void SHIP::SetLightAndFog(bool bSetLight)
{
    if (isDead() && isVisible())
    {
        bSetLightAndFog = true;

        // ambient
        const auto fScale = Bring2Range(0.0f, 1.0f, -35.0f, 0.0f, State.vPos.y);
        pRS->GetRenderState(D3DRS_AMBIENT, &dwSaveAmbient);
        const CVECTOR vAmbient = fScale * COLOR2VECTOR(dwSaveAmbient);
        pRS->SetRenderState(D3DRS_AMBIENT, ARGB((dwSaveAmbient >> 24L), vAmbient.x, vAmbient.y, vAmbient.z));

        // light
        D3DLIGHT9 newLight;
        pRS->GetLight(0, &saveLight);
        newLight = saveLight;
        newLight.Diffuse.r *= fScale;
        newLight.Diffuse.g *= fScale;
        newLight.Diffuse.b *= fScale;
        newLight.Diffuse.a *= fScale;
        newLight.Specular.r *= fScale;
        newLight.Specular.g *= fScale;
        newLight.Specular.b *= fScale;
        newLight.Specular.a *= fScale;
        newLight.Ambient.r *= fScale;
        newLight.Ambient.g *= fScale;
        newLight.Ambient.b *= fScale;
        newLight.Ambient.a *= fScale;
        pRS->SetLight(0, &newLight);

        // fog
        const auto fFogScale = Bring2Range(0.0f, 1.0f, -30.0f, 0.0f, State.vPos.y);
        pRS->GetRenderState(D3DRS_FOGCOLOR, &dwSaveFogColor);
        const CVECTOR vFogColor = fFogScale * COLOR2VECTOR(dwSaveFogColor);
        pRS->SetRenderState(D3DRS_FOGCOLOR, ARGB((dwSaveFogColor >> 24L), vFogColor.x, vFogColor.y, vFogColor.z));
    }
}

void SHIP::RestoreLightAndFog()
{
    if (bSetLightAndFog)
    {
        bSetLightAndFog = false;
        pRS->SetRenderState(D3DRS_AMBIENT, dwSaveAmbient);
        pRS->SetRenderState(D3DRS_FOGCOLOR, dwSaveFogColor);
        pRS->SetLight(0, &saveLight);
    }
}

void SHIP::Realize(uint32_t dtime)
{
    if (!bMounted)
        return;

    // if (dtime)
    //    ShipRocking2(float(dtime) * 0.001f);

    auto *pM = GetModel();
    Assert(pM);

    bSetLightAndFog = false;

    SetLightAndFog(true);
    SetLights();

    pRS->SetRenderState(D3DRS_LIGHTING, true);
    pM->ProcessStage(Stage::realize, dtime);
    pRS->SetRenderState(D3DRS_LIGHTING, false);

    UnSetLights();
    RestoreLightAndFog();

    // draw model
    if (bModelUpperShip && pModelUpperShip)
    {
        CMatrix m1;
        m1.BuildMatrix(0.0f, fUpperShipAY, 0.0f, State.vPos.x + fXOffset, State.vPos.y + fUpperShipY,
                       State.vPos.z + fZOffset);
        pModelUpperShip->mtx = m1;
        pModelUpperShip->ProcessStage(Stage::realize, dtime);
    }

    if (bMassaShow)
    {
        pRS->Print(0, 120, "Massa: %.2f", GetAShip()->getProperty("Massa").get<float>());
        pRS->Print(0, 140, "Volume: %.2f", GetAShip()->getProperty("Volume").get<float>());
    }
}

void SHIP::SetLights()
{
    if (pShipsLights) {
        pShipsLights->SetLights(this);
    }
}

void SHIP::UnSetLights()
{
    if (pShipsLights) {
        pShipsLights->UnSetLights(this);
    }
}

void SHIP::Fire(const CVECTOR &vPos)
{
    if (pShipsLights) {
        pShipsLights->AddDynamicLights(this, vPos);
    }
}

float SHIP::GetMaxSpeedZ()
{
    return GetACharacter()->getProperty("Ship")["MaxSpeedZ"].get<float>(0.0f);
}

float SHIP::GetMaxSpeedY()
{
    return GetACharacter()->getProperty("Ship")["MaxSpeedY"].get<float>(0.0f);
}

float SHIP::GetWindAgainst()
{
    return GetACharacter()->getProperty("Ship")["WindAgainstSpeed"].get<float>(0.0f);
}

bool SHIP::DelStrength(long iIdx)
{
    if (iIdx < RESERVED_STRENGTH || iIdx >= MAX_STRENGTH)
        return false;
    Strength[iIdx].bUse = false;
    return true;
}

long SHIP::AddStrength(STRENGTH *strength)
{
    long i;
    for (i = RESERVED_STRENGTH; i < MAX_STRENGTH; i++)
        if (!Strength[i].bUse)
        {
            Strength[i] = *strength;
            Strength[i].bUse = true;
            return i;
        }
    return -1;
}

uint64_t SHIP::ProcessMessage(MESSAGE &message)
{
    entid_t entity;
    CVECTOR cpos, cang;
    float fov;
    const auto code = message.Long();
    char str[256], str1[256], str2[256];

    switch (code)
    {
    case MSG_SEA_REFLECTION_DRAW:
        Realize(0);
        break;
    case AI_MESSAGE_SET_LAYERS: {
        const std::string_view format = message.Format();
        if (format == "lll") {
            ExecuteLayer = message.Long();
            RealizeLayer = message.Long();
        }
        else if (format == "lss") {
            message.String(256, str);
            ExecuteLayer = GetLayerIDByOldName(str).value();
            message.String(256, str);
            RealizeLayer = GetLayerIDByOldName(str).value();
        }
        else {
            throw std::runtime_error(fmt::format("Unsupported message format: '{}'", format));
        }
        break;
    }
    case MSG_SHIP_CREATE:
        SetACharacter(message.AttributePointer());
        Mount(message.AttributePointer());
        LoadPositionFromAttributes();
        break;
    case MSG_SHIP_SET_SAIL_STATE:
        SetSpeed(message.Float());
        break;
    case MSG_SHIP_GET_SAIL_STATE: {
        auto *pV = message.ScriptVariablePointer();
        pV->Set(GetSpeed());
    }
    break;
    case MSG_SHIP_SET_POS:
        pRS->GetCamera(cpos, cang, fov);
        State.vPos = cpos;
        State.vAng = cang;
        if (pSea)
            State.vPos.y = pSea->WaveXZ(State.vPos.x, State.vPos.z);
        fXOffset = fZOffset = 0.f;
        RecalculateWorldOffset();
        break;
    case MSG_SHIP_ACTIVATE_FIRE_PLACE: {
        const auto dwFPIndex = static_cast<uint32_t>(message.Long());
        message.String(sizeof(str), str);
        message.String(sizeof(str1), str1);
        message.String(sizeof(str2), str2);
        // long iSoundID = message.Long();
        const auto fRunTime = message.Float();
        const auto iBallCharacterIndex = message.Long();
        Assert(dwFPIndex != INVALID_ARRAY_INDEX && dwFPIndex < aFirePlaces.size());
        aFirePlaces[dwFPIndex].Run(str, str1, iBallCharacterIndex, str2, fRunTime);
    }
    break;
    case MSG_SHIP_GET_CHARACTER_INDEX: {
        auto *pVData = message.ScriptVariablePointer();
        pVData->Set(static_cast<long>(GetIndex(GetACharacter())));
    }
    break;
    case MSG_SHIP_GET_NUM_FIRE_PLACES: {
        auto *pVData = message.ScriptVariablePointer();
        pVData->Set(static_cast<long>(aFirePlaces.size()));
    }
    break;
    case MSG_SHIP_RESET_TRACK: {
        if (const auto eidTmp = EntityManager::GetEntityId("ShipTracks"))
        {
            auto *pST = static_cast<ShipTracks *>(EntityManager::GetEntityPointer(eidTmp));
            pST->ResetTrack(this);
        }
    }
    break;
    case MSG_SHIP_ADD_MOVE_IMPULSE: {
        STRENGTH st;

        st.bInertia = message.Long() != 0;

        CVECTOR vSpeed;

        vSpeed.x = message.Float();
        vSpeed.y = message.Float();
        vSpeed.z = message.Float();

        RotateAroundY(vSpeed.x, vSpeed.z, cosf(-State.vAng.y), sinf(-State.vAng.y));

        st.vSpeed = vSpeed;

        st.vRotate.x = message.Float();
        st.vRotate.y = message.Float();
        st.vRotate.z = message.Float();

        AddStrength(&st);
    }
    break;
    case MSG_SHIP_SETLIGHTSOFF: {
        const auto fTime = message.Float();
        const auto bLigths = message.Long() != 0;
        const auto bFlares = message.Long() != 0;
        const auto bNow = message.Long() != 0;
        if (pShipsLights)
            pShipsLights->SetLightsOff(this, fTime, bLigths, bFlares, bNow);
    }
    break;
    case MSG_MAST_DELGEOMETRY: {
        auto *const pNode = (NODE *)message.Pointer();
        if (pShipsLights) {
            pShipsLights->KillMast(this, pNode, true);
        }
    }
    break;
    case MSG_SHIP_SAFE_DELETE:
        // all system which have particles - must be deleted here
        aFirePlaces.clear();
        break;
        // boal 20.08.06 redrawing the flag -->
    case MSG_SHIP_FLAG_REFRESH:
        core.Send_Message(flag_id, "li", MSG_FLAG_DEL_GROUP, GetModelEID());
        if (flag_id = EntityManager::GetEntityId("flag"))
            core.Send_Message(flag_id, "lili", MSG_FLAG_INIT, GetModelEID(), GetNation(GetACharacter()), GetId());
        break;
        // boal 20.08.06 redrawing the flag <--
    case MSG_SHIP_LIGHTSRESET:
        UnSetLights();
        break;
    case MSG_SHIP_DO_FAKE_FIRE: {
        char cBort[256];
        message.String(sizeof(cBort), cBort);
        float fRandTime = message.Float();
        FakeFire(cBort, fRandTime);
    }
    break;
    case MSG_MODEL_SET_TECHNIQUE: {
        char sTech[256];
        message.String(sizeof(sTech), sTech);
        core.Send_Message(GetModelEID(), "ls", MSG_MODEL_SET_TECHNIQUE, sTech);
        //       MODEL * pModel = GetModel();
        //       NODE* pNode = pModel->GetNode(0);
    }
    break;
    }
    return 0;
}

void SHIP::FakeFire(char *sBort, float fRandTime)
{
    GEOS::LABEL label;
    GEOS::INFO info;
    NODE *pNode;

    MODEL *pModel = GetModel();
    Assert(pModel);

    // search cannons
    uint32_t dwIdx = 0;
    while (pNode = pModel->GetNode(dwIdx))
    {
        pNode->geo->GetInfo(info);
        for (uint32_t i = 0; i < (uint32_t)info.nlabels; i++)
        {
            pNode->geo->GetLabel(i, label);
            if (strcmp(sBort, label.group_name) == 0)
            {
                CVECTOR vPos, vCurPos, vDir;
                CMatrix m;
                CMatrix mNode = pNode->glob_mtx;
                CMatrix mRot;
                memcpy(m, label.m, sizeof(m));

                vPos = m.Pos();
                vCurPos = mNode * vPos;

                GetMatrix()->Get3X3(mRot);
                vDir = CVECTOR(m.Vz().x, 0.0f, m.Vz().z);

                CVECTOR vDirTemp = mRot * vDir;
                float fDir = NormalizeAngle(atan2f(vDirTemp.x, vDirTemp.z));

                core.Event("Ship_FakeFire", "ffff", vCurPos.x, vCurPos.y, vCurPos.z, fDir);
            }
        }
        dwIdx++;
    }
}

void SHIP::LoadPositionFromAttributes()
{
    Assert(GetACharacter() != nullptr);
    const Attribute& attr = *GetACharacter();
    const Attribute& ship = attr["ship"];
    const Attribute& pos = ship["pos"];
    const Attribute& ang = ship["ang"];

    pos["x"].get_to(State.vPos.x);
    pos["z"].get_to(State.vPos.z);
    pos["y"].get_to(State.vAng.y);
    fXOffset = fZOffset = 0.f;
    RecalculateWorldOffset();
}

bool SHIP::Mount(Attribute *_pAShip)
{
    Assert(_pAShip);
    pAShip = _pAShip;

    auto iIndex = GetIndex(pAShip);

    core.Event("Ship_StartLoad", "a", GetACharacter());
    core.Event(SEA_GET_LAYERS, "i", GetId());

    EntityManager::AddToLayer(SEA_REFLECTION2, GetId(), 100);
    EntityManager::AddToLayer(RAIN_DROPS, GetId(), 100);

    EntityManager::AddToLayer(RealizeLayer, GetId(), iShipPriorityRealize);
    EntityManager::AddToLayer(ExecuteLayer, GetId(), iShipPriorityExecute);

    EntityManager::AddToLayer(SHIP_CANNON_TRACE, GetId(), iShipPriorityExecute);

    Assert(GetAShip() != nullptr);
    const Attribute& aShip = *GetAShip();
    const Attribute& aName = aShip["Name"];
    if (aName.empty())
    {
        core.Trace("SHIP::Mount : Can't find attribute name in ShipsTypes %d, char: %d, %s, %s, %s",
                   aShip["index"].get<long>(), GetACharacter()->getProperty("index").get<long>(),
                   GetACharacter()->getProperty("name").get<const char*>(), GetACharacter()->getProperty("lastname").get<const char*>(),
                   GetACharacter()->getProperty("id").get<const char*>());
        // GetACharacter()->Dump(GetACharacter(), 0);
        bMounted = false;
        return false;
    }

    strcpy_s(cShipIniName, aName.get<const char*>());

    // core.Trace("Create ship with type = %s", cShipIniName);

    Assert(GetACharacter() != nullptr);
    Attribute& aCharShip = GetACharacter()->getProperty("Ship");
    float fNewSailState = aCharShip["Speed"]["z"].get<float>(0.f);
    if (aCharShip["stopped"].get<bool>(false)) {
        fNewSailState = 0.0f;
    }
    SetSpeed(fNewSailState);

    uniIDX = GetACharacter()->getProperty("index").get<long>();
    if (uniIDX >= 900)
        uniIDX = uniIDX - 900 + 2;

    State.vPos = 0.0f;
    State.vPos.x = 10.0f + uniIDX * 20.0f;
    fXOffset = fZOffset = 0.f;
    RecalculateWorldOffset();
    bUse = uniIDX == 0;

    char temp_str[1024];
    sprintf_s(temp_str, "ships\\%s\\%s", cShipIniName, cShipIniName);

    model_id = EntityManager::CreateEntity("MODELR");
    core.Send_Message(GetModelEID(), "ls", MSG_MODEL_LOAD_GEO, temp_str);
    EntityManager::AddToLayer(HULL_TRACE, GetModelEID(), 10);
    EntityManager::AddToLayer(SUN_TRACE, GetModelEID(), 10);
    EntityManager::AddToLayer(MAST_SHIP_TRACE, GetId(), 10);

    // sails
    if (sail_id = EntityManager::GetEntityId("sail"))
        core.Send_Message(sail_id, "liil", MSG_SAIL_INIT, GetId(), GetModelEID(), GetSpeed() ? 1 : 0);

    // ropes
    if (rope_id = EntityManager::GetEntityId("rope"))
        core.Send_Message(rope_id, "lii", MSG_ROPE_INIT, GetId(), GetModelEID());

    // flags
    if (flag_id = EntityManager::GetEntityId("flag"))
        core.Send_Message(flag_id, "lili", MSG_FLAG_INIT, GetModelEID(), GetNation(GetACharacter()), GetId());

    // vants
    if (vant_id = EntityManager::GetEntityId("vant"))
        core.Send_Message(vant_id, "lii", MSG_VANT_INIT, GetId(), GetModelEID());

    if (vantl_id = EntityManager::GetEntityId("vantl"))
        core.Send_Message(vantl_id, "lii", MSG_VANT_INIT, GetId(), GetModelEID());

    if (vantz_id = EntityManager::GetEntityId("vantz"))
        core.Send_Message(vantz_id, "lii", MSG_VANT_INIT, GetId(), GetModelEID());

    // blots
    if (blots_id = EntityManager::CreateEntity("blots"))
    {
        core.Send_Message(blots_id, "lia", MSG_BLOTS_SETMODEL, GetModelEID(), GetACharacter());
        EntityManager::AddToLayer(RealizeLayer, blots_id, iShipPriorityRealize + 4);
        EntityManager::AddToLayer(ExecuteLayer, blots_id, iShipPriorityExecute + 4);
    }

    LoadShipParameters();

    const entid_t temp_id = GetId();
    core.Send_Message(touch_id, "li", MSG_SHIP_CREATE, temp_id);
    core.Send_Message(sea_id, "lic", MSG_SHIP_CREATE, temp_id,
                      CVECTOR(State.vPos.x + fXOffset, State.vPos.y, State.vPos.z + fZOffset));

    GEOS::INFO ginfo;
    MODEL *pModel = GetModel();
    Assert(pModel);
    NODE *pNode = pModel->GetNode(0);
    Assert(pNode);
    pNode->geo->GetInfo(ginfo);

    CalcRealBoxsize();

    State.vBoxSize.x = ginfo.boxsize.x;
    State.vBoxSize.y = ginfo.boxsize.y;
    State.vBoxSize.z = ginfo.boxsize.z;

    SP.fLength = State.vBoxSize.z;
    SP.fWidth = State.vBoxSize.x;
    fGravity = 9.81f;

    const auto fCapacity = static_cast<float>(SP.iCapacity);
    auto fLength = static_cast<float>(30);

    const float maxw = 10000.0f;
    float min_inertia = 0.28f;
    float max_inertia = 0.003f;

    State.fLoad = 0.0f;

    State.fShipSpeedZ = SP.fSpeedRate;
    State.fShipSpeedY = SP.fTurnRate / 444.0f;
    State.fInertiaFactor = fCapacity / maxw;
    State.fMassInertia = 0.10f; // min_inertia - (min_inertia - max_inertia) * State.fInertiaFactor;

    State.fWeight = SP.fWeight;

    const bool bLights = aCharShip["lights"].get<bool>(false);
    const bool bFlares = aCharShip["flares"].get<bool>(false);

    NODE *pFonarDay = pModel->FindNode("fd");
    NODE *pFonarNight = pModel->FindNode("fn");

    if (bLights)
    {
        if (pFonarDay)
            pFonarDay->flags &= ~NODE::VISIBLE_TREE;
        if (pFonarNight)
            pFonarNight->flags |= NODE::VISIBLE_TREE;
    }
    else
    {
        if (pFonarNight)
            pFonarNight->flags &= ~NODE::VISIBLE_TREE;
        if (pFonarDay)
            pFonarDay->flags |= NODE::VISIBLE_TREE;
    }

    // Add lights and flares
    if (const auto eidTmp = EntityManager::GetEntityId("shiplights"))
    {
        pShipsLights = static_cast<IShipLights *>(EntityManager::GetEntityPointer(eidTmp));

        pShipsLights->AddLights(this, GetModel(), bLights, bFlares);
        pShipsLights->ProcessStage(Stage::execute, 0);
    }

    if (!pShipsLights) {
        spdlog::warn("Mounted ship with no active ShipsLights");
    }
    // Assert(pShipsLights);

    NODE *pFDay = pModel->FindNode("fonar_day");
    NODE *pFNight = pModel->FindNode("fonar_night");

    if (pFDay && pFNight)
        ((bLights) ? pFDay : pFNight)->flags &= (~NODE::VISIBLE);

    // add fireplaces
    ScanShipForFirePlaces();
    // add masts
    BuildMasts();

    if (pShipsLights) {
        for (long i = 0; i < iNumMasts; i++)
            if (pMasts[i].bBroken)
            {
                pShipsLights->KillMast(this, pMasts[i].pNode, true);
            }
    }

    BuildHulls();

    // create model upper ship if needed
    entid_t model_uppership_id;
    const Attribute& aUpperShipModel = aCharShip["upper_model"];
    if (!aUpperShipModel.empty())
    {
        aUpperShipModel["ay"].get_to(fUpperShipAY, 0.0f);
        aUpperShipModel["y"].get_to(fUpperShipY, State.vBoxSize.y * 2.0f + 10.0f);

        strcpy_s(temp_str, aUpperShipModel.get<const char*>());

        bModelUpperShip = true;
        model_uppership_id = EntityManager::CreateEntity("MODELR");
        core.Send_Message(model_uppership_id, "ls", MSG_MODEL_LOAD_GEO, temp_str);
        pModelUpperShip = static_cast<MODEL *>(EntityManager::GetEntityPointer(model_uppership_id));
    }

    // event to script
    core.Event(SHIP_CREATE, "li", GetACharacter()->getProperty("index").get<const char*>(), GetId());
    core.Event("Ship_EndLoad", "a", GetACharacter());

    // add to ship tracks
    if (const auto eidTmp = EntityManager::GetEntityId("ShipTracks"))
    {
        auto *pST = static_cast<ShipTracks *>(EntityManager::GetEntityPointer(eidTmp));
        if (pST)
            pST->AddShip(this);
    }

    if (pSea)
        State.vPos.y = pSea->WaveXZ(State.vPos.x + fXOffset, State.vPos.z + fZOffset);

    Attribute& aBox = aCharShip["boxsize"];
    aBox.get_to(State.vRealBoxSize);

    bMounted = true;
    return true;
}

void SHIP::CalcRealBoxsize()
{
    GEOS::INFO ginfo;
    float x1 = 1e+8f, x2 = -1e+8f, y1 = 1e+8f, y2 = -1e+8f, z1 = 1e+8f, z2 = -1e+8f;

    MODEL *pM = GetModel();
    Assert(pM);

    for (long i = 0;; i++)
    {
        NODE *pN = pM->GetNode(i);
        i++;
        if (!pN)
            break;
        pN->geo->GetInfo(ginfo);
        CVECTOR vGlobPos = pN->glob_mtx.Pos();
        const CVECTOR vBC = vGlobPos + CVECTOR(ginfo.boxcenter.x, ginfo.boxcenter.y, ginfo.boxcenter.z);
        const CVECTOR vBS = CVECTOR(ginfo.boxsize.x, ginfo.boxsize.y, ginfo.boxsize.z) / 2.0f;
        if (vBC.x - vBS.x < x1)
            x1 = vBC.x - vBS.x;
        if (vBC.x + vBS.x > x2)
            x2 = vBC.x + vBS.x;
        if (vBC.y - vBS.y < y1)
            y1 = vBC.y - vBS.y;
        if (vBC.y + vBS.y > y2)
            y2 = vBC.y + vBS.y;
        if (vBC.z - vBS.z < z1)
            z1 = vBC.z - vBS.z;
        if (vBC.z + vBS.z > z2)
            z2 = vBC.z + vBS.z;
    }

    State.vRealBoxSize = CVECTOR(x2 - x1, y2 - y1, z2 - z1);
}

void SHIP::ScanShipForFirePlaces()
{
    GEOS::LABEL label;
    GEOS::INFO info;
    NODE *pNode;

    MODEL *pModel = GetModel();
    Assert(pModel);

    // search and add fire places
    const std::string sFirePlace = "fireplace";
    const std::string sFirePlaces = "fireplaces";
    uint32_t dwIdx = 0;
    while (pNode = pModel->GetNode(dwIdx))
    {
        pNode->geo->GetInfo(info);
        for (uint32_t i = 0; i < static_cast<uint32_t>(info.nlabels); i++)
        {
            pNode->geo->GetLabel(i, label);
            if (sFirePlace == label.group_name || sFirePlaces == label.group_name)
            {
                aFirePlaces.push_back(FirePlace{});
                aFirePlaces.back().Init(pSea, this, label);
            }
        }
        dwIdx++;
    }
    if (aFirePlaces.size() == 0)
    {
        core.Trace("Ship %s doesn't have fire places", cShipIniName);
    }
}

BOOL SHIP::LoadShipParameters()
{
    Assert(GetAShip() != nullptr);
    const Attribute& aShip = *GetAShip();

    // standard values
    aShip["Class"].get_to(SP.iClass);
    aShip["SpeedRate"].get_to(SP.fSpeedRate);
    aShip["TurnRate"].get_to(SP.fTurnRate);
    aShip["MinCrew"].get_to(SP.iCrewMin);
    aShip["MaxCrew"].get_to(SP.iCrewMax);
    aShip["Capacity"].get_to(SP.iCapacity);
    aShip["Weight"].get_to(SP.fWeight);

    // depend values
    aShip["SpeedDependWeight"].get_to(SP.fSpeedDependW);
    aShip["SubSeaDependWeight"].get_to(SP.fSubSeaDependW);
    aShip["TurnDependWeight"].get_to(SP.fTurnDependW);
    aShip["TurnDependSpeed"].get_to(SP.fTurnDependS);

    aShip["WaterLine"].get_to(SP.fWaterLine);

    aShip["InertiaAccelerationY"].get_to(State.vInertiaAccel.y);
    aShip["InertiaBrakingY"].get_to(State.vInertiaBrake.y);
    aShip["InertiaAccelerationX"].get_to(State.vInertiaAccel.x);
    aShip["InertiaBrakingX"].get_to(State.vInertiaBrake.x);
    aShip["InertiaAccelerationZ"].get_to(State.vInertiaAccel.z);
    aShip["InertiaBrakingZ"].get_to(State.vInertiaBrake.z);

    return true;
}

float SHIP::Trace(const CVECTOR &src, const CVECTOR &dst)
{
    MODEL *pModel = GetModel();
    Assert(pModel);
    return pModel->Trace(src, dst);
};

float SHIP::Cannon_Trace(long iBallOwner, const CVECTOR &vSrc, const CVECTOR &vDst)
{
    MODEL *pModel = GetModel();
    Assert(pModel);

    const long iOurIndex = GetIndex(GetACharacter());
    if (iBallOwner == iOurIndex)
        return 2.0f;

    for (long i = 0; i < iNumMasts; i++)
        if (!pMasts[i].bBroken)
        {
            mast_t *pM = &pMasts[i];
            const float fRes = pM->pNode->Trace(vSrc, vDst);

            if (fRes <= 1.0f)
            {
                const CVECTOR v1 = vSrc + fRes * (vDst - vSrc);
                VDATA *pV = core.Event(SHIP_MAST_DAMAGE, "llffffaa", SHIP_MAST_TOUCH_BALL, pM->iMastNum, v1.x, v1.y,
                                       v1.z, pM->fDamage, GetACharacter(), iBallOwner);
                pM->fDamage = Clamp(pV->GetFloat());
                MastFall(pM);
            }
        }

    for (long i = 0; i < iNumHulls; i++)
        if (!pHulls[i].bBroken)
        {
            hull_t *pM = &pHulls[i];
            const float fRes = pM->pNode->Trace(vSrc, vDst);

            if (fRes <= 1.0f)
            {
                const CVECTOR v1 = vSrc + fRes * (vDst - vSrc);
                VDATA *pV = core.Event(SHIP_HULL_DAMAGE, "llffffaas", SHIP_HULL_TOUCH_BALL, pM->iHullNum, v1.x, v1.y,
                                       v1.z, pM->fDamage, GetACharacter(), iBallOwner, pM->pNode->GetName());
                pM->fDamage = Clamp(pV->GetFloat());
                HullFall(pM);
            }
        }

    float fRes = pModel->Trace(vSrc, vDst);
    if (fRes <= 1.0f)
    {
        CVECTOR vDir = !(vDst - vSrc);
        CVECTOR vTemp = vSrc + fRes * (vDst - vSrc);
        // search nearest fire place
        float fMinDistance = 1e8f;
        long iBestIndex = -1;
        for (uint32_t i = 0; i < aFirePlaces.size(); i++)
            if (!aFirePlaces[i].isActive())
            {
                const float fDistance = aFirePlaces[i].GetDistance(vTemp);
                if (fDistance < fMinDistance)
                {
                    fMinDistance = fDistance;
                    iBestIndex = i;
                }
            }
        core.Event(SHIP_HULL_HIT, "illffflf", GetId(), iBallOwner, iOurIndex, vTemp.x, vTemp.y, vTemp.z, iBestIndex,
                   fMinDistance);
        core.Send_Message(blots_id, "lffffff", MSG_BLOTS_HIT, vTemp.x, vTemp.y, vTemp.z, vDir.x, vDir.y, vDir.z);
    }
    return fRes;
}

uint32_t SHIP::AttributeChanged(Attribute &pAttribute)
{
    return 0;
}

CVECTOR SHIP::GetBoxsize() const
{
    return State.vBoxSize;
};
entid_t SHIP::GetModelEID() const
{
    return model_id;
}
MODEL *SHIP::GetModel() const
{
    return static_cast<MODEL *>(EntityManager::GetEntityPointer(GetModelEID()));
}
CMatrix *SHIP::GetMatrix()
{
    return &GetModel()->mtx;
}
void SHIP::SetMatrix(CMatrix &mtx)
{
    GetModel()->mtx = mtx;
}
CVECTOR SHIP::GetAng() const
{
    return State.vAng;
}
CVECTOR SHIP::GetPos() const
{
    return CVECTOR(State.vPos.x + fXOffset, State.vPos.y, State.vPos.z + fZOffset);
}

void SHIP::SetPos(const CVECTOR &vNewPos)
{
    State.vPos = vNewPos;
    if (pSea)
        State.vPos.y = pSea->WaveXZ(State.vPos.x, State.vPos.z);
    fXOffset = fZOffset = 0.f;
    RecalculateWorldOffset();
}

void SHIP::SetSpeed(float fSpeed)
{
    fSailState = fSpeed;
}
float SHIP::GetSpeed()
{
    return fSailState;
}

void SHIP::SetRotate(float fRotSpd)
{
    Strength[0].vRotate.y = fRotSpd;
}
float SHIP::GetRotate()
{
    return Strength[0].vRotate.y;
}

float SHIP::GetCurrentSpeed()
{
    return KNOTS2METERS(State.vSpeed.z);
}

void SHIP::SetACharacter(Attribute *pAP)
{
    VAI_OBJBASE::SetACharacter(pAP);

    if (pAP && pAP->getProperty("index").get<long>(-1) >= 0)
    {
        VDATA *pVDat = static_cast<VDATA *>(core.GetScriptVariable("Characters"));
        if (pVDat)
            pVDat->Set(GetId(), pAP->getProperty("index").get<long>(0));
    }

    if (bMounted)
    {
        EntityManager::EraseEntity(blots_id);
        blots_id = EntityManager::CreateEntity("blots");
        core.Send_Message(blots_id, "lia", MSG_BLOTS_SETMODEL, GetModelEID(), GetACharacter());
        EntityManager::AddToLayer(RealizeLayer, blots_id, iShipPriorityRealize + 4);
        EntityManager::AddToLayer(ExecuteLayer, blots_id, iShipPriorityExecute + 4);
    }
}

void SHIP::Save(CSaveLoad *pSL)
{
    uint32_t i;

    pSL->SaveAPointer("character", GetACharacter());
    pSL->SaveAPointer("ship", pAShip);
    pSL->SaveDword(RealizeLayer);
    pSL->SaveDword(ExecuteLayer);
    pSL->SaveString(std::string(cShipIniName));
    pSL->SaveLong(iShipPriorityExecute);
    pSL->SaveFloat(fGravity);
    pSL->SaveFloat(fSailState);

    pSL->SaveLong(uniIDX);
    pSL->SaveDword(bUse);
    pSL->SaveBuffer((const char *)&ShipPoints[0][0], sizeof(ShipPoints));
    pSL->SaveVector(vSpeedAccel);
    pSL->SaveBuffer((const char *)&SP, sizeof(SP));
    pSL->SaveVector(CVECTOR(vPos.x + fXOffset, vPos.y, vPos.z + fZOffset));
    pSL->SaveVector(vAng);
    pSL->SaveFloat(fWaterLine);
    pSL->SaveDword(bDead);
    pSL->SaveDword(bVisible);
    pSL->SaveVector(vDeadDir);
    pSL->SaveVector(vCurDeadDir);
    pSL->SaveBuffer((const char *)&vKeelContour[0], sizeof(vKeelContour));
    pSL->SaveDword(bShip2Strand);
    pSL->SaveDword(bMounted);
    pSL->SaveDword(bKeelContour);
    pSL->SaveDword(bPerkTurnActive);
    pSL->SaveFloat(fInitialPerkAngle);
    pSL->SaveFloat(fResultPerkAngle);
    pSL->SaveBuffer((const char *)&Strength[0], sizeof(Strength));
    pSL->SaveDword(bSetLightAndFog);
    pSL->SaveDword(dwSaveAmbient);
    pSL->SaveDword(dwSaveFogColor);
    pSL->SaveBuffer((const char *)&saveLight, sizeof(saveLight));
    pSL->SaveBuffer((const char *)&State, sizeof(State));

    pSL->SaveLong(iNumMasts);
    for (i = 0; i < static_cast<uint32_t>(iNumMasts); i++)
    {
        pSL->SaveVector(pMasts[i].vSrc);
        pSL->SaveVector(pMasts[i].vDst);
        pSL->SaveLong(pMasts[i].iMastNum);
        pSL->SaveDword(pMasts[i].bBroken);
        pSL->SaveFloat(pMasts[i].fDamage);
    }

    pSL->SaveLong(iNumHulls);
    for (i = 0; i < (uint32_t)iNumHulls; i++)
    {
        pSL->SaveVector(pHulls[i].vSrc);
        pSL->SaveVector(pHulls[i].vDst);
        pSL->SaveLong(pHulls[i].iHullNum);
        pSL->SaveDword(pHulls[i].bBroken);
        pSL->SaveFloat(pHulls[i].fDamage);
    }

    pSL->SaveDword(aFirePlaces.size());
    for (i = 0; i < aFirePlaces.size(); i++)
        aFirePlaces[i].Save(pSL);
}

void SHIP::Load(CSaveLoad *pSL)
{
    uint32_t i;

    SetACharacter(pSL->LoadAPointer("character"));
    pAShip = pSL->LoadAPointer("ship");

    RealizeLayer = pSL->LoadDword();
    ExecuteLayer = pSL->LoadDword();
    const std::string sTmp = pSL->LoadString();
    strcpy_s(cShipIniName, sTmp.c_str());
    pSL->LoadLong();
    fGravity = pSL->LoadFloat();
    fSailState = pSL->LoadFloat();

    Mount(pAShip);

    uniIDX = pSL->LoadLong();
    bUse = pSL->LoadDword() != 0;
    pSL->Load2Buffer(&ShipPoints[0][0]);
    vSpeedAccel = pSL->LoadVector();
    pSL->Load2Buffer(&SP);
    vPos = pSL->LoadVector();
    vAng = pSL->LoadVector();
    fWaterLine = pSL->LoadFloat();
    bDead = pSL->LoadDword() != 0;

    State.vPos = vPos;
    State.vAng = vAng;
    fXOffset = fZOffset = 0.f;
    RecalculateWorldOffset();
    if (bDead)
    {
        bDead = false;
        SetDead();
    }

    bVisible = pSL->LoadDword() != 0;
    vDeadDir = pSL->LoadVector();
    vCurDeadDir = pSL->LoadVector();
    pSL->Load2Buffer(&vKeelContour[0]);
    bShip2Strand = pSL->LoadDword() != 0;
    bMounted = pSL->LoadDword() != 0;
    bKeelContour = pSL->LoadDword() != 0;
    bPerkTurnActive = pSL->LoadDword() != 0;
    fInitialPerkAngle = pSL->LoadFloat();
    fResultPerkAngle = pSL->LoadFloat();
    pSL->Load2Buffer(&Strength[0]);
    bSetLightAndFog = pSL->LoadDword() != 0;
    dwSaveAmbient = pSL->LoadDword();
    dwSaveFogColor = pSL->LoadDword();
    pSL->Load2Buffer(&saveLight);
    pSL->Load2Buffer(&State);

    iNumMasts = pSL->LoadLong();
    // pMasts = new mast_t[iNumMasts];
    for (i = 0; i < static_cast<uint32_t>(iNumMasts); i++)
    {
        pMasts[i].vSrc = pSL->LoadVector();
        pMasts[i].vDst = pSL->LoadVector();
        pMasts[i].iMastNum = pSL->LoadLong();
        pMasts[i].bBroken = pSL->LoadDword() != 0;
        pMasts[i].fDamage = pSL->LoadFloat();
    }

    iNumHulls = pSL->LoadLong();
    for (i = 0; i < (uint32_t)iNumHulls; i++)
    {
        pHulls[i].vSrc = pSL->LoadVector();
        pHulls[i].vDst = pSL->LoadVector();
        pHulls[i].iHullNum = pSL->LoadLong();
        pHulls[i].bBroken = pSL->LoadDword() != 0;
        pHulls[i].fDamage = pSL->LoadFloat();
    }

    uint32_t dwNum = pSL->LoadDword();
    for (i = 0; i < dwNum; i++)
    {
        aFirePlaces[i].Load(pSL);
        if (aFirePlaces[i].isActive())
            core.Event(SHIP_LOAD_SHIPACTIVATEFIREPLACE, "lllf", GetIndex(GetACharacter()),
                       aFirePlaces[i].GetBallCharacterIndex(), i, aFirePlaces[i].GetRunTime());
    }

    ZERO(ShipPoints);
}

void SHIP::SetFixedSpeed(bool _bSetFixed, float _fFixedSpeed)
{
    bSetFixed = _bSetFixed;
    fFixedSpeed = _fFixedSpeed;
}
