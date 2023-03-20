#pragma once
#include "MouseMoveListener.h"

using namespace DirectX;

class Camera : public MouseMoveListener
{
public:
    Camera();
    ~Camera();

    void Update(float dt);

    XMVECTOR GetPosition() const;
    XMFLOAT3 GetPosition3f() const;

    void SetPosition(float x, float y, float z);
    XMVECTOR GetRight() const;
    XMFLOAT3 GetRight3f() const;

    XMVECTOR GetUp() const;
    XMFLOAT3 GetUp3f() const;

    XMVECTOR GetLook() const;
    XMFLOAT3 GetLook3f() const;

    void SetFrustumAsPerspective(float fovY, float aspect, float zn, float zf);
    void SetFrustumAsOrthographic(float view_width, float view_height, float zn, float zf);
    void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
    void Strafe(float velocity, double dt);
    void Walk(float velocity, double dt);
    void Pitch(float angle);
    void RotateWorldY(float angle);

    XMMATRIX GetView() const;
    XMFLOAT4X4 GetView4x4() const;
    XMMATRIX GetProj() const;
    XMFLOAT4X4 GetProj4x4() const;

    void OnViewPortChanged(const float viewport_width, const float viewport_height);

    virtual void OnMouseMove(float yaw_angle_radians, float pitch_angle_radians) override;

private:
    void UpdateViewMatrix();

    float m_fov;
    float m_aspectRatio;
    float m_nearZ;
    float m_farZ;

    XMFLOAT3 m_position;
    XMFLOAT3 m_right;
    XMFLOAT3 m_up;
    XMFLOAT3 m_look;

    XMFLOAT4X4 m_view;
    XMFLOAT4X4 m_proj;

    bool m_view_should_update = false;
};