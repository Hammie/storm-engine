#pragma once

#include <Windows.h>
#include <cstdint>

#include <storm/scripting/tokens.hpp>

using S_TOKEN_TYPE = storm::scripting::TokenType;

#define PROGRAM_STEPS_CACHE 16

#define TOKENHASHTABLE_SIZE 256

struct THLINE
{
    THLINE()
    {
        dwNum = 0;
        pIndex = nullptr;
    };
    uint32_t dwNum;
    uint8_t *pIndex;
};

class TOKEN
{
    THLINE KeywordsHash[TOKENHASHTABLE_SIZE];
    S_TOKEN_TYPE eTokenType;
    ptrdiff_t TokenDataBufferSize;
    long Lines_in_token;
    char *pTokenData;
    ptrdiff_t ProgramSteps[PROGRAM_STEPS_CACHE];
    long ProgramStepsNum;
    char *Program;
    char *ProgramBase;
    uint32_t dwKeywordsNum;

  public:
    TOKEN();
    ~TOKEN();
    void Release();
    void Reset();
    void SetProgram(char *pProgramBase, char *pProgramControl);
    void SetProgramControl(char *pProgramControl);
    char *GetProgramControl();
    char *GetProgramBase()
    {
        return ProgramBase;
    };
    ptrdiff_t GetProgramOffset();

    S_TOKEN_TYPE Get(bool bKeepData = false);
    S_TOKEN_TYPE ProcessToken(char *&pointer, bool bKeepData = false);
    S_TOKEN_TYPE GetType();
    void CacheToken(const char *pointer);
    bool StepBack();
    long SetTokenData(const char *pointer, bool bKeepControlSymbols = false);
    ptrdiff_t SetNTokenData(const char *pointer, ptrdiff_t Data_size);
    long StopArgument(const char *pointer, bool bKeepControlSymbols = false);
    void StartArgument(char *&pointer, bool bKeepControlSymbols = false);
    const char *GetTypeName(S_TOKEN_TYPE code);
    const char *GetTypeName();
    const char *GetData();
    bool Is(S_TOKEN_TYPE ttype);
    bool IsNumber(const char *pointer);
    bool IsFloatNumber(const char *pointer);
    long TokenLines();
    void LowCase();

    S_TOKEN_TYPE FormatGet();

    S_TOKEN_TYPE Keyword2TokenType(const char *pString);
    uint32_t MakeHashValue(const char *string, uint32_t max_syms = 0);
    bool InitializeHashTable();
};

