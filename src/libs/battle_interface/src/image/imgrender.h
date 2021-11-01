#pragma once

#include "material.h"

#include <gsl/gsl>

class BIImageRender final
{
  public:
    explicit BIImageRender(gsl::not_null<VDX9RENDER *> pRS);
    ~BIImageRender() noexcept;

    void Render();

    gsl::not_null<IBIImage *> CreateImage(BIImageType type, const char *pcTextureName, uint32_t color, const FRECT &uv,
                                          const RECT &pos, long nPrior = ImagePrioritet_DefaultValue,
                                          const char *pcTechniqueName = nullptr);

    BIImageMaterial *FindMaterial(const char *pcTextureName, const char *pcTechniqueName);
    gsl::not_null<BIImageMaterial *> CreateMaterial(const char *pcTextureName, const char *pcTechniqueName = nullptr);
    void DeleteMaterial(BIImageMaterial *pMat);

    void ReleaseAllImages();
    size_t GetImageQuantity();

    void MaterialSorting();

    void TranslateBasePosToRealPos(float fXBase, float fYBase, float &fXReal, float &fYReal) const;

  private:
    gsl::not_null<IBIImage *> CreateImage(BIImageType type, const char *pcTextureName, uint32_t color, const FRECT &uv,
                                          long nLeft, long nTop, long nRight, long nBottom,
                                          long nPrior = ImagePrioritet_DefaultValue,
                                          const char *pcTechniqueName = nullptr);

    bool GetFirstPrioritetRange();
    bool GetNextPrioritetRange();

    long m_nBeginOutputPrioritet;
    long m_nEndOutputPrioritet;

    gsl::not_null<VDX9RENDER *> m_pRS;
    std::vector<BIImageMaterial *> m_apMaterial;

    static constexpr const float m_fHScale = 1.f;
    static constexpr const float m_fVScale = 1.f;
    static constexpr const float m_fHOffset = 0.f;
    static constexpr const float m_fVOffset = 0.f;
};

inline void BIImageRender::TranslateBasePosToRealPos(float fXBase, float fYBase, float &fXReal, float &fYReal) const
{
    fXReal = (m_fHOffset + fXBase) * m_fHScale;
    fYReal = (m_fVOffset + fYBase) * m_fVScale;
}
