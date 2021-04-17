#include "token.h"
#include <cstdio>

#include <storm/scripting/compiler.hpp>
#include <storm/scripting/keywords.hpp>

#include "defines.h"
#include "storm_assert.h"

namespace
{

constexpr const size_t MAX_PROGRAM_STEPS_CACHE = 16;

} // namespace

#define INVALID_ARG_DCHARS 32

TOKEN::TOKEN()
{
    ProgramSteps.reserve(MAX_PROGRAM_STEPS_CACHE);

    eTokenType = S_TOKEN_TYPE::UNKNOWN;
    Lines_in_token = 0;
}

TOKEN::~TOKEN()
{
    Reset();
}

void TOKEN::SetProgram(const std::string_view &pProgramBase, const std::string_view &pProgramControl)
{
    ProgramPointer = pProgramControl;
    m_ProgramBase = pProgramBase;
    ProgramSteps.clear();
}

void TOKEN::Reset()
{
    m_TokenData.clear();
    eTokenType = S_TOKEN_TYPE::UNKNOWN;
    ProgramSteps.clear();
    ProgramPointer = "";
    m_ProgramBase = "";
}

void TOKEN::LowCase()
{
    _strlwr(m_TokenData.data());
}

const char *TOKEN::GetData()
{
    // if(pTokenData[0] == 0) return 0;
    return m_TokenData.c_str();
}

S_TOKEN_TYPE TOKEN::GetType()
{
    return eTokenType;
}

std::string_view TOKEN::GetTypeName(S_TOKEN_TYPE code) const
{
    return storm::scripting::Token(code).Typename();
}

S_TOKEN_TYPE TOKEN::Get(bool bKeepData)
{
    ptrdiff_t counter;

    eTokenType = S_TOKEN_TYPE::UNKNOWN;

    StartArgument(ProgramPointer);
    CacheToken(ProgramPointer);

    Lines_in_token = 0;

    if (ProgramPointer.empty()) {
        // end of program
        m_TokenData.clear();
        eTokenType = S_TOKEN_TYPE::END_OF_PROGRAMM;
        return eTokenType;
    }

    auto sym = ProgramPointer.front();
    switch (sym)
    {
    case ';':
        m_TokenData.clear();
        eTokenType = S_TOKEN_TYPE::SEPARATOR;
        ProgramPointer.remove_prefix(1);
        return eTokenType;
    case 0xd:
        m_TokenData.clear();
        eTokenType = S_TOKEN_TYPE::DEBUG_LINEFEED;
        ProgramPointer.remove_prefix(1);
        if (ProgramPointer[0] == 0xa)
            ProgramPointer.remove_prefix(1);
        return eTokenType;
    case 0xa:
        m_TokenData.clear();
        eTokenType = S_TOKEN_TYPE::DEBUG_LINEFEED;
        ProgramPointer.remove_prefix(1);
        if (ProgramPointer[0] == 0xd)
            ProgramPointer.remove_prefix(1);
        return eTokenType;

        // commented text
    case '/': {
        sym = ProgramPointer[1];
        if (sym != '*')
            break;
        ProgramPointer.remove_prefix(1);
        ProgramPointer.remove_prefix(1);

        const std::string_view base = ProgramPointer;

        do
        {
            sym = ProgramPointer.front();
            if (sym == '*')
            {
                sym = ProgramPointer[1];
                if (sym == '/')
                {
                    m_TokenData = base.substr(0, std::distance(base.data(), ProgramPointer.data()));
                    eTokenType = S_TOKEN_TYPE::COMMENT;
                    ProgramPointer.remove_prefix(1);
                    ProgramPointer.remove_prefix(1);
                    return eTokenType;
                }
            }
            else
            {
                switch (sym)
                {
                case 0xd:
                    if (ProgramPointer[1] == 0xa)
                        ProgramPointer.remove_prefix(1);
                    Lines_in_token++;
                    break;
                case 0xa:
                    if (ProgramPointer[1] == 0xd)
                        ProgramPointer.remove_prefix(1);
                    Lines_in_token++;
                    break;
                }
            }
            ProgramPointer.remove_prefix(1);
        } while (sym != 0);
        counter = std::distance(base.data(), ProgramPointer.data());
        if (counter > INVALID_ARG_DCHARS)
            counter = INVALID_ARG_DCHARS;
        m_TokenData = base.substr(0, counter);
        eTokenType = S_TOKEN_TYPE::INVALID_TOKEN;
        return eTokenType;
    }
    case '"':
        ProgramPointer.remove_prefix(1);
        const std::string_view base = ProgramPointer;
        do
        {
            sym = ProgramPointer.front();
            if (sym == '"')
            {
                m_TokenData = base.substr(0, std::distance(base.data(), ProgramPointer.data()));
                eTokenType = S_TOKEN_TYPE::STRING;
                ProgramPointer.remove_prefix(1);
                return eTokenType;
            }
            ProgramPointer.remove_prefix(1);
        } while (sym != 0);
        counter = std::distance(base.data(), ProgramPointer.data());
        if (counter > INVALID_ARG_DCHARS)
            counter = INVALID_ARG_DCHARS;
        m_TokenData = base.substr(0, counter);
        eTokenType = S_TOKEN_TYPE::INVALID_TOKEN;
        return eTokenType;
    }
    const auto stt = ProcessToken(ProgramPointer, bKeepData);
    if (stt == S_TOKEN_TYPE::HOLD_COMPILATION)
    {
        __debugbreak();
        // stt == HOLD_COMPILATION;
    }
    return stt;
}

S_TOKEN_TYPE TOKEN::FormatGet()
{
    char sym;
    const char *pBase;
    ptrdiff_t counter;

    eTokenType = S_TOKEN_TYPE::UNKNOWN;

    StartArgument(ProgramPointer, true);
    CacheToken(ProgramPointer);

    if (ProgramPointer.empty()) {
        // end of program
        m_TokenData.clear();
        eTokenType = S_TOKEN_TYPE::END_OF_PROGRAMM;
        return eTokenType;
    }

    Lines_in_token = 0;
    sym = ProgramPointer.front();
    switch (sym)
    {
    case ';':
        // DISCARD_DATABUFFER
        m_TokenData = ";";

        eTokenType = S_TOKEN_TYPE::SEPARATOR;
        ProgramPointer.remove_prefix(1);
        return eTokenType;
    case 0xd:
        // DISCARD_DATABUFFER

        eTokenType = S_TOKEN_TYPE::DEBUG_LINEFEED;
        ProgramPointer.remove_prefix(1);
        if (ProgramPointer[0] == 0xa)
        {
            m_TokenData = std::string(ProgramPointer.data() - 1, 2);
            ProgramPointer.remove_prefix(1);
        }
        else
            m_TokenData = sym;
        return eTokenType;
    case 0xa:
        // DISCARD_DATABUFFER

        eTokenType = S_TOKEN_TYPE::DEBUG_LINEFEED;
        ProgramPointer.remove_prefix(1);
        if (ProgramPointer[0] == 0xd)
        {
            m_TokenData = std::string(ProgramPointer.data() - 1, 2);
            ProgramPointer.remove_prefix(1);
        }
        else
            m_TokenData = sym;
        return eTokenType;

        // commented text
    case '/':
        pBase = ProgramPointer.data();
        sym = ProgramPointer[1];
        if (sym != '*')
            break;
        ProgramPointer.remove_prefix(1);
        ProgramPointer.remove_prefix(1);

        // pBase = Program;

        do
        {
            sym = ProgramPointer.front();
            if (sym == '*')
            {
                sym = ProgramPointer[1];
                if (sym == '/')
                {
                    // SetNTokenData(pBase,(DWORD)Program - (DWORD)pBase);
                    eTokenType = S_TOKEN_TYPE::COMMENT;
                    ProgramPointer.remove_prefix(1);
                    ProgramPointer.remove_prefix(1);
                    m_TokenData = std::string(pBase, std::distance(pBase, ProgramPointer.data()));
                    return eTokenType;
                }
            }
            else
            {
                switch (sym)
                {
                case 0xd:
                    if (ProgramPointer[1] == 0xa)
                        ProgramPointer.remove_prefix(1);
                    Lines_in_token++;
                    break;
                case 0xa:
                    if (ProgramPointer[1] == 0xd)
                        ProgramPointer.remove_prefix(1);
                    Lines_in_token++;
                    break;
                }
            }
            ProgramPointer.remove_prefix(1);
        } while (sym != 0);
        counter = std::distance(pBase, ProgramPointer.data());
        if (counter > INVALID_ARG_DCHARS)
            counter = INVALID_ARG_DCHARS;
        m_TokenData = std::string(pBase, counter);
        eTokenType = S_TOKEN_TYPE::INVALID_TOKEN;
        return eTokenType;
    case '"':
        // Program++;
        pBase = ProgramPointer.data();
        ProgramPointer.remove_prefix(1);
        do
        {
            sym = ProgramPointer.front();
            if (sym == '"')
            {
                ProgramPointer.remove_prefix(1);
                m_TokenData = std::string(pBase, std::distance(pBase, ProgramPointer.data()));
                eTokenType = S_TOKEN_TYPE::STRING;
                // Program++;
                return eTokenType;
            }
            ProgramPointer.remove_prefix(1);
        } while (sym != 0);
        counter = std::distance(ProgramPointer.data(), pBase);
        if (counter > INVALID_ARG_DCHARS)
            counter = INVALID_ARG_DCHARS;
        m_TokenData = std::string(pBase, counter);
        eTokenType = S_TOKEN_TYPE::INVALID_TOKEN;
        return eTokenType;
    }

    return ProcessToken(ProgramPointer, true);
}

// copy argument data to buffer and close the termination 0
long TOKEN::SetTokenData(const std::string_view &input, bool bKeepControlSymbols)
{
    // if(!IsOperator(pointer,Data_size))
    const auto Data_size = storm::scripting::Compiler::getNextTokenLength(input, bKeepControlSymbols);
    m_TokenData = input.substr(0, Data_size);
    return Data_size;
}

// advance program pointer until not found significant argument symbol
void TOKEN::StartArgument(std::string_view &pointer, bool bKeepControlSymbols)
{
    do
    {
        if (pointer.empty()) {
            return;
        }
        const auto sym = pointer.front();
        Assert(sym != '\0');
        if (sym == 0xa || sym == 0xd)
            return;
        if (bKeepControlSymbols)
        {
            if (sym == '\t' || sym == ' ')
                return;
        }
        if (sym <= ' ')
            pointer.remove_prefix(1);
        else
            return;
    } while (true);
}

bool TOKEN::IsNumber(const char *pointer)
{
    if (pointer == nullptr)
        return false;
    for (uint32_t n = 0; pointer[n]; n++)
    {
        if (pointer[n] < 0x20 && n > 0)
            return true; // end on white space
        if (pointer[n] < 0x30 || pointer[n] > 0x39)
            return false; // not digit symbol
    }
    return true;
}

// this function can interpreted integer as float, so always check using IsNumber function
bool TOKEN::IsFloatNumber(const char *pointer)
{
    if (pointer == nullptr)
        return false;
    for (uint32_t n = 0; pointer[n]; n++)
    {
        if (pointer[n] == '.')
        {
            if (n > 0)
                continue;
        }
        if (pointer[n] < 0x20 && n > 0)
            return true; // end on white space
        if (pointer[n] < 0x30 || pointer[n] > 0x39)
            return false; // not digit symbol
    }
    return true;
}

/*
bool TOKEN::IsOperator(char * pointer)
{
    DWORD operators_num,n,i;
    bool found;
    operators_num = sizeof(Operators)/sizeof(char *);

    for(n=0;n<operators_num;n++)
    {
        i = 0;
        found = true;
        while(Operators[n][i] && pointer[i])
        {
            if(Operators[n][i] != pointer[i]) { found = false; break; }
            i++;
        }
        if(found)
        {
            if(Operators[n][0] == '.')
            {
                if(pointer[i] >= 0x30 && pointer[i] <= 0x39)
                {
                    return false;    // this is float number
                }
            }
            return true;
        }
    }
    return false;
}
*/
/*
bool TOKEN::IsOperator(char * pointer, long & syms)
{
    DWORD operators_num,n,i;
    bool found;
    operators_num = sizeof(Operators)/sizeof(char *);
    syms = 0;
    for(n=0;n<operators_num;n++)
    {
        i = 0;
        found = true;
        while(Operators[n][i] && pointer[i])
        {
            if(Operators[n][i] != pointer[i]) { found = false; break; }
            i++;
        }
        if(found)
        {
            syms = i;
            if(Operators[n][0] == '.')
            {
                if(pointer[i] >= 0x30 && pointer[i] <= 0x39)
                {
                    return false;    // this is float number
                }
            }
            syms = i;
            return true;
        }
    }
    return false;
}
*/
void TOKEN::CacheToken(const std::string_view &pointer)
{
    if (ProgramSteps.size() < MAX_PROGRAM_STEPS_CACHE)
    {
        ProgramSteps.push_back(0);
    }
    std::shift_right(ProgramSteps.begin(), ProgramSteps.end(), 1);
    ProgramSteps[0] = std::distance(m_ProgramBase.data(), pointer.data());
}

// set pointer to previous (processed) token, return false if no pointers in cache
void TOKEN::StepBack()
{
    ProgramPointer = m_ProgramBase;
    ProgramPointer.remove_prefix(ProgramSteps.front());
    std::shift_left(ProgramSteps.begin(), ProgramSteps.end(), 1);
    ProgramSteps.pop_back();
}

S_TOKEN_TYPE TOKEN::ProcessToken(std::string_view &pointer, bool bKeepData)
{
    char sym;
    // long keywords_num;
    // long n;

    pointer.remove_prefix(SetTokenData(pointer, bKeepData));

    eTokenType = S_TOKEN_TYPE::UNKNOWN;
    if (GetData() == nullptr)
    {
        pointer.remove_prefix(1);
        m_TokenData.clear();
        return eTokenType;
    }

    /*keywords_num = sizeof(Keywords)/sizeof(S_KEYWORD);
    for(n=0;n<keywords_num;n++)
    {
      if(_stricmp(pTokenData,Keywords[n].name) == 0)
      {
        eTokenType = Keywords[n].type;
        break;
      }
    }*/

    storm::scripting::KeywordManager keyword_manager;
    eTokenType = keyword_manager.getTokenForKeyword(m_TokenData);

    // if(IsOperator(GetData())) eTokenType = OPERATOR;
    // else
    if (IsNumber(GetData()))
        eTokenType = S_TOKEN_TYPE::NUMBER;
    else if (IsFloatNumber(GetData()))
        eTokenType = S_TOKEN_TYPE::FLOAT_NUMBER;

    switch (eTokenType)
    {
    case S_TOKEN_TYPE::INCLUDE_LIBRIARY:
        Get(); //    file name (string)
        eTokenType = S_TOKEN_TYPE::INCLUDE_LIBRIARY;
        break;
    case S_TOKEN_TYPE::INCLIDE_FILE:
        Get(); //    file name (string)
        eTokenType = S_TOKEN_TYPE::INCLIDE_FILE;
        break;
    case S_TOKEN_TYPE::DEBUG_FILE_NAME:
        Get(); //    file name (string)
        eTokenType = S_TOKEN_TYPE::DEBUG_FILE_NAME;
        break;
    case S_TOKEN_TYPE::BLOCK_IN:
        if (bKeepData)
            break;
        m_TokenData.clear();
        // pointer++;
        break;
    case S_TOKEN_TYPE::BLOCK_OUT:
        if (bKeepData)
            break;
        m_TokenData.clear();
        // pointer++;
        break;
    case S_TOKEN_TYPE::LINE_COMMENT: {
        const std::string_view base = bKeepData ? std::string_view(ProgramPointer.data() - 2, ProgramPointer.size() + 2) : ProgramPointer;
        eTokenType = S_TOKEN_TYPE::COMMENT;
        do
        {
            sym = ProgramPointer.front();
            ProgramPointer.remove_prefix(1);
            if (sym == 0xd || sym == 0xa)
                break;
        } while (sym != 0);
        m_TokenData = base.substr(0, std::distance(base.data(), ProgramPointer.data()));
        break;
    }
    case S_TOKEN_TYPE::CALL:
    case S_TOKEN_TYPE::IMPORT:
    case S_TOKEN_TYPE::EXTERN:
    case S_TOKEN_TYPE::COMMA:
    case S_TOKEN_TYPE::DOT:
    case S_TOKEN_TYPE::AND:
    case S_TOKEN_TYPE::OP_MODUL:
    case S_TOKEN_TYPE::OP_MINUS:
    case S_TOKEN_TYPE::OP_PLUS:
    case S_TOKEN_TYPE::OP_MULTIPLY:
    case S_TOKEN_TYPE::OP_DIVIDE:
    case S_TOKEN_TYPE::OP_POWER:
    case S_TOKEN_TYPE::OP_NOT_EQUAL:
    case S_TOKEN_TYPE::OP_GREATER:
    case S_TOKEN_TYPE::OP_GREATER_OR_EQUAL:
    case S_TOKEN_TYPE::OP_LESSER:
    case S_TOKEN_TYPE::OP_LESSER_OR_EQUAL:
    case S_TOKEN_TYPE::OP_EQUAL:
    case S_TOKEN_TYPE::OP_BOOL_EQUAL:
    case S_TOKEN_TYPE::OP_BOOL_AND:
    case S_TOKEN_TYPE::OP_BOOL_OR:
    case S_TOKEN_TYPE::OP_INC:
    case S_TOKEN_TYPE::OP_DEC:
    case S_TOKEN_TYPE::OP_INCADD:
    case S_TOKEN_TYPE::OP_DECADD:
    case S_TOKEN_TYPE::OP_MULTIPLYEQ:
    case S_TOKEN_TYPE::OP_DIVIDEEQ:
    case S_TOKEN_TYPE::SQUARE_OPEN_BRACKET:
    case S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET:
    case S_TOKEN_TYPE::OPEN_BRACKET:
    case S_TOKEN_TYPE::CLOSE_BRACKET:
    case S_TOKEN_TYPE::MAKEREF_COMMAND:
    case S_TOKEN_TYPE::MAKEAREF_COMMAND:
    case S_TOKEN_TYPE::LABEL:
    case S_TOKEN_TYPE::SWITCH_COMMAND:
    case S_TOKEN_TYPE::CASE_COMMAND:
    case S_TOKEN_TYPE::WHILE_BLOCK:
    case S_TOKEN_TYPE::IF_BLOCK:
    case S_TOKEN_TYPE::ELSE_BLOCK:
    case S_TOKEN_TYPE::FOR_BLOCK:
    case S_TOKEN_TYPE::STRING:
    case S_TOKEN_TYPE::NUMBER:
    case S_TOKEN_TYPE::FLOAT_NUMBER:
    case S_TOKEN_TYPE::UNKNOWN:
    case S_TOKEN_TYPE::CLASS_DECL:
    case S_TOKEN_TYPE::CONTINUE_COMMAND:
    case S_TOKEN_TYPE::BREAK_COMMAND:
    case S_TOKEN_TYPE::FUNCTION_RETURN:
    case S_TOKEN_TYPE::FUNCTION_RETURN_VOID:
    case S_TOKEN_TYPE::TRUE_CONST:
    case S_TOKEN_TYPE::FALSE_CONST:
    case S_TOKEN_TYPE::OP_BOOL_NEG:
    case S_TOKEN_TYPE::GOTO_COMMAND:
    case S_TOKEN_TYPE::EVENT_HANDLER:
    case S_TOKEN_TYPE::STACK_ALLOC:
    case S_TOKEN_TYPE::STACK_PUSH:
    case S_TOKEN_TYPE::STACK_POP:
    case S_TOKEN_TYPE::STACK_READ:
    case S_TOKEN_TYPE::STACK_WRITE:
    case S_TOKEN_TYPE::STACK_WRITE_BXINDEX:
    case S_TOKEN_TYPE::STACK_COMPARE:
    case S_TOKEN_TYPE::STACK_POP_VOID:
    case S_TOKEN_TYPE::LEFT_OPERAND:

        break;
    default:
        if (bKeepData)
            break;

        StartArgument(pointer);
        pointer.remove_prefix(SetTokenData(pointer));
        break;
    }
    return eTokenType;
}

long TOKEN::TokenLines()
{
    return Lines_in_token;
}
