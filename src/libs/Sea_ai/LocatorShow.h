#ifndef SEA_LOCATOR_SHOW_HPP
#define SEA_LOCATOR_SHOW_HPP

#include "AIHelper.h"

class SeaLocatorShow : public Entity
{
    Attribute *pALocators;
    bool bShow;
    float fScale;

    float fWidth, fHeight;
    CMatrix view, mtx;

    struct SphVertex
    {
        CVECTOR v;
        uint32_t c;
    };

    uint32_t sphereNumTrgs;
    SphVertex *sphereVertex;

    void CreateSphere();

    bool isLocator(const Attribute &pA);
    CVECTOR GetLocatorPos(const Attribute &attr);
    float GetLocatorAng(const Attribute &attr);
    float GetLocatorRadius(const Attribute &attr);
    const char *GetRealLocatorName(const Attribute &attr);
    const char *GetLocatorName(const Attribute &attr);
    const char *GetLocatorGroupName(const Attribute &attr);

    void PrintLocator(const Attribute &attr);
    void ProcessLocators(const Attribute &pA);

  public:
    SeaLocatorShow();
    ~SeaLocatorShow();

    bool Init() override;
    void SetDevice();

    void Realize(uint32_t Delta_Time);
    void Execute(uint32_t Delta_Time) const;

    bool CreateState(ENTITY_STATE_GEN *state_gen);
    bool LoadState(ENTITY_STATE *state);

    void ProcessMessage(uint32_t iMsg, uint32_t wParam, uint32_t lParam);
    uint64_t ProcessMessage(MESSAGE &message) override;

    void ProcessStage(Stage stage, uint32_t delta) override
    {
        switch (stage)
        {
        case Stage::execute:
            Execute(delta);
            break;
        case Stage::realize:
            Realize(delta);
            break;
            /*case Stage::lost_render:
              LostRender(delta); break;
            case Stage::restore_render:
              RestoreRender(delta); break;*/
        }
    }

    uint32_t AttributeChanged(Attribute &pAttribute) override;
};

#endif
