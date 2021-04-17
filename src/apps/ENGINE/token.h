#pragma once

#include <Windows.h>
#include <cstdint>
#include <string_view>
#include <vector>

#include <storm/scripting/tokens.hpp>

using S_TOKEN_TYPE = storm::scripting::TokenType;

class TOKEN
{
    S_TOKEN_TYPE eTokenType;
    long Lines_in_token;
    std::vector<ptrdiff_t> ProgramSteps;
    std::string_view ProgramPointer{};

  public:
    TOKEN();
    ~TOKEN();
    void Reset();
    void SetProgram(const std::string_view &pProgramBase, const std::string_view &pProgramControl);

    [[nodiscard]] ptrdiff_t GetProgramOffset() const noexcept {
        return std::distance(m_ProgramBase.data(), ProgramPointer.data());
    }

    S_TOKEN_TYPE Get(bool bKeepData = false);
    S_TOKEN_TYPE ProcessToken(std::string_view &pointer, bool bKeepData = false);
    S_TOKEN_TYPE GetType();
    void CacheToken(const std::string_view &pointer);
    void StepBack();
    long SetTokenData(const std::string_view &input, bool bKeepControlSymbols = false);
    void StartArgument(std::string_view &pointer, bool bKeepControlSymbols = false);
    [[nodiscard]] std::string_view GetTypeName(S_TOKEN_TYPE code) const;
    const char *GetData();
    bool IsNumber(const char *pointer);
    bool IsFloatNumber(const char *pointer);
    long TokenLines();
    void LowCase();

    S_TOKEN_TYPE FormatGet();

  private:
    std::string m_TokenData{};
    std::string_view m_ProgramBase{};
};
