#pragma once

#include "image_defines.h"
#include <gsl/gsl>
#include <string>
#include <string_view>
#include <vector>

class BIImage;
class BIImageRender;

class BIImageMaterial final
{
  public:
    BIImageMaterial(gsl::not_null<VDX9RENDER *> pRS, gsl::not_null<BIImageRender *> pImgRender);
    ~BIImageMaterial() noexcept;

    void Render(long nBegPrior, long nEndPrior);

    bool IsUseTexture(const char *pcTextureName) const
    {
        return (m_sTextureName == pcTextureName);
    }

    bool IsUseTechnique(const char *pcTechniqueName) const
    {
        return (m_sTechniqueName == pcTechniqueName);
    }

    const gsl::not_null<BIImage *> CreateImage(BIImageType type, uint32_t color, const FRECT &uv, long nLeft, long nTop,
                                               long nRight, long nBottom, long nPrior);
    void DeleteImage(const BIImage *pImg);

    void SetTexture(const char *pcTextureName);

    void SetTechnique(const char *pcTechniqueName)
    {
        if (pcTechniqueName)
            m_sTechniqueName = pcTechniqueName;
    }

    void UpdateFlagOn()
    {
        m_bMakeBufferUpdate = true;
    }

    [[nodiscard]] size_t GetImageQuantity() const noexcept
    {
        return m_apImage.size();
    }

    void ReleaseAllImages();

    [[nodiscard]] long GetMinPriority() const noexcept
    {
        return m_nMinPriority;
    }

    [[nodiscard]] long GetMaxPriority() const noexcept
    {
        return m_nMaxPriority;
    }

    [[nodiscard]] BIImageRender &GetImgRender() const noexcept
    {
        return *m_pImageRender;
    }

  private:
    void UpdateImageBuffers(long nStartIdx, size_t nEndIdx);
    void RemakeBuffers();
    bool GetOutputRangeByPriority(long nBegPrior, long nEndPrior, size_t &nStartIndex, size_t &nTriangleQuantity);
    void RecalculatePriorityRange();
    void InsertImageToList(BIImage *pImg);

    gsl::not_null<VDX9RENDER *> m_pRS;
    gsl::not_null<BIImageRender *> m_pImageRender;

    std::string m_sTextureName;
    std::string m_sTechniqueName;

    long m_nTextureID;
    long m_nVBufID;
    long m_nIBufID;
    size_t m_nVertexQuantity;
    size_t m_nTriangleQuantity;

    std::vector<BIImage *> m_apImage;

    long m_nMinPriority;
    long m_nMaxPriority;

    bool m_bMakeBufferUpdate;
    bool m_bDeleteEverything;
};
