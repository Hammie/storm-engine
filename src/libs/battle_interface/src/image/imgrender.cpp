#include "imgrender.h"
#include "storm_assert.h"

#include "image.h"

BIImageRender::BIImageRender(gsl::not_null<VDX9RENDER *> pRS)
    : m_pRS(pRS), m_nBeginOutputPrioritet(0), m_nEndOutputPrioritet(0)
{
}

BIImageRender::~BIImageRender() noexcept
{
    // destructors themselves erase pointers from vectors

    while (!m_apMaterial.empty())
        delete m_apMaterial.front();
}

void BIImageRender::Render()
{
    if (GetFirstPrioritetRange())
        do
        {
            for (auto &n : m_apMaterial)
            {
                n->Render(m_nBeginOutputPrioritet, m_nEndOutputPrioritet);
            }
        } while (GetNextPrioritetRange());
}

gsl::not_null<IBIImage *> BIImageRender::CreateImage(BIImageType type, const char *pcTextureName, uint32_t color,
                                                     const FRECT &uv, long nLeft, long nTop, long nRight, long nBottom,
                                                     long nPrior, const char *pcTechniqueName)
{
    auto pMaterial = CreateMaterial(pcTextureName, pcTechniqueName);
    return pMaterial->CreateImage(type, color, uv, nLeft, nTop, nRight, nBottom, nPrior);
}

gsl::not_null<IBIImage *> BIImageRender::CreateImage(BIImageType type, const char *pcTextureName, uint32_t color,
                                                     const FRECT &uv, const RECT &pos, long nPrior,
                                                     const char *pcTechniqueName)
{
    auto pMaterial = CreateMaterial(pcTextureName, pcTechniqueName);
    return pMaterial->CreateImage(type, color, uv, pos.left, pos.top, pos.right, pos.bottom, nPrior);
}

BIImageMaterial *BIImageRender::FindMaterial(const char *pcTextureName, const char *pcTechniqueName)
{
    for (auto &n : m_apMaterial)
        if (n->IsUseTexture(pcTextureName) && n->IsUseTechnique(pcTechniqueName))
            return n;
    return nullptr;
}

gsl::not_null<BIImageMaterial *> BIImageRender::CreateMaterial(const char *pcTextureName, const char *pcTechniqueName)
{
    auto *pMaterial = FindMaterial(pcTextureName, pcTechniqueName ? pcTechniqueName : "battle_tex_col_Rectangle");
    if (!pMaterial)
    {
        pMaterial = new BIImageMaterial(m_pRS, this);
        Assert(pMaterial);
        pMaterial->SetTexture(pcTextureName);
        pMaterial->SetTechnique(pcTechniqueName);
        pMaterial->UpdateFlagOn();
        long n;
        for (n = 0; n < m_apMaterial.size(); n++)
            if (m_apMaterial[n]->GetMinPriority() > pMaterial->GetMinPriority())
                break;
        m_apMaterial.insert(m_apMaterial.begin() + n, pMaterial);
        // m_apMaterial.Insert(n);
        // m_apMaterial[n] = pMaterial;
    }
    return pMaterial;
}

void BIImageRender::DeleteMaterial(BIImageMaterial *pMat)
{
    // long n = m_apMaterial.Find( pMat );
    // if( n >= 0 )
    //    m_apMaterial.DelIndex( n );
    const auto it = std::find(m_apMaterial.begin(), m_apMaterial.end(), pMat);
    if (it != m_apMaterial.end())
        m_apMaterial.erase(it);
}

void BIImageRender::ReleaseAllImages()
{
    for (auto &n : m_apMaterial)
        n->ReleaseAllImages();
}

size_t BIImageRender::GetImageQuantity()
{
    size_t nRetVal = 0;
    for (auto &n : m_apMaterial)
        nRetVal += n->GetImageQuantity();
    return nRetVal;
}

void BIImageRender::MaterialSorting()
{
    for (auto bContinue = true; bContinue;)
    {
        bContinue = false;
        for (long n = 1; n < m_apMaterial.size(); n++)
        {
            if (m_apMaterial[n]->GetMinPriority() < m_apMaterial[n - 1]->GetMinPriority())
            {
                std::swap(m_apMaterial[n - 1], m_apMaterial[n]);
                // m_apMaterial.Swap(n-1, n);
                bContinue = true;
            }
        }
    }
}

bool BIImageRender::GetFirstPrioritetRange()
{
    if (m_apMaterial.empty())
        return false;
    m_nBeginOutputPrioritet = m_apMaterial[0]->GetMinPriority();
    m_nEndOutputPrioritet = m_apMaterial[0]->GetMaxPriority();

    if (m_apMaterial.size() > 1 && m_nEndOutputPrioritet > m_apMaterial[1]->GetMinPriority())
        m_nEndOutputPrioritet = m_apMaterial[1]->GetMinPriority();
    return true;
}

bool BIImageRender::GetNextPrioritetRange()
{
    m_nBeginOutputPrioritet = ++m_nEndOutputPrioritet;
    long n;
    for (n = 0; n < m_apMaterial.size(); n++)
    {
        if (m_apMaterial[n]->GetMaxPriority() >= m_nBeginOutputPrioritet)
        {
            m_nEndOutputPrioritet = m_apMaterial[n]->GetMaxPriority();
            for (long i = n + 1; i < m_apMaterial.size(); i++)
                if (m_nBeginOutputPrioritet < m_apMaterial[i]->GetMinPriority())
                    m_nEndOutputPrioritet = m_apMaterial[i]->GetMinPriority();
            break;
        }
    }
    return (n < m_apMaterial.size());
}
