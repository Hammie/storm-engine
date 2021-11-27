#pragma once

namespace storm
{

class Color
{
  public:
    constexpr Color() noexcept = default;

    explicit constexpr Color(uint32_t color) noexcept
        : alpha_((color & 0xFF'00'00'00) >> 24), red_((color & 0x00'FF'00'00) >> 16),
          green_((color & 0x00'00'FF'00) >> 8), blue_(color & 0x00'00'00'FF)
    {
    }

    constexpr Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
        : alpha_(alpha), red_(red), green_(green), blue_(blue)
    {
    }

    explicit constexpr operator uint32_t() const noexcept
    {
        return (static_cast<uint32_t>(alpha_) << 24ul) | (static_cast<uint32_t>(red_) << 16ul) |
               (static_cast<uint32_t>(green_) << 8ul) | static_cast<uint32_t>(blue_);
    }

    [[nodiscard]] constexpr uint8_t Alpha() const noexcept
    {
        return alpha_;
    }

    [[nodiscard]] constexpr uint8_t Red() const noexcept
    {
        return red_;
    }

    [[nodiscard]] constexpr uint8_t Green() const noexcept
    {
        return green_;
    }

    [[nodiscard]] constexpr uint8_t Blue() const noexcept
    {
        return blue_;
    }

  private:
    uint8_t alpha_ = 255;
    uint8_t red_ = 0;
    uint8_t green_ = 0;
    uint8_t blue_ = 0;
};

} // namespace storm
