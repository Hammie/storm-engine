#ifndef STRING_CODEC_H
#define STRING_CODEC_H
#pragma once

#include <unordered_map>
#include <array>

using StringReference = uint32_t;

class AbstractStringCodec
{
  public:
    virtual ~AbstractStringCodec() noexcept = default;

    [[nodiscard]] virtual uint32_t GetNum() = 0;

    virtual void Clear() = 0;

    [[nodiscard]] virtual const char *Convert(StringReference code) const = 0;

    [[nodiscard]] virtual StringReference Convert(const char *pString, bool &bNew) = 0;

    [[nodiscard]] virtual const char *Get() = 0;

    [[nodiscard]] virtual const char *GetNext() = 0;

    void VariableChanged();

    StringReference Convert(const char *pString, long iLen);

    StringReference Convert(const char *pString);

    StringReference Convert(const std::string_view &pString);
};

class STRING_CODEC final : public AbstractStringCodec
{
  public:
    using AbstractStringCodec::Convert;

    [[nodiscard]]
    uint32_t GetNum() override
    {
        return table.size();
    };

    void Clear() override
    {
        table.clear();
    }

    [[nodiscard]]
    const char* Convert(StringReference code) const override;

    [[nodiscard]]
    StringReference Convert(const char *pString, bool &bNew) override;

    [[nodiscard]]
    const char *Get() override;

    [[nodiscard]]
    const char *GetNext() override;

  private:
    std::unordered_map<StringReference, std::string> table{};
    std::unordered_map<StringReference, std::string>::iterator iterator{};
};

#endif