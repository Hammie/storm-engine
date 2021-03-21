#ifndef STRING_CODEC_H
#define STRING_CODEC_H
#pragma once

#include <unordered_map>
#include <array>

class AbstractStringCodec
{
  public:
    virtual ~AbstractStringCodec() noexcept = default;

    [[nodiscard]] virtual uint32_t GetNum() = 0;

    virtual void Clear() = 0;

    [[nodiscard]] virtual const char *Convert(uint32_t code) const = 0;

    [[nodiscard]] virtual uint32_t Convert(const char *pString, bool &bNew) = 0;

    [[nodiscard]] virtual const char *Get() = 0;

    [[nodiscard]] virtual const char *GetNext() = 0;

    void VariableChanged();

    uint32_t Convert(const char *pString, long iLen);

    uint32_t Convert(const char *pString);
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
    const char* Convert(uint32_t code) const override;

    [[nodiscard]]
    uint32_t Convert(const char *pString, bool &bNew) override;

    [[nodiscard]]
    const char *Get() override;

    [[nodiscard]]
    const char *GetNext() override;

  private:
    std::unordered_map<uint32_t, std::string> table{};
    std::unordered_map<uint32_t, std::string>::iterator iterator{};
};

#endif