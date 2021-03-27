#include "SeaGroup.h"
#include "../../Utils.h"

BI_SeaGroup::BI_SeaGroup(BI_ManagerBase *pManager) : BI_BaseGroup(pManager)
{
}

BI_SeaGroup::~BI_SeaGroup()
{
}

void BI_SeaGroup::Init()
{
    BI_BaseGroup::Init();

    if (Manager()->AttributesPointer == nullptr) {
        return;
    }
    Attribute& pARoot = Manager()->AttributesPointer->getProperty("sea");

    FRECT uv;
    RECT pos;

    // back
    Attribute& attr = pARoot["back"];
    if (!attr.empty()) {
        auto texture = attr["texture"].get<std::string_view>();
        auto color = attr["color"].get<uint32_t>();
        FULLRECT(uv);
        BIUtils::ReadRectFromAttr(attr, "uv", uv, uv);
        ZERO(pos);
        BIUtils::ReadRectFromAttr(attr, "pos", pos, pos);

        auto *const pNod = Manager()->CreateImageNode(texture.data(), uv, pos, color, BIImagePrioritet_Group_Beg);
        if (pNod)
            m_aNodes.push_back(pNod);
        // Manager()->GetImageRender()->CreateImage( BIType_square, texture, color, uv, pos, BIImagePrioritet_Group_Beg
        // );
    }
}
