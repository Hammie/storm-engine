#pragma once

#include "defines.h"
#include "dx9render.h"
#include "entity.h"

#include "storm/dialog/sprite_renderer.hpp"

struct ScreenScale
{
    float x = 1.f;
    float y = 1.f;
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

    void CreateBack();

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

    int32_t selectedLink_ = 0;

    int32_t mainFont_{};
    int32_t nameFont_{};
    int32_t subFont_{};

    int32_t interfaceTexture_{};

    size_t textureLines_{};

    std::unique_ptr<storm::SpriteRenderer> spriteRenderer_;

    ScreenScale screenScale_{};

    entid_t headModel_{};
};
