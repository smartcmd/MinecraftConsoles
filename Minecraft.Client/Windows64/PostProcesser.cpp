#include "stdafx.h"
#include "..\Common\PostProcesser.h"
#include <d3dcompiler.h>
#include <vector>

#pragma comment(lib, "d3dcompiler.lib")

extern ID3D11Device* g_pd3dDevice;
extern ID3D11DeviceContext* g_pImmediateContext;
extern IDXGISwapChain* g_pSwapChain;
extern ID3D11RenderTargetView* g_pRenderTargetView;

const char* PostProcesser::g_gammaVSCode =
    "void main(uint id : SV_VertexID, out float4 pos : SV_Position, out float2 uv : TEXCOORD0)\n"
    "{\n"
    "    uv = float2((id << 1) & 2, id & 2);\n"
    "    pos = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);\n"
    "}\n";

const char* PostProcesser::g_gammaPSCode =
    "cbuffer GammaCB : register(b0)\n"
    "{\n"
    "    float gamma;\n"
    "    float3 pad;\n"
    "};\n"
    "Texture2D sceneTex : register(t0);\n"
    "SamplerState sceneSampler : register(s0);\n"
    "float4 main(float4 pos : SV_Position, float2 uv : TEXCOORD0) : SV_Target\n"
    "{\n"
    "    float4 color = sceneTex.Sample(sceneSampler, uv);\n"
    "\n"
    "    color.rgb = max(color.rgb, 0.0);\n"
    "\n"
    "    color.rgb = pow(color.rgb, 1.0 / gamma);\n"
    "\n"
    "    return color;\n"
    "}\n";

PostProcesser::PostProcesser() = default;

PostProcesser::~PostProcesser()
{
    Cleanup();
}

bool PostProcesser::IsRunningUnderWine()
{
    const HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll)
    {
        if (GetProcAddress(ntdll, "wine_get_version"))
            return true;
    }
    return false;
}

void PostProcesser::SetGamma(float gamma)
{
    m_gamma = gamma;
}

void PostProcesser::Init()
{
    if (!g_pd3dDevice || !g_pSwapChain)
        return;

    if (m_initialized)
        return;

    m_wineMode = IsRunningUnderWine();

    HRESULT hr;
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));
    if (FAILED(hr))
        return;

    D3D11_TEXTURE2D_DESC bbDesc;
    pBackBuffer->GetDesc(&bbDesc);
    pBackBuffer->Release();

    m_gammaTexWidth = bbDesc.Width;
    m_gammaTexHeight = bbDesc.Height;

    DXGI_FORMAT texFormat = bbDesc.Format;
    if (m_wineMode)
    {
        texFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    }

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = bbDesc.Width;
    texDesc.Height = bbDesc.Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = texFormat;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    hr = g_pd3dDevice->CreateTexture2D(&texDesc, nullptr, &m_pGammaOffscreenTex);
    if (FAILED(hr))
        return;

    hr = g_pd3dDevice->CreateShaderResourceView(m_pGammaOffscreenTex, nullptr, &m_pGammaOffscreenSRV);
    if (FAILED(hr))
        return;

    hr = g_pd3dDevice->CreateRenderTargetView(m_pGammaOffscreenTex, nullptr, &m_pGammaOffscreenRTV);
    if (FAILED(hr))
        return;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* errBlob = nullptr;
    UINT compileFlags = D3DCOMPILE_ENABLE_STRICTNESS;
    if (m_wineMode)
    {
        compileFlags |= D3DCOMPILE_AVOID_FLOW_CONTROL;
    }
    hr = D3DCompile(g_gammaVSCode, strlen(g_gammaVSCode), "GammaVS", nullptr, nullptr, "main", "vs_4_0", compileFlags, 0, &vsBlob, &errBlob);
    if (FAILED(hr))
    {
        if (errBlob)
            errBlob->Release();
        return;
    }

    hr = g_pd3dDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_pGammaVS);
    vsBlob->Release();
    if (errBlob)
        errBlob->Release();
    if (FAILED(hr))
        return;

    errBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    hr = D3DCompile(g_gammaPSCode, strlen(g_gammaPSCode), "GammaPS", nullptr, nullptr, "main", "ps_4_0", compileFlags, 0, &psBlob, &errBlob);
    if (FAILED(hr))
    {
        if (errBlob)
            errBlob->Release();
        return;
    }

    hr = g_pd3dDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pGammaPS);
    psBlob->Release();
    if (errBlob)
        errBlob->Release();
    if (FAILED(hr))
        return;

    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory(&cbDesc, sizeof(cbDesc));
    cbDesc.ByteWidth = sizeof(GammaCBData);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    GammaCBData initData = {1.0f, {0, 0, 0}};
    D3D11_SUBRESOURCE_DATA srData;
    srData.pSysMem = &initData;
    srData.SysMemPitch = 0;
    srData.SysMemSlicePitch = 0;

    hr = g_pd3dDevice->CreateBuffer(&cbDesc, &srData, &m_pGammaCB);
    if (FAILED(hr))
        return;

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    hr = g_pd3dDevice->CreateSamplerState(&sampDesc, &m_pGammaSampler);
    if (FAILED(hr))
        return;

    D3D11_RASTERIZER_DESC rasDesc;
    ZeroMemory(&rasDesc, sizeof(rasDesc));
    rasDesc.FillMode = D3D11_FILL_SOLID;
    rasDesc.CullMode = D3D11_CULL_NONE;
    rasDesc.DepthClipEnable = FALSE;
    rasDesc.ScissorEnable = FALSE;

    hr = g_pd3dDevice->CreateRasterizerState(&rasDesc, &m_pGammaRastState);
    if (FAILED(hr))
        return;

    D3D11_DEPTH_STENCIL_DESC dsDesc;
    ZeroMemory(&dsDesc, sizeof(dsDesc));
    dsDesc.DepthEnable = FALSE;
    dsDesc.StencilEnable = FALSE;

    hr = g_pd3dDevice->CreateDepthStencilState(&dsDesc, &m_pGammaDepthState);
    if (FAILED(hr))
        return;

    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(blendDesc));
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    hr = g_pd3dDevice->CreateBlendState(&blendDesc, &m_pGammaBlendState);
    if (FAILED(hr))
        return;

    m_initialized = true;
}

void PostProcesser::Apply() const
{
    if (!m_initialized)
        return;

    if (m_gamma > 0.99f && m_gamma < 1.01f)
        return;

    ID3D11DeviceContext *ctx = g_pImmediateContext;

    ID3D11Texture2D *pBackBuffer = nullptr;
    HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID *>(&pBackBuffer));
    if (FAILED(hr))
        return;

    D3D11_BOX srcBox = { 0, 0, 0, m_gammaTexWidth, m_gammaTexHeight, 1 };
    ctx->CopySubresourceRegion(m_pGammaOffscreenTex, 0, 0, 0, 0, pBackBuffer, 0, &srcBox);
    pBackBuffer->Release();

    D3D11_MAPPED_SUBRESOURCE mapped;
    const D3D11_MAP mapType = m_wineMode ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD;
    hr = ctx->Map(m_pGammaCB, 0, mapType, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        GammaCBData *cb = static_cast<GammaCBData *>(mapped.pData);
        cb->gamma = m_gamma;
        ctx->Unmap(m_pGammaCB, 0);
    }

    ID3D11RenderTargetView* oldRTV = nullptr;
    ID3D11DepthStencilView* oldDSV = nullptr;
    ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);

    UINT numViewports = 1;
    D3D11_VIEWPORT oldViewport = {};
    ctx->RSGetViewports(&numViewports, &oldViewport);

    ID3D11RenderTargetView* bbRTV = g_pRenderTargetView;
    ctx->OMSetRenderTargets(1, &bbRTV, nullptr);

    D3D11_VIEWPORT vp;
    if (m_useCustomViewport)
    {
        vp = m_customViewport;
    }
    else
    {
        vp.Width = static_cast<FLOAT>(m_gammaTexWidth);
        vp.Height = static_cast<FLOAT>(m_gammaTexHeight);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
    }

    ctx->RSSetViewports(1, &vp);

    ctx->IASetInputLayout(nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(m_pGammaVS, nullptr, 0);
    ctx->PSSetShader(m_pGammaPS, nullptr, 0);
    ctx->PSSetShaderResources(0, 1, &m_pGammaOffscreenSRV);
    ctx->PSSetSamplers(0, 1, &m_pGammaSampler);
    ctx->PSSetConstantBuffers(0, 1, &m_pGammaCB);
    ctx->RSSetState(m_pGammaRastState);
    ctx->OMSetDepthStencilState(m_pGammaDepthState, 0);

    constexpr float blendFactor[4] = {0, 0, 0, 0};
    ctx->OMSetBlendState(m_pGammaBlendState, blendFactor, 0xFFFFFFFF);

    ctx->Draw(3, 0);

    ID3D11ShaderResourceView* nullSrv = nullptr;
    ctx->PSSetShaderResources(0, 1, &nullSrv);

    ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
    ctx->RSSetViewports(1, &oldViewport);

    if (oldRTV)
        oldRTV->Release();
    if (oldDSV)
        oldDSV->Release();
}

void PostProcesser::SetViewport(const D3D11_VIEWPORT& viewport)
{
    m_customViewport = viewport;
    m_useCustomViewport = true;
}

void PostProcesser::ResetViewport()
{
    m_useCustomViewport = false;
}

void PostProcesser::CopyBackbuffer()
{
    if (!m_initialized)
        return;

    ID3D11DeviceContext* ctx = g_pImmediateContext;

    ID3D11Texture2D* pBackBuffer = nullptr;
    HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));
    if (FAILED(hr))
        return;

    D3D11_BOX srcBox = { 0, 0, 0, m_gammaTexWidth, m_gammaTexHeight, 1 };
    ctx->CopySubresourceRegion(m_pGammaOffscreenTex, 0, 0, 0, 0, pBackBuffer, 0, &srcBox);
    pBackBuffer->Release();
}

void PostProcesser::ApplyFromCopied() const
{
    if (!m_initialized)
        return;

    if (m_gamma > 0.99f && m_gamma < 1.01f)
        return;

    ID3D11DeviceContext* ctx = g_pImmediateContext;

    D3D11_MAPPED_SUBRESOURCE mapped;
    const D3D11_MAP mapType = m_wineMode ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD;
    const HRESULT hr = ctx->Map(m_pGammaCB, 0, mapType, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        const auto cb = static_cast<GammaCBData*>(mapped.pData);
        cb->gamma = m_gamma;
        ctx->Unmap(m_pGammaCB, 0);
    }

    ID3D11RenderTargetView* oldRTV = nullptr;
    ID3D11DepthStencilView* oldDSV = nullptr;
    ctx->OMGetRenderTargets(1, &oldRTV, &oldDSV);

    UINT numViewports = 1;
    D3D11_VIEWPORT oldViewport = {};
    ctx->RSGetViewports(&numViewports, &oldViewport);

    ID3D11RenderTargetView* bbRTV = g_pRenderTargetView;
    ctx->OMSetRenderTargets(1, &bbRTV, nullptr);

    D3D11_VIEWPORT vp;
    if (m_useCustomViewport)
    {
        vp = m_customViewport;
    }
    else
    {
        vp.Width = static_cast<FLOAT>(m_gammaTexWidth);
        vp.Height = static_cast<FLOAT>(m_gammaTexHeight);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
    }

    ctx->RSSetViewports(1, &vp);

    ctx->IASetInputLayout(nullptr);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx->VSSetShader(m_pGammaVS, nullptr, 0);
    ctx->PSSetShader(m_pGammaPS, nullptr, 0);
    ctx->PSSetShaderResources(0, 1, &m_pGammaOffscreenSRV);
    ctx->PSSetSamplers(0, 1, &m_pGammaSampler);
    ctx->PSSetConstantBuffers(0, 1, &m_pGammaCB);
    ctx->RSSetState(m_pGammaRastState);
    ctx->OMSetDepthStencilState(m_pGammaDepthState, 0);

    constexpr float blendFactor[4] = {0, 0, 0, 0};
    ctx->OMSetBlendState(m_pGammaBlendState, blendFactor, 0xFFFFFFFF);

    ctx->Draw(3, 0);

    ID3D11ShaderResourceView* nullSrv = nullptr;
    ctx->PSSetShaderResources(0, 1, &nullSrv);

    ctx->OMSetRenderTargets(1, &oldRTV, oldDSV);
    ctx->RSSetViewports(1, &oldViewport);

    if (oldRTV)
        oldRTV->Release();
    if (oldDSV)
        oldDSV->Release();
}

void PostProcesser::Cleanup()
{
    if (m_pGammaBlendState)
    {
        m_pGammaBlendState->Release();
        m_pGammaBlendState = nullptr;
    }
    if (m_pGammaDepthState)
    {
        m_pGammaDepthState->Release();
        m_pGammaDepthState = nullptr;
    }
    if (m_pGammaRastState)
    {
        m_pGammaRastState->Release();
        m_pGammaRastState = nullptr;
    }
    if (m_pGammaSampler)
    {
        m_pGammaSampler->Release();
        m_pGammaSampler = nullptr;
    }
    if (m_pGammaCB)
    {
        m_pGammaCB->Release();
        m_pGammaCB = nullptr;
    }
    if (m_pGammaPS)
    {
        m_pGammaPS->Release();
        m_pGammaPS = nullptr;
    }
    if (m_pGammaVS)
    {
        m_pGammaVS->Release();
        m_pGammaVS = nullptr;
    }
    if (m_pGammaOffscreenRTV)
    {
        m_pGammaOffscreenRTV->Release();
        m_pGammaOffscreenRTV = nullptr;
    }
    if (m_pGammaOffscreenSRV)
    {
        m_pGammaOffscreenSRV->Release();
        m_pGammaOffscreenSRV = nullptr;
    }
    if (m_pGammaOffscreenTex)
    {
        m_pGammaOffscreenTex->Release();
        m_pGammaOffscreenTex = nullptr;
    }

    m_initialized = false;
}
