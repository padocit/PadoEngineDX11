#include "Camera.h"

#include <iostream>

using namespace std;
using namespace DirectX;

Camera::Camera()
{
    UpdateViewDir();
}

Matrix Camera::GetViewRow()
{
    return Matrix::CreateTranslation(-position) *
           Matrix::CreateRotationY(-yaw) *
           Matrix::CreateRotationX(
               -pitch); // m_pitch가 양수이면 고개를 드는 방향
}

Vector3 Camera::GetEyePos()
{
    return position;
}

void Camera::UpdateViewDir()
{
    // 이동할 때 기준이 되는 정면/오른쪽 방향 계산
    viewDir = Vector3::Transform(Vector3(0.0f, 0.0f, 1.0f),
                                   Matrix::CreateRotationY(this->yaw));
    rightDir = upDir.Cross(viewDir);
}

void Camera::UpdateKeyboard(const float dt, bool const keyPressed[256])
{
    if (useFirstPersonView)
    {
        if (keyPressed['W'])
            MoveForward(dt);
        if (keyPressed['S'])
            MoveForward(-dt);
        if (keyPressed['D'])
            MoveRight(dt);
        if (keyPressed['A'])
            MoveRight(-dt);
        if (keyPressed['E'])
            MoveUp(dt);
        if (keyPressed['Q'])
            MoveUp(-dt);
    }
}

void Camera::UpdateMouse(float mouseNdcX, float mouseNdcY)
{
    if (useFirstPersonView)
    {
        // 얼마나 회전할지 계산
        yaw = mouseNdcX * DirectX::XM_2PI;       // 좌우 360도
        pitch = -mouseNdcY * DirectX::XM_PIDIV2; // 위 아래 90도
        UpdateViewDir();
    }
}

void Camera::MoveForward(float dt)
{
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    position += viewDir * speed * dt;
}

void Camera::MoveUp(float dt)
{
    // 이동후의_위치 = 현재_위치 + 이동방향 * 속도 * 시간차이;
    position += upDir * speed * dt;
}

void Camera::MoveRight(float dt)
{
    position += rightDir * speed * dt;
}

void Camera::SetAspectRatio(float Aspect)
{
    aspect = Aspect;
}

void Camera::PrintView()
{
    cout << "Current view settings:" << endl;
    cout << "Vector3 m_position = Vector3(" << position.x << "f, "
         << position.y << "f, " << position.z << "f);" << endl;
    cout << "float yaw = " << yaw << "f, pitch = " << pitch << "f;"
         << endl;

    cout << "AppBase::m_camera.Reset(Vector3(" << position.x << "f, "
         << position.y << "f, " << position.z << "f), " << yaw << "f, "
         << pitch << "f);" << endl;
}

Matrix Camera::GetProjRow()
{
    return usePerspectiveProjection
               ? XMMatrixPerspectiveFovLH(XMConvertToRadians(projFovAngleY),
                                          aspect, nearZ, farZ)
               : XMMatrixOrthographicOffCenterLH(-aspect, aspect, -1.0f,
                                                 1.0f, nearZ, farZ);
}
