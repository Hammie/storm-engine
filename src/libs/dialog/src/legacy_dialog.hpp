#pragma once

#include "defines.h"
#include "dx9render.h"
#include "entity.h"

struct SpriteBuffer
{
    int32_t indexBuffer;
    int32_t vertexBuffer;
};

struct ScreenScale
{
    float x = 1.f;
    float y = 1.f;
};

struct Sprite
{
    FRECT uv;
    FRECT position;
};

class LegacyDialog final : public Entity
{
    static VDX9RENDER *RenderService;

public:
    ~LegacyDialog() noexcept override;

    bool Init() override;

    void Realize(uint32_t delta_time);

    void ProcessStage(Stage stage, uint32_t delta) override;

    uint32_t AttributeChanged(ATTRIBUTES *) override;

    uint64_t ProcessMessage(MESSAGE &msg) override;

private:
    SpriteBuffer CreateBack();

    bool forceEmergencyClose_ = false;
    std::string selectedLinkName_;

    std::string chName_;

    int32_t mainFont_{};
    int32_t nameFont_{};

    int32_t interfaceTexture_{};

    SpriteBuffer spriteBuffer_{};

    ScreenScale screenScale_{};
    ScreenScale textureScale_{};
};
