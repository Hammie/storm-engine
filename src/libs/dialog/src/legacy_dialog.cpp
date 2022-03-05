#include "legacy_dialog.hpp"

#include "dialog.h"
#include "core.h"
#include "message.h"
#include "v_module_api.h"
#include "shared/messages.h"
#include "model.h"
#include "geometry.h"

CREATE_CLASS(LegacyDialog)

VDX9RENDER *LegacyDialog::RenderService = nullptr;

namespace
{
constexpr const uint32_t COLOR_NORMAL = 0xFFFFFFFF;
constexpr const uint32_t COLOR_LINK_UNSELECTED = ARGB(255, 127, 127, 127);
constexpr const size_t DIALOG_MAX_LINES = 8;
constexpr const float DIVIDER_HEIGHT = 14;

constexpr const char* DEFAULT_DIALOG_TEXTURE = "dialog/dialog.tga";

constexpr FRECT scaleUv(FRECT uv, ScreenScale scale)
{
    return {
        uv.x1 * scale.x,
        uv.y1 * scale.y,
        uv.x2 * scale.x,
        uv.y2 * scale.y,
    };
}

} // namespace

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

    spriteRenderer_ = std::make_unique<storm::SpriteRenderer>(*RenderService);

    D3DVIEWPORT9 viewport;
    RenderService->GetViewport(&viewport);
    const auto screenSize = core.GetScreenSize();

    screenScale_.x = static_cast<float>(viewport.Width) / static_cast<float>(screenSize.width);
    screenScale_.y = static_cast<float>(viewport.Height) / static_cast<float>(screenSize.height);

    std::array<char, MAX_PATH> string_buffer{};

    auto ini = fio->OpenIniFile("Resource/Ini/dialog.ini");

    ini->ReadString("DIALOG", "mainfont", string_buffer.data(), MAX_PATH, "DIALOG0");
    mainFont_ = RenderService->LoadFont(string_buffer.data());
    ini->ReadString("DIALOG", "namefont", string_buffer.data(), MAX_PATH, "DIALOG0");
    nameFont_ = RenderService->LoadFont(string_buffer.data());
    ini->ReadString("DIALOG", "subfont", string_buffer.data(), MAX_PATH, "DIALOG0");
    subFont_ = RenderService->LoadFont(string_buffer.data());

    ini.reset();

    const char *texture = this->AttributesPointer->GetAttribute("texture");
    if (texture == nullptr)
    {
        texture = DEFAULT_DIALOG_TEXTURE;
    }
    interfaceTexture_ = RenderService->TextureCreate(texture);

    CreateBack();
    UpdateLinks();

    return true;
}

void LegacyDialog::Realize(uint32_t delta_time)
{
    D3DVIEWPORT9 vp;
    RenderService->GetViewport(&vp);

    CONTROL_STATE cs;
    core.Controls->GetControlState("DlgUp", cs);
    if (cs.state == CST_ACTIVATED && selectedLink_ > 0)
    {
        --selectedLink_;
    }
    core.Controls->GetControlState("DlgDown", cs);
    if (cs.state == CST_ACTIVATED && selectedLink_ < links_.size() - 1)
    {
        ++selectedLink_;
    }
    core.Controls->GetControlState("DlgAction", cs);
    if (cs.state == CST_ACTIVATED)
    {
        ATTRIBUTES *links_attr = AttributesPointer->GetAttributeClass("Links");
        if (links_attr)
        {
            ATTRIBUTES *selected_attr = links_attr->GetAttributeClass(selectedLink_);
            if (selected_attr)
            {
                const char* go = selected_attr->GetAttribute("go");
                AttributesPointer->SetAttribute("CurrentNode", go);
                selectedLink_ = 0;
                core.Event("DialogEvent");
            }
        }
    }

    spriteRenderer_->DrawRange(2, 7 + textureLines_);
    const bool drawDivider = !formattedLinks_.empty();
    spriteRenderer_->DrawRange(7 + DIALOG_MAX_LINES, 7 + DIALOG_MAX_LINES + (drawDivider ? 2 : 1));

    // Draw head
    if (headModel_) {
        CMatrix mtx, view, prj;
        uint32_t lightingState, zenableState;
        RenderService->GetTransform(D3DTS_VIEW, view);
        RenderService->GetTransform(D3DTS_PROJECTION, prj);
        RenderService->GetRenderState(D3DRS_LIGHTING, &lightingState);
        RenderService->GetRenderState(D3DRS_ZENABLE, &zenableState);

        mtx.BuildViewMatrix(CVECTOR(0.0f, 0.0f, 0.0f), CVECTOR(0.0f, 0.0f, 1.0f), CVECTOR(0.0f, 1.0f, 0.0f));
        RenderService->SetTransform(D3DTS_VIEW,(D3DXMATRIX*)&mtx);

        mtx.BuildProjectionMatrix(PId2-1.49f, screenScale_.x * 116, screenScale_.y * 158, 1.0f, 10.0f);
        RenderService->SetTransform(D3DTS_PROJECTION,(D3DXMATRIX*)&mtx);

        D3DVIEWPORT9 headViewport{};
        headViewport.X = static_cast<int32_t>(screenScale_.x * 31);
        headViewport.Y = static_cast<int32_t>(screenScale_.y * 28);
        headViewport.Width = static_cast<int32_t>(screenScale_.x * 115);
        headViewport.Height = static_cast<int32_t>(screenScale_.y * 157);
        headViewport.MinZ = 0.0f;
        headViewport.MaxZ = 1.0f;

        RenderService->SetViewport(&headViewport);
        RenderService->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        RenderService->SetRenderState(D3DRS_LIGHTING, TRUE);
        RenderService->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);

        D3DLIGHT9 oldLight{};
        BOOL oldLightEnabled = FALSE;
        RenderService->GetLight(0, &oldLight);
        RenderService->GetLightEnable(0, &oldLightEnabled);

        D3DLIGHT9 headLight{};
        headLight.Type = D3DLIGHT_DIRECTIONAL;
        headLight.Diffuse.r = 1.f;
        headLight.Diffuse.g = 1.f;
        headLight.Diffuse.b = 1.f;
        headLight.Diffuse.a = 1.f;
        headLight.Direction.x = -1.f;
        headLight.Direction.y = -1.f;
        headLight.Direction.z = 2.f;

        RenderService->SetLight(0, &headLight);
        RenderService->LightEnable(0, TRUE);
        const auto model = dynamic_cast<MODEL *>(EntityManager::GetEntityPointer(headModel_));
        model->ProcessStage(Entity::Stage::realize, delta_time);

        RenderService->SetLight(0, &oldLight);
        RenderService->LightEnable(0, oldLightEnabled);
        RenderService->SetTransform(D3DTS_VIEW, (D3DXMATRIX *)&view);
        RenderService->SetTransform(D3DTS_PROJECTION, (D3DXMATRIX *)&prj);
        RenderService->SetViewport(&vp);
        RenderService->SetRenderState(D3DRS_LIGHTING, lightingState);
        RenderService->SetRenderState(D3DRS_ZENABLE, zenableState);
    }

    spriteRenderer_->DrawRange(0, 2);

    // Draw text
    const auto fontScale = static_cast<float>(vp.Height) / 600.f;

    if (!chName_.empty())
    {
        RenderService->ExtPrint(nameFont_, COLOR_NORMAL, 0, PR_ALIGN_LEFT, true, fontScale, 0, 0,
                                static_cast<int32_t>(screenScale_.x * 168), static_cast<int32_t>(screenScale_.y * 28),
                                chName_.c_str());
    }

    const int32_t line_height = static_cast<int32_t>(RenderService->CharHeight(mainFont_) * fontScale);

    int32_t line_offset = 0;
    int32_t offset = line_height * formattedLinks_.size();
    for (size_t i = 0; i < formattedLinks_.size(); ++i)
    {
        const auto &link = formattedLinks_[i];
        RenderService->ExtPrint(subFont_, link.lineIndex == selectedLink_ ? COLOR_NORMAL : COLOR_LINK_UNSELECTED, 0, PR_ALIGN_LEFT,
                                true, fontScale, 0, 0,
                                static_cast<int32_t>(screenScale_.x * 35),
                                static_cast<int32_t>(screenScale_.y * 450 - offset) + line_offset, link.text.c_str());
        line_offset += line_height;
    }

    if (!dialogText_.empty())
    {
        line_offset = 0;
        offset += line_height * formattedDialogText_.size();
        offset += drawDivider ? static_cast<int32_t>(DIVIDER_HEIGHT * screenScale_.y) : 0;
        for (const std::string& text : formattedDialogText_)
        {
            RenderService->ExtPrint(mainFont_, COLOR_NORMAL, 0, PR_ALIGN_LEFT, true, fontScale, 0, 0,
                                    static_cast<int32_t>(screenScale_.x * 35),
                                    static_cast<int32_t>(screenScale_.y * 450 - offset) + line_offset,
                                    text.c_str());
            line_offset += line_height;
            
        }
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
    UpdateText();
    UpdateLinks();

    if (storm::iEquals(attributes->GetThisName(), "texture"))
    {
        RenderService->TextureRelease(interfaceTexture_);
        interfaceTexture_ = RenderService->TextureCreate(attributes->GetThisAttr());

        for (size_t i = 0; i < DIALOG_MAX_LINES + 9; ++i) {
            spriteRenderer_->UpdateSpriteTexture(i, interfaceTexture_);
        }
    }
    else if (storm::iEquals(attributes->GetThisName(), "headModel"))
    {
        EntityManager::EraseEntity(headModel_);

        std::string headModel = fmt::format("Heads/{}", attributes->GetThisAttr());

        headModel_ = EntityManager::CreateEntity("MODELR");

        VGEOMETRY *gs = static_cast<VGEOMETRY *>(core.GetService("geometry"));
        gs->SetTexturePath("characters\\");

        core.Send_Message(headModel_, "ls", MSG_MODEL_LOAD_GEO, headModel.c_str());
        core.Send_Message(headModel_, "ls", MSG_MODEL_LOAD_ANI, headModel.c_str());

        const auto model = dynamic_cast<MODEL *>(EntityManager::GetEntityPointer(headModel_));

        static CMatrix mtx;
        mtx.BuildPosition(0.f, 0.025f, 0.f);

        static CMatrix mtx2;
        mtx2.m[0][0] = 1.0f;
        mtx2.m[1][1] = 1.0f;
        mtx2.m[2][2] = 1.0f;

        static CMatrix mtx3;
        mtx3.BuildMatrix(0.1f, PI - 0.1f, 0.0f);

        static CMatrix mtx4;
        mtx4.BuildPosition(0.f, 0.f, 4.f);

        model->mtx = mtx * mtx2 * mtx3 * mtx4;

        model->GetAnimation()->CopyPlayerState(0, 1);
        model->GetAnimation()->Player(0).SetAction("dialog_all");
        model->GetAnimation()->Player(0).Play();
        model->GetAnimation()->Player(0).SetAutoStop(false);
    }
    else
    {
        D3DVIEWPORT9 vp;
        RenderService->GetViewport(&vp);
        const auto fontScale = static_cast<float>(vp.Height) / 600.f;
        const auto screenScale = static_cast<float>(vp.Height) / 480.f;
        const int32_t line_height = static_cast<int32_t>(fontScale * RenderService->CharHeight(mainFont_));
        const size_t text_lines = formattedDialogText_.size() + formattedLinks_.size();
        textureLines_ = static_cast<size_t>(
            std::floor(static_cast<double>((5 + text_lines * line_height) / screenScale) / TILED_LINE_HEIGHT));

        if (!formattedLinks_.empty())
        {
            textureLines_ += 1;
        }

        spriteRenderer_->UpdateSpritePosition(
            7 + DIALOG_MAX_LINES, {-39, static_cast<float>(479 - 67 + 39 - (textureLines_ + 1) * TILED_LINE_HEIGHT),
                                   639 + 39, static_cast<float>(479 - 67 + 39 - textureLines_ * TILED_LINE_HEIGHT)});

        float dividerOffset = 450 - formattedLinks_.size() * line_height / screenScale;
        spriteRenderer_->UpdateSpritePosition(8 + DIALOG_MAX_LINES,
                                              {35, static_cast<float>(dividerOffset - (DIVIDER_HEIGHT / 2)), 605,
                                               static_cast<float>(dividerOffset + (DIVIDER_HEIGHT / 2))});
    }

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

void LegacyDialog::UpdateText()
{
    const char *text_attr = AttributesPointer->GetAttribute("Text");
    if (text_attr)
    {
        dialogText_ = text_attr;
    }
    formattedDialogText_.clear();
    if (!dialogText_.empty())
    {
        D3DVIEWPORT9 vp;
        RenderService->GetViewport(&vp);
        const auto fontScale = static_cast<float>(vp.Height) / 600.f;

        const int32_t text_width_limit = static_cast<int32_t>(570.f * (vp.Width / 640.f));

        DIALOG::AddToStringArrayLimitedByWidth(dialogText_, mainFont_, fontScale, text_width_limit, formattedDialogText_,
                                               RenderService, nullptr, 0);
    }
}

void LegacyDialog::UpdateLinks()
{
    links_.clear();
    formattedLinks_.clear();
    ATTRIBUTES *links_attr = AttributesPointer->GetAttributeClass("Links");
    if (links_attr)
    {
        D3DVIEWPORT9 vp;
        RenderService->GetViewport(&vp);

        const auto fontScale = static_cast<float>(vp.Height) / 600.f;
        const int32_t text_width_limit = static_cast<int32_t>(570.f * (vp.Width / 640.f));

        const size_t number_of_links = links_attr->GetAttributesNum();
        for (size_t i = 0; i < number_of_links; ++i)
        {
            const std::string_view link_text = links_attr->GetAttribute(i);
            links_.emplace_back(link_text);

            std::vector<std::string> link_texts;
            DIALOG::AddToStringArrayLimitedByWidth(link_text, subFont_, fontScale, text_width_limit, link_texts,
                                                   RenderService, nullptr, 0);

            for (const auto &text : link_texts)
            {
                formattedLinks_.emplace_back(text, static_cast<int32_t>(i));
            }
        }
    }
}

void LegacyDialog::CreateBack()
{
    constexpr const ScreenScale textureScale = {
        .x = 1.f / 1024.f,
        .y = 1.f / 256.f,
    };

    // Head overlay
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {29, 25, 147, 37},
        .uv = scaleUv({904, 91, 904 + 119, 91 + 12}, textureScale),
        .texture = interfaceTexture_,
    });
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {29, 173, 146, 185},
        .uv = scaleUv({904, 105, 904 + 119, 105 + 11}, textureScale),
        .texture = interfaceTexture_,
    });

    // General
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {-39, -39, 169, 216},
        .uv = scaleUv({0, 0, 208, 255}, textureScale),
        .texture = interfaceTexture_,
    });
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {169, -39, 678, 79},
        .uv = scaleUv({208, 0, 757, 118}, textureScale),
        .texture = interfaceTexture_,
    });
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {-39, 451, 678, 518},
        .uv = scaleUv({209, 189, 1023, 255}, textureScale),
        .texture = interfaceTexture_,
    });
    // Static vertices 2
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {29, 25, 147, 37},
        .uv = scaleUv({904, 91, 1023, 103}, textureScale),
        .texture = interfaceTexture_,
    });
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {29, 173, 146, 185},
        .uv = scaleUv({904, 105, 1023, 116}, textureScale),
        .texture = interfaceTexture_,
    });

    for (size_t i = 0; i < DIALOG_MAX_LINES; ++i)
    {
        spriteRenderer_->CreateSprite(storm::Sprite{
            .position = {-39, static_cast<float>(479 - 67 + 39 - (26 * (i + 1))), 639 + 39,
                         static_cast<float>(479 - 67 + 39 - (26 * i))},
            .uv = scaleUv({209, 155, 1023, 186}, textureScale),
            .texture = interfaceTexture_,
        });
    }
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {-39, static_cast<float>(479 - 67 + 39 - (26 * (DIALOG_MAX_LINES + 1))), 639 + 39,
                     static_cast<float>(479 - 67 + 39 - (26 * DIALOG_MAX_LINES))},
        .uv = scaleUv({209, 119, 1023, 156}, textureScale),
        .texture = interfaceTexture_,
    });
    // Divider
    spriteRenderer_->CreateSprite(storm::Sprite{
        .position = {35, static_cast<float>(479 - 67 + 39 - DIVIDER_HEIGHT), 605, static_cast<float>(479 - 67 + 39)},
        .uv = scaleUv({209, 94, 602, 116}, textureScale),
        .texture = interfaceTexture_,
    });
}
