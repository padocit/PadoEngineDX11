#pragma once

#include <directxtk/SimpleMath.h>

using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Vector3;

class Camera
{
public:
    Camera();

    Matrix GetViewRow();
    Matrix GetProjRow();
    Vector3 GetEyePos();

    void Reset(Vector3 Pos, float Yaw, float Pitch)
    {
        position = Pos;
        yaw = Yaw;
        pitch = Pitch;
        UpdateViewDir();
    }

    void UpdateViewDir();
    void UpdateKeyboard(const float dt, bool const keyPressed[256]);
    void UpdateMouse(float mouseNdcX, float mouseNdcY);
    void MoveForward(float dt);
    void MoveRight(float dt);
    void MoveUp(float dt);
    void SetAspectRatio(float Aspect);
    void PrintView();

public:
    bool useFirstPersonView = false;

private:
    Vector3 position = Vector3(0.312183f, 0.957463f, -1.88458f);
    Vector3 viewDir = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 upDir = Vector3(0.0f, 1.0f, 0.0f); // +Y 방향으로 고정
    Vector3 rightDir = Vector3(1.0f, 0.0f, 0.0f);

    // roll, pitch, yaw
    // https://en.wikipedia.org/wiki/Aircraft_principal_axes
    float yaw = -0.0589047f, pitch = 0.213803f;

    float speed = 3.0f; // 움직이는 속도

    // 프로젝션 옵션도 카메라 클래스로 이동
    float projFovAngleY = 90.0f * 0.5f; // Luna 교재 기본 설정
    float nearZ = 0.01f;
    float farZ = 100.0f;
    float aspect = 16.0f / 9.0f;
    bool usePerspectiveProjection = true;
};
