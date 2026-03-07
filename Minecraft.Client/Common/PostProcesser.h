#pragma once

#include <d3d11.h>

class PostProcesser
{
public:
    static PostProcesser& GetInstance()
    {
        static PostProcesser instance;
        return instance;
    }

    void Init();
    void Apply() const;
    void SetViewport(const D3D11_VIEWPORT& viewport);
    void ResetViewport();
    void CopyBackbuffer(); // Copy backbuffer once before multi-pass gamma
    void ApplyFromCopied() const; // Apply gamma using already-copied offscreen texture
    void Cleanup();
    void SetGamma(float gamma);
    float GetGamma() const { return m_gamma; }
    PostProcesser(const PostProcesser&) = delete;
    PostProcesser& operator=(const PostProcesser&) = delete;

private:
    PostProcesser();
    ~PostProcesser();

    static bool IsRunningUnderWine();

    ID3D11Texture2D* m_pGammaOffscreenTex = nullptr;
    ID3D11ShaderResourceView* m_pGammaOffscreenSRV = nullptr;
    ID3D11RenderTargetView* m_pGammaOffscreenRTV = nullptr;
    ID3D11VertexShader* m_pGammaVS = nullptr;
    ID3D11PixelShader* m_pGammaPS = nullptr;
    ID3D11Buffer* m_pGammaCB = nullptr;
    ID3D11SamplerState* m_pGammaSampler = nullptr;
    ID3D11RasterizerState* m_pGammaRastState = nullptr;
    ID3D11DepthStencilState* m_pGammaDepthState = nullptr;
    ID3D11BlendState* m_pGammaBlendState = nullptr;

    bool m_initialized = false;
    float m_gamma = 1.0f;
    bool m_wineMode = false;
    D3D11_VIEWPORT m_customViewport;
    bool m_useCustomViewport = false;

    struct GammaCBData
    {
        float gamma;
        float pad[3];
    };

    static const char* g_gammaVSCode;
    static const char* g_gammaPSCode;
};