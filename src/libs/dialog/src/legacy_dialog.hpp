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
    void UpdateText();
    void UpdateLinks();

    SpriteBuffer CreateBack();

    bool forceEmergencyClose_ = false;
    std::string selectedLinkName_;

    std::string chName_;
    std::string dialogText_;
    std::vector<std::string> formattedDialogText_;

    struct LinkEntry
    {
        std::string text;
        int32_t lineIndex = 0;
    };
    std::vector<std::string> links_;
    std::vector<LinkEntry> formattedLinks_;
    std::vector<Sprite> sprites_;

    int32_t selectedLink_ = 0;

    int32_t mainFont_{};
    int32_t nameFont_{};
    int32_t subFont_{};

    int32_t interfaceTexture_{};

    size_t textureLines_{};

    SpriteBuffer spriteBuffer_{};

    ScreenScale screenScale_{};
    ScreenScale textureScale_{};
};
