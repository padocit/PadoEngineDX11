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
               -pitch); // m_pitch�� ����̸� ���� ��� ����
}

Vector3 Camera::GetEyePos()
{
    return position;
}

void Camera::UpdateViewDir()
{
    // �̵��� �� ������ �Ǵ� ����/������ ���� ���
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
        // �󸶳� ȸ������ ���
        yaw = mouseNdcX * DirectX::XM_2PI;       // �¿� 360��
        pitch = -mouseNdcY * DirectX::XM_PIDIV2; // �� �Ʒ� 90��
        UpdateViewDir();
    }
}

void Camera::MoveForward(float dt)
{
    // �̵�����_��ġ = ����_��ġ + �̵����� * �ӵ� * �ð�����;
    position += viewDir * speed * dt;
}

void Camera::MoveUp(float dt)
{
    // �̵�����_��ġ = ����_��ġ + �̵����� * �ӵ� * �ð�����;
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
