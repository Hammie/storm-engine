#pragma once

#include "image_defines.h"
#include <vector>

#include <gsl/gsl>

class BIImageMaterial;

class BIImage final : public IBIImage
{
  public:
    explicit BIImage(gsl::not_null<BIImageMaterial *> pMaterial);
    ~BIImage() override;

    [[nodiscard]] size_t GetVertexQuantity() const noexcept
    {
        return m_nVertexQuantity;
    }

    [[nodiscard]] size_t GetTriangleQuantity() const noexcept
    {
        return m_nTriangleQuantity;
    }

    void FillBuffers(BI_IMAGE_VERTEX *pV, uint16_t *pT, size_t &nV, size_t &nT);

    void SetColor(uint32_t color) override;
    void SetPosition(long nLeft, long nTop, long nRight, long nBottom) override;
    void Set3DPosition(const CVECTOR &vPos, float fWidth, float fHeight) override;
    void SetUV(const FRECT &uv) override;
    void SetType(BIImageType type);

    void CutSide(float fleft, float fright, float ftop, float fbottom) override;
    void CutClock(float fBegin, float fEnd, float fFactor) override;

    [[nodiscard]] long GetPriority() const noexcept
    {
        return priority_;
    }

    void SetPriority(long priority) noexcept
    {
        priority_ = priority;
    }

  private:
    static FPOINT &GetClockPoint(float fAng, FPOINT &fp);
    static float GetNextClockCorner(float fAng);
    static float GetPrevClockCorner(float fAng);

    gsl::not_null<BIImageMaterial *> m_pMaterial;

    size_t m_nVertexQuantity = 4;
    size_t m_nTriangleQuantity = 2;

    FRECT m_BasePos{};
    FRECT m_BaseUV{};
    uint32_t m_dwColor{};

    std::vector<FPOINT> m_aRelPos;
    BIImageType m_eType{};

    long priority_ = ImagePrioritet_DefaultValue;
};
