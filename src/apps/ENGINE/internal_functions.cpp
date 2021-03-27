#include "compiler.h"
#include "core.h"

#include "compatibility.hpp"

#define INVALID_FA "Invalid function argument"
#define BAD_FA "Bad function argument"
#define MISSING_PARAMETER "Missing function parameter(s)"

extern bool bSteam;

//#define _TOFF

/*
char * FuncNameTable[]=
{
    "Rand",
    "frnd",
    "CreateClass",
    "CreateEntity",
    "DeleteClass",
    "SetEventHandler",
    "ExitProgram",
    "GetEventData",
    "Execute",
    "Stop",
    "SendMessage",
    "LoadSegment",
    "UnloadSegment",
    "Trace",
    "MakeInt",
    "MakeFloat",
    "LayerCreate",
    "LayerDelete",
    "LayerDeleteContent",
    "LayerSetRealize",
    "LayerSetExecute",
    "LayerSetMessages",
    "LayerAddObject",
    "LayerDelObject",
    "LayerFreeze",
    "abs",
    "sqrt",
    "sqr",
    "sin",
    "cos",
    "tan",
    "atan",
    "atan2",
    "DeleteAttribute",
    "SegmentIsLoaded",
    "GetAttributesNum",
    "GetAttributeN",
    "GetAttributeName",
    "DelEventHandler",
    "EntityUpdate",
    "IsEntity",
    "DumpAttributes",
    "sti",
    "stf",
    "CheckAttribute",
    "argb",
    "DeleteEntities",
    "ClearEvents",
    "SaveEngineState",
    "LoadEngineState",
    "Event",
    "PostEvent",
    "fts",
    "ClearPostEvents",
    "SetArraySize",
    "GetAttributeValue",
    "Vartype",
    "Breakpoint",
    "Pow",
    "CopyAttributes",
    "GetEntityPointer",
    "GetEntityNext",
    "GetEntityName",
    "strcut",
    "findSubStr",
    "ClearRef",
    "strlen",
    "GetDeltaTime",
    "EventsBreak",
    "shl",
    "shr",
    "and",
    "or",
    "DeleteEntitiesByType",
    "CreateControl",
    "DeleteControl",
    "MapControl",
    "SetControlFlags",
    "ClearEntityAP",
    "GetArraySize",
    "GetTargetPlatform",
    "FindClass",
    "GetSymbol",
    "IsDigit",
    "SaveVariable",
    "LoadVariable",
    "FindClassNext",
    "SetControlTreshold",
    "LockControl",
    "TestRef",
    "SetTimeScale",
};
*/

/*
DWORD FuncArguments[]=
{
    1,//"Rand",
    0,//"frnd",
    1,//"CreateClass",
    2,//"CreateEntity",
    1,//"DeleteClass",
    3,//"SetEventHandler",
    0,//"ExitProgram",
    0,//"GetEventData",
    1,//"Execute",
    0,//"Stop",
    0,//"SendMessage",
    1,//"LoadSegment",
    1,//"UnloadSegment",
    1,//"Trace",
    1,//"MakeInt",
    1,//"MakeFloat",
    2,//"LayerCreate",
    1,//"LayerDelete",
    1,//"LayerDeleteContent",
    2,//"LayerSetRealize",
    2,//"LayerSetExecute",
    2,//"LayerSetMessages",
    3,//"LayerAddObject",
    2,//"LayerDelObject",
    2,//"LayerFreeze",
    1,//"abs",
    1,//"sqrt",
    1,//"sqr",
    1,//"sin",
    1,//"cos",
    1,//"tan",
    1,//"atan",
    2,//"atan2",
    2,//"DeleteAttribute",
    1,//"SegmentIsLoaded",
    1,//"GetAttributesNum",
    2,//"GetAttributeN",
    1,//"GetAttributeName",
    2,//"DelEventHandler",
    1,//"EntityUpdate",
    1,//"IsEntity",
    1,//"DumpAttributes",
    1,//"sti",
    1,//"stf",
    2,// "CheckAttribute",
    4,//"argb",
    0,//"DeleteEntity"
    0,//"ClearEvents"
    1,//"SaveEngineState",
    1,//"LoadEngineState",
    0,//"Event",
    0,//"PostEvent",
    2,//"fts",
    0,//"ClearPostEvents",
    2,//"SetArraySize",
    1,//"GetAttributeValue",
    1,//"Vartype"
    0,//"Breakpoint",
    2,//"Pow",
    2,//"CopyAttributes",
    2,//"GetEntityPointer",
    1,//"GetEntityNext",
    1,//"GetEntityName",
    3,//"strcut",
    3,//"findSubStr",
    1,//"ClearRef",
    1,//"strlen"
    //2,//"SetSaveData"
    //1,//"GetSaveData"
    0,//"GetDeltaTime"
    0,//"EventsBreak"
    2,//"shl",
    2,//"shr",
    2,//"and",
    2,//"or",
    1,//"DeleteEntitiesByType",
    1,//"CreateControl",
    1,//"DeleteControl",
    2,//"MapControl",
    2,//"SetControlFlags",
    1,//"ClearEntityAP"
    1,//"GetArraySize"
    0,//"GetTargetPlatform"
    2,//"FindClass"
    2,//"GetSymbol"
    2,//"IsDigit"
    2,//"SaveVariable",
    2,//"LoadVariable",
    1,//"FindClassNext",
    2,//"SetControlTreshold",
    2,//"LockControl",
    1,//"TestRef"
    1,//"SetTimeScale"
};
*/

bool COMPILER::IsIntFuncVarArgsNum(uint32_t code)
{
    // if(code == FUNC_SEND_MESSAGE) return true;
    switch (code)
    {
    case FUNC_SEND_MESSAGE:
    case FUNC_EVENT:
    case FUNC_POSTEVENT:
    case FUNC_LAYER_SET_REALIZE:
    case FUNC_LAYER_SET_EXECUTE:

        return true;
    }
    return false;
}

uint32_t COMPILER::GetIntFunctionCode(const char *func_name)
{
    // functions_num = sizeof(FuncNameTable)/sizeof(char *);
    const uint32_t functions_num = sizeof(IntFuncTable) / sizeof(INTFUNCDESC);

    for (uint32_t n = 0; n < functions_num; n++)
    {
        // if(strcmp(func_name,FuncNameTable[n])==0) return n;
        if (strcmp(func_name, IntFuncTable[n].pName) == 0)
            return n;
    }
    return INVALID_ORDINAL_NUMBER;
}

DATA *COMPILER::BC_CallIntFunction(uint32_t func_code, DATA *&pVResult, uint32_t arguments)
{
    //    char Format_string[MAX_PATH];
    char Message_string[2 * MAX_PATH];
    entid_t ent;
    uint32_t functions_num;
    uint32_t function_code;

    // functions_num = sizeof(FuncNameTable)/sizeof(char *);
    functions_num = sizeof(IntFuncTable) / sizeof(INTFUNCDESC);

    if (func_code >= functions_num)
        return nullptr;

    DATA *pResult;
    DATA *pV;
    DATA *pV2;
    DATA *pV3;
    DATA *pV4;

    DATA Access;
    Access.SetVCompiler(this);
    float TempFloat1;
    float TempFloat2;
    long TempLong1;
    long TempLong2;
    long TempLong;
    bool TempBool;
    const char *pChar;
    const char *pChar2;
    entid_t TempEid;
    entid_t pEid = 0;
    uint32_t n;
    Attribute *pA;
    Attribute *pRoot;
    Entity *pE;
    MESSAGE_SCRIPT ms;
    uint32_t s_off;

    static EntityManager::EntityVector *entVec;
    static EntityManager::EntityVector::const_iterator it;

    pResult = nullptr;
    TempFloat1 = 0;
    TempLong1 = 0;

    pVResult = nullptr; // default - no return value

    long slen, slen2;
    char sVarName[64];
    char sBuff2[2];

    switch (func_code)
    {
    case FUNC_GET_STEAM_ENABLED:
        pV = SStack.Push();
        pV->Set((long)core.isSteamEnabled());
        pVResult = pV;
        return pV;
        break;

    case FUNC_GET_DLC_ENABLED:
        if (bSteam)
        {
            pV = SStack.Pop();
            if (pV->GetType() == VAR_INTEGER)
            {
                pV->Get(TempLong1);
            }
            else
            {
                SetError("incorrect argument type");
                break;
            }
            TempBool = core.isDLCActive(TempLong1);
            pV = SStack.Push();
            if (TempBool)
                pV->Set((long)1);
            else
                pV->Set((long)0);
        }
        else
        {
            pV = SStack.Pop();
            pV = SStack.Push();
            pV->Set((long)1);
        }
        pVResult = pV;
        return pV;
        break;

    case FUNC_GET_DLC_COUNT:
        pV = SStack.Push();
        if (bSteam)
        {
            TempLong = core.getDLCCount();
            pV->Set(TempLong);
        }
        else
        {
            pV->Set((long)0);
        }
        pVResult = pV;
        return pV;
        break;

    case FUNC_GET_DLC_DATA:
        if (bSteam)
        {
            pV = SStack.Pop();
            if (pV->GetType() == VAR_INTEGER)
            {
                pV->Get(TempLong1);
            }
            else
            {
                SetError("incorrect argument type");
                break;
            }
            TempLong = core.getDLCDataByIndex(TempLong1);
            pV = SStack.Push();
            pV->Set(TempLong);
        }
        else
        {
            pV = SStack.Push();
            pV->Set((long)0);
        }
        pVResult = pV;
        return pV;
        break;

    case FUNC_DLC_START_OVERLAY:
        if (bSteam)
        {
            pV = SStack.Pop();
            if (pV->GetType() == VAR_INTEGER)
            {
                pV->Get(TempLong1);
            }
            else
            {
                SetError("incorrect argument type");
                break;
            }
            TempBool = core.activateGameOverlayDLC(TempLong1);
            pV = SStack.Push();
            if (TempBool)
                pV->Set((long)1);
            else
                pV->Set((long)0);
        }
        else
        {
            pV = SStack.Push();
            pV->Set((long)0);
        }
        pVResult = pV;
        return pV;
        break;

    case FUNC_GETENGINEVERSION:
        pV = SStack.Push();
        pV->Set(static_cast<long>(ENGINE_SCRIPT_VERSION));
        pVResult = pV;
        return pV;
        break;
    case FUNC_CHECKFUNCTION:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        if (pV->GetType() == VAR_STRING)
        {
            pV->Get(pChar);
            if (FuncTab.FindFunc(pChar) == INVALID_FUNC_CODE)
            {
                pV = SStack.Push();
                pV->Set(static_cast<long>(0));
            }
            else
            {
                pV = SStack.Push();
                pV->Set(static_cast<long>(1));
            }
            pVResult = pV;
        }
        else
            SetError("incorrect argument type");
        break;
    case FUNC_SETTIMESCALE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        if (pV->GetType() == VAR_FLOAT)
        {
            pV->Get(TempFloat1);
            core.SetTimeScale(TempFloat1);
        }
        else if (pV->GetType() == VAR_INTEGER)
        {
            pV->Get(TempLong1);
            core.SetTimeScale(static_cast<float>(TempLong1));
        }
        else
            SetError("incorrect argument type");
        break;
    case FUNC_TEST_REF:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (pV == nullptr)
        {
            TempLong1 = 0;
        }
        else
        {
            switch (pV->GetType())
            {
            case VAR_REFERENCE:
                if (pV->pReference)
                    TempLong1 = 1;
                else
                    TempLong1 = 0;
                break;
            case VAR_AREFERENCE:
                if (pV->AttributesClass)
                    TempLong1 = 1;
                else
                    TempLong1 = 0;
                break;
            default:
                TempLong1 = 1;
                break;
            }
        }
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_LOCK_CONTROL:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        pV2->Get(TempLong1);
        if (core.Controls != nullptr)
            core.Controls->LockControl(pChar, TempLong1 != 0);
        break;
        /*case FUNC_SAVEVARIABLE:
            pV = SStack.Pop(); if(!pV){SetError(INVALID_FA); break;};    // var ref
            pV2 = SStack.Pop(); if(!pV2){SetError(INVALID_FA); break;};    // file name

            pV2->Get(pChar);


            fh = fio->_CreateFile(pChar,GENERIC_WRITE,FILE_SHARE_READ,CREATE_ALWAYS);
            if(fh == INVALID_HANDLE_VALUE)
            {
                pV = SStack.Push();
                pV->Set((long)0);
                pVResult = pV;
                return pV;
            }

            hSaveFileFileHandle = fh;
            nIOFullSize = 0;
            nIOBufferSize = IOBUFFER_SIZE;
            pIOBuffer = (char *)RESIZE(pIOBuffer,IOBUFFER_SIZE);
            if(!pIOBuffer)
            {
                fio->_CloseHandle(fh);
                pV = SStack.Push();
                pV->Set((long)0);
                pVResult = pV;
                return pV;
            }

            SaveVariable(pV->GetVarPointer());

            if(nIOFullSize > 0)
            {
                DWORD bytes;
                fio->_WriteFile(hSaveFileFileHandle,pIOBuffer,nIOFullSize,&bytes);
            }
            fio->_CloseHandle(fh);
            delete pIOBuffer; pIOBuffer = 0;
            nIOBufferSize = 0;
            nIOFullSize = 0;

            pV = SStack.Push();
            pV->Set((long)1);
            pVResult = pV;
        return pV;

        case FUNC_LOADVARIABLE:
            pV = SStack.Pop(); if(!pV){SetError(INVALID_FA); break;};    // var name
            pV2 = SStack.Pop(); if(!pV2){SetError(INVALID_FA); break;};    // file name

            pV2->Get(pChar);

            pV->Get(pChar2);


            fh = fio->_CreateFile(pChar,GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING);
            if(fh == INVALID_HANDLE_VALUE)
            {
                pV = SStack.Push();
                pV->Set((long)0);
                pVResult = pV;
                return pV;
            }

            hSaveFileFileHandle = fh;

            ReadVariable(pChar2);

            fio->_CloseHandle(fh);
            pV = SStack.Push();
            pV->Set((long)1);
            pVResult = pV;
        return pV;*/

    case FUNC_ISDIGIT:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = pV2->GetVarPointer(); // string
        pV->Get(TempLong1);
        pV2->Get(pChar);
        if (static_cast<uint32_t>(TempLong1) >= strlen(pChar))
        {
            pV = SStack.Push();
            pV->Set(static_cast<long>(0));
            pVResult = pV;
            return pV;
        }
        pV = SStack.Push();
        if (pChar[TempLong1] >= 0x30 && pChar[TempLong1] <= 0x39)
            pV->Set(static_cast<long>(1));
        else
            pV->Set(static_cast<long>(0));
        pVResult = pV;
        return pV;

        break;
    case FUNC_GETSYMBOL:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = pV2->GetVarPointer(); // string
        pV->Get(TempLong1);
        pV2->Get(pChar);
        if (static_cast<uint32_t>(TempLong1) >= strlen(pChar))
        {
            pV = SStack.Push();
            pV->Set("");
            pVResult = pV;
            return pV;
        }
        sBuff2[0] = pChar[TempLong1];
        sBuff2[1] = 0;
        pV = SStack.Push();
        pV->Set(sBuff2);
        pVResult = pV;
        return pV;

    case FUNC_GETENTITY:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }

        pV->Convert(VAR_STRING);
        pV->Get(pChar);

        ent = EntityManager::GetEntityId(pChar);

        pV2 = pV2->GetVarPointer();
        pV2->Set(ent);
        pV2->SetType(VAR_AREFERENCE);
        pV2->SetAReference(core.Entity_GetAttributePointer(ent));

        if (EntityManager::GetEntityPointer(ent))
            TempLong1 = 1;
        else
            TempLong1 = 0;

        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_FINDENTITY:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }

        pV->Convert(VAR_STRING);
        pV->Get(pChar);

        delete entVec;
        entVec = new EntityManager::EntityVector(EntityManager::GetEntityIdVector(pChar));
        it = entVec->begin();
        ent = *it;

        pV2 = pV2->GetVarPointer();
        pV2->Set(ent);
        pV2->SetType(VAR_AREFERENCE);
        pV2->SetAReference(core.Entity_GetAttributePointer(ent));

        if (EntityManager::GetEntityPointer(ent))
            TempLong1 = 1;
        else
            TempLong1 = 0;
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_FINDENTITYNEXT:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }

        ++it;
        ent = it != entVec->end() ? *it : invalid_entity;

        pV2 = pV2->GetVarPointer();
        pV2->Set(ent);
        pV2->SetType(VAR_AREFERENCE);
        pV2->SetAReference(core.Entity_GetAttributePointer(ent));

        if (EntityManager::GetEntityPointer(ent))
            TempLong1 = 1;
        else
            TempLong1 = 0;

        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_GETTARGETPLATFORM:
        pV = SStack.Push();
        pV->Set("pc");
        pVResult = pV;
        return pV;

    case FUNC_CLEAR_Entity_AP:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempEid);
        core.Entity_SetAttributePointer(TempEid, nullptr);
        break;
    case FUNC_CREATE_CONTROL:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        if (core.Controls != nullptr)
            TempLong1 = core.Controls->CreateControl(pChar);
        else
            TempLong1 = -1;
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_DELETE_CONTROL:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);

        break;

    case FUNC_MAP_CONTROL:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2->Get(TempLong1);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong2);

        core.Controls->MapControl(TempLong2, TempLong1);
        break;

    case FUNC_SET_CONTROL_TRESHOLD:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2->Get(TempFloat1);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong2);

        core.Controls->SetControlTreshold(TempLong2, TempFloat1);

        break;

    case FUNC_SET_CONTROL_FLAGS:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2->Get(TempLong1);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong2);

        core.Controls->SetControlFlags(TempLong2, TempLong1);
        break;

    case FUNC_DELETEENTITIESBYTYPE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        {
            const auto entities = EntityManager::GetEntityIdVector(pChar);
            for (auto ent : entities)
            {
                EntityManager::EraseEntity(ent);
            }
        }
        break;
    case FUNC_SHL:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong2);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong1);
        TempLong1 = TempLong1 << TempLong2;

        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_SHR:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong2);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong1);
        TempLong1 = TempLong1 >> TempLong2;

        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_AND:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong2);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong1);
        TempLong1 = TempLong1 & TempLong2;

        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_OR:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong2);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong1);
        TempLong1 = TempLong1 | TempLong2;

        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_EVENTSBREAK:
        bEventsBreak = true;
        break;
    case FUNC_GETDELTATIME:
        pV = SStack.Push();
        pV->Set(static_cast<long>(core.GetDeltaTime()));
        pVResult = pV;
        return pV;

        /*case FUNC_SETSAVEDATA:
            pV = SStack.Pop(); if(!pV){SetError(INVALID_FA);break;}
            pV2 = SStack.Pop(); if(!pV2){SetError(INVALID_FA);break;}

            pV->Get(pChar);
            pV2->Get(pChar2);
            SetSaveData(pChar2,pChar);

            pV = SStack.Push();    pV->Set((long)1);    pVResult = pV;
        return pV;

        case FUNC_GETSAVEDATA:
            pV = SStack.Pop(); if(!pV){SetError(INVALID_FA);break;}
            pV->Get(pChar);
            GetSaveData(pChar,&Access);
            pV = SStack.Push();
            Access.Get(pChar);

            pV->Set(pChar);    pVResult = pV;
        return pV;*/

    case FUNC_STRLEN:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        } // string or ref
        pV = pV->GetVarPointer();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        if (pV->GetType() != VAR_STRING)
        {
            SetError("invalid argument type");
            break;
        }
        pV->Get(pChar);
        if (pChar == nullptr)
        {
            TempLong1 = 0;
        }
        else
        {
            TempLong1 = strlen(pChar);
        }
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;
    case FUNC_CLEARREF:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        switch (pV->GetType())
        {
        case VAR_REFERENCE:
            pV->SetReference(nullptr);
            break;
        case VAR_AREFERENCE:
            pV->SetAReference(nullptr);
            break;
        }
        break;

    case FUNC_STRCUT:

        pV3 = SStack.Pop();
        if (!pV3)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        if (pChar == nullptr)
        {
            SetError("Invalid string argument");
            pV = SStack.Push();
            pV->Set("");
            pVResult = pV;
            return pV;
        }
        slen = strlen(pChar);
        pV2->Get(TempLong1);
        pV3->Get(TempLong2);
        if (TempLong1 > TempLong2 || TempLong1 >= slen || TempLong2 >= slen)
        {
            SetError("Invalid range");
            pV = SStack.Push();
            pV->Set("");
            pVResult = pV;
            return pV;
        }

        if (TempLong1 == TempLong2)
        {
            Message_string[0] = pChar[TempLong1];
            Message_string[1] = 0;
            pV = SStack.Push();
            pV->Set(Message_string);
            pVResult = pV;
            return pV;
        }
        if (TempLong2 - TempLong1 >= sizeof(Message_string))
        {
            SetError("internal: buffer too small");
            pV = SStack.Push();
            pV->Set("");
            pVResult = pV;
            return pV;
        }
        memcpy(Message_string, pChar + TempLong1, TempLong2 - TempLong1 + 1);
        Message_string[TempLong2 - TempLong1 + 1] = 0;
        pV = SStack.Push();
        pV->Set(Message_string);
        pVResult = pV;
        return pV;
        break;

    case FUNC_FINDSUBSTR:
        pV3 = SStack.Pop();
        if (!pV3)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        pV2->Get(pChar2);
        pV3->Get(TempLong1);
        if (pChar == nullptr || pChar2 == nullptr)
        {
            SetError("Invalid string argument");
            pV = SStack.Push();
            pV->Set("");
            pVResult = pV;
            return pV;
        }
        slen = strlen(pChar);
        slen2 = strlen(pChar2);
        if (slen < slen2)
        {
            pV = SStack.Push();
            pV->Set(static_cast<long>(-1));
            pVResult = pV;
            return pV;
        }

        n = TempLong1;
        while (n + static_cast<uint32_t>(slen2) <= static_cast<uint32_t>(slen))
        {
            if (_strnicmp(pChar + n, pChar2, slen2) == 0)
            {
                pV = SStack.Push();
                pV->Set(static_cast<long>(n));
                pVResult = pV;
                return pV;
            }
            n++;
        }
        pV = SStack.Push();
        pV->Set(static_cast<long>(-1));
        pVResult = pV;
        return pV;

    case FUNC_GETEntityNAME: {
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        };
        pV->Get(ent);
        pV = SStack.Push();
        VMA *pClass;
        const auto class_code = EntityManager::GetClassCode(EntityManager::GetEntityId(ent));
        pClass = core.FindVMA(class_code);
        if (pClass)
            pV->Set(pClass->GetName());
        else
            pV->Set("unknown class");
        pVResult = pV;
        return pV;
    }
        /*case FUNC_GETEntity:
            pV2 = SStack.Pop(); if(!pV2){SetError(INVALID_FA);break;};

            pV = SStack.Pop(); if(!pV){SetError(INVALID_FA);break;};
            pV->Convert(VAR_STRING);
            pV->Get(pChar);

      walker = core.LayerGetWalker(pChar);
            ent = walker();

            pV2 = pV2->GetVarPointer();
            pV2->Set(ent);
            pV2->SetType(VAR_AREFERENCE);
      pV2->SetAReference(core.Entity_GetAttributePointer(ent));

            if(EntityManager::GetEntityPointer(ent)) TempLong1 = 1;
            else TempLong1 = 0;
            pV = SStack.Push();
            pV->Set(TempLong1);
            pVResult = pV;
        return pV;*/
    case FUNC_GETEntityNEXT: {
        SetError(INVALID_FA);
        break;

//        const auto class_code = EntityManager::GetClassCode(EntityManager::GetEntityId(ent));
//        VMA *pClass = core.FindVMA(class_code);
//        ent = pClass->Next()->GetHash();
//        pV = SStack.Pop(); if(!pV){SetError(INVALID_FA);break;};
//        pV->GetVarPointer();
//        pV->Set(ent);
//        pV->SetType(VAR_AREFERENCE);
//        pV->SetAReference(core.Entity_GetAttributePointer(ent));
//        if(EntityManager::GetEntityPointer(ent)) TempLong1 = 1;
//        else TempLong1 = 0;
//        pV = SStack.Push();
//        pV->Set(TempLong1);
//        pVResult = pV;
//        return pV;
    }
    case FUNC_POW:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Convert(VAR_FLOAT);
        pV->Get(TempFloat1);

        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2->Convert(VAR_FLOAT);
        pV2->Get(TempFloat2);

        TempFloat1 = static_cast<float>(pow(TempFloat2, TempFloat1));
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;

    case FUNC_BREAKPOINT:
#ifdef _DEBUG
        __debugbreak();
#endif
        break;
    case FUNC_VARTYPE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetReference();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        if (pV->IsReference())
            strcpy_s(sVarName, "ref:");
        else
            sVarName[0] = 0;
        pV = pV->GetVarPointer();
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            strcat_s(sVarName, "int");
            break;
        case VAR_FLOAT:
            strcat_s(sVarName, "float");
            break;
        case VAR_STRING:
            strcat_s(sVarName, "string");
            break;
        case VAR_OBJECT:
            strcat_s(sVarName, "object");
            break;
        case VAR_REFERENCE:
            strcat_s(sVarName, "ref");
            break;
        case VAR_AREFERENCE:
            strcat_s(sVarName, "aref");
            break;
        }
        pV = SStack.Push();
        pV->Set(sVarName);
        pVResult = pV;
        return pV;

    case FUNC_SET_ARRAY_SIZE:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        if (pV->GetType() != VAR_REFERENCE)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (pV == nullptr)
        {
            SetError(INVALID_FA);
            break;
        }
        if (!pV->IsArray())
        {
            SetError("Not array");
            break;
        }
        pV2->Get(TempLong1);
        if (TempLong1 <= 0)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->SetElementsNum(TempLong1);

        if (pV->nGlobalVarTableIndex != 0xffffffff)
            VarTab.ArraySizeChanged(pV->nGlobalVarTableIndex, TempLong1);

        break;

    case FUNC_GET_ARRAY_SIZE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        if (pV->GetType() != VAR_REFERENCE)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (pV == nullptr)
        {
            SetError(INVALID_FA);
            break;
        }
        if (!pV->IsArray())
        {
            SetError("Not array");
            break;
        }
        TempLong1 = pV->GetElementsNum();
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

        /*case FUNC_LAYER_DELETE_CONTENT:
            pV = SStack.Pop(); if(!pV) {SetError(INVALID_FA); break;};
            pV->Get(pChar);
      core.LayerDeleteContent(pChar);*/
        break;
    case FUNC_LAYER_SET_REALIZE:
        if (arguments == 2) {
            pV2 = SStack.Pop();
            if (!pV2)
            {
                SetError(INVALID_FA);
                break;
            }
        }

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        if (pV->GetType() == VAR_INTEGER) {
            pV->Get(TempLong1);
        }
        else if (ENABLE_SCRIPT_COMPATIBILITY && pV->GetType() == VAR_STRING) {
            const auto layerId = GetLayerIDByOldName(pV->GetString());
            if (!layerId) {
                SetError(INVALID_FA);
                break;
            }
            TempLong1 = layerId.value();
        }
        else {
            SetError(INVALID_FA);
            break;
        }

        // pV2->Get(TempLong1);
        // if(TempLong1 == 0) core.LayerSetRealize(pChar,false);
        // else core.LayerSetRealize(pChar,true);
        EntityManager::SetLayerType(TempLong1, EntityManager::Layer::Type::realize);
        break;
    case FUNC_LAYER_SET_EXECUTE:
        if (arguments == 2) {
            pV2 = SStack.Pop();
            if (!pV2)
            {
                SetError(INVALID_FA);
                break;
            }
        }

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        if (pV->GetType() == VAR_INTEGER) {
            pV->Get(TempLong1);
        }
        else if (ENABLE_SCRIPT_COMPATIBILITY && pV->GetType() == VAR_STRING) {
            const auto layerId = GetLayerIDByOldName(pV->GetString());
            if (!layerId) {
                SetError(INVALID_FA);
                break;
            }
            TempLong1 = layerId.value();
        }
        else {
            SetError(INVALID_FA);
            break;
        }

        // pV2->Get(TempLong1);
        // if(TempLong1 == 0) core.LayerSetExecute(pChar,false);
        // else core.LayerSetExecute(pChar,true);
        EntityManager::SetLayerType(TempLong1, EntityManager::Layer::Type::execute);
        break;
        /*case FUNC_LAYER_SET_MESSAGES:
            pV2 = SStack.Pop(); if(!pV2){SetError(INVALID_FA);break;};
            pV = SStack.Pop(); if(!pV){SetError(INVALID_FA);break;};
            pV->Get(pChar);
            pV2->Get(TempLong1);
      if(TempLong1 == 0) core.LayerClrFlags(pChar,LRFLAG_SYS_MESSAGES);
      else core.LayerSetFlags(pChar,LRFLAG_SYS_MESSAGES);
        break;*/
    case FUNC_LAYER_ADDOBJECT:
        pV3 = SStack.Pop();
        if (!pV3)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        if (pV->GetType() == VAR_INTEGER) {
            pV->Get(TempLong2);
        }
        else if (ENABLE_SCRIPT_COMPATIBILITY && pV->GetType() == VAR_STRING) {
            const auto layerId = GetLayerIDByOldName(pV->GetString());
            if (!layerId) {
                SetError(INVALID_FA);
                break;
            }
            TempLong2 = layerId.value();
        }
        else {
            SetError(INVALID_FA);
            break;
        }

        pV2->Get(TempEid);
        pV3->Get(TempLong1);
        EntityManager::AddToLayer(TempLong2, TempEid, TempLong1);
        break;
    case FUNC_LAYER_DELOBJECT:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        if (pV->GetType() == VAR_INTEGER) {
            pV->Get(TempLong2);
        }
        else if (ENABLE_SCRIPT_COMPATIBILITY && pV->GetType() == VAR_STRING) {
            const auto layerId = GetLayerIDByOldName(pV->GetString());
            if (!layerId) {
                SetError(INVALID_FA);
                break;
            }
            TempLong2 = layerId.value();
        }
        else {
            SetError(INVALID_FA);
            break;
        }

        pV2->Get(TempEid);
        EntityManager::RemoveFromLayer(TempLong2, TempEid);
        break;
    case FUNC_LAYER_FREEZE:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        if (pV->GetType() == VAR_INTEGER) {
            pV->Get(TempLong2);
        }
        else if (ENABLE_SCRIPT_COMPATIBILITY && pV->GetType() == VAR_STRING) {
            const auto layerId = GetLayerIDByOldName(pV->GetString());
            if (!layerId) {
                SetError(INVALID_FA);
                break;
            }
            TempLong2 = layerId.value();
        }
        else {
            SetError(INVALID_FA);
            break;
        }

        pV2->Get(TempLong1);
        EntityManager::SetLayerFrozen(TempLong2, TempLong1);
        break;

    case FUNC_IS_Entity_LOADED:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        pV->Get(TempEid);
        pV = SStack.Push();
        if (EntityManager::GetEntityPointer(TempEid) != nullptr)
            TempLong1 = 1;
        else
            TempLong1 = 0;
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;

    case FUNC_Entity_UPDATE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong1);
        if (TempLong1)
            bEntityUpdate = true;
        else
            bEntityUpdate = false;
        break;

    case FUNC_FRAND:
        TempFloat1 = static_cast<float>(rand()) / RAND_MAX;
        pV = SStack.Push();
        // TempFloat1 = 1.0f;    // ***
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;

    case FUNC_RAND: {
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        const auto result = func_rand(pV->Get<long>());
        pVResult = SStack.Push(result);

        return pVResult;
    }
        // create entity
    case FUNC_CREATE_Entity:

        pV2 = SStack.Pop(); // class name
        if (!pV2)
        {
            SetError(MISSING_PARAMETER);
            break;
        }
        // pV = SStack.Pop();    // object reference
        pV = SStack.Read(); // object reference
        if (!pV)
        {
            SetError(MISSING_PARAMETER);
            break;
        }

        pV2->Get(pChar);
        if (ent = EntityManager::CreateEntity(pChar, pV->GetAClass()))
        {
            // core.Entity_SetAttributePointer(&entid_t,pV->GetAClass());
            pV->Set(ent);
            SStack.Pop();
            pV = SStack.Push();
            pV->Set(static_cast<long>(1));
            pVResult = pV;
            return pV;
        }
        SStack.Pop();
        pV = SStack.Push();
        pV->Set(static_cast<long>(0));
        pVResult = pV;
        SetError("Cant create class: %s", pChar);
        return pV;
        break;

    case FUNC_CREATE_CLASS:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        if (ent = EntityManager::CreateEntity(pChar))
        {
            pV = SStack.Push();
            pV->Set(ent);
            pVResult = pV;
            return pV;
        }
        SetError("Cant create class: %s", pChar);
        break;
        //
    case FUNC_DELETE_Entity:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(ent);
        EntityManager::EraseEntity(ent);
        break;
        //
    case FUNC_DEL_EVENT_HANDLER:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        pV2->Get(pChar2);
        DelEventHandler(pChar, pChar2);
        break;
    case FUNC_SET_EVENT_HANDLER:
        pV3 = SStack.Pop();
        if (!pV3)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        pV2->Get(pChar2);
        pV3->Get(TempLong1);
        SetEventHandler(pChar, pChar2, TempLong1);
        break;
        //
    case FUNC_EXIT_PROGRAM:
        ExitProgram();
        // core.Exit();
        break;
        //
    case FUNC_GET_EVENTDATA:
        if (pEventMessage == nullptr)
        {
            SetError("No data on this event");
            return nullptr;
        }
        char format_sym;
        format_sym = pEventMessage->GetCurrentFormatType();
        if (format_sym == 0)
        {
            SetError("No (more) data on this event");
            return nullptr;
        }
        switch (format_sym)
        {
        case 'a':
            pResult = SStack.Push();
            pResult->SetType(VAR_AREFERENCE);
            pResult->SetAReference(pEventMessage->AttributePointer());
            pVResult = pResult;
            return pResult;
        case 'l':
            pResult = SStack.Push();
            pResult->Set(pEventMessage->Long());
            pVResult = pResult;
            return pResult;
        case 'f':
            pResult = SStack.Push();
            pResult->Set(pEventMessage->Float());
            pVResult = pResult;
            return pResult;
        case 's':
            pResult = SStack.Push();
            pEventMessage->String(sizeof(Message_string), Message_string);
            pResult->Set(Message_string);
            pVResult = pResult;
            return pResult;
        case 'i':
            pResult = SStack.Push();
            pResult->SetType(VAR_AREFERENCE);
            ent = pEventMessage->EntityID();
            pResult->Set(ent);
            pResult->SetAReference(core.Entity_GetAttributePointer(ent));

            pVResult = pResult;
            return pResult;
        case 'e':
            pResult = SStack.Push();
            DATA *pE;
            pE = static_cast<DATA *>(pEventMessage->ScriptVariablePointer());
            pResult->SetReference(pE);
            pVResult = pResult;
            return pResult;
        default:
            SetError("Invalid data type in event message: '%c'", format_sym);
            return nullptr;
        }
        break;
        /*case FUNC_EXECUTE:
            pV = SStack.Pop(); if(!pV){SetError(INVALID_FA);break;};
            pV->Get(pChar);
      core.Execute(pChar);
        break;*/
    case FUNC_LOAD_SEGMENT:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        Access.Copy(pV);
        // pV->Get(pChar);
        Access.Get(pChar);
        pV = SStack.Push();
        if (BC_LoadSegment(pChar))
            pV->Set(static_cast<long>(1));
        else
            pV->Set(static_cast<long>(0));
        pVResult = pV;
        return pV;
        //
    case FUNC_UNLOAD_SEGMENT:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        UnloadSegment(pChar);
        break;
    case FUNC_SEGMENT_IS_LOADED:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        pV = SStack.Push();
        if (BC_SegmentIsLoaded(pChar))
            pV->Set(static_cast<long>(1));
        else
            pV->Set(static_cast<long>(0));
        pVResult = pV;
        break;
        //
    case FUNC_STOP:
        bCompleted = true;
        break;
        //

    case FUNC_EVENT:
        s_off = SStack.GetDataNum() - arguments; // set stack offset
        pV = SStack.Read(s_off, 0);
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        if (arguments > 1)
        {
            CreateMessage(&ms, s_off, 1);
            ms.ResetIndex();
            ProcessEvent(pChar, &ms);
        }
        else
            ProcessEvent(pChar);
        for (n = 0; n < arguments; n++)
        {
            SStack.Pop();
        }
        // set stack pointer to correct position (vars in stack remain valid)
        break;
    case FUNC_POSTEVENT:
        MESSAGE_SCRIPT *pMS;
        S_EVENTMSG *pEM;
        s_off = SStack.GetDataNum() - arguments; // set stack offset
        pV = SStack.Read(s_off, 0);
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(pChar);
        pV = SStack.Read(s_off, 1);
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong1);
        if (arguments >= 4) // event w/o message
        {
            pMS = new MESSAGE_SCRIPT;
            CreateMessage(pMS, s_off, 2);
            pMS->ResetIndex();
        }
        else
            pMS = nullptr;

        pEM = new S_EVENTMSG(pChar, pMS, TempLong1);
        EventMsg.Add(pEM);
        for (n = 0; n < arguments; n++)
        {
            SStack.Pop();
        }
        break;
    case FUNC_SEND_MESSAGE: {
        s_off = SStack.GetDataNum() - arguments; // set stack offset

        pV = SStack.Read(s_off, 0);
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(ent);

        CreateMessage(&ms, s_off, 1);

        uint64_t mresult = -1;
        pE = EntityManager::GetEntityPointer(ent);
        if (pE)
        {
            ms.ResetIndex();
            mresult = pE->ProcessMessage(ms);
        }
        for (n = 0; n < arguments; n++)
        {
            SStack.Pop();
        }
        // set stack pointer to correct position (vars in stack remain valid)

        pV = SStack.Push();
        pV->SetPtr(mresult); // SendMessage returns uint64_t, could be truncated to 32
        pVResult = pV;

        return pV;
        // break;
    }
    case FUNC_TRACE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->RefConvert();
        pV->Convert(VAR_STRING);
        pV->Get(pChar);
#ifndef _TOFF
        DTrace(pChar);
#endif
        break;
    case FUNC_STI:
    case FUNC_MAKE_INT:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Convert(VAR_INTEGER);
        pV->Get(TempLong1);
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;
    case FUNC_STF:
    case FUNC_MAKE_FLOAT:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Convert(VAR_FLOAT);
        pV->Get(TempFloat1);
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;
    case FUNC_FTS:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        if (pV->GetType() != VAR_FLOAT)
        {
            SetError(INVALID_FA);
            break;
        }
        if (pV2->GetType() != VAR_INTEGER)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempFloat1);
        pV2->Get(TempLong1);
        pV = SStack.Push();
        _gcvt(TempFloat1, TempLong1, gs);
        pV->Set(gs);
        pVResult = pV;
        return pV;
    case FUNC_ABS:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            TempLong1 = abs(TempLong1);
            pV = SStack.Push();
            pV->Set(TempLong1);
            pVResult = pV;
            return pV;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            TempFloat1 = static_cast<float>(fabs(TempFloat1));
            pV = SStack.Push();
            pV->Set(TempFloat1);
            pVResult = pV;
            return pV;
        default:
            SetError("Invalid func 'abs' argument");
            return nullptr;
        }
        break;

    case FUNC_SQRT:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            if (TempLong1 < 0)
            {
                SetError("Negative func 'sqrt' argument");
                return nullptr;
            }
            TempLong1 = static_cast<long>(sqrtf(static_cast<float>(TempLong1)));
            pV = SStack.Push();
            pV->Set(TempLong1);
            pVResult = pV;
            return pV;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            if (TempFloat1 < 0)
            {
                SetError("Negative func 'sqrt' argument");
                return nullptr;
            }
            TempFloat1 = static_cast<float>(sqrt(TempFloat1));
            pV = SStack.Push();
            pV->Set(TempFloat1);
            pVResult = pV;
            return pV;
        default:
            SetError("Invalid func 'sqrt' argument");
            return nullptr;
        }
        break;
    case FUNC_SQR:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            TempLong1 = TempLong1 * TempLong1;
            pV = SStack.Push();
            pV->Set(TempLong1);
            pVResult = pV;
            return pV;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            TempFloat1 = TempFloat1 * TempFloat1;
            pV = SStack.Push();
            pV->Set(TempFloat1);
            pVResult = pV;
            return pV;
        default:
            SetError("Invalid func 'sqr' argument");
            return nullptr;
        }
        break;
    case FUNC_SIN:
        pV = SStack.Pop();
        if (pV == nullptr)
        {
            SetError("Missing func 'sin' argument(s)");
            return nullptr;
        }
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            TempFloat1 = static_cast<float>(sinf(static_cast<float>(TempLong1)));
            break;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            TempFloat1 = static_cast<float>(sin(TempFloat1));
            break;
        default:
            SetError("Invalid func 'sin' argument");
            return nullptr;
        }
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;

    case FUNC_COS:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            TempFloat1 = static_cast<float>(cosf(static_cast<float>(TempLong1)));
            break;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            TempFloat1 = static_cast<float>(cos(TempFloat1));
            break;
        default:
            SetError("Invalid func 'cos' argument");
            return nullptr;
        }
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;
    case FUNC_TAN:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            TempFloat1 = static_cast<float>(tanf(static_cast<float>(TempLong1)));
            break;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            TempFloat1 = static_cast<float>(tan(TempFloat1));
            break;
        default:
            SetError("Invalid func 'tan' argument");
            return nullptr;
        }
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;
    case FUNC_ATAN:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            TempFloat1 = static_cast<float>(atanf(static_cast<float>(TempLong1)));
            break;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            TempFloat1 = static_cast<float>(atan(TempFloat1));
            break;
        default:
            SetError("Invalid func 'atan' argument");
            return nullptr;
        }
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;
    case FUNC_ATAN2:

        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        switch (pV->GetType())
        {
        case VAR_FLOAT:
        case VAR_INTEGER:
            pV->Convert(VAR_FLOAT);
            switch (pV->GetType())
            {
            case VAR_FLOAT:
            case VAR_INTEGER:
                pV2->Convert(VAR_FLOAT);
                break;
            default:
                SetError("Invalid func 'atan2' argument");
                return nullptr;
            }
            pV->Get(TempFloat1);
            pV2->Get(TempFloat2);
            TempFloat1 = static_cast<float>(atan2(TempFloat1, TempFloat2));
            pV = SStack.Push();
            pV->Set(TempFloat1);
            pVResult = pV;
            return pV;
        default:
            SetError("Invalid func 'atan2' argument");
            return nullptr;
        }
        break;
    case FUNC_ASIN:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        };
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            if (TempLong1 < -1 || TempLong1 > 1)
            {
                SetError("Illegal func 'asin' argument");
                return nullptr;
            }
            TempFloat1 = (float)asinf((float)TempLong1);
            break;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            if (TempFloat1 < -1.0f || TempFloat1 > 1.0f)
            {
                SetError("Illegal func 'asin' argument");
                return nullptr;
            }
            TempFloat1 = (float)asin(TempFloat1);
            break;
        default:
            SetError("Invalid func 'asin' argument");
            return nullptr;
        }
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;
    case FUNC_ACOS:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        };
        switch (pV->GetType())
        {
        case VAR_INTEGER:
            pV->Get(TempLong1);
            if (TempLong1 < -1 || TempLong1 > 1)
            {
                SetError("Illegal func 'acos' argument");
                return nullptr;
            }
            TempFloat1 = (float)acosf((float)TempLong1);
            break;
        case VAR_FLOAT:
            pV->Get(TempFloat1);
            if (TempFloat1 < -1.0f || TempFloat1 > 1.0f)
            {
                SetError("Illegal func 'acos' argument");
                return nullptr;
            }
            TempFloat1 = (float)acos(TempFloat1);
            break;
        default:
            SetError("Invalid func 'acos' argument");
            return nullptr;
        }
        pV = SStack.Push();
        pV->Set(TempFloat1);
        pVResult = pV;
        return pV;
    case FUNC_COPYATTRIBUTES:
        // source
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        // destination
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        pRoot = pV->GetAClass();
        pA = pV2->GetAClass();

        if (pA == nullptr || pRoot == nullptr)
        {
            SetError("AClass ERROR n1");
            break;
        }
        *pRoot = *pA;
        break;
    case FUNC_DELETE_ATTRIBUTE:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2->Get(pChar);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        // pV->Get(TempEid);
        pRoot = pV->GetAClass();
        if (pRoot == nullptr)
        {
            SetError("AClass ERROR n1");
            break;
        }
        pA = &pRoot->getProperty(pChar);
        pA->clear();
        break;
    case FUNC_CHECK_ATTRIBUTE:
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2->Get(pChar);
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        pV = pV->GetVarPointer();
        if (!pV)
            TempLong1 = 0;
        else
        {
            switch (pV->GetType())
            {
            case VAR_AREFERENCE:
                if (!pV->AttributesClass)
                {
                    TempLong1 = 0;
                    break;
                }

            default:
                pRoot = pV->GetAClass();
                if (pRoot)
                {
                    pA = &pRoot->getProperty(pChar);
                    if (!pA->empty())
                        TempLong1 = 1;
                    else
                        TempLong1 = 0;
                }
                else
                    TempLong1 = 0;
                break;
            }
        }
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;
    case FUNC_GET_ATTRIBUTES_NUM:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (!(pV->GetType() == VAR_AREFERENCE || pV->GetType() == VAR_OBJECT))
        {
            SetError(BAD_FA);
            break;
        }
        pA = pV->GetAClass();
        if (pA)
            TempLong1 = std::distance(pA->begin(), pA->end());
        else
            TempLong1 = 0;
        pV = SStack.Push();
        pV->Set(TempLong1);
        pVResult = pV;
        return pV;
    case FUNC_GET_ATTRIBUTE_BYN:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Get(TempLong1);

        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (!(pV->GetType() == VAR_AREFERENCE || pV->GetType() == VAR_OBJECT))
        {
            SetError(BAD_FA);
            break;
        }
        pA = pV->GetAClass();

        if (pA)
            pA = &pA->getProperty(TempLong1);
        if (pA == nullptr)
        {
            SetError("incorrect argument index");
            break;
        }
        pV = SStack.Push();
        pV->SetType(VAR_AREFERENCE);
        pV->SetAReference(pA);
        pVResult = pV;
        return pV;
    case FUNC_GET_ATTRIBUTE_VALUE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (!(pV->GetType() == VAR_AREFERENCE || pV->GetType() == VAR_OBJECT))
        {
            SetError(BAD_FA);
            break;
        }
        pA = pV->GetAClass();
        if (pA)
            pChar = pA->get<const char*>();
        else
            pChar = "AClass ERROR n1";
        pV = SStack.Push();
        pV->Set(pChar);
        pVResult = pV;
        return pV;
    case FUNC_GET_ATTRIBUTE_NAME:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (!(pV->GetType() == VAR_AREFERENCE || pV->GetType() == VAR_OBJECT))
        {
            SetError(BAD_FA);
            break;
        }
        pA = pV->GetAClass();
        if (pA)
            pChar = pA->getName().data();
        else
            pChar = "AClass ERROR n1";
        pV = SStack.Push();
        pV->Set(pChar);
        pVResult = pV;
        return pV;
    case FUNC_DUMP_ATTRIBUTES:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = pV->GetVarPointer();
        if (!(pV->GetType() == VAR_AREFERENCE || pV->GetType() == VAR_OBJECT))
        {
            SetError(BAD_FA);
            break;
        }
        pA = pV->GetAClass();
        if (pA == nullptr)
        {
            SetError("AClass ERROR n1");
            break;
        }
#ifndef _TOFF
        DumpAttributes(pA, 0);
#endif
        break;
    case FUNC_ARGB:
        pV4 = SStack.Pop();
        if (!pV4)
        {
            SetError(INVALID_FA);
            break;
        }
        pV3 = SStack.Pop();
        if (!pV3)
        {
            SetError(INVALID_FA);
            break;
        }
        pV2 = SStack.Pop();
        if (!pV2)
        {
            SetError(INVALID_FA);
            break;
        }
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }

        pV->Get(TempLong);
        TempLong = TempLong << 24;
        pV2->Get(TempLong2);
        TempLong2 = TempLong2 << 16;
        TempLong = TempLong | TempLong2;
        pV3->Get(TempLong2);
        TempLong2 = TempLong2 << 8;
        TempLong = TempLong | TempLong2;
        pV4->Get(TempLong2);
        TempLong = TempLong | TempLong2;

        pV = SStack.Push();
        pV->Set(TempLong);
        pVResult = pV;
        return pVResult;
    case FUNC_DELETE_ENTITIES:
        core.EraseEntities();
        break;
    case FUNC_CLEAR_EVENTS:
        core.ClearEvents();
        break;
    case FUNC_CLEAR_POST_EVENTS:
        // EventMsg.Release();
        EventMsg.InvalidateAll();
        break;
    case FUNC_SAVEENGINESTATE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Convert(VAR_STRING);
        pV->Get(pChar);
        core.SaveState(pChar);
        break;
    case FUNC_LOADENGINESTATE:
        pV = SStack.Pop();
        if (!pV)
        {
            SetError(INVALID_FA);
            break;
        }
        pV->Convert(VAR_STRING);
        pV->Get(pChar);
        core.InitiateStateLoading(pChar);
        break;
    }

    // Compatibility functions
    if (ENABLE_SCRIPT_COMPATIBILITY) {
        switch(func_code) {
            case FUNC_LAYER_CREATE: {
                pV2 = SStack.Pop();
                if (!pV2)
                {
                    SetError(INVALID_FA);
                    break;
                }
                pV = SStack.Pop();
                if (!pV)
                {
                    SetError(INVALID_FA);
                    break;
                }

                const char* layerName = nullptr;
                pV->Get(layerName); // Layer name
                pV2->Get(TempLong1); // Not sure, but second parameter to LayerCreate function

                spdlog::warn("Tried to create layer '{}', custom layer creation is no longer supported", layerName);
            } break;

            case FUNC_LAYER_DELETE:
                pV = SStack.Pop();
                if (!pV)
                {
                    SetError(INVALID_FA);
                    break;
                }

                pV->Get(pChar); // Layer name

                spdlog::warn("Tried to delete layer '{}', custom layer creation is no longer supported", pChar);
                break;
        }
    }

    return nullptr;
}

void COMPILER::DumpAttributes(Attribute *pA, long level)
{
    char buffer[128];
    if (pA == nullptr)
        return;

    if (level >= 128)
        level = 127;
    if (level != 0)
        memset(buffer, ' ', level);
    buffer[level] = 0;

    for (Attribute& attr : *pA) {
        DTrace("%s%s = %s", buffer, attr.getName().data(), attr.get<const char*>());
        DumpAttributes(&attr, level + 2);
    }
}

// assume first param - format string
bool COMPILER::CreateMessage(MESSAGE_SCRIPT *pMs, uint32_t s_off, uint32_t var_offset, bool s2s)
{
    uintptr_t TempPtr;
    long TempLong1;
    float TempFloat1;
    entid_t TempEid;
    Attribute *pA;
    const char *Format_string;
    const char *pChar;

    if (pMs == nullptr)
        return false;

    // read format string
    auto *pV = SStack.Read(s_off, var_offset);
    if (!pV)
    {
        SetError(INVALID_FA);
        return false;
    }
    var_offset++;
    // set pointer to format string
    pV->Get(Format_string);
    if (Format_string == nullptr)
    {
        SetError("format string is null");
        return false;
    }
    // reset message class data
    pMs->Reset(Format_string);
    // scan format string
    uint32_t n = 0;
    while (Format_string[n])
    {
        // read stack data
        pV = SStack.Read(s_off, var_offset);
        var_offset++;
        if (!pV)
        {
            SetError("No data in CreateMessage()");
            return false;
        }

        switch (Format_string[n])
        {
        case 'l':
            pV = pV->GetVarPointer();
            if (pV->GetType() != VAR_INTEGER)
            {
                SetError("CreateMessage: Invalid Data");
                return false;
            }
            pV->Get(TempLong1);
            pMs->Set((char *)&TempLong1);
            break;
        case 'p':
            pV = pV->GetVarPointer();
            if (pV->GetType() != VAR_PTR)
            {
                SetError("CreateMessage: Invalid Data");
                return false;
            }
            pV->GetPtr(TempPtr);
            pMs->Set((char *)&TempPtr);
            break;
        case 'f':
            pV = pV->GetVarPointer();
            if (pV->GetType() != VAR_FLOAT)
            {
                if (pV->GetType() == VAR_INTEGER)
                {
                    pV->Convert(VAR_FLOAT);
                }
                else
                {
                    SetError("CreateMessage: Invalid Data");
                    return false;
                }
            }
            pV->Get(TempFloat1);
            pMs->Set((char *)&TempFloat1);
            break;
        case 'i':
            pV = pV->GetVarPointer();
            if (!(pV->GetType() == VAR_OBJECT || pV->GetType() == VAR_AREFERENCE))
            {
                SetError("CreateMessage: Invalid Data");
                return false;
            }
            pV->Get(TempEid);
            pMs->Set((char *)&TempEid);
            break;
        case 'e':
            pV = pV->GetVarPointer();
            pMs->Set((char *)&pV);
            break;
        case 's':
            if (pV->GetType() != VAR_STRING)
            {
                SetError("CreateMessage: Invalid Data");
                return false;
            }
            pV->Get(pChar);
            pMs->Set((char *)pChar);
            break;
        case 'a':
            pV = pV->GetVarPointer();
            if (!(pV->GetType() == VAR_OBJECT || pV->GetType() == VAR_AREFERENCE))
            {
                SetError("CreateMessage: Invalid Data");
                return false;
            }
            pA = pV->GetAClass();
            pMs->Set((char *)&pA);
            break;
        }
        n++;
    }
    return true;
}

long func_rand(long max)
{
    int value = ((max + 1) * rand()) / RAND_MAX;
    if (value > max)
    {
        value = max;
    }
    return value;
}
