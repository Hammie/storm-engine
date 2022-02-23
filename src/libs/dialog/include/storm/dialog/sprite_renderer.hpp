#pragma once

#include "defines.h"
#include "dx9render.h"

#include <vector>

namespace storm
{

struct Sprite
{
  public:
    FRECT position;
    FRECT uv;
    int32_t texture;
};

using SpriteHandle = size_t;

class SpriteRenderer
{
  public:
    explicit SpriteRenderer(VDX9RENDER &renderer);
    ~SpriteRenderer() noexcept;

    SpriteHandle CreateSprite(const Sprite &sprite);
    void UpdateSpritePosition(SpriteHandle handle, const FRECT &new_position);
    void UpdateSpriteUV(SpriteHandle handle, const FRECT &new_uv);
    void UpdateSpriteTexture(SpriteHandle handle, int32_t new_texture);

    void Draw(SpriteHandle sprite_handle);
    void DrawRange(SpriteHandle start, SpriteHandle end);

  private:
    void UpdateBuffers();

    enum UpdateFlags
    {
        UPDATE_VERTICES = 1,
        UPDATE_INDICES = 2,
    };

    VDX9RENDER &renderer_;

    std::vector<Sprite> sprites_;

    int32_t vertexBuffer_ = -1;
    int32_t indexBuffer_ = -1;

    int updateFlags_{};
};

} // namespace storm
