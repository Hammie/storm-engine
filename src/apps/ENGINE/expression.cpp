#include "compiler.h"

char ERR_INVALID_EXPRESSION[] = "Invalid Expression";

bool COMPILER::BC_ProcessExpression(DATA *value)
{
    DATA *pV;
    DATA value2;

    value2.SetVCompiler(this);
    value->SetVCompiler(this);
    value->ClearType();

    auto check_equal = false;

    while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
    {
    }
    if (TokenType() == S_TOKEN_TYPE::SEPARATOR)
    {
        SetError("Invalid expression");
        return false;
    }

    //--------------------------------------------------------------------------
    if (TokenIs(S_TOKEN_TYPE::AND))
    {
        VARINFO vi;

        auto Token_type = BC_TokenGet();
        if (TokenType() != S_TOKEN_TYPE::VARIABLE)
        {
            if (TokenType() != S_TOKEN_TYPE::LOCAL_VARIABLE)
            {
                SetError("invalid '&'");
                return false;
            }
        }

        const auto var_code = *((uint32_t *)&pRunCodeBase[TLR_DataOffset]);
        if (TokenType() == S_TOKEN_TYPE::VARIABLE)
        {
            if (!VarTab.GetVar(vi, var_code))
            {
                SetError("Global variable not found");
                return false;
            }
            pV = vi.pDClass;
        }
        else
        {
            // local variable
            if (pRun_fi)
            {
                pV = SStack.Read(pRun_fi->stack_offset, var_code);
                if (pV == nullptr)
                {
                    SetError("Local variable not found");
                    return false;
                }
            }
            else
                return false;
        }

        Token_type = BC_TokenGet();
        if (Token_type == S_TOKEN_TYPE::SQUARE_OPEN_BRACKET)
        {
            DATA array_index;
            long index;
            array_index.SetVCompiler(this);

            if (!pV->IsArray())
            {
                SetError("invalid '[' operator");
                return false;
            }
            BC_TokenGet(); // beginning of subexpression
            BC_ProcessExpression_L2(&array_index);
            array_index.Get(index);
            if (!TokenIs(S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET))
                SetError("missed ']' in expression");

            value->SetType(S_TOKEN_TYPE::VAR_REFERENCE);
            // pV = pV->GetVarPointer();
            pV = pV->GetArrayElement(index);
            value->SetReference(pV); //->GetVarPointer());
            // BC_TokenGet();
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            return true;
        }
        value->SetType(S_TOKEN_TYPE::VAR_REFERENCE);
        value->SetReference(pV); //->GetVarPointer());
        return true;
    }

    BC_ProcessExpression_L0(value);
    //--------------------------------------------------------------------------
    /*    BC_ProcessExpression_L2(value);
      S_TOKEN_TYPE op;
      op = TokenType();
      switch(op)
      {
        case S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET: break;
        case CLOSE_BRACKET: break;
        case OP_EQUAL: break;

        case OP_BOOL_AND:
        case OP_BOOL_OR:
        case OP_NOT_EQUAL:
        case OP_LESSER_OR_EQUAL:
        case OP_LESSER:
        case OP_GREATER_OR_EQUAL:
        case OP_GREATER:
        case OP_BOOL_EQUAL:
          //BC_TokenGet();
          while(BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE){};
          BC_ProcessExpression_L2(&value2);
          value->CompareAndSetResult(&value2,op);
        break;

        case COMMA: break;
        case DOT: break;
        case ACCESS_WORD: break;
        case ACCESS_VAR: break;
        case S_TOKEN_TYPE::DEBUG_LINE_CODE: break;

        case S_TOKEN_TYPE::SEPARATOR: break;//return value;
        default:
          SetError("Invalid expression break");
        return false;
      }//*/
    return true;
}

// logical
void COMPILER::BC_ProcessExpression_L0(DATA *value)
{
    long lRes;
    DATA value2;
    value2.SetVCompiler(this);

    BC_ProcessExpression_L1(value);
    while (TokenIs(S_TOKEN_TYPE::OP_BOOL_AND) || TokenIs(S_TOKEN_TYPE::OP_BOOL_OR))
    {
        const auto op = TokenType();
        switch (op)
        {
        case S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET:
            break;
        case S_TOKEN_TYPE::CLOSE_BRACKET:
            break;
        case S_TOKEN_TYPE::OP_EQUAL:
            break;
        case S_TOKEN_TYPE::COMMA:
            break;
        case S_TOKEN_TYPE::DOT:
            break;
        case S_TOKEN_TYPE::ACCESS_WORD:
            break;
        case S_TOKEN_TYPE::ACCESS_WORD_CODE:
            break;
        case S_TOKEN_TYPE::ACCESS_VAR:
            break;
        case S_TOKEN_TYPE::DEBUG_LINE_CODE:
            break;
        case S_TOKEN_TYPE::SEPARATOR:
            break;

        case S_TOKEN_TYPE::OP_BOOL_AND:
            value->BoolConvert();
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            value->Get(lRes);
            if (lRes == 0)
                BC_ProcessExpression_L1(&value2, true); // SkipExpression_L1();
            else
            {
                BC_ProcessExpression_L1(&value2);
                value->RefConvert();
                value->CompareAndSetResult(&value2, op);
            }
            break;
        case S_TOKEN_TYPE::OP_BOOL_OR:
            value->BoolConvert();
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            value->Get(lRes);
            if (lRes)
                BC_ProcessExpression_L1(&value2, true); // SkipExpression_L1();
            else
            {
                BC_ProcessExpression_L1(&value2);
                value->RefConvert();
                value->CompareAndSetResult(&value2, op);
            }
            break;

        default:
            SetError("Invalid expression break");
            return;
        }
    }
}

// logical
void COMPILER::BC_ProcessExpression_L1(DATA *value, bool bSkip)
{
    DATA value2;
    value2.SetVCompiler(this);

    BC_ProcessExpression_L2(value, bSkip);
    while (TokenIs(S_TOKEN_TYPE::OP_NOT_EQUAL) || TokenIs(S_TOKEN_TYPE::OP_LESSER_OR_EQUAL) || TokenIs(S_TOKEN_TYPE::OP_LESSER) || TokenIs(S_TOKEN_TYPE::OP_GREATER_OR_EQUAL) ||
           TokenIs(S_TOKEN_TYPE::OP_GREATER) || TokenIs(S_TOKEN_TYPE::OP_BOOL_EQUAL))
    {
        const auto op = TokenType();
        switch (op)
        {
        case S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET:
            break;
        case S_TOKEN_TYPE::CLOSE_BRACKET:
            break;
        case S_TOKEN_TYPE::OP_EQUAL:
            break;
        case S_TOKEN_TYPE::COMMA:
            break;
        case S_TOKEN_TYPE::DOT:
            break;
        case S_TOKEN_TYPE::ACCESS_WORD:
            break;
        case S_TOKEN_TYPE::ACCESS_WORD_CODE:
            break;
        case S_TOKEN_TYPE::ACCESS_VAR:
            break;
        case S_TOKEN_TYPE::DEBUG_LINE_CODE:
            break;
        case S_TOKEN_TYPE::SEPARATOR:
            break;
        case S_TOKEN_TYPE::OP_NOT_EQUAL:
        case S_TOKEN_TYPE::OP_LESSER_OR_EQUAL:
        case S_TOKEN_TYPE::OP_LESSER:
        case S_TOKEN_TYPE::OP_GREATER_OR_EQUAL:
        case S_TOKEN_TYPE::OP_GREATER:
        case S_TOKEN_TYPE::OP_BOOL_EQUAL:
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            BC_ProcessExpression_L2(&value2, bSkip);
            if (!bSkip)
            {
                value->RefConvert();
                value->CompareAndSetResult(&value2, op);
            }
            break;
        default:
            SetError("Invalid expression break");
            return;
        }
    }
}

// '+' and '-' operations
void COMPILER::BC_ProcessExpression_L2(DATA *value, bool bSkip)
{
    DATA TempVal;
    TempVal.SetVCompiler(this);
    BC_ProcessExpression_L3(value, bSkip);
    while (TokenIs(S_TOKEN_TYPE::OP_PLUS) || TokenIs(S_TOKEN_TYPE::OP_MINUS))
    {
        const auto op = TokenType();
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        TempVal.ClearType();
        BC_ProcessExpression_L3(&TempVal, bSkip);
        if (!bSkip)
        {
            switch (op)
            {
            case S_TOKEN_TYPE::OP_PLUS:
                value->Plus(&TempVal);
                break;
            case S_TOKEN_TYPE::OP_MINUS:
                value->Minus(&TempVal);
                break;
            }
        }
    }
}

// '*' and '/' operations
void COMPILER::BC_ProcessExpression_L3(DATA *value, bool bSkip)
{
    DATA TempVal;
    TempVal.SetVCompiler(this);

    BC_ProcessExpression_L4(value, bSkip);
    while (TokenIs(S_TOKEN_TYPE::OP_MULTIPLY) || TokenIs(S_TOKEN_TYPE::OP_DIVIDE) || TokenIs(S_TOKEN_TYPE::OP_MODUL))
    {
        const S_TOKEN_TYPE op = TokenType();
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        BC_ProcessExpression_L4(&TempVal, bSkip);
        if (!bSkip)
        {
            switch (op)
            {
            case S_TOKEN_TYPE::OP_MULTIPLY:
                value->Multiply(&TempVal);
                break;
            case S_TOKEN_TYPE::OP_DIVIDE:
                value->Divide(&TempVal);
                break;
            case S_TOKEN_TYPE::OP_MODUL:
                value->Modul(&TempVal);
                break;
            }
        }
    }
}

// '^' operations
void COMPILER::BC_ProcessExpression_L4(DATA *value, bool bSkip)
{
    DATA TempVal;
    TempVal.SetVCompiler(this);
    long Deg;

    BC_ProcessExpression_L5(value, bSkip);
    if (TokenIs(S_TOKEN_TYPE::OP_POWER))
    {
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        BC_ProcessExpression_L5(&TempVal, bSkip);
        if (!bSkip)
        {
            if (TempVal.GetType() == S_TOKEN_TYPE::VAR_INTEGER)
            {
                TempVal.Get(Deg);
                value->Power(Deg);
            }
        }
    }
}

// sign
void COMPILER::BC_ProcessExpression_L5(DATA *value, bool bSkip)
{
    const S_TOKEN_TYPE op = TokenType();
    if (TokenIs(S_TOKEN_TYPE::OP_PLUS) || TokenIs(S_TOKEN_TYPE::OP_MINUS) || TokenIs(S_TOKEN_TYPE::OP_BOOL_NEG))
    {
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
    }
    BC_ProcessExpression_L6(value, bSkip);

    if (!bSkip)
    {
        if (op == S_TOKEN_TYPE::OP_MINUS)
            value->Inverse();
        else if (op == S_TOKEN_TYPE::OP_BOOL_NEG)
        {
            long iVal;
            const char *sVal;

            switch (value->GetType())
            {
            case S_TOKEN_TYPE::VAR_FLOAT:
                value->Convert(S_TOKEN_TYPE::VAR_INTEGER);
            case S_TOKEN_TYPE::VAR_INTEGER:
                value->Get(iVal);
                if (iVal != 0)
                    iVal = 0;
                else
                    iVal = 1;
                value->Set(iVal);
                break;
            case S_TOKEN_TYPE::VAR_STRING:
                value->Get(sVal);
                if (sVal == nullptr)
                    iVal = 1;
                else if (sVal[0] == 0)
                    iVal = 1;
                else
                    iVal = 0;
                value->Convert(S_TOKEN_TYPE::VAR_INTEGER);
                value->Set(iVal);
                break;
            default:
                SetError("Invalid argument type for '!' opearator");
                break;
            }
        }
    }
}

// '(' and ')'
void COMPILER::BC_ProcessExpression_L6(DATA *value, bool bSkip)
{
    if (TokenIs(S_TOKEN_TYPE::OPEN_BRACKET))
    {
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        BC_ProcessExpression_L1(value, bSkip);
        if (!TokenIs(S_TOKEN_TYPE::CLOSE_BRACKET))
            SetError("No matching ')' in expression");
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
    }
    else
        BC_ProcessExpression_L7(value, bSkip);
}

void COMPILER::BC_ProcessExpression_L7(DATA *value, bool bSkip)
{
    VARINFO vi;
    uint32_t var_code;
    long index;
    DATA array_index;
    DATA TempData;
    TempData.SetVCompiler(this);
    array_index.SetVCompiler(this);
    DATA *pV;
    DATA *pVV;
    DATA Access;
    Access.SetVCompiler(this);
    DATA *pFuncResult;
    ATTRIBUTES *pRoot;
    const char *pString;
    uint32_t func_code, ip;
    S_TOKEN_TYPE vtype;

    switch (TokenType())
    {
    case S_TOKEN_TYPE::NUMBER:
        if (bSkip)
        {
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            return;
        }
        value->Set(*((long *)&pRunCodeBase[TLR_DataOffset]));
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        return;
    case S_TOKEN_TYPE::FLOAT_NUMBER:
        if (bSkip)
        {
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            return;
        }
        value->Set(*((float *)&pRunCodeBase[TLR_DataOffset]));
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        break;
    case S_TOKEN_TYPE::STRING:
        if (bSkip)
        {
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            return;
        }
        value->Set((char *)&pRunCodeBase[TLR_DataOffset + 4]); // 4 - string length
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        break;
    case S_TOKEN_TYPE::CALL:
        if (bSkip)
        {
            long local_in;
            BC_TokenGet(); // string var
            if (BC_TokenGet() != S_TOKEN_TYPE::OPEN_BRACKET)
            {
                SetError("missing '('");
                return;
            }
            local_in = 1;
            do
            {
                switch (BC_TokenGet())
                {
                case S_TOKEN_TYPE::OPEN_BRACKET:
                    local_in++;
                    break;
                case S_TOKEN_TYPE::CLOSE_BRACKET:
                    local_in--;
                    break;
                }
            } while (local_in > 0);
            BC_TokenGet();
            return;
        }
        vtype = BC_TokenGet();                               // read variable
        var_code = *((long *)&pRunCodeBase[TLR_DataOffset]); // var code
        if (vtype == S_TOKEN_TYPE::VARIABLE)
        {
            if (!VarTab.GetVar(vi, var_code))
            {
                SetError("Global variable not found");
                return;
            }
            pVV = vi.pDClass;
        }
        else
        {
            pVV = SStack.Read(pRun_fi->stack_offset, var_code);
            if (pVV == nullptr)
            {
                SetError("Local variable not found");
                return;
            }
        }
        pVV = pVV->GetVarPointer();
        if (pVV->GetType() != S_TOKEN_TYPE::VAR_STRING)
        {
            SetError("string call argument isnt string var");
            return;
        }
        pVV->Get(pString); // func name
        func_code = FuncTab.FindFunc(pString);
        if (func_code == INVALID_FUNC_CODE)
        {
            SetError("function '%s' not found", pString);
            return;
        }

        pFuncResult = nullptr;
        if (!BC_CallFunction(func_code, ip, pFuncResult))
            return;
        if (pFuncResult)
        {
            SStack.Pop();
            value->Copy(pFuncResult);
        }
        else
            SetError("void function result in expression");
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }

        break;
    case S_TOKEN_TYPE::CALL_FUNCTION:
        if (bSkip)
        {
            long local_in;
            if (BC_TokenGet() != S_TOKEN_TYPE::OPEN_BRACKET)
            {
                SetError("missing '('");
                return;
            }
            local_in = 1;
            do
            {
                switch (BC_TokenGet())
                {
                case S_TOKEN_TYPE::OPEN_BRACKET:
                    local_in++;
                    break;
                case S_TOKEN_TYPE::CLOSE_BRACKET:
                    local_in--;
                    break;
                }
            } while (local_in > 0);
            BC_TokenGet();
            // while(BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE){};
            return;
        }
        // DWORD func_code,ip;
        memcpy(&func_code, &pRunCodeBase[TLR_DataOffset], sizeof(uint32_t));
        pFuncResult = nullptr;
        if (!BC_CallFunction(func_code, ip, pFuncResult))
            return;
        if (pFuncResult)
        {
            SStack.Pop();
            value->Copy(pFuncResult);
        }
        else
            SetError("void function result in expression");
        while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
        }
        break;
    case S_TOKEN_TYPE::LOCAL_VARIABLE:
    case S_TOKEN_TYPE::VARIABLE:
        var_code = *((long *)&pRunCodeBase[TLR_DataOffset]);
        if (TokenType() == S_TOKEN_TYPE::VARIABLE)
        {
            if (!VarTab.GetVar(vi, var_code))
            {
                SetError("Global variable not found");
                break;
            }
            pV = vi.pDClass;
        }
        else
        {
            // local variable
            if (pRun_fi)
            {
                pV = SStack.Read(pRun_fi->stack_offset, var_code);
                if (pV == nullptr)
                {
                    SetError("Local variable not found");
                    break;
                }
            }
            else
                return;
        }

        // *****

        bool bref2a;
        bref2a = false;
        if (pV->IsReference())
        {
            pVV = pV->GetVarPointer();
            if (pVV == nullptr)
            {
                SetError("Bad Reference");
                break;
            }
            if (pVV->IsArray())
                bref2a = true;
        }

        if (BC_TokenGet() == S_TOKEN_TYPE::SQUARE_OPEN_BRACKET)
        {
            if (!(pV->IsArray() || bref2a))
            {
                SetError("wrong '[' in expression");
                return;
            }
            // beginning of subexpression
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
            BC_ProcessExpression_L2(&array_index, bSkip);
            array_index.Get(index);
            if (!TokenIs(S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET))
                SetError("missed ']' in expression");
            if (!bSkip)
            {
                if (bref2a)
                    pV = pVV->GetArrayElement(index);
                else
                    pV = pV->GetArrayElement(index);
                if (pV == nullptr)
                {
                    SetError("invalid array index");
                    while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
                    {
                    }
                    break;
                }
            }
            BC_TokenGet();
        }

        //-----

        /*
        if(pV->IsArray() || bref2a)
        {
          // compute array index
          if(BC_TokenGet() != S_TOKEN_TYPE::SQUARE_OPEN_BRACKET) { SetError("missed '[' in expression"); return; }
          // beginning of subexpression
          while(BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE){};
          BC_ProcessExpression_L2(&array_index,bSkip);
          array_index.Get(index);
          if(!TokenIs(S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET)) SetError("missed ']' in expression");
          if(!bSkip)
          {
            if(bref2a) pV = pVV->GetArrayElement(index);
            else pV = pV->GetArrayElement(index);
            if(pV == 0)
            {
              SetError("invalid array index");
              while(BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE){};
              break;
            }
          }
        }//*/

        while (TokenType() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
        {
            BC_TokenGet();
        }
        // while(BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE){};
        if (TokenType() != S_TOKEN_TYPE::ACCESS_WORD_CODE && TokenType() != S_TOKEN_TYPE::ACCESS_WORD && TokenType() != S_TOKEN_TYPE::ACCESS_VAR)
        {
            // pV = pV->GetVarPointer();
            // if(!pV) {SetError("invalid ref in process expression"); return;}
            if (!bSkip)
            {
                if (!pV->IsArray())
                {
                    value->Copy(pV);
                    return;
                }                       // copy single value
                value->Copy(pV, index); // copy array element value
            }
            return;
        }

        if (bSkip)
        {
            while (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE || TokenType() == S_TOKEN_TYPE::ACCESS_WORD || TokenType() == S_TOKEN_TYPE::ACCESS_VAR)
            {
                BC_TokenGet();
                while (TokenType() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
                {
                    BC_TokenGet();
                }
            }
            return;
        }
        // accessing to atribute (string) value
        pRoot = pV->GetAClass(); // root attribute class
        if (pRoot == nullptr)
        {
            SetError("zero AClass root");
            value->Set("ERROR: zero AClass root");
            while (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE || TokenType() == S_TOKEN_TYPE::ACCESS_WORD || TokenType() == S_TOKEN_TYPE::ACCESS_VAR)
            {
                BC_TokenGet();
            }
            return;
            // throw std::runtime_error("zero AClass root");
        }
        while (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE || TokenType() == S_TOKEN_TYPE::ACCESS_WORD || TokenType() == S_TOKEN_TYPE::ACCESS_VAR)
        {
            if (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE)
            {
                if (pRoot == nullptr)
                {
                    value->Set("");
                    SetError("missed attribute: %s", SCodec->Convert(*((long *)&pRunCodeBase[TLR_DataOffset])));
                    while (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE || TokenType() == S_TOKEN_TYPE::ACCESS_WORD || TokenType() == S_TOKEN_TYPE::ACCESS_VAR)
                    {
                        BC_TokenGet();
                    }
                    return;
                }
                pRoot = pRoot->GetAttributeClassByCode(*((long *)&pRunCodeBase[TLR_DataOffset]));
                if (pRoot == nullptr)
                {
                    value->Set("");
                    SetError("missed attribute: %s", SCodec->Convert(*((long *)&pRunCodeBase[TLR_DataOffset])));
                    while (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE || TokenType() == S_TOKEN_TYPE::ACCESS_WORD || TokenType() == S_TOKEN_TYPE::ACCESS_VAR)
                    {
                        BC_TokenGet();
                    }
                    return;
                }
            }
            else if (TokenType() == S_TOKEN_TYPE::ACCESS_WORD)
            {
                if (pRoot == nullptr)
                {
                    value->Set("");
                    SetError("missed attribute: %s", (char *)&pRunCodeBase[TLR_DataOffset]);
                    while (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE || TokenType() == S_TOKEN_TYPE::ACCESS_WORD || TokenType() == S_TOKEN_TYPE::ACCESS_VAR)
                    {
                        BC_TokenGet();
                    }
                    return;
                }
                pRoot = pRoot->GetAttributeClass((char *)&pRunCodeBase[TLR_DataOffset]);
                if (pRoot == nullptr)
                {
                    value->Set("");
                    SetError("missed attribute: %s", (char *)&pRunCodeBase[TLR_DataOffset]);
                    while (TokenType() == S_TOKEN_TYPE::ACCESS_WORD_CODE || TokenType() == S_TOKEN_TYPE::ACCESS_WORD || TokenType() == S_TOKEN_TYPE::ACCESS_VAR)
                    {
                        BC_TokenGet();
                    }
                    return;
                }
            }
            else
            {
                // BC_TokenGet();
                while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
                {
                }
                var_code = *((long *)&pRunCodeBase[TLR_DataOffset]);
                if (TokenType() == S_TOKEN_TYPE::VARIABLE)
                {
                    if (!VarTab.GetVar(vi, var_code))
                    {
                        SetError("Global variable not found");
                        break;
                    }
                    pV = vi.pDClass;
                }
                else
                {
                    // local variable
                    if (pRun_fi)
                    {
                        pV = SStack.Read(pRun_fi->stack_offset, var_code);
                        if (pV == nullptr)
                        {
                            SetError("Local variable not found");
                            break;
                        }
                    }
                    else
                        return;
                }

                TempData.Copy(pV);
                TempData.Convert(S_TOKEN_TYPE::VAR_STRING);
                TempData.Get(pString);

                pRoot = pRoot->FindAClass(pRoot, pString);
                if (pRoot == nullptr)
                {
                    value->Set("");
                    SetError("missed attribute %s", pString);
                    while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
                    {
                    }
                    return;
                }
                // pRoot = pRoot->GetAttributeClass((char *)&pRunCodeBase[TLR_DataOffset]);
            }
            // BC_TokenGet();
            while (BC_TokenGet() == S_TOKEN_TYPE::DEBUG_LINE_CODE)
            {
            }
        }
        pString = pRoot->GetThisAttr();
        value->Set(pString);
        break;
    }
}

//===============================================================================================================

// test for makeref
bool COMPILER::CompileExpression(SEGMENT_DESC &Segment)
{
    uint32_t dwVarCode;
    VARINFO vi;
    LVARINFO lvi;

    const S_TOKEN_TYPE Token_type = CompileAuxiliaryTokens(Segment);
    if (Token_type == S_TOKEN_TYPE::SEPARATOR)
    {
        SetError(ERR_INVALID_EXPRESSION);
        return false;
    }

    if (Token_type == S_TOKEN_TYPE::AND)
    {
        // make reference operation

        S_TOKEN_TYPE sttVariableType = Token.Get();

        if (sttVariableType != S_TOKEN_TYPE::UNKNOWN)
        {
            SetError("invalid '&' usage");
            return false;
        }

        sttVariableType = DetectUnknown(dwVarCode);
        if (!(sttVariableType == S_TOKEN_TYPE::VARIABLE || sttVariableType == S_TOKEN_TYPE::LOCAL_VARIABLE))
        {
            SetError("variable not found");
            return false;
        }

        const S_TOKEN_TYPE sttTokenType = Token.Get();

        // alloc for result
        CompileToken(Segment, S_TOKEN_TYPE::STACK_ALLOC);

        if (sttTokenType == S_TOKEN_TYPE::SQUARE_OPEN_BRACKET)
        {
            if (sttVariableType == S_TOKEN_TYPE::VARIABLE)
            {
                VarTab.GetVarX(vi, dwVarCode);
                // check for possibilities of '[' operator
                if (!vi.bArray)
                {
                    if (vi.type != S_TOKEN_TYPE::VAR_REFERENCE)
                    {
                        SetError("EN: %d", vi.elements);
                        SetError(" A Invalid '[' operator, %s - isnt array", vi.name);
                        return false;
                    }
                }
            }
            else
            {
                FuncTab.GetVar(lvi, CurrentFuncCode, dwVarCode);
                // check for possibilities of '[' operator
                if (!lvi.bArray)
                {
                    if (lvi.type != S_TOKEN_TYPE::VAR_REFERENCE)
                    {
                        SetError(" B Invalid '[' operator, %s - isnt array", lvi.name);
                        return false;
                    }
                }
            }

            // alloc for array index
            // CompileToken(Segment,STACK_ALLOC);
            // after top of the stack must be array index
            Token.Get();
            if (!CompileExpression_L2(Segment))
            {
                SetError("invalid syntax");
                return false;
            }
            // test for ']'
            if (Token.GetType() != S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET)
            {
                SetError("missing ']'");
                return false;
            }

            // copy array index to BX
            CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
            CompileToken(Segment, S_TOKEN_TYPE::BX);

            CompileToken(Segment, S_TOKEN_TYPE::SETREF_BXINDEX);
            CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
            CompileToken(Segment, sttVariableType, 1, &dwVarCode, sizeof(uint32_t));

            // next after ']'
            CompileAuxiliaryTokens(Segment);
        }
        else
        {
            CompileToken(Segment, S_TOKEN_TYPE::SETREF);
            CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
            CompileToken(Segment, sttVariableType, 1, &dwVarCode, sizeof(uint32_t));
        }

        // proceed to next token
        // CompileAuxiliaryTokens(Segment);

        return true;
    }
    return CompileExpression_L0(Segment);
}

// logical operators (OR,AND)
bool COMPILER::CompileExpression_L0(SEGMENT_DESC &Segment)
{
    uint32_t dwJumpOffset1;
    uint32_t dwJumpOffset2;

    if (!CompileExpression_L1(Segment))
    {
        SetError(ERR_INVALID_EXPRESSION);
        return false;
    }

    S_TOKEN_TYPE sttOpType = Token.GetType();

    while (sttOpType == S_TOKEN_TYPE::OP_BOOL_AND || sttOpType == S_TOKEN_TYPE::OP_BOOL_OR)
    {
        sttOpType = Token.GetType();

        CompileAuxiliaryTokens(Segment);

        // copy to EX value of first operand
        /*CompileToken(Segment,MOVE);
        CompileToken(Segment,EX);
        CompileToken(Segment,STACK_TOP);*/

        CompileToken(Segment, S_TOKEN_TYPE::STACK_READ);
        CompileToken(Segment, S_TOKEN_TYPE::EX);

        // save jump update position
        const uint32_t dwUpdateOffset1 = Segment.BCode_Program_size + 2;
        // point of jump not determined yet
        dwJumpOffset1 = 0;
        // compile jump command
        if (sttOpType == S_TOKEN_TYPE::OP_BOOL_AND)
        {
            // AND : if 1st operand is zero - doesnt need to calculate second - jump to jump1
            CompileToken(Segment, S_TOKEN_TYPE::JUMP_Z, 1, (char *)&dwJumpOffset1, sizeof(uint32_t));
        }
        else
        {
            // OR : if 1st operand is non zero - doesnt need to calculate second - jump to jump1
            CompileToken(Segment, S_TOKEN_TYPE::JUMP_NZ, 1, (char *)&dwJumpOffset1, sizeof(uint32_t));
        }

        if (!CompileExpression_L1(Segment))
        {
            SetError(ERR_INVALID_EXPRESSION);
            return false;
        }

        CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);

        CompileToken(Segment, sttOpType);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);

        // skip next section - proceed to jump2
        const uint32_t dwUpdateOffset2 = Segment.BCode_Program_size + 2;
        dwJumpOffset2 = 0;
        CompileToken(Segment, S_TOKEN_TYPE::JUMP, 1, (char *)&dwJumpOffset2, sizeof(uint32_t));

        // jump1
        // just convert stack top to bool
        memcpy(Segment.pCode.data() + dwUpdateOffset1, &Segment.BCode_Program_size, sizeof(uint32_t));
        CompileToken(Segment, S_TOKEN_TYPE::OP_BOOL_CONVERT);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);

        // jump2
        // end of operation
        memcpy(Segment.pCode.data() + dwUpdateOffset2, &Segment.BCode_Program_size, sizeof(uint32_t));

        sttOpType = Token.GetType();
    }
    return true;
}

// logical compare (>,<,==,>=,<=,!=)
bool COMPILER::CompileExpression_L1(SEGMENT_DESC &Segment)
{
    if (!CompileExpression_L2(Segment))
    {
        SetError(ERR_INVALID_EXPRESSION);
        return false;
    }

    S_TOKEN_TYPE sttOpType = Token.GetType();

    while (sttOpType == S_TOKEN_TYPE::OP_NOT_EQUAL || sttOpType == S_TOKEN_TYPE::OP_LESSER_OR_EQUAL || sttOpType == S_TOKEN_TYPE::OP_LESSER ||
           sttOpType == S_TOKEN_TYPE::OP_GREATER_OR_EQUAL || sttOpType == S_TOKEN_TYPE::OP_GREATER || sttOpType == S_TOKEN_TYPE::OP_BOOL_EQUAL)
    {
        sttOpType = Token.GetType();

        CompileAuxiliaryTokens(Segment);

        if (!CompileExpression_L2(Segment))
        {
            SetError(ERR_INVALID_EXPRESSION);
            return false;
        }

        CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);
        CompileToken(Segment, sttOpType);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);

        sttOpType = Token.GetType();
    }

    return true;
}

// '+' and '-' operators
bool COMPILER::CompileExpression_L2(SEGMENT_DESC &Segment)
{
    if (!CompileExpression_L3(Segment))
    {
        SetError(ERR_INVALID_EXPRESSION);
        return false;
    }

    S_TOKEN_TYPE sttOpType = Token.GetType();

    while (sttOpType == S_TOKEN_TYPE::OP_PLUS || sttOpType == S_TOKEN_TYPE::OP_MINUS)
    {
        sttOpType = Token.GetType();

        CompileAuxiliaryTokens(Segment);

        if (!CompileExpression_L3(Segment))
        {
            SetError(ERR_INVALID_EXPRESSION);
            return false;
        }

        CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);
        CompileToken(Segment, sttOpType);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);

        sttOpType = Token.GetType();
    }
    return true;
}

// '*' and '/' operators
bool COMPILER::CompileExpression_L3(SEGMENT_DESC &Segment)
{
    if (!CompileExpression_L4(Segment))
    {
        SetError(ERR_INVALID_EXPRESSION);
        return false;
    }

    S_TOKEN_TYPE sttOpType = Token.GetType();

    while (sttOpType == S_TOKEN_TYPE::OP_MULTIPLY || sttOpType == S_TOKEN_TYPE::OP_DIVIDE || sttOpType == S_TOKEN_TYPE::OP_MODUL)
    {
        sttOpType = Token.GetType();
        CompileAuxiliaryTokens(Segment);

        if (!CompileExpression_L4(Segment))
        {
            SetError(ERR_INVALID_EXPRESSION);
            return false;
        }

        CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);
        CompileToken(Segment, sttOpType);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);

        sttOpType = Token.GetType();
    }
    return true;
}

// '^' operations
bool COMPILER::CompileExpression_L4(SEGMENT_DESC &Segment)
{
    if (!CompileExpression_L5(Segment))
    {
        SetError(ERR_INVALID_EXPRESSION);
        return false;
    }

    if (Token.GetType() == S_TOKEN_TYPE::OP_POWER)
    {
        CompileAuxiliaryTokens(Segment);

        if (!CompileExpression_L5(Segment))
        {
            SetError(ERR_INVALID_EXPRESSION);
            return false;
        }

        CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);
        CompileToken(Segment, S_TOKEN_TYPE::OP_POWER);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
        CompileToken(Segment, S_TOKEN_TYPE::AX);
    }
    return true;
}

// sign
bool COMPILER::CompileExpression_L5(SEGMENT_DESC &Segment)
{
    const S_TOKEN_TYPE sttOpType = Token.GetType();
    if (sttOpType == S_TOKEN_TYPE::OP_PLUS || sttOpType == S_TOKEN_TYPE::OP_MINUS || sttOpType == S_TOKEN_TYPE::OP_BOOL_NEG)
        CompileAuxiliaryTokens(Segment);

    if (!CompileExpression_L6(Segment))
    {
        SetError(ERR_INVALID_EXPRESSION);
        return false;
    }

    switch (sttOpType)
    {
    case S_TOKEN_TYPE::OP_MINUS:
        CompileToken(Segment, S_TOKEN_TYPE::OP_SMINUS);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
        break;
    case S_TOKEN_TYPE::OP_BOOL_NEG:
        CompileToken(Segment, S_TOKEN_TYPE::OP_BOOL_NEG);
        CompileToken(Segment, S_TOKEN_TYPE::STACK_TOP);
        break;
    }
    return true;
}

// '(' and ')'
bool COMPILER::CompileExpression_L6(SEGMENT_DESC &Segment)
{
    if (Token.GetType() == S_TOKEN_TYPE::OPEN_BRACKET)
    {
        CompileAuxiliaryTokens(Segment);
        if (!CompileExpression_L1(Segment))
        {
            SetError(ERR_INVALID_EXPRESSION);
            return false;
        }
        if (Token.GetType() != S_TOKEN_TYPE::CLOSE_BRACKET)
        {
            SetError("No matching ')' in expression");
            return false;
        }
        CompileAuxiliaryTokens(Segment);
        return true;
    }
    return CompileExpression_L7(Segment);
}

// read value
bool COMPILER::CompileExpression_L7(SEGMENT_DESC &Segment)
{
    uint32_t dwRCode = 0;
    S_TOKEN_TYPE sttResult;
    S_TOKEN_TYPE sttVariableField;
    S_TOKEN_TYPE sttFuncCallType;
    uint32_t dwVarCode;
    uint32_t dwAWCode;
    VARINFO vi;
    LVARINFO lvi;
    FUNCINFO fi;

    bool bDynamicCall = false;
    switch (Token.GetType())
    {
    case S_TOKEN_TYPE::SEPARATOR:

        return true;
    case S_TOKEN_TYPE::NUMBER:
        CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
        CompileNumber(Segment);
        CompileAuxiliaryTokens(Segment);
        return true;
    case S_TOKEN_TYPE::FLOAT_NUMBER:
        CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
        CompileFloatNumber(Segment);
        CompileAuxiliaryTokens(Segment);
        return true;
    case S_TOKEN_TYPE::STRING:
        CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
        CompileString(Segment);
        CompileAuxiliaryTokens(Segment);
        return true;
    case S_TOKEN_TYPE::TRUE_CONST:
        CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
        dwRCode = 1;
        CompileToken(Segment, S_TOKEN_TYPE::NUMBER, 1, (char *)&dwRCode, sizeof(uint32_t));
        CompileAuxiliaryTokens(Segment);
        return true;
    case S_TOKEN_TYPE::FALSE_CONST:
        CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
        dwRCode = 0;
        CompileToken(Segment, S_TOKEN_TYPE::NUMBER, 1, (char *)&dwRCode, sizeof(uint32_t));
        CompileAuxiliaryTokens(Segment);
        return true;

    case S_TOKEN_TYPE::CALL:
        bDynamicCall = true;
    case S_TOKEN_TYPE::UNKNOWN:
        if (!bDynamicCall)
        {
            sttResult = DetectUnknown(dwRCode);
            // set func call method to standart
            sttFuncCallType = S_TOKEN_TYPE::CALL_FUNCTION;
        }
        else
        {
            sttResult = S_TOKEN_TYPE::CALL;
            sttFuncCallType = S_TOKEN_TYPE::CALL_FUNCTION;
        }

        switch (sttResult)
        {
        case S_TOKEN_TYPE::DEFINE_VAL:
            DEFINFO di;
            DefTab.GetDef(di, dwRCode);
            switch (di.deftype)
            {
            case S_TOKEN_TYPE::NUMBER:
                CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
                CompileToken(Segment, S_TOKEN_TYPE::NUMBER, 1, (char *)&di.data4b, sizeof(uint32_t));
                CompileAuxiliaryTokens(Segment);
                break;
            case S_TOKEN_TYPE::FLOAT_NUMBER:
                CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
                CompileToken(Segment, S_TOKEN_TYPE::FLOAT_NUMBER, 1, (char *)&di.data4b, sizeof(uint32_t));
                CompileAuxiliaryTokens(Segment);
                break;
            case S_TOKEN_TYPE::STRING:
                CompileToken(Segment, S_TOKEN_TYPE::STACK_PUSH);
                uint32_t string_size;
                string_size = strlen((char *)di.data4b) + 1;
                CompileToken(Segment, S_TOKEN_TYPE::STRING, 2, (char *)&string_size, sizeof(uint32_t), (char *)di.data4b,
                             string_size);
                CompileAuxiliaryTokens(Segment);
                break;
            }
            return true;
        case S_TOKEN_TYPE::CALL:
            // this is dynamic function call

            // call type - dynamic
            sttFuncCallType = S_TOKEN_TYPE::CALL;

            if (Token.Get() != S_TOKEN_TYPE::UNKNOWN)
            {
                SetError("invalid dynamic call syntax");
                return false;
            }

            // check for string variable
            sttVariableField = DetectUnknown(dwVarCode);

            if (sttVariableField == S_TOKEN_TYPE::VARIABLE)
            {
                VarTab.GetVarX(vi, dwVarCode);
                if (vi.type != S_TOKEN_TYPE::VAR_STRING)
                {
                    SetError("'%s' must be string variable", Token.GetData());
                    return false;
                }
            }
            else if (sttVariableField == S_TOKEN_TYPE::LOCAL_VARIABLE)
            {
                FuncTab.GetVar(lvi, CurrentFuncCode, dwVarCode);
                if (lvi.type != S_TOKEN_TYPE::VAR_STRING)
                {
                    SetError("'%s' must be string variable", Token.GetData());
                    return false;
                }
            }
            else
            {
                SetError("invalid dynamic call syntax: '%s' must be string variable", Token.GetData());
                return false;
            }

            // proceed to standart func call routine

        case S_TOKEN_TYPE::CALL_FUNCTION:
            // this is function call

            // first prepare stack for calling function

            if (Token.Get() != S_TOKEN_TYPE::OPEN_BRACKET)
            {
                SetError("missing '('");
                return false;
            }

            // just get, already checked by DetectUnknown
            FuncTab.GetFuncX(fi, dwRCode);

            // if(fi.return_type == TVOID){ SetError("void function in expression: %s",fi.name); return false; }

            uint32_t func_args;
            func_args = 0;

            if (fi.arguments == 0 && !IsIntFuncVarArgsNum(dwRCode) && sttFuncCallType != S_TOKEN_TYPE::CALL)
            {
                // function without parameters
                if (Token.Get() != S_TOKEN_TYPE::CLOSE_BRACKET)
                {
                    SetError("missing ')'");
                    return false;
                }
            }
            else
            {
                // proceed with parameters list
                do
                {
                    // compile expression leave in stack top expression result
                    if (!CompileExpression(Segment))
                    {
                        SetError("invalid function argument");
                        return false;
                    }
                    if (Token.GetType() != S_TOKEN_TYPE::COMMA)
                    {
                        if (Token.GetType() != S_TOKEN_TYPE::CLOSE_BRACKET)
                        {
                            SetError("invalid function argument");
                            return false;
                        }
                    }
                    func_args++;
                } while (Token.GetType() == S_TOKEN_TYPE::COMMA);
            }

            /*
            // off for debug needs
            if(sttFuncCallType != CALL)    // skip argument checking for dynamic function call
            {
              if(func_args != fi.arguments)
              {
                if(fi.segment_id != INTERNAL_SEGMENT_ID)
                {
                  // not internal function cant accept different arguments number
                  SetError("function %s(args:%d) doesnt accept %d arguments",fi.name,fi.arguments,func_args);
                  return false;
                }
                else
                {
                  if(!IsIntFuncVarArgsNum(dwRCode))
                  {
                    SetError("function '%s(args:%d)' doesnt accept %d arguments",fi.name,fi.arguments,func_args);
                    return false;
                  }

                }
              }
            }//*/

            // call function
            if (sttFuncCallType == S_TOKEN_TYPE::CALL_FUNCTION)
                CompileToken(Segment, S_TOKEN_TYPE::CALL_FUNCTION, 1, (char *)&dwRCode, sizeof(uint32_t));
            else
            {
                // code of function must be find out on the fly from variable
                CompileToken(Segment, S_TOKEN_TYPE::CALL);
                CompileToken(Segment, sttVariableField, 1, (char *)&dwVarCode, sizeof(uint32_t));
            }
            CompileToken(Segment, S_TOKEN_TYPE::ARGS_NUM, 1, (char *)&func_args, sizeof(uint32_t));

            // ...
            // proceed to next token
            CompileAuxiliaryTokens(Segment);

            return true;
        case S_TOKEN_TYPE::LOCAL_VARIABLE:
        case S_TOKEN_TYPE::VARIABLE:

            // set local or global type
            sttVariableField = sttResult;

            // variable code
            dwVarCode = dwRCode;

            // we going to place into stack variable value or attribute string - alloc stack data
            CompileToken(Segment, S_TOKEN_TYPE::STACK_ALLOC);

            sttResult = Token.Get();
            if (sttResult == S_TOKEN_TYPE::SQUARE_OPEN_BRACKET)
            {
                // way for array element

                // doesnt need to check, becose already pass DetectUnknown function

                if (sttVariableField == S_TOKEN_TYPE::VARIABLE)
                {
                    VarTab.GetVarX(vi, dwVarCode);
                    // check for possibilities of '[' operator
                    if (!vi.bArray)
                    {
                        if (vi.type != S_TOKEN_TYPE::VAR_REFERENCE)
                        {
                            SetError(" C Invalid '[' operator, %s - isnt array", vi.name);
                            return false;
                        }
                    }
                }
                else
                {
                    FuncTab.GetVar(lvi, CurrentFuncCode, dwVarCode);
                    // check for possibilities of '[' operator
                    if (!lvi.bArray)
                    {
                        if (lvi.type != S_TOKEN_TYPE::VAR_REFERENCE)
                        {
                            SetError(" D Invalid '[' operator, %s - isnt array", lvi.name);
                            return false;
                        }
                    }
                }

                // CompileToken(Segment,STACK_ALLOC);
                // after top of the stack must be array index
                Token.Get();
                if (!CompileExpression_L2(Segment))
                    return false;
                // test for ']'
                if (Token.GetType() != S_TOKEN_TYPE::SQUARE_CLOSE_BRACKET)
                {
                    SetError("missing ']'");
                    return false;
                }
                // proceed to next token
                sttResult = CompileAuxiliaryTokens(Segment);

                // if not accessing attribute - copy value and return
                if (sttResult != S_TOKEN_TYPE::DOT)
                {
                    // optimiztion possible: MOVE [STACK-1],[STACK]

                    // copy array index to BX
                    CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
                    CompileToken(Segment, S_TOKEN_TYPE::BX);

                    // write to stack element value (index in X)
                    CompileToken(Segment, S_TOKEN_TYPE::STACK_WRITE_BXINDEX);
                    CompileToken(Segment, sttVariableField, 1, (char *)&dwVarCode, sizeof(uint32_t));

                    // proceed to next token
                    // CompileAuxiliaryTokens(Segment);

                    return true;
                }

                // in other case want to access attribute string value

                // copy array index to BX
                CompileToken(Segment, S_TOKEN_TYPE::STACK_POP);
                CompileToken(Segment, S_TOKEN_TYPE::BX);

                // move ap to AP register
                CompileToken(Segment, S_TOKEN_TYPE::MOVEAP_BXINDEX);
                CompileToken(Segment, S_TOKEN_TYPE::AP);
                CompileToken(Segment, sttVariableField, 1, (char *)&dwVarCode, sizeof(uint32_t));

                // proceed to ADVANCE AP
            }
            else
            {
                // way for non array var

                if (sttResult != S_TOKEN_TYPE::DOT)
                {
                    // copy variavle to stack
                    CompileToken(Segment, S_TOKEN_TYPE::STACK_WRITE);
                    CompileToken(Segment, sttVariableField, 1, (char *)&dwVarCode, sizeof(uint32_t));
                    return true;
                }

                // in other case want to access attribute string value

                // move ap to AP register
                CompileToken(Segment, S_TOKEN_TYPE::MOVEAP);
                CompileToken(Segment, S_TOKEN_TYPE::AP);
                CompileToken(Segment, sttVariableField, 1, (char *)&dwVarCode, sizeof(uint32_t));
            }

            // ADVANCE AP

            sttResult = Token.Get();
            do
            {
                // if(sttResult == OPEN_BRACKET)
                if (Token.GetType() == S_TOKEN_TYPE::OPEN_BRACKET)
                {
                    // access var
                    sttResult = Token.Get();
                    if (sttResult != S_TOKEN_TYPE::UNKNOWN)
                    {
                        SetError("Invalid access var syntax");
                        return false;
                    }
                    sttResult = DetectUnknown(dwVarCode);
                    if (!(sttResult == S_TOKEN_TYPE::VARIABLE || sttResult == S_TOKEN_TYPE::LOCAL_VARIABLE))
                    {
                        SetError("not variable: %s", Token.GetData());
                        return false;
                    }
                    CompileToken(Segment, S_TOKEN_TYPE::ADVANCE_AP);
                    CompileToken(Segment, sttResult, 1, &dwVarCode, sizeof(uint32_t));
                    if (Token.Get() != S_TOKEN_TYPE::CLOSE_BRACKET)
                    {
                        SetError("missing ')'");
                        return false;
                    }
                }
                else
                {
                    // access word
                    if (Token.GetData() == nullptr)
                    {
                        SetError("Invalid access string");
                        return false;
                    }
                    Token.LowCase();
                    dwAWCode = SCodec->Convert(Token.GetData());
                    CompileToken(Segment, S_TOKEN_TYPE::ADVANCE_AP);
                    CompileToken(Segment, S_TOKEN_TYPE::ACCESS_WORD_CODE, 1, &dwAWCode, sizeof(uint32_t));
                }
                sttResult = Token.Get();
                if (sttResult == S_TOKEN_TYPE::DOT)
                    Token.Get(); // get next token (after dot)
            } while (sttResult == S_TOKEN_TYPE::DOT);

            // write attribute string to stack top
            CompileToken(Segment, S_TOKEN_TYPE::STACK_WRITE);
            CompileToken(Segment, S_TOKEN_TYPE::AP_VALUE);

            return true;
        default:
            // ???
            SetError(ERR_INVALID_EXPRESSION);
            return false;
        }
        return true;
    }
    SetError(ERR_INVALID_EXPRESSION);
    return false;
}
