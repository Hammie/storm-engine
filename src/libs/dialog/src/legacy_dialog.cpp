#include "legacy_dialog.hpp"

#include "dialog.h"
#include "core.h"
#include "message.h"
#include "v_module_api.h"

CREATE_CLASS(LegacyDialog)

VDX9RENDER *LegacyDialog::RenderService = nullptr;

namespace
{
constexpr const uint32_t COLOR_NORMAL = 0xFFFFFFFF;

std::array<XI_TEX_VERTEX, 4> createSpriteMesh(const Sprite &sprite, ScreenScale scale, ScreenScale uvScale)
{
    return {
        XI_TEX_VERTEX{
            .pos{scale.x * sprite.position.left, scale.y * sprite.position.top, 1.f},
            .rhw{0.5f},
            .color{COLOR_NORMAL},
            .u{uvScale.x * sprite.uv.left},
            .v{uvScale.y * sprite.uv.top}},
        XI_TEX_VERTEX{
            .pos{scale.x * sprite.position.right, scale.y * sprite.position.top, 1.f},
            .rhw{0.5f},
            .color{COLOR_NORMAL},
            .u{uvScale.x * sprite.uv.right},
            .v{uvScale.y * sprite.uv.top}},
        XI_TEX_VERTEX{
            .pos{scale.x * sprite.position.left, scale.y * sprite.position.bottom, 1.f},
            .rhw{0.5f},
            .color{COLOR_NORMAL},
            .u{uvScale.x * sprite.uv.left},
            .v{uvScale.y * sprite.uv.bottom}},
        XI_TEX_VERTEX{
            .pos{scale.x * sprite.position.right, scale.y * sprite.position.bottom, 1.f},
            .rhw{0.5f},
            .color{COLOR_NORMAL},
            .u{uvScale.x * sprite.uv.right},
            .v{uvScale.y * sprite.uv.bottom}},
    };
}
}

LegacyDialog::~LegacyDialog() noexcept
{
    if (interfaceTexture_)
    {
        RenderService->TextureRelease(interfaceTexture_);
    }
}

bool LegacyDialog::Init()
{
    RenderService = static_cast<VDX9RENDER *>(core.GetService("dx9render"));
    Assert(RenderService);

    D3DVIEWPORT9 viewport;
    RenderService->GetViewport(&viewport);
    const auto screenSize = core.GetScreenSize();

    screenScale_.x = static_cast<float>(viewport.Width) / static_cast<float>(screenSize.width);
    screenScale_.y = static_cast<float>(viewport.Height) / static_cast<float>(screenSize.height);

    textureScale_ = {
        .x = 1.f / 1024.f,
        .y = 1.f / 256.f,
    };

    std::array<char, MAX_PATH> string_buffer{};

    auto ini = fio->OpenIniFile("Resource/Ini/dialog.ini");

    ini->ReadString("DIALOG", "mainfont", string_buffer.data(), MAX_PATH, "DIALOG0");
    mainFont_ = RenderService->LoadFont(string_buffer.data());
    ini->ReadString("DIALOG", "namefont", string_buffer.data(), MAX_PATH, "DIALOG0");
    nameFont_ = RenderService->LoadFont(string_buffer.data());

    ini.reset();

    interfaceTexture_ = RenderService->TextureCreate("dialog/dialog_vanilla.tga");

    spriteBuffer_ = CreateBack();

    return true;
}

void LegacyDialog::Realize(uint32_t delta_time)
{
    D3DVIEWPORT9 vp;
    RenderService->GetViewport(&vp);

    RenderService->TextureSet(0, interfaceTexture_);
    RenderService->DrawBuffer(spriteBuffer_.vertexBuffer, sizeof(XI_TEX_VERTEX), spriteBuffer_.indexBuffer, 0, 12, 0, 6,
                              "texturedialogfon");

    if (!chName_.empty())
    {
        const auto scale = static_cast<float>(vp.Height) / 600.f;
        RenderService->ExtPrint(nameFont_, COLOR_NORMAL, 0, PR_ALIGN_LEFT, true, scale, 0, 0,
                                static_cast<int32_t>(screenScale_.x * 168), static_cast<int32_t>(screenScale_.y * 28),
                                chName_.c_str());
    }
}

void LegacyDialog::ProcessStage(Stage stage, uint32_t delta)
{
    switch (stage)
    {
    // case Stage::execute:
    //    Execute(delta); break;
    case Stage::realize:
        Realize(delta);
        break;
    /*case Stage::lost_render:
        LostRender(delta); break;
    case Stage::restore_render:
        RestoreRender(delta); break;*/
    }
}

uint32_t LegacyDialog::AttributeChanged(ATTRIBUTES *attributes)
{
    return Entity::AttributeChanged(attributes);
}

uint64_t LegacyDialog::ProcessMessage(MESSAGE &msg)
{
    switch (msg.Long())
    {
    case 0: {
        // Get character ID
        // persId = msg.EntityID();
        // persMdl = msg.EntityID();
        break;
    }
    case 1: {
        // Get person ID
        const entid_t charId = msg.EntityID();
        const entid_t charMdl = msg.EntityID();
        const auto name_attr = core.Entity_GetAttribute(charId, "name");
        const auto last_name_attr = core.Entity_GetAttribute(charId, "lastname");
        const std::string_view name = name_attr ? name_attr : "";
        const std::string_view last_name = last_name_attr ? last_name_attr : "";
        chName_ = fmt::format("{} {}", name, last_name);
        std::transform(chName_.begin(), chName_.end(), chName_.begin(), ::toupper);
        break;
    }
    }

    return 0;
}

SpriteBuffer LegacyDialog::CreateBack()
{
    std::vector<Sprite> sprites{
        {
            .uv = {0, 0, 208, 255},
            .position = {-39, -39, 169, 216},
        },
        {
            .uv = {208, 0, 757, 118},
            .position = {169, -39, 678, 79},
        },
        {
            .uv = {209, 189, 1023, 255},
            .position = {-39, 451, 678, 518},
        }
    };

    const size_t squareCount = sprites.size();

    const size_t indexCount = 6 * squareCount;
    const size_t vertexCount = 4 * squareCount;

    const int32_t vertexBuffer =
        RenderService->CreateVertexBuffer(XI_TEX_FVF, vertexCount * sizeof(XI_TEX_VERTEX), D3DUSAGE_WRITEONLY);
    const int32_t indexBuffer = RenderService->CreateIndexBuffer(indexCount * sizeof(uint16_t));

    auto *pI = static_cast<uint16_t *>(RenderService->LockIndexBuffer(indexBuffer));
    for (long n = 0; n < squareCount; n++)
    {
        pI[n * 6 + 0] = static_cast<uint16_t>(n * 4 + 0);
        pI[n * 6 + 1] = static_cast<uint16_t>(n * 4 + 2);
        pI[n * 6 + 2] = static_cast<uint16_t>(n * 4 + 1);
        pI[n * 6 + 3] = static_cast<uint16_t>(n * 4 + 1);
        pI[n * 6 + 4] = static_cast<uint16_t>(n * 4 + 2);
        pI[n * 6 + 5] = static_cast<uint16_t>(n * 4 + 3);
    }
    RenderService->UnLockIndexBuffer(indexBuffer);

    // float left, top, right, bottom;
    auto *pV = static_cast<XI_TEX_VERTEX *>(RenderService->LockVertexBuffer(vertexBuffer));
    size_t vi = 0;
    for (const Sprite &sprite : sprites)
    {
        const auto &vertices = createSpriteMesh(sprite, screenScale_, textureScale_);
        pV[vi++] = vertices[0];
        pV[vi++] = vertices[1];
        pV[vi++] = vertices[2];
        pV[vi++] = vertices[3];
    }
    RenderService->UnLockVertexBuffer(vertexBuffer);

    return {indexBuffer, vertexBuffer};
}