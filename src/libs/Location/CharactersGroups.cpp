//============================================================================================
//    Spirenkov Maxim aka Sp-Max Shaman, 2001
//--------------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------------
//    CharactersGroups
//--------------------------------------------------------------------------------------------
//
//============================================================================================

#include "CharactersGroups.h"
#include "NPCharacter.h"
#include "storm_assert.h"

#define CGS_LOOK 15.0f
#define CGS_HEAR 2.5f
#define CGS_SAY 5.0f
#define CGS_START_ALARM 0.0f
#define CGS_ALARMDOWN 0.02f
#define CGS_ALARMMIN 0.3f
#define CGS_ALARMMAX 0.7f
#define CGS_ALARMVIS 0.3f
#define CGS_DEFMAXTIME 20.0f
#define CGS_VIEWANGLE 100.0f

// ============================================================================================
// Construction, destruction
// ============================================================================================

CharactersGroups::CharactersGroups()
{
    numGroups = 0;
    maxGroups = 0;
    location = nullptr;
    waveTime = 10.0f;
    curExecuteChr = -1;
}

CharactersGroups::~CharactersGroups()
{
    for (long i = 0; i < maxGroups; i++)
    {
        if (groups[i])
        {
            if (groups[i]->relations)
                delete groups[i]->relations;
            delete groups[i];
        }
    }
}

CharactersGroups::String::String()
{
    name = nullptr;
    len = 0;
    max = 0;
    hash = 0;
}

CharactersGroups::String::String(const char *str)
{
    name = nullptr;
    len = 0;
    max = 0;
    hash = 0;
    *this = str;
}

CharactersGroups::String::~String()
{
    if (name)
        delete name;
}

void CharactersGroups::String::operator=(const char *str)
{
    if (!str || !str[0])
    {
        len = 0;
        name = new char[1];
        name[0] = 0;
    }
    else
    {
        len = strlen(str);
        if (len + 1 > max)
        {
            if (name)
                delete name;
            max = (len + 16) & ~15;
            name = new char[max];
        }
        strcpy_s(name, len + 1, str);
        hash = LocatorArray::CalcHashString(str);
    }
}

bool CharactersGroups::String::Cmp(const char *str, long l, long h) const
{
    if (!name || !name[0])
    {
        if (!str || !str[0])
            return true;
        return false;
    }
    if (hash != h)
        return false;
    if (len != l)
        return false;
    return _stricmp(name, str) == 0;
}

long CharactersGroups::String::GetHash(const char *str)
{
    if (!str)
        return 0;
    return LocatorArray::CalcHashString(str);
}

long CharactersGroups::String::GetLen(const char *str)
{
    if (!str)
        return 0;
    return strlen(str);
}

// Initialization
bool CharactersGroups::Init()
{
    // Location Pointer
    const auto loc = EntityManager::GetEntityId("location");
    location = static_cast<Location *>(EntityManager::GetEntityPointer(loc));
    if (!location)
        return false;
    RegistryGroup("");
    // core.LayerCreate("execute", true, false);
    EntityManager::SetLayerType(EXECUTE, EntityManager::Layer::Type::execute);
    EntityManager::AddToLayer(EXECUTE, GetId(), 10);
    return true;
}

// Execution
void CharactersGroups::Execute(uint32_t delta_time)
{
    // Time
    const auto dltTime = delta_time * 0.001f;
    // perform groups
    const auto playerGroup = FindGroupIndex("Player");
    auto playerAlarm = 0.0f;
    auto playerActive = false;
    auto isDeactivate = false;
    for (long i = 0; i < numGroups; i++)
    {
        auto *const rel = groups[i]->relations;
        for (long j = 0; j <= i; j++)
        {
            rel[j].alarm -= dltTime * rel[j].alarmdown;
            if (rel[j].alarm < 0.0f)
                rel[j].alarm = 0.0f;
            const auto lastActive = rel[j].isActive;
            if (rel[j].alarm <= rel[j].alarmmin)
                rel[j].isActive = false;
            if (rel[j].alarm >= rel[j].alarmmax)
                rel[j].isActive = true;
            if (rel[j].isActive != lastActive)
            {
                if (!rel[j].isActive)
                {
                    rel[j].curState = rel[j].relState;
                    isDeactivate = true;
                }
                else
                {
                    rel[j].curState = rel[j].actState;
                }
            }
        }
        if (playerGroup >= 0 && playerGroup != i && groups[i]->numChr > 0)
        {
            auto &rl = FindRelation(playerGroup, i);
            if (rl.curState != rs_enemy)
                continue;
            if (playerAlarm < rl.alarm)
                playerAlarm = rl.alarm;
            long n;
            for (n = 0; n < groups[i]->numChr; n++)
            {
                auto *cg = static_cast<Character *>(EntityManager::GetEntityPointer(groups[i]->c[n]));
                if (cg && cg->IsSetBlade())
                    break;
            }
            if (n >= groups[i]->numChr)
                continue;
            if (rl.isActive)
                playerActive = true;
        }
    }
    // Remove all wrong targets
    if (isDeactivate)
        RemoveAllInvalidTargets();
    // inform about the player's current state of affairs
    core.Event("CharacterGroup_UpdateAlarm", "fl", playerAlarm, playerActive);
    // Executing the characters
    waveTime += dltTime;
    if (curExecuteChr >= 0)
    {
        if (curExecuteChr < location->supervisor.numCharacters)
        {
            auto *const c = location->supervisor.character[curExecuteChr].c;
            CharacterVisibleCheck(c);
            curExecuteChr++;
        }
        else
            curExecuteChr = -1;
    }
    else
    {
        if (waveTime >= 0.5f)
        {
            waveTime = 0.0f;
            curExecuteChr = 0;
        }
    }
}

// Checking the character detects others
void CharactersGroups::CharacterVisibleCheck(Character *chr)
{
    if (!chr)
        return;
    // remove the fulfilled targets
    RemoveInvalidTargets(chr);
    // Character group
    const auto gi = GetCharacterGroup(chr);
    if (gi < 0)
        return;
    auto *const grp = groups[gi];
    // Visible area
    long num;
    if (location->supervisor.FindCharacters(fnd, num, chr, grp->look, CGS_VIEWANGLE, 0.05f))
    {
        FindEnemyFromFindList(chr, grp, num, true);
    }
    // Audible area
    if (location->supervisor.FindCharacters(fnd, num, chr, grp->hear))
    {
        FindEnemyFromFindList(chr, grp, num, false);
    }
}

// Check found characters for enemies
void CharactersGroups::FindEnemyFromFindList(Character *chr, Group *grp, long num, bool visCheck)
{
    Character *targets[MAX_CHARACTERS];
    long numTrg = 0;
    // For all found characters
    for (long i = 0; i < num; i++)
    {
        // Found character group
        const auto gi = GetCharacterGroup(fnd[i].c);
        if (gi < 0)
            continue;
        auto &r = FindRelation(grp->index, gi);
        if (r.curState != rs_enemy)
            continue;
        // Enemy detected, if invisible, then let him pass
        if (visCheck && !chr->VisibleTest(fnd[i].c))
            continue;
        // Found an enemy, add
        if (AddEnemyTarget(chr, fnd[i].c))
        {
            if (numTrg < MAX_CHARACTERS)
                targets[numTrg++] = fnd[i].c;
        }
    }
    // Inform others about the detected targets
    if (numTrg > 0 && location->supervisor.FindCharacters(fnd, num, chr, grp->say))
    {
        for (long i = 0; i < num; i++)
        {
            auto *const c = fnd[i].c;
            // If invisible, then skip it
            if (!chr->VisibleTest(c))
                continue;
            // Get the character group
            const auto cgrp = GetCharacterGroup(c);
            if (cgrp < 0)
                continue;
            // Found character group
            auto &r = FindRelation(grp->index, cgrp);
            if (r.curState != rs_friend)
                continue;
            for (long j = 0; j < numTrg; j++)
            {
                if (grp->index != cgrp)
                {
                    const auto tgrp = GetCharacterGroup(targets[j]);
                    if (tgrp >= 0)
                    {
                        auto &tr = FindRelation(cgrp, tgrp);
                        if (tr.curState == rs_friend)
                        {
                            // Both groups are friendly to the current one, look at the priorities
                            if (groups[tgrp]->priority >= groups[grp->index]->priority)
                                continue;
                        }
                    }
                }
                AddEnemyTarget(c, targets[j]);
            }
        }
    }
}

// Add or update an enemy
bool CharactersGroups::AddEnemyTarget(Character *chr, Character *enemy, float maxtime)
{
    if (!chr || !enemy)
        return false;
    // restore the alarm
    const auto isSelf = false;
    auto &r = FindRelation(GetCharacterGroup(chr), GetCharacterGroup(enemy));
    Assert(!isSelf);
    r.alarm = 1.0f;
    if (r.alarm >= r.alarmmax)
        r.isActive = true;
    if (r.isActive)
        r.curState = r.actState;
    if (r.actState != rs_enemy)
        return false;
    // Looking among added
    for (long i = 0; i < chr->numTargets; i++)
    {
        if (enemy == EntityManager::GetEntityPointer(chr->grpTargets[i].chr))
        {
            chr->grpTargets[i].time = 0.0f;
            return true;
        }
    }
    if (chr->numTargets >= sizeof(chr->grpTargets) / sizeof(Character::GrpTarget))
        return false;
    // Add a new target
    Assert(_stricmp(chr->group, enemy->group) != 0);
    auto &trg = chr->grpTargets[chr->numTargets++];
    trg.chr = enemy->GetId();
    trg.time = 0.0f;
    if (maxtime < 0.0f)
        trg.timemax = 3.0f + (rand() & 7);
    else
        trg.timemax = maxtime;
    return true;
}

// Remove all inactive or invalid targets
void CharactersGroups::RemoveAllInvalidTargets()
{
    // Update goal lists
    for (long i = 0; i < location->supervisor.numCharacters; i++)
    {
        RemoveInvalidTargets(location->supervisor.character[i].c);
    }
}

// Remove inactive or invalid targets
bool CharactersGroups::RemoveInvalidTargets(Character *chr, Character *check)
{
    if (!chr)
        return false;
    const auto gi = GetCharacterGroup(chr);
    if (gi < 0)
    {
        chr->numTargets = 0;
        return false;
    }
    auto isValidate = false;
    for (long i = 0; i < chr->numTargets;)
    {
        auto isDelete = true;
        auto &trg = chr->grpTargets[i];
        auto *c = static_cast<Character *>(EntityManager::GetEntityPointer(trg.chr));
        if (c && (trg.time < trg.timemax || trg.timemax < 0.0f))
        {
            if (!c->IsDead())
            {
                // The character exists and is remembered
                const auto gc = GetCharacterGroup(c);
                auto &r = FindRelation(gi, gc);
                if (r.curState == rs_enemy)
                {
                    isDelete = false;
                    if (check == c)
                        isValidate = true;
                }
            }
        }
        if (isDelete)
        {
            trg = chr->grpTargets[--chr->numTargets];
        }
        else
            i++;
    }
    return isValidate;
}

// Messages
uint64_t CharactersGroups::ProcessMessage(MESSAGE &message)
{
    char cmd[64];
    message.String(sizeof(cmd), cmd);
    cmd[sizeof(cmd) - 1] = 0;
    if (!cmd[0])
        return 0;
    if (_stricmp(cmd, "VldTrg") == 0)
    {
        return MsgIsValidateTarget(message);
    }
    if (_stricmp(cmd, "GetTrg") == 0)
    {
        return MsgGetOptimalTarget(message);
    }
    if (_stricmp(cmd, "IsEnemy") == 0)
    {
        return MsgIsEnemy(message);
    }
    if (_stricmp(cmd, "MoveChr") == 0)
    {
        return MoveCharacterToGroup(message);
    }
    if (_stricmp(cmd, "Attack") == 0)
    {
        MsgAttack(message);
        return 1;
    }
    if (_stricmp(cmd, "AddTarget") == 0)
    {
        MsgAddTarget(message);
        return 1;
    }
    if (_stricmp(cmd, "UpdChrTrg") == 0)
    {
        MsgUpdChrTrg(message);
        return 1;
    }
    if (_stricmp(cmd, "RegistryGroup") == 0)
    {
        MsgRegistryGroup(message);
        return 1;
    }
    if (_stricmp(cmd, "ReleaseGroup") == 0)
    {
        MsgReleaseGroup(message);
        return 1;
    }
    if (_stricmp(cmd, "SetRelation") == 0)
    {
        MsgSetRelation(message);
        return 1;
    }
    if (_stricmp(cmd, "SetAlarmReaction") == 0)
    {
        MsgSetAlarmReaction(message);
        return 1;
    }
    if (_stricmp(cmd, "SetGroupLook") == 0)
    {
        return MsgSetGroupLook(message);
    }
    if (_stricmp(cmd, "SetGroupHear") == 0)
    {
        return MsgSetGroupHear(message);
    }
    if (_stricmp(cmd, "SetGroupSay") == 0)
    {
        return MsgSetGroupSay(message);
    }
    if (_stricmp(cmd, "SetGroupPriority") == 0)
    {
        return MsgSetGroupPriority(message);
    }
    if (_stricmp(cmd, "UnloadCharacter") == 0)
    {
        UnloadCharacter(message);
        return 1;
    }
    if (_stricmp(cmd, "ResetWaveTime") == 0)
    {
        waveTime = 1000.0f;
        return 1;
    }
    if (_stricmp(cmd, "SetAlarm") == 0)
    {
        return MsgSetAlarm(message);
    }
    if (_stricmp(cmd, "SetAlarmDown") == 0)
    {
        return MsgSetAlarmDown(message);
    }
    if (_stricmp(cmd, "ClearAllTargets") == 0)
    {
        ClearAllTargets();
        return 1;
    }
    if (_stricmp(cmd, "SaveData") == 0)
    {
        SaveData();
        return 1;
    }
    if (_stricmp(cmd, "LoadDataRelations") == 0)
    {
        LoadDataRelations();
        return 1;
    }
    if (_stricmp(cmd, "RestoreStates") == 0)
    {
        RestoreStates();
        return 1;
    }
    if (_stricmp(cmd, "DeleteEmptyGroups") == 0)
    {
        DeleteEmptyGroups();
        return 1;
    }
    if (_stricmp(cmd, "DumpRelations") == 0)
    {
        DumpRelations();
        return 1;
    }
    return 0;
}

// Changing an attribute
uint32_t CharactersGroups::AttributeChanged(Attribute &apnt)
{
    return 0;
}

// Check target for validity
bool CharactersGroups::MsgIsValidateTarget(MESSAGE &message)
{
    const auto chr = message.EntityID();
    const auto trg = message.EntityID();
    auto *c = static_cast<Character *>(EntityManager::GetEntityPointer(chr));
    if (!c)
        return false;
    auto *en = static_cast<Character *>(EntityManager::GetEntityPointer(trg));
    if (!en)
        return false;
    CVECTOR vP1, vP2;
    c->GetPosition(vP1);
    en->GetPosition(vP2);
    const auto fDistance = sqrtf(~(vP1 - vP2));

    Assert(AttributesPointer != nullptr);
    Attribute& attr = *AttributesPointer;
    attr["distance"] = fDistance;

    return RemoveInvalidTargets(c, en);
}

// Find the optimal goal
bool CharactersGroups::MsgGetOptimalTarget(MESSAGE &message) const
{
    const auto chr = message.EntityID();
    auto *c = static_cast<Character *>(EntityManager::GetEntityPointer(chr));
    if (!c)
        return false;
    auto *vd = message.ScriptVariablePointer();
    if (!vd)
        return false;
    if (c->numTargets <= 0)
        return false;
    long s = 0;
    if (c->numTargets > 1)
    {
        CVECTOR pos, p;
        c->GetPosition(pos);
        const auto numChr = location->supervisor.numCharacters;
        auto *const cEx = location->supervisor.character;
        // choose the optimal goal
        float value;
        s = -1;
        for (long i = 0; i < c->numTargets; i++)
        {
            // Character pointer
            auto *nc = static_cast<NPCharacter *>(EntityManager::GetEntityPointer(c->grpTargets[i].chr));
            if (!nc)
                continue;
            if (!nc->IsSetBlade())
                continue;
            // collect the number of characters fighting with this guy
            long n = 0;
            for (long j = 0; j < numChr; j++)
            {
                if (cEx[j].c == nc || cEx[j].c == c)
                    continue;
                if (static_cast<NPCharacter *>(cEx[j].c)->GetAttackedCharacter() == nc)
                    n++;
            }
            // Find the distance
            nc->GetPosition(p);
            auto v = ~(pos - p);
            // calculate the heuristic value
            v *= 0.5f + n * n * 0.5f;
            // take into account the presence of his weapon
            // if(!nc->IsSetBlade()) v *= v*100.0f;
            // take into account the choice of goal
            if (s >= 0)
            {
                if (v < value)
                {
                    value = v;
                    s = i;
                }
            }
            else
            {
                value = v;
                s = i;
            }
        }
        if (s < 0)
            s = 0;
    }
    c = static_cast<Character *>(EntityManager::GetEntityPointer(c->grpTargets[s].chr));
    // if(!c->IsSetBlade()) return false;
    if (c->AttributesPointer)
    {
        vd->Set(static_cast<long>(c->AttributesPointer->getProperty("index").get<uint32_t>(-1)));
    }
    else
    {
        vd->Set(static_cast<long>(-1));
    }
    return true;
}

// Is this character an enemy
bool CharactersGroups::MsgIsEnemy(MESSAGE &message)
{
    const auto g1 = GetCharacterGroup(static_cast<Character *>(EntityManager::GetEntityPointer(message.EntityID())));
    const auto g2 = GetCharacterGroup(static_cast<Character *>(EntityManager::GetEntityPointer(message.EntityID())));
    if (g1 < 0 || g2 < 0)
        return false;
    auto isSelf = false;
    auto &r = FindRelation(g1, g2, &isSelf);
    if (isSelf)
        return false;
    return r.curState == rs_enemy;
}

// Group reaction to attack
void CharactersGroups::MsgAttack(MESSAGE &message)
{
    // Get the groups
    auto *const gAttack = GetGroup(message);
    auto *const gHit = GetGroup(message);
    Assert(gAttack);
    Assert(gHit);
    auto &r = FindRelation(gAttack->index, gHit->index);
    r.alarm = 1.0f;
    if (r.alarm >= r.alarmmax)
        r.isActive = true;
    if (r.isActive)
        r.curState = r.actState;
    // Establish a hostile relationship among all victim-friendly groups
    for (long i = 0; i < numGroups; i++)
    {
        if (gHit == groups[i])
            continue;
        if (gAttack == groups[i])
            continue;
        auto &r = FindRelation(gHit->index, i);
        if (r.curState != rs_friend)
            continue;
        // fight with the attackers
        auto &ra = FindRelation(gAttack->index, i);
        ra.alarm = 1.0f;
        if (ra.alarm >= ra.alarmmax)
            ra.isActive = true;
        if (ra.isActive)
            ra.curState = ra.actState;
    }
}

void CharactersGroups::MsgAddTarget(MESSAGE &message)
{
    // get characters
    auto eid = message.EntityID();
    auto *chr = static_cast<Character *>(EntityManager::GetEntityPointer(eid));
    eid = message.EntityID();
    auto *enemy = static_cast<Character *>(EntityManager::GetEntityPointer(eid));
    if (!chr || !enemy)
        return;
    // Checking for hostility
    const auto g1 = GetCharacterGroup(chr);
    const auto g2 = GetCharacterGroup(enemy);
    if (g1 < 0 || g2 < 0)
        return;
    auto isSelf = false;
    auto &r = FindRelation(g1, g2, &isSelf);
    if (isSelf)
        return;
    if (r.curState != rs_enemy)
        return;
    // Adding the enemy
    AddEnemyTarget(chr, enemy, message.Float());
    // Informing others about the new goal
    long num = 0;
    if (location->supervisor.FindCharacters(fnd, num, chr, groups[g1]->say))
    {
        for (long i = 0; i < num; i++)
        {
            auto *const c = fnd[i].c;
            // If invisible, then skip it
            if (!chr->VisibleTest(c))
                continue;
            // Found character group
            auto &r = FindRelation(g1, GetCharacterGroup(c));
            if (r.curState != rs_friend)
                continue;
            AddEnemyTarget(c, enemy);
        }
    }
}

// Refresh goals for this character
void CharactersGroups::MsgUpdChrTrg(MESSAGE &message)
{
    const auto eid = message.EntityID();
    auto *chr = static_cast<Character *>(EntityManager::GetEntityPointer(eid));
    if (chr)
        CharacterVisibleCheck(chr);
}

// Register a group
void CharactersGroups::MsgRegistryGroup(MESSAGE &message)
{
    char grpName[128];
    message.String(sizeof(grpName), grpName);
    grpName[sizeof(grpName) - 1] = 0;
    RegistryGroup(grpName);
}

// Delete group
void CharactersGroups::MsgReleaseGroup(MESSAGE &message)
{
    char grpName[128];
    message.String(sizeof(grpName), grpName);
    grpName[sizeof(grpName) - 1] = 0;
    ReleaseGroup(grpName);
}

// Register a group
long CharactersGroups::RegistryGroup(const char *groupName)
{
    const auto idxgrp = FindGroupIndex(groupName);
    if (idxgrp >= 0)
        return idxgrp;
    // register
    if (numGroups >= maxGroups)
    {
        maxGroups += 16;
        groups.resize(maxGroups);
        for (int i = numGroups; i < maxGroups; ++i) // >:-(
        {
            groups[i] = nullptr;
        }
    }
    auto *grp = new Group();
    grp->index = numGroups;
    grp->name = groupName;
    grp->look = CGS_LOOK;
    grp->hear = CGS_HEAR;
    grp->say = CGS_SAY;
    grp->priority = 10;
    groups[numGroups++] = grp;
    if (numGroups)
    {
        // Relationship table
        grp->relations = new Relation[numGroups];
        for (long i = 0; i < numGroups - 1; i++)
        {
            grp->relations[i].alarm = CGS_START_ALARM;
            grp->relations[i].alarmdown = CGS_ALARMDOWN;
            grp->relations[i].alarmmin = CGS_ALARMMIN;
            grp->relations[i].alarmmax = CGS_ALARMMAX;
            grp->relations[i].isActive = false;
            if (grp->relations[i].alarm >= grp->relations[i].alarmmax)
                grp->relations[i].isActive = true;
            grp->relations[i].curState = rs_neitral;
            grp->relations[i].actState = rs_enemy;
            grp->relations[i].relState = rs_neitral;
        }
        // always friends with ourselves
        grp->relations[numGroups - 1].alarm = 0.0f;
        grp->relations[numGroups - 1].alarmdown = 0.0f;
        grp->relations[numGroups - 1].alarmmin = 0.0f;
        grp->relations[numGroups - 1].alarmmax = 10.0f;
        grp->relations[numGroups - 1].isActive = false;
        grp->relations[numGroups - 1].curState = rs_friend;
        grp->relations[numGroups - 1].actState = rs_friend;
        grp->relations[numGroups - 1].relState = rs_friend;
    }
    else
        grp->relations = nullptr;
    grp->numChr = 0;
    return numGroups - 1;
}

void CharactersGroups::ReleaseGroup(const char *groupName)
{
    const auto idxgrp = FindGroupIndex(groupName);
    if (idxgrp < 0)
        return; // no group nothing to delete

    Group *oldGroup = groups[idxgrp];
    groups[idxgrp] = groups[numGroups - 1];
    groups[numGroups - 1] = nullptr;

    if (oldGroup->relations)
        delete oldGroup->relations;
    delete oldGroup;

    for (long othergrp = idxgrp + 1; othergrp < numGroups - 1;
         ++othergrp) // restore relations of other groups taking into account the shift
        groups[othergrp]->relations[idxgrp] = groups[idxgrp]->relations[othergrp];

    if (idxgrp < numGroups - 1)
        groups[idxgrp]->relations[idxgrp] = groups[idxgrp]->relations[numGroups - 1];

    numGroups--;
}

// Set the visibility radius for the group
bool CharactersGroups::MsgSetGroupLook(MESSAGE &message)
{
    // Current group
    auto *const grp = GetGroup(message, false);
    if (!grp)
        return false;
    // Radius
    grp->look = message.Float();
    if (grp->look < 0.0f)
        grp->look = 0.0f;
    return true;
}

// Set the radius of hearing for the group
bool CharactersGroups::MsgSetGroupHear(MESSAGE &message)
{
    // Current group
    auto *const grp = GetGroup(message, false);
    if (!grp)
        return false;
    // Radius
    grp->hear = message.Float();
    if (grp->hear < 0.0f)
        grp->hear = 0.0f;
    return true;
}

// Set message radius for the group
bool CharactersGroups::MsgSetGroupSay(MESSAGE &message)
{
    // Current group
    auto *const grp = GetGroup(message, false);
    if (!grp)
        return false;
    // Radius
    grp->say = message.Float();
    if (grp->say < 0.0f)
        grp->say = 0.0f;
    return true;
}

// Set group priority
bool CharactersGroups::MsgSetGroupPriority(MESSAGE &message)
{
    // Current group
    auto *const grp = GetGroup(message, false);
    if (!grp)
        return false;
    // Priority
    grp->priority = message.Long();
    return true;
}

// Set speed alarm level
bool CharactersGroups::MsgSetAlarm(MESSAGE &message)
{
    // Current group
    auto *const g1 = GetGroup(message, false);
    if (!g1)
        return false;
    auto *const g2 = GetGroup(message, false);
    if (!g2)
        return false;
    auto isSelf = false;
    auto &r = FindRelation(g1->index, g2->index, &isSelf);
    if (isSelf)
        return false;
    // Alarm level
    r.alarm = message.Float();
    if (r.alarm < 0.0f)
        r.alarm = 0.0f;
    if (r.alarm > 1.0f)
        r.alarm = 1.0f;
    if (r.alarm <= r.alarmmin)
        r.isActive = false;
    if (r.alarm >= r.alarmmax)
        r.isActive = true;
    RemoveAllInvalidTargets();
    return true;
}

// Set alarm fading speed
bool CharactersGroups::MsgSetAlarmDown(MESSAGE &message)
{
    // Current group
    auto *const g1 = GetGroup(message, false);
    if (!g1)
        return false;
    auto *const g2 = GetGroup(message, false);
    if (!g2)
        return false;
    auto isSelf = false;
    auto &r = FindRelation(g1->index, g2->index, &isSelf);
    if (isSelf)
        return false;
    // Decrease rate
    r.alarmdown = message.Float();
    if (r.alarmdown < 0.0f)
        r.alarmdown = CGS_ALARMDOWN;
    return true;
}

// Add character to group
bool CharactersGroups::MoveCharacterToGroup(MESSAGE &message)
{
    const auto eid = message.EntityID();
    auto *chr = static_cast<Character *>(EntityManager::GetEntityPointer(eid));
    if (!chr)
        return false;
    // create a group
    char grpName[128];
    message.String(sizeof(grpName), grpName);
    grpName[sizeof(grpName) - 1] = 0;
    auto *grp = FindGroup(grpName);
    if (!grp)
        RegistryGroup(grpName);
    grp = FindGroup(grpName);
    Assert(grp);
    // Remove the character from the previous group
    RemoveCharacterFromAllGroups(eid);
    // Check for free space in the group
    // boal fix for intel cpp if(grp->numChr >= sizeof(CharactersGroups::Group::c)/sizeof(Character *)) return false;
    if (grp->numChr >= MAX_CHARACTERS)
        return false; // fix
    // Place in the new
    grp->c[grp->numChr++] = eid;
    strcpy_s(chr->group, grpName);
    RemoveInvalidTargets(chr);
    return true;
}

// Establish relationships between groups
void CharactersGroups::MsgSetRelation(MESSAGE &message)
{
    auto isSelf = false;
    auto &r = FindRelation(message, &isSelf);
    if (isSelf)
        return;
    char buf[32];
    message.String(sizeof(buf), buf);
    buf[sizeof(buf) - 1] = 0;
    auto actState = rs_enemy;
    auto relState = rs_neitral;
    if (_stricmp(buf, "friend") == 0)
    {
        r.curState = rs_friend;
        actState = rs_enemy;
        relState = rs_neitral;
        r.alarm = 0.0f;
        r.alarmdown = CGS_ALARMDOWN;
        r.alarmmin = CGS_ALARMMIN;
        r.alarmmax = CGS_ALARMMAX;
        r.isActive = false;
    }
    else if (_stricmp(buf, "neitral") == 0)
    {
        r.curState = rs_neitral;
        actState = rs_enemy;
        relState = rs_neitral;
        r.alarm = 0.0f;
        r.alarmdown = CGS_ALARMDOWN;
        r.alarmmin = CGS_ALARMMIN;
        r.alarmmax = CGS_ALARMMAX;
    }
    else if (_stricmp(buf, "enemy") == 0)
    {
        r.curState = rs_enemy;
        actState = rs_enemy;
        relState = rs_enemy;
        r.alarm = 1.0f;
        r.isActive = true;
        r.alarmdown = 0.0f;
        r.alarmmin = 0.0f;
        r.alarmmax = 10.0f;
    }
    r.actState = actState;
    r.relState = relState;
    if (r.isActive)
        r.curState = r.actState;
}

// Set alarm response for a pair of groups
void CharactersGroups::MsgSetAlarmReaction(MESSAGE &message)
{
    auto isSelf = false;
    auto &r = FindRelation(message, &isSelf);
    if (isSelf)
        return;
    char act[32];
    message.String(sizeof(act), act);
    act[sizeof(act) - 1] = 0;
    char rel[32];
    message.String(sizeof(rel), rel);
    rel[sizeof(rel) - 1] = 0;
    auto actState = rs_enemy;
    if (_stricmp(act, "neitral") == 0)
    {
        actState = rs_neitral;
    }
    else if (_stricmp(act, "friend") == 0)
    {
        actState = rs_friend;
    }
    auto relState = rs_neitral;
    if (_stricmp(rel, "enemy") == 0)
    {
        relState = rs_enemy;
    }
    else if (_stricmp(rel, "friend") == 0)
    {
        relState = rs_friend;
    }
    r.actState = actState;
    r.relState = relState;
    if (r.isActive)
        r.curState = r.actState;
}

// Remove character from all groups
void CharactersGroups::RemoveCharacterFromAllGroups(entid_t chr)
{
    auto *const ch = chr ? static_cast<Character *>(EntityManager::GetEntityPointer(chr)) : nullptr;
    // Remove the character from the previous group
    for (long i = 0; i < numGroups; i++)
    {
        auto *g = groups[i];
        auto *const cid = g->c;
        for (long j = 0; j < g->numChr;)
        {
            auto *c = static_cast<Character *>(EntityManager::GetEntityPointer(cid[j]));
            if (c == nullptr || c == ch)
            {
                cid[j] = cid[--g->numChr];
            }
            else
                j++;
        }
    }
}

// Delete all empty groups
void CharactersGroups::DeleteEmptyGroups()
{
    for (long i = 0; i < numGroups; i++)
    {
        Group *g = groups[i];

        if (g->numChr == 0)
        {
            ReleaseGroup(g->name.name);
            --i; // the last group moved to this position
        }
    }
}

// Unloading a character
void CharactersGroups::UnloadCharacter(MESSAGE &message)
{
    const auto eid = message.EntityID();
    RemoveCharacterFromAllGroups(eid);
}

// Get group from message
CharactersGroups::Group *CharactersGroups::GetGroup(MESSAGE &message, bool isRegistry)
{
    char grpName[128];
    message.String(sizeof(grpName), grpName);
    grpName[sizeof(grpName) - 1] = 0;
    auto *grp = FindGroup(grpName);
    if (!grp && isRegistry)
    {
        RegistryGroup(grpName);
        grp = FindGroup(grpName);
        Assert(grp);
    }
    return grp;
}

// Find a group by name
CharactersGroups::Group *CharactersGroups::FindGroup(const char *name)
{
    const auto gi = FindGroupIndex(name);
    if (gi < 0)
        return nullptr;
    return groups[gi];
}

// Find a group by name
long CharactersGroups::FindGroupIndex(const char *name)
{
    if (!name)
        return -1;
    const auto l = String::GetLen(name);
    const auto h = String::GetHash(name);

    // looking among registered
    for (long i = 0; i < numGroups; i++)
    {
        if (groups[i]->name.Cmp(name, l, h))
            return i;
    }
    return -1;
}

// Find group relationship
CharactersGroups::Relation &CharactersGroups::FindRelation(MESSAGE &message, bool *selfgroup)
{
    char grpName1[128];
    message.String(sizeof(grpName1), grpName1);
    grpName1[sizeof(grpName1) - 1] = 0;
    char grpName2[128];
    message.String(sizeof(grpName2), grpName2);
    grpName2[sizeof(grpName2) - 1] = 0;
    return FindRelation(grpName1, grpName2, selfgroup);
}

// Find group relationship
CharactersGroups::Relation &CharactersGroups::FindRelation(const char *name1, const char *name2, bool *selfgroup)
{
    auto g1 = FindGroupIndex(name1);
    if (g1 < 0)
    {
        RegistryGroup(name1);
        g1 = FindGroupIndex(name1);
        Assert(g1 >= 0);
    }
    auto g2 = FindGroupIndex(name2);
    if (g2 < 0)
    {
        RegistryGroup(name2);
        g2 = FindGroupIndex(name2);
        Assert(g2 >= 0);
    }
    return FindRelation(g1, g2, selfgroup);
}

// Find group relationship
CharactersGroups::Relation &CharactersGroups::FindRelation(long g1, long g2, bool *selfgroup)
{
    Assert(g1 >= 0 && g1 < numGroups);
    Assert(g2 >= 0 && g2 < numGroups);
    if (selfgroup)
        *selfgroup = (g1 == g2);
    if (g1 <= g2)
    {
        return groups[g2]->relations[g1];
    }
    return groups[g1]->relations[g2];
}

// Get character group index
long CharactersGroups::GetCharacterGroup(Character *c)
{
    if (!c)
        return -1;
    if (c->groupID >= 0 && c->groupID < numGroups)
    {
        if (_stricmp(c->group, groups[c->groupID]->name) == 0)
        {
            return c->groupID;
        }
    }
    c->groupID = FindGroupIndex(c->group);
    return c->groupID;
}

// Delete all targets
void CharactersGroups::ClearAllTargets() const
{
    // Update goal lists
    for (long i = 0; i < location->supervisor.numCharacters; i++)
    {
        location->supervisor.character[i].c->numTargets = 0;
    }
}

// Save data to object
void CharactersGroups::SaveData()
{
    // Root attribute for saving data
    if (!AttributesPointer)
    {
        core.Trace("CharactersGroups::SaveData -> no attributes");
        return;
    }

    Attribute& attr = *AttributesPointer;
    Attribute& saveData = attr["savedata"];
    if (!saveData.empty())
        saveData.clear();
    // Maintaining group relationships
    for (long i = 0, cnt = 0; i < numGroups; i++)
    {
        for (long j = 0; j < i; j++)
        {
            // Relationship section
            char buf[16];
            sprintf_s(buf, "r%.4i", cnt++);
            Attribute& group = saveData[buf];

            // Save group parameters
            group["name1"] = groups[i]->name.name;
            group["look1"] = groups[i]->look;
            group["hear1"] = groups[i]->hear;
            group["say1"] = groups[i]->say;
            group["prt1"] = groups[i]->priority;

            group["name2"] = groups[j]->name.name;
            group["look2"] = groups[j]->look;
            group["hear2"] = groups[j]->hear;
            group["say2"] = groups[j]->say;
            group["prt2"] = groups[j]->priority;

            // keep the relationship
            auto &r = FindRelation(i, j);
            group["alarm"] = r.alarm;
            group["alarmdown"] = r.alarmdown;
            group["alarmmin"] = r.alarmmin;
            group["alarmmax"] = r.alarmmax;
            group["isactive"] = r.isActive;
            group["curState"] = r.curState;
            group["actState"] = r.actState;
            group["relState"] = r.relState;
        }
    }
    // Save character parameters
    /*
    saveData = AttributesPointer->FindAClass(AttributesPointer, "characters");
    if(saveData) AttributesPointer->DeleteAttributeClassX(saveData);
    saveData = AttributesPointer->CreateSubAClass(AttributesPointer, "characters");
    */
}

// Read data from object
void CharactersGroups::LoadDataRelations()
{
    // Root attribute for saving data
    if (!AttributesPointer)
    {
        core.Trace("CharactersGroups::LoadDataRelations -> no attributes");
        return;
    }

    const Attribute& attr = *AttributesPointer;
    const Attribute& saveData = attr["savedata"];
    if (saveData.empty())
        return;

    for (const Attribute& group : saveData) {
        // Registering the first group
        const auto gname1 = group["name1"].get<std::string_view>();
        const auto g1 = RegistryGroup(gname1.data());
        group["look1"].get_to(groups[g1]->look);
        group["hear1"].get_to(groups[g1]->hear);
        group["say1"].get_to(groups[g1]->say);
        group["prt1"].get_to(groups[g1]->priority);
        // Registering the second group
        const auto gname2 = group["name2"].get<std::string_view>();
        const auto g2 = RegistryGroup(gname1.data());
        group["look2"].get_to(groups[g2]->look);
        group["hear2"].get_to(groups[g2]->hear);
        group["say2"].get_to(groups[g2]->say);
        group["prt2"].get_to(groups[g2]->priority);

        // restore relations
        Assert(g1 != g2);
        auto &r = FindRelation(g1, g2);
        group["alarm"].get_to(r.alarm);
        if (r.alarm < 0.0f)
            r.alarm = 0.0f;
        group["alarmdown"].get_to(r.alarmdown);
        group["alarmmin"].get_to(r.alarmmin);
        group["alarmmax"].get_to(r.alarmmax);
        group["isactive"].get_to(r.isActive);
        group["curState"].get_to(r.curState);
        group["actState"].get_to(r.actState);
        group["relState"].get_to(r.relState);

        if (r.curState <= rs_beginvalue || r.curState >= rs_endvalue)
        {
            core.Trace("CharactersGroups::LoadDataRelations -> invalide curState value, set this neitral");
            r.curState = rs_neitral;
        }
        r.curState = static_cast<RelState>(r.curState);
        if (r.actState <= rs_beginvalue || r.actState >= rs_endvalue)
        {
            core.Trace("CharactersGroups::LoadDataRelations -> invalide actState value, set this enemy");
            r.actState = rs_enemy;
        }
        r.actState = static_cast<RelState>(r.actState);
        if (r.relState <= rs_beginvalue || r.relState >= rs_endvalue)
        {
            core.Trace("CharactersGroups::LoadDataRelations -> invalide relState value, set this neitral");
            r.relState = rs_neitral;
        }
        r.relState = static_cast<RelState>(r.relState);
    }
}

// Establish relationships for active groups
void CharactersGroups::RestoreStates()
{
    for (long i = 0, cnt = 0; i < numGroups; i++)
    {
        for (long j = 0; j < i; j++)
        {
            auto &r = FindRelation(i, j);
            const auto oldState = r.isActive;
            if (r.alarmdown > 0)
                r.alarm = 0.0f;
            if (r.alarm <= r.alarmmin)
                r.isActive = false;
            if (r.alarm >= r.alarmmax)
                r.isActive = true;
            if (oldState != r.isActive)
            {
                if (r.isActive)
                    r.curState = r.actState;
                else
                    r.curState = r.relState;
            }
        }
    }
    RemoveAllInvalidTargets();
}

// Display information about relationships
void CharactersGroups::DumpRelations()
{
    // Maintaining group relationships
    for (long i = 0; i < numGroups; i++)
    {
        for (long j = 0; j < i; j++)
        {
            core.Trace("\"%s\" <-> \"%s\"", groups[i]->name.name, groups[j]->name.name);
            // keep the relationship
            auto &r = FindRelation(i, j);
            core.Trace("alarm: %f", r.alarm);
            core.Trace("alarmdown: %f", r.alarmdown);
            core.Trace("alarmmin: %f", r.alarmmin);
            core.Trace("alarmmax: %f", r.alarmmax);
            core.Trace("isActive: %s", r.isActive ? "true" : "false");
            core.Trace("curState: \"%s\"", GetTextState(r.curState));
            core.Trace("actState: \"%s\"", GetTextState(r.actState));
            core.Trace("relState: \"%s\"", GetTextState(r.relState));
            core.Trace("");
        }
    }
    core.Trace("Groups info:");
    core.Trace("");
    for (long i = 0; i < numGroups; i++)
    {
        core.Trace("name: \"%s\"", groups[i]->name.name);
        core.Trace("    look: %f", groups[i]->look);
        core.Trace("    hear: %f", groups[i]->hear);
        core.Trace("    say: %f", groups[i]->say);
        core.Trace("");
    }
}

// Get the state as a string
const char *CharactersGroups::GetTextState(RelState state)
{
    switch (state)
    {
    case rs_friend:
        return "friend";
    case rs_neitral:
        return "neitral";
    case rs_enemy:
        return "enemy";
    }
    return "unknow value";
}
