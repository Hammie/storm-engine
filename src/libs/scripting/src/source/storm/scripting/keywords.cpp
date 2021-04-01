#include "storm/scripting/keywords.hpp"

#include <storm/string_compare.hpp>

#include <map>

namespace storm::scripting {

namespace
{

auto getKeywordMap()
{
    return std::map<std::string_view, S_TOKEN_TYPE, decltype(&storm::iLess<std::string_view>)>(
        {
            {"#hold", HOLD_COMPILATION},
            {"#include", INCLIDE_FILE},
            {"#event_handler", EVENT_HANDLER},
            {"#libriary", INCLUDE_LIBRIARY},
            {"int", VAR_INTEGER},
            {"ptr", VAR_PTR},
            {"float", VAR_FLOAT},
            {"string", VAR_STRING},
            {"object", VAR_OBJECT},
            {"ref", VAR_REFERENCE},
            {"aref", VAR_AREFERENCE},
            {"{", BLOCK_IN},
            {"}", BLOCK_OUT},
            {";", SEPARATOR},
            {"return", FUNCTION_RETURN},
            {"for", FOR_BLOCK},
            {"if", IF_BLOCK},
            {"while", WHILE_BLOCK},
            {"continue", CONTINUE_COMMAND},
            {"break", BREAK_COMMAND},
            {"goto", GOTO_COMMAND},
            {":", LABEL},
            {"void", TVOID},
            {"switch", SWITCH_COMMAND},
            {"case", CASE_COMMAND},
            {"#define", DEFINE_COMMAND},
            {"makeref", MAKEREF_COMMAND},
            {"makearef", MAKEAREF_COMMAND},
            {"(", OPEN_BRACKET},
            {")", CLOSE_BRACKET},
            {"[", SQUARE_OPEN_BRACKET},
            {"]", SQUARE_CLOSE_BRACKET},
            {"=", OP_EQUAL},
            {"==", OP_BOOL_EQUAL},
            {">", OP_GREATER},
            {">=", OP_GREATER_OR_EQUAL},
            {"<", OP_LESSER},
            {"<=", OP_LESSER_OR_EQUAL},
            {"!=", OP_NOT_EQUAL},
            {"+=", OP_INCADD},
            {"-=", OP_DECADD},
            {"*=", OP_MULTIPLYEQ},
            {"/=", OP_DIVIDEEQ},
            {"-", OP_MINUS},
            {"+", OP_PLUS},
            {"*", OP_MULTIPLY},
            {"/", OP_DIVIDE},
            {"^", OP_POWER},
            {"%", OP_MODUL},
            {"++", OP_INC},
            {"--", OP_DEC},
            {"//", LINE_COMMENT},
            {"#ifndef", LINE_COMMENT},
            {"#endif", LINE_COMMENT},
            {",", COMMA},
            {".", DOT},
            {"&&", OP_BOOL_AND},
            {"||", OP_BOOL_OR},
            {"&", AND},
            {"#file", DEBUG_FILE_NAME},
            {"extern", EXTERN},
            {"else", ELSE_BLOCK},
            {"bool", VAR_INTEGER},
            {"true", TRUE_CONST},
            {"false", FALSE_CONST},
            {"!", OP_BOOL_NEG},
            {"class", CLASS_DECL},
            {"native", IMPORT},
            {"call", CALL},
        },
        storm::iLess);
}

} // namespace

S_TOKEN_TYPE KeywordManager::getTokenForKeyword(const std::string_view &keyword)
{
    static auto keyword_map = getKeywordMap();

    const auto find = keyword_map.find(keyword);
    if (find != keyword_map.end()) {
        return find->second;
    }

    return S_TOKEN_TYPE::UNKNOWN;
}

} // namespace storm::scripting
