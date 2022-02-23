#include "storm/dialog/sprite_renderer.hpp"

#include "core.h"

namespace storm
{

namespace
{
constexpr int32_t XI_TEX_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXTUREFORMAT2;

constexpr const uint32_t COLOR_NORMAL = 0xFFFFFFFF;

struct XI_TEX_VERTEX
{
    CVECTOR pos;
    float rhw;
    uint32_t color;
    float u, v;
};

struct ScreenScale
{
    float x = 1.f;
    float y = 1.f;
};

bool rectEqual(const FRECT &first, const FRECT &second)
{
    constexpr float threshold = 1e-5f;
    return std::abs(first.left - second.left) < threshold && std::abs(first.top - second.top) < threshold &&
           std::abs(first.right - second.right) < threshold && std::abs(first.bottom - second.bottom) < threshold;
}

std::array<XI_TEX_VERTEX, 4> createSpriteMesh(const Sprite &sprite, ScreenScale scale)
{
    return {
        XI_TEX_VERTEX{.pos{scale.x * sprite.position.left, scale.y * sprite.position.top, 1.f},
                      .rhw{0.5f},
                      .color{COLOR_NORMAL},
                      .u{sprite.uv.left},
                      .v{sprite.uv.top}},
        XI_TEX_VERTEX{.pos{scale.x * sprite.position.right, scale.y * sprite.position.top, 1.f},
                      .rhw{0.5f},
                      .color{COLOR_NORMAL},
                      .u{sprite.uv.right},
                      .v{sprite.uv.top}},
        XI_TEX_VERTEX{.pos{scale.x * sprite.position.left, scale.y * sprite.position.bottom, 1.f},
                      .rhw{0.5f},
                      .color{COLOR_NORMAL},
                      .u{sprite.uv.left},
                      .v{sprite.uv.bottom}},
        XI_TEX_VERTEX{.pos{scale.x * sprite.position.right, scale.y * sprite.position.bottom, 1.f},
                      .rhw{0.5f},
                      .color{COLOR_NORMAL},
                      .u{sprite.uv.right},
                      .v{sprite.uv.bottom}},
    };
}

void updateVertexBuffer(VDX9RENDER &renderService, int32_t vertexBuffer, const std::vector<Sprite> &sprites)
{
    D3DVIEWPORT9 viewport;
    renderService.GetViewport(&viewport);
    const auto screenSize = core.GetScreenSize();

    ScreenScale screenScale{
        static_cast<float>(viewport.Width) / static_cast<float>(screenSize.width),
        static_cast<float>(viewport.Height) / static_cast<float>(screenSize.height),
    };

    auto *pV = static_cast<XI_TEX_VERTEX *>(renderService.LockVertexBuffer(vertexBuffer));
    size_t vi = 0;
    for (const Sprite &sprite : sprites)
    {
        const auto &vertices = createSpriteMesh(sprite, screenScale);
        pV[vi++] = vertices[0];
        pV[vi++] = vertices[1];
        pV[vi++] = vertices[2];
        pV[vi++] = vertices[3];
    }
    renderService.UnLockVertexBuffer(vertexBuffer);
}

} // namespace

SpriteRenderer::SpriteRenderer(VDX9RENDER &renderer) : renderer_(renderer)
{
}

SpriteRenderer::~SpriteRenderer() noexcept
{
    if (vertexBuffer_ != -1) {
        renderer_.ReleaseVertexBuffer(vertexBuffer_);
    }

    if (indexBuffer_ != -1) {
        renderer_.ReleaseIndexBuffer(indexBuffer_);
    }
}

SpriteHandle SpriteRenderer::CreateSprite(const Sprite &sprite)
{
    updateFlags_ |= UpdateFlags::UPDATE_INDICES;

    sprites_.push_back(sprite);
    return sprites_.size() - 1;
}

void SpriteRenderer::UpdateSpritePosition(SpriteHandle handle, const FRECT &new_position)
{
    auto &sprite = sprites_[handle];

    if (!rectEqual(sprite.position, new_position))
    {
        updateFlags_ |= UpdateFlags::UPDATE_VERTICES;
        sprite.position = new_position;
    }
}

void SpriteRenderer::UpdateSpriteUV(SpriteHandle handle, const FRECT &new_uv)
{
    auto &sprite = sprites_[handle];

    if (!rectEqual(sprite.uv, new_uv))
    {
        updateFlags_ |= UpdateFlags::UPDATE_VERTICES;
        sprite.uv = new_uv;
    }
}

void SpriteRenderer::UpdateSpriteTexture(SpriteHandle handle, int32_t new_texture)
{
    auto &sprite = sprites_[handle];
    sprite.texture = new_texture;
}

void SpriteRenderer::Draw(SpriteHandle sprite_handle)
{
    UpdateBuffers();

    const auto &sprite = sprites_[sprite_handle];

    renderer_.TextureSet(0, sprite.texture);
    renderer_.DrawBuffer(vertexBuffer_, sizeof(XI_TEX_VERTEX), indexBuffer_, 0, sprites_.size() * 4, sprite_handle * 6,
                         2, "texturedialogfon");
}

void SpriteRenderer::DrawRange(SpriteHandle start, SpriteHandle end)
{
    UpdateBuffers();

    for (SpriteHandle current = start; current < end;) {
        const int32_t texture = sprites_[current].texture;
        SpriteHandle span_end = current + 1;
        while (span_end < end && sprites_[span_end].texture == texture) {
            ++span_end;
        }
        const size_t length = span_end - current;
        renderer_.TextureSet(0, texture);
        renderer_.DrawBuffer(vertexBuffer_, sizeof(XI_TEX_VERTEX), indexBuffer_, 0, sprites_.size() * 4, current * 6,
                             length * 2, "texturedialogfon");
        current = span_end;
    }
}

void SpriteRenderer::UpdateBuffers()
{
    const auto sprite_count = sprites_.size();
    const auto vertex_count = 4 * sprite_count;
    const auto index_count = 6 * sprite_count;

    if (vertexBuffer_ == -1)
    {
        vertexBuffer_ =
            renderer_.CreateVertexBuffer(XI_TEX_FVF, vertex_count * sizeof(XI_TEX_VERTEX), D3DUSAGE_WRITEONLY);
        updateFlags_ |= UpdateFlags::UPDATE_VERTICES;
    }

    if (updateFlags_ & UpdateFlags::UPDATE_VERTICES)
    {
        updateVertexBuffer(renderer_, vertexBuffer_, sprites_);
    }

    if (indexBuffer_ == -1)
    {
        indexBuffer_ = renderer_.CreateIndexBuffer(index_count * sizeof(uint16_t));
        updateFlags_ |= UpdateFlags::UPDATE_INDICES;
    }

    if (updateFlags_ & UpdateFlags::UPDATE_INDICES)
    {
        auto *pI = static_cast<uint16_t *>(renderer_.LockIndexBuffer(indexBuffer_));
        for (long n = 0; n < sprite_count; n++)
        {
            pI[n * 6 + 0] = static_cast<uint16_t>(n * 4 + 0);
            pI[n * 6 + 1] = static_cast<uint16_t>(n * 4 + 2);
            pI[n * 6 + 2] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 3] = static_cast<uint16_t>(n * 4 + 1);
            pI[n * 6 + 4] = static_cast<uint16_t>(n * 4 + 2);
            pI[n * 6 + 5] = static_cast<uint16_t>(n * 4 + 3);
        }
        renderer_.UnLockIndexBuffer(indexBuffer_);
    }

    updateFlags_ = 0;
}

} // namespace storm