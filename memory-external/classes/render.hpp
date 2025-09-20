#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <cmath>
#include <wrl/client.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")

using namespace DirectX;
using Microsoft::WRL::ComPtr;

IDXGISwapChain* gSwapChain = nullptr;
ID3D11Device* gDevice = nullptr;
HRESULT hr;

ID3D11DeviceContext* gContext = nullptr;
ID2D1Factory* gD2DFactory = nullptr;
IDWriteFactory* gDWriteFactory = nullptr;
ID2D1RenderTarget* gD2DRenderTarget = nullptr;
IDWriteTextFormat* gTextFormat = nullptr;
ID2D1SolidColorBrush* gBrush = nullptr;

namespace render
{
    struct Color
    {
        float r, g, b, a;

        Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
            : r(red), g(green), b(blue), a(alpha) {
        }

        Color(COLORREF color, float alpha = 1.0f)
        {
            r = static_cast<float>(GetRValue(color)) / 255.0f;
            g = static_cast<float>(GetGValue(color)) / 255.0f;
            b = static_cast<float>(GetBValue(color)) / 255.0f;
            a = alpha;
        }
    };


    struct Vertex
    {
        XMFLOAT3 pos;
        XMFLOAT4 color;
    };

    class DX11Renderer
    {
    public:
        ComPtr<ID3D11Device> m_device;

    private:
        ComPtr<ID3D11DeviceContext> m_context;
        ComPtr<IDXGISwapChain> m_swapChain;
        ComPtr<ID3D11RenderTargetView> m_renderTargetView;
        ComPtr<ID3D11VertexShader> m_vertexShader;
        ComPtr<ID3D11PixelShader> m_pixelShader;
        ComPtr<ID3D11InputLayout> m_inputLayout;
        ComPtr<ID3D11Buffer> m_vertexBuffer;
        ComPtr<ID3D11Buffer> m_constantBuffer;
        ComPtr<ID3D11RasterizerState> m_rasterizerState;
        ComPtr<ID3D11BlendState> m_blendState;


        struct ConstantBuffer
        {
            XMMATRIX worldViewProjection;
        };

        int m_screenWidth;
        int m_screenHeight;
        std::vector<Vertex> m_vertices;
        std::vector<Vertex> m_triangleVertices;

        bool InitializeShaders();
        void CreateProjectionMatrix(XMMATRIX& matrix);

    public:
        DX11Renderer();
        ~DX11Renderer();

        bool Initialize(HWND hwnd, int width, int height);
        void InitTextRenderer();
        void Shutdown();
        void RenderText(float x, float y, const char* text, COLORREF color, int size);
        void BeginFrame();
        void EndFrame();
        void Present();

        void DrawLine(int x1, int y1, int x2, int y2, const Color& color);
        void DrawCircle(int x, int y, int radius, const Color& color, int segments = 32);
        void DrawBorderBox(int x, int y, int w, int h, const Color& borderColor);
        void DrawFilledBox(int x, int y, int width, int height, const Color& color);

    private:
        void FlushLines();
        void FlushTriangles();
        void AddLineVertex(float x, float y, const Color& color);
        void AddTriangleVertex(float x, float y, const Color& color);
        XMFLOAT2 ScreenToNDC(int x, int y);
    };

    DX11Renderer* g_renderer = nullptr;

    DX11Renderer::DX11Renderer()
        : m_screenWidth(0), m_screenHeight(0)
    {
    }

    DX11Renderer::~DX11Renderer()
    {
        Shutdown();
    }

    bool DX11Renderer::Initialize(HWND hwnd, int width, int height)
    {
        m_screenWidth = width;
        m_screenHeight = height;

        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = 1;
        swapChainDesc.BufferDesc.Width = width;
        swapChainDesc.BufferDesc.Height = height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = hwnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = TRUE;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

        D3D_FEATURE_LEVEL featureLevel;
        UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            deviceFlags,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &swapChainDesc,
            m_swapChain.GetAddressOf(),
            m_device.GetAddressOf(),
            &featureLevel,
            m_context.GetAddressOf());
        if (FAILED(hr))
            return false;

        ComPtr<ID3D11Texture2D> backBufferTex;
        hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBufferTex.GetAddressOf()));
        if (FAILED(hr)) return false;

        hr = m_device->CreateRenderTargetView(backBufferTex.Get(), nullptr, m_renderTargetView.GetAddressOf());
        if (FAILED(hr)) return false;

        D3D11_VIEWPORT viewport = {};
        viewport.Width = static_cast<float>(width);
        viewport.Height = static_cast<float>(height);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        m_context->RSSetViewports(1, &viewport);

        if (!InitializeShaders()) return false;

        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.ByteWidth = sizeof(Vertex) * 10000;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_vertexBuffer.GetAddressOf());
        if (FAILED(hr)) return false;

        bufferDesc = {};
        bufferDesc.Usage = D3D11_USAGE_DEFAULT;
        bufferDesc.ByteWidth = sizeof(ConstantBuffer);
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        hr = m_device->CreateBuffer(&bufferDesc, nullptr, m_constantBuffer.GetAddressOf());
        if (FAILED(hr)) return false;

        D3D11_RASTERIZER_DESC rastDesc = {};
        rastDesc.FillMode = D3D11_FILL_SOLID;
        rastDesc.CullMode = D3D11_CULL_NONE;
        rastDesc.FrontCounterClockwise = false;
        rastDesc.AntialiasedLineEnable = true;
        hr = m_device->CreateRasterizerState(&rastDesc, m_rasterizerState.GetAddressOf());
        if (FAILED(hr)) return false;

        D3D11_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        hr = m_device->CreateBlendState(&blendDesc, m_blendState.GetAddressOf());
        if (FAILED(hr)) return false;

        InitTextRenderer();

        return true;
    }

    void DX11Renderer::InitTextRenderer()
    {
        // ensure COM initialized on this thread before calling this function
        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &gD2DFactory);
        if (FAILED(hr)) return;

        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(&gDWriteFactory));
        if (FAILED(hr)) return;

        ComPtr<IDXGISurface> dxgiBackBuffer;
        if (m_swapChain)
        {
            hr = m_swapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(dxgiBackBuffer.GetAddressOf()));
            if (FAILED(hr)) return;
        }

        D2D1_RENDER_TARGET_PROPERTIES rtProps =
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
                96.0f, 96.0f);

        if (dxgiBackBuffer)
        {
            hr = gD2DFactory->CreateDxgiSurfaceRenderTarget(dxgiBackBuffer.Get(), &rtProps, &gD2DRenderTarget);
            if (FAILED(hr)) return;
        }

        if (gD2DRenderTarget)
        {
            hr = gD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &gBrush);
            if (FAILED(hr)) {
                std::cerr << "brush creation failure, contact @hydrophobis on github or discord";
            }
            gD2DRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);
        }
    }



    bool DX11Renderer::InitializeShaders()
    {
        // Vertex shader source
        const char* vsSource = R"(
            cbuffer ConstantBuffer : register(b0)
            {
                matrix worldViewProjection;
            }

            struct VS_INPUT
            {
                float3 pos : POSITION;
                float4 color : COLOR;
            };

            struct PS_INPUT
            {
                float4 pos : SV_POSITION;
                float4 color : COLOR;
            };

            PS_INPUT main(VS_INPUT input)
            {
                PS_INPUT output;
                output.pos = mul(float4(input.pos, 1.0f), worldViewProjection);
                output.color = input.color;
                return output;
            }
        )";

        // Pixel shader source
        const char* psSource = R"(
            struct PS_INPUT
            {
                float4 pos : SV_POSITION;
                float4 color : COLOR;
            };

            float4 main(PS_INPUT input) : SV_Target
            {
                return input.color;
            }
        )";

        // Compile and create vertex shader
        ComPtr<ID3DBlob> vsBlob, psBlob;
        HRESULT hr = D3DCompile(vsSource, strlen(vsSource), nullptr, nullptr, nullptr,
            "main", "vs_5_0", 0, 0, &vsBlob, nullptr);
        if (FAILED(hr)) return false;

        hr = m_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            nullptr, &m_vertexShader);
        if (FAILED(hr)) return false;

        // Compile and create pixel shader
        hr = D3DCompile(psSource, strlen(psSource), nullptr, nullptr, nullptr,
            "main", "ps_5_0", 0, 0, &psBlob, nullptr);
        if (FAILED(hr)) return false;

        hr = m_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
            nullptr, &m_pixelShader);
        if (FAILED(hr)) return false;

        // Create input layout
        D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
        };

        hr = m_device->CreateInputLayout(inputElementDesc, 2,
            vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            &m_inputLayout);
        return SUCCEEDED(hr);
    }

    void DX11Renderer::BeginFrame()
    {
        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // Transparent black
        m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
        m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);

        // Set up pipeline state
        m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
        m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
        m_context->IASetInputLayout(m_inputLayout.Get());
        m_context->RSSetState(m_rasterizerState.Get());
        m_context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);

        // Update constant buffer with projection matrix
        ConstantBuffer cb;
        CreateProjectionMatrix(cb.worldViewProjection);
        m_context->UpdateSubresource(m_constantBuffer.Get(), 0, nullptr, &cb, 0, 0);
        m_context->VSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());

        m_vertices.clear();
        m_triangleVertices.clear();
    }

    void DX11Renderer::EndFrame()
    {
        FlushLines();
        FlushTriangles();
    }

    void DX11Renderer::Present()
    {
        m_swapChain->Present(1, 0);
    }

    void DX11Renderer::DrawLine(int x1, int y1, int x2, int y2, const Color& color)
    {
        AddLineVertex(static_cast<float>(x1), static_cast<float>(y1), color);
        AddLineVertex(static_cast<float>(x2), static_cast<float>(y2), color);
    }

    void DX11Renderer::DrawCircle(int x, int y, int radius, const Color& color, int segments)
    {
        float angleStep = 2.0f * XM_PI / segments;

        for (int i = 0; i < segments; ++i)
        {
            float angle1 = i * angleStep;
            float angle2 = (i + 1) * angleStep;

            float x1 = x + radius * cosf(angle1);
            float y1 = y + radius * sinf(angle1);
            float x2 = x + radius * cosf(angle2);
            float y2 = y + radius * sinf(angle2);

            AddLineVertex(x1, y1, color);
            AddLineVertex(x2, y2, color);
        }
    }

    void DX11Renderer::DrawBorderBox(int x, int y, int w, int h, const Color& borderColor)
    {
        // Top edge
        DrawLine(x, y, x + w, y, borderColor);
        // Right edge
        DrawLine(x + w, y, x + w, y + h, borderColor);
        // Bottom edge
        DrawLine(x + w, y + h, x, y + h, borderColor);
        // Left edge
        DrawLine(x, y + h, x, y, borderColor);
    }

    void DX11Renderer::DrawFilledBox(int x, int y, int width, int height, const Color& color)
    {
        // Create two triangles to form a rectangle
        // Triangle 1
        AddTriangleVertex(static_cast<float>(x), static_cast<float>(y), color);
        AddTriangleVertex(static_cast<float>(x + width), static_cast<float>(y), color);
        AddTriangleVertex(static_cast<float>(x), static_cast<float>(y + height), color);

        // Triangle 2
        AddTriangleVertex(static_cast<float>(x + width), static_cast<float>(y), color);
        AddTriangleVertex(static_cast<float>(x + width), static_cast<float>(y + height), color);
        AddTriangleVertex(static_cast<float>(x), static_cast<float>(y + height), color);
    }

    void DX11Renderer::FlushLines()
    {
        if (m_vertices.empty()) return;

        // Map vertex buffer
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr)) return;

        // Copy vertices
        memcpy(mappedResource.pData, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
        m_context->Unmap(m_vertexBuffer.Get(), 0);

        // Set vertex buffer and draw lines
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_context->Draw(static_cast<UINT>(m_vertices.size()), 0);
    }

    void DX11Renderer::FlushTriangles()
    {
        if (m_triangleVertices.empty()) return;

        // Map vertex buffer
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT hr = m_context->Map(m_vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(hr)) return;

        // Copy vertices
        memcpy(mappedResource.pData, m_triangleVertices.data(), m_triangleVertices.size() * sizeof(Vertex));
        m_context->Unmap(m_vertexBuffer.Get(), 0);

        // Set vertex buffer and draw triangles
        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_context->Draw(static_cast<UINT>(m_triangleVertices.size()), 0);
    }


    void DX11Renderer::AddLineVertex(float x, float y, const Color& color)
    {
        XMFLOAT2 ndc = ScreenToNDC(static_cast<int>(x), static_cast<int>(y));
        m_vertices.push_back({ {ndc.x, ndc.y, 0.0f}, {color.r, color.g, color.b, color.a} });
    }

    void DX11Renderer::AddTriangleVertex(float x, float y, const Color& color)
    {
        XMFLOAT2 ndc = ScreenToNDC(static_cast<int>(x), static_cast<int>(y));
        m_triangleVertices.push_back({ {ndc.x, ndc.y, 0.0f}, {color.r, color.g, color.b, color.a} });
    }

    XMFLOAT2 DX11Renderer::ScreenToNDC(int x, int y)
    {
        float ndcX = (2.0f * x) / m_screenWidth - 1.0f;
        float ndcY = 1.0f - (2.0f * y) / m_screenHeight;
        return XMFLOAT2(ndcX, ndcY);
    }

    void DX11Renderer::CreateProjectionMatrix(XMMATRIX& matrix)
    {
        matrix = XMMatrixIdentity();
    }

    void DX11Renderer::Shutdown()
    {
    }

    inline void DrawLine(int x1, int y1, int x2, int y2, COLORREF color)
    {
        if (g_renderer)
            g_renderer->DrawLine(x1, y1, x2, y2, Color(color, 1.0f));
    }

    inline void DrawCircle(int x, int y, int radius, COLORREF color)
    {
        if (g_renderer)
            g_renderer->DrawCircle(x, y, radius, Color(color, 1.0f));
    }

    inline void DrawBorderBox(int x, int y, int w, int h, COLORREF borderColor)
    {
        if (g_renderer)
            g_renderer->DrawBorderBox(x, y, w, h, Color(borderColor, 1.0f));
    }

    inline void DrawFilledBox(int x, int y, int width, int height, COLORREF color)
    {
        if (g_renderer)
            g_renderer->DrawFilledBox(x, y, width, height, Color(color, 1.0f));
    }

    void DX11Renderer::RenderText(float x, float y, const char* text, COLORREF color, int size)
    {
        if (!gD2DRenderTarget || !gBrush || !text) return;

        int wlen = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
        if (wlen <= 1) return;
        std::wstring wtext(wlen - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text, -1, &wtext[0], wlen);

        Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat;
        gDWriteFactory->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            10.0f, // font size in DIPs
            L"en-us",
            &textFormat
        );

        m_context->Flush();
        gD2DRenderTarget->BeginDraw();

        float rf = GetRValue(color) / 255.0f;
        float gf = GetGValue(color) / 255.0f;
        float bf = GetBValue(color) / 255.0f;
        gBrush->SetColor(D2D1::ColorF(rf, gf, bf, 1.0f));

        D2D1_RECT_F layoutRect = D2D1::RectF(x, y, x + 1000.0f, y + 200.0f);
        gD2DRenderTarget->DrawText(
            wtext.c_str(),
            static_cast<UINT32>(wtext.length()),
            textFormat.Get(),
            &layoutRect,
            gBrush
        );

        hr = gD2DRenderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            std::cerr << "d2d target lost, please recreate resources\n";
        }
    }

}