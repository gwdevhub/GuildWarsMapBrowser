#pragma once
#include "InputManager.h"
#include "Camera.h"
#include "MeshManager.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "PerFrameCB.h"
#include "PerCameraCB.h"

using namespace DirectX;

class MapRenderer
{
public:
    MapRenderer(ID3D11Device* device, ID3D11DeviceContext* deviceContext, InputManager* input_manager)
        : m_device(device)
        , m_deviceContext(deviceContext)
        , m_input_manager(input_manager)
    {
        m_mesh_manager = std::make_unique<MeshManager>(m_device, m_deviceContext);
        m_user_camera = std::make_unique<Camera>();
    }

    void Initialize(const float viewport_width, const float viewport_height)
    {
        // Initialize cameras
        float fov_degrees = 80.0f;
        m_user_camera->Initialize(XMFLOAT3(1.0f, 1000.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),
                                  XMFLOAT3(0.0f, 1.0f, 0.0f), fov_degrees * (XM_PI / 180.0f),
                                  viewport_width / viewport_height, 0.1, 10000);

        m_input_manager->AddMouseMoveListener(m_user_camera.get());

        // Add a sphere at (0,0,0) in world coordinates. For testing the renderer.
        m_mesh_manager->AddBox({200, 200, 200});

        // Create and initialize the VertexShader
        m_vertex_shader = std::make_unique<VertexShader>(m_device, m_deviceContext);
        m_vertex_shader->Initialize(L"VertexShader.hlsl");

        // Create and initialize the PixelShader
        m_pixel_shader = std::make_unique<PixelShader>(m_device, m_deviceContext);
        m_pixel_shader->Initialize(L"PixelShader.hlsl");

        // Set up the constant buffer for the camera
        D3D11_BUFFER_DESC buffer_desc = {};
        buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
        buffer_desc.ByteWidth = sizeof(PerFrameCB);
        buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        m_device->CreateBuffer(&buffer_desc, nullptr, m_per_frame_cb.GetAddressOf());
        buffer_desc.ByteWidth = sizeof(PerCameraCB);
        m_device->CreateBuffer(&buffer_desc, nullptr, m_per_camera_cb.GetAddressOf());

        m_deviceContext->VSSetConstantBuffers(PER_FRAME_CB_SLOT, 1, m_per_frame_cb.GetAddressOf());
        m_deviceContext->VSSetConstantBuffers(PER_CAMERA_CB_SLOT, 1, m_per_camera_cb.GetAddressOf());

        m_deviceContext->VSSetShader(m_vertex_shader->GetShader(), nullptr, 0);
        m_deviceContext->IASetInputLayout(m_vertex_shader->GetInputLayout());

        // Set the pixel shader and sampler state
        m_deviceContext->PSSetShader(m_pixel_shader->GetShader(), nullptr, 0);
        const auto sampler_state = m_pixel_shader->GetSamplerState();
        m_deviceContext->PSSetSamplers(0, 1, &sampler_state);
    }

    void SetTerrain(const Mesh& terrain_mesh)
    {
        if (m_is_terrain_mesh_set)
        {
            m_mesh_manager->RemoveMesh(m_terrain_mesh_id);
            m_is_terrain_mesh_set = false;
        }

        m_terrain_mesh_id = m_mesh_manager->AddCustomMesh(terrain_mesh);
        m_is_terrain_mesh_set = true;
    }

    void UnsetTerrain()
    {
        if (m_is_terrain_mesh_set)
        {
            m_mesh_manager->RemoveMesh(m_terrain_mesh_id);
            m_is_terrain_mesh_set = false;
        }
    }

    void OnViewPortChanged(const float viewport_width, const float viewport_height)
    {
        m_user_camera->OnViewPortChanged(viewport_width, viewport_height);
    }

    void CreateRasterizerStates()
    {
        // Setup various rasterizer states
        D3D11_RASTERIZER_DESC rsDesc;
        ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
        rsDesc.FillMode = D3D11_FILL_SOLID;
        rsDesc.CullMode = D3D11_CULL_NONE;
        rsDesc.FrontCounterClockwise = false;
        rsDesc.DepthClipEnable = true;

        auto hr = m_device->CreateRasterizerState(&rsDesc, m_solid_no_cull_rs.GetAddressOf());
        if (FAILED(hr))
            throw "Failed creating solid frame rasterizer state.";
        rsDesc.FillMode = D3D11_FILL_WIREFRAME;
        hr = m_device->CreateRasterizerState(&rsDesc, m_wireframe_no_cull_rs.GetAddressOf());
        if (FAILED(hr))
            throw "Failed creating solid frame rasterizer state.";
        rsDesc.CullMode = D3D11_CULL_BACK;
        hr = m_device->CreateRasterizerState(&rsDesc, m_wireframe_rs.GetAddressOf());
        if (FAILED(hr))
            throw "Failed creating solid frame rasterizer state.";
        rsDesc.FillMode = D3D11_FILL_SOLID;
        hr = m_device->CreateRasterizerState(&rsDesc, m_solid_rs.GetAddressOf());
        if (FAILED(hr))
            throw "Failed creating solid frame rasterizer state.";
    }

    void Update(const float dt)
    {
        const bool is_w_down = m_input_manager->IsKeyDown('W');
        const bool is_a_down = m_input_manager->IsKeyDown('A');
        const bool is_s_down = m_input_manager->IsKeyDown('S');
        const bool is_d_down = m_input_manager->IsKeyDown('D');
        m_user_camera->Update(dt, is_a_down, is_w_down, is_s_down, is_d_down);

        // Override for testing purposes
        const auto pos = XMFLOAT3{0, 2000, 0};
        const auto target = XMFLOAT3{1, 0, 1};
        m_user_camera->LookAt(pos, target);

        DirectionalLight m_directionalLight;
        m_directionalLight.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        m_directionalLight.diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        m_directionalLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
        m_directionalLight.direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
        m_directionalLight.pad = 0.0f;

        // Update per frame CB
        static auto frameCB = PerFrameCB();
        frameCB.directionalLight = m_directionalLight;

        // Update the per frame constant buffer
        D3D11_MAPPED_SUBRESOURCE mappedResourceFrame;
        ZeroMemory(&mappedResourceFrame, sizeof(D3D11_MAPPED_SUBRESOURCE));
        m_deviceContext->Map(m_per_frame_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceFrame);
        memcpy(mappedResourceFrame.pData, &frameCB, sizeof(PerFrameCB));
        m_deviceContext->Unmap(m_per_frame_cb.Get(), 0);

        // Update camera CB
        static auto cameraCB = PerCameraCB();
        XMStoreFloat4x4(&cameraCB.view, XMMatrixTranspose(m_user_camera->GetViewMatrix()));
        XMStoreFloat4x4(&cameraCB.projection, XMMatrixTranspose(m_user_camera->GetProjectionMatrix()));
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));
        m_deviceContext->Map(m_per_camera_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        memcpy(mappedResource.pData, &cameraCB, sizeof(PerCameraCB));
        m_deviceContext->Unmap(m_per_camera_cb.Get(), 0);

        m_mesh_manager->Update(dt);
    }

    void Render()
    {
        m_deviceContext->RSSetState(m_solid_no_cull_rs.Get());
        m_mesh_manager->Render();
    }

private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    InputManager* m_input_manager;
    std::unique_ptr<MeshManager> m_mesh_manager;
    std::unique_ptr<Camera> m_user_camera;
    std::unique_ptr<VertexShader> m_vertex_shader;
    std::unique_ptr<PixelShader> m_pixel_shader;

    Microsoft::WRL::ComPtr<ID3D11Buffer> m_per_frame_cb;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_per_camera_cb;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireframe_rs;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireframe_no_cull_rs;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_solid_rs;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_solid_no_cull_rs;

    bool m_is_terrain_mesh_set = false;
    int m_terrain_mesh_id = -1;
};