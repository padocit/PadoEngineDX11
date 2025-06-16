#include "Engine.h"
#include "GeometryGenerator.h"

// imgui_impl_win32.cpp에 정의된 메시지 처리 함수에 대한 전방 선언
// Vcpkg를 통해 IMGUI를 사용할 경우 빨간줄로 경고가 뜰 수 있음
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

using namespace std;
using namespace DirectX;
using namespace DirectX::SimpleMath;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Ray;

// Singleton
std::unique_ptr<Engine> Engine::instance = nullptr;

bool Engine::Create(std::unique_ptr<Engine> sample)
{
    if (instance != nullptr)
        return false;

    instance = std::move(sample);
    return true;
}

Engine *Engine::Get()
{
    return instance.get();
}

// WndProc Callback my own MsgProc
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Engine::Get()->MsgProc(hWnd, msg, wParam, lParam);
}

Engine::Engine(int Width, int Height)
    : resolution({Width, Height}), mainWindow(0)
{
    aspect = float(Width) / float(Height);

    camera.SetAspectRatio(aspect);
    camera_debug.SetAspectRatio(aspect);

    level = make_shared<Level>();
}

Engine::~Engine()
{
    instance = nullptr;

    // Cleanup
    guiManager.Shutdown();
    DestroyWindow(mainWindow);

    // DX11 Resources = ComPtr 자동해제
}

int Engine::Run()
{
    // Main Message Loop
    while (/*quit == false*/ true)
    {
        // Handle Input
        MSG msg = {0};
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (WM_QUIT == msg.message)
            {
                quit = true;
                break;
            }
        }

        // GUI
        guiManager.BeginFrame("Scene Control", resolution);
        UpdateGUI();
        guiManager.EndFrame(resolution);
        renderer.guiWidth = guiManager.guiWidth;
        camera.SetAspectRatio(renderer.GetAspectRatio());

        // Update
        Update(guiManager.GetDeltaTime());

        // Render
        Render();
    }

    return 0;
}

bool Engine::Initialize()
{
    if (!InitMainWindow(resolution))
        return false;

    if (!InitDirect3D(resolution))
        return false;

    if (!InitGUI())
        return false;

    if (!InitLevel())
        return false;

    SetForegroundWindow(mainWindow);

    return true;
}

// 여러 Sample에서 공통적으로 사용하기 좋은 장면 설정
bool Engine::InitLevel()
{
    // 환경 박스 초기화
    MeshData skyboxMesh = GeometryGenerator::MakeBox(40.0f);
    std::reverse(skyboxMesh.indices.begin(), skyboxMesh.indices.end());
    shared_ptr<Actor> skybox = make_shared<Actor>(
        renderer.GetDevice(), renderer.GetContext(), vector{skyboxMesh});
    skybox->name = "SkyBox";
    skybox->SetPSO(Graphics::skyboxWirePSO, Graphics::skyboxSolidPSO);

    level->SetSkybox(skybox);

    // 조명 설정
    {
        auto &gConsts = renderer.globalConstsCPU;
        // point light
        gConsts.lights[0].radiance = Vector3(5.0f);
        gConsts.lights[0].position = Vector3(0.0f, 1.5f, 1.1f);
        gConsts.lights[0].radius = 0.04f;
        gConsts.lights[0].type =
            LIGHT_POINT | LIGHT_SHADOW; // Point with shadow

        // off
        gConsts.lights[1].type = LIGHT_OFF;
        gConsts.lights[2].type = LIGHT_OFF;
    }

    // 조명 위치 표시 (Debugging)
    {
        for (int i = 0; i < MAX_LIGHTS; i++)
        {
            auto &lightSphere = renderer.lightSphere[i];
            MeshData sphere = GeometryGenerator::MakeSphere(1.0f, 20, 20);
            lightSphere = make_shared<Actor>(
                renderer.GetDevice(), renderer.GetContext(), vector{sphere});
            lightSphere->UpdateWorldRow(Matrix::CreateTranslation(
                renderer.globalConstsCPU.lights[i].position));
            lightSphere->materialConsts.GetCpu().albedoFactor =
                Vector3(0.0f);
            lightSphere->materialConsts.GetCpu().emissionFactor =
                Vector3(1.0f, 1.0f, 0.0f);
            lightSphere->castShadow =
                false; // 조명 표시 물체들은 그림자 X

            //if (renderer.globalConstsCPU.lights[i].type == 0)
            lightSphere->isVisible = false; // 기본으로 꺼둠

            lightSphere->name = "LightSphere" + std::to_string(i);
            lightSphere->isPickable = false;

            // using default PSO
            level->AddActor(lightSphere); // 리스트에 등록
        }
    }

    // 커서 표시 (Main sphere와의 충돌이 감지되면 월드 공간에 작게 그려지는 구)
    {
        MeshData sphere = GeometryGenerator::MakeSphere(0.01f, 10, 10);
        cursorSphere = make_shared<Actor>(
            renderer.GetDevice(), renderer.GetContext(), vector{sphere});
        cursorSphere->isVisible = false;  // 마우스가 눌렸을 때만 보임
        cursorSphere->castShadow = false; // 그림자 X
        cursorSphere->materialConsts.GetCpu().albedoFactor = Vector3(0.0f);
        cursorSphere->materialConsts.GetCpu().emissionFactor =
            Vector3(0.0f, 1.0f, 0.0f);

        level->AddActor(cursorSphere);
    }


    return false;
}

void Engine::Update(float dt)
{
    // 카메라 이동
    camera.UpdateKeyboard(dt, keyPressed);
    // ProcessMouseControl();

    renderer.Update(&camera, dt);

    ProcessMouseControl();
    
    level->Update(renderer.GetDevice(), renderer.GetContext());
}

void Engine::UpdateGUI()
{
    // 공통으로 쓰는 GUI
    if (ImGui::TreeNode("General"))
    {
        ImGui::Checkbox("FPV (F key)", &camera.useFirstPersonView);
        ImGui::Checkbox("Wireframe", &renderer.drawAsWire);
        if (ImGui::Checkbox("MSAA ON", &renderer.useMSAA))
        {
            renderer.CreateBuffers();
        }
        ImGui::SliderFloat("LodBias", &renderer.globalConstsCPU.lodBias, 0.0f,
                           10.0f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Skybox"))
    {
        ImGui::SliderFloat("Strength", &renderer.globalConstsCPU.strengthIBL, 0.0f,
                           0.5f);
        ImGui::RadioButton("Env", &renderer.globalConstsCPU.textureToDraw, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Specular", &renderer.globalConstsCPU.textureToDraw,
                           1);
        ImGui::SameLine();
        ImGui::RadioButton("Irradiance", &renderer.globalConstsCPU.textureToDraw, 2);
        ImGui::SliderFloat("EnvLodBias", &renderer.globalConstsCPU.envLodBias, 0.0f,
                           10.0f);
        ImGui::TreePop();
    }

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Post Effects"))
    {
        int flag = 0;
        flag += ImGui::RadioButton("Render", &renderer.postEffectsConstsCPU.mode, 1);
        ImGui::SameLine();
        flag +=
            ImGui::RadioButton("Depth", &renderer.postEffectsConstsCPU.mode, 2);
        flag += ImGui::SliderFloat(
            "DepthScale", &renderer.postEffectsConstsCPU.depthScale, 0.0, 1.0);
        flag += ImGui::SliderFloat(
            "Fog", &renderer.postEffectsConstsCPU.fogStrength,
                                   0.0, 10.0);

        if (flag)
            renderer.UpdatePostEffectsConstants();

        ImGui::TreePop();
    }


    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Post Processing"))
    {

        int &flag = renderer.postProcessFlag = 0;
        flag += ImGui::SliderFloat(
            "Bloom Strength",
            &renderer.postProcess.combineFilter.constData.strength, 0.0f, 1.0f);
        flag += ImGui::SliderFloat(
            "Exposure",
            &renderer.postProcess.combineFilter.constData.option1,
            0.0f, 10.0f);
        flag += ImGui::SliderFloat(
            "Gamma", &renderer.postProcess.combineFilter.constData.option2,
            0.1f,
            5.0f);

        ImGui::TreePop();
    }


    //ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    //if (ImGui::TreeNode("Post Effects"))
    //{
    //    int& flag = renderer.postEffectsFlag = 0;
    //    flag += ImGui::RadioButton("Render", &renderer.postEffectsConstsCPU.mode, 1);
    //    ImGui::SameLine();
    //    flag +=
    //        ImGui::RadioButton("Depth", &renderer.postEffectsConstsCPU.mode, 2);
    //    flag += ImGui::SliderFloat(
    //        "DepthScale", &renderer.postEffectsConstsCPU.depthScale, 0.0, 1.0);
    //    flag += ImGui::SliderFloat(
    //        "Fog", &renderer.postEffectsConstsCPU.fogStrength,
    //                               0.0, 10.0);

    //    ImGui::TreePop();
    //}

    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode("Light"))
    {
        ImGui::SliderFloat3("Position",
                            &renderer.globalConstsCPU.lights[0].position.x,
                            -5.0f, 5.0f);
        ImGui::SliderFloat("Radius", &renderer.globalConstsCPU.lights[0].radius,
                           0.0f,
                           0.5f);
        ImGui::Checkbox("Debug Sphere", &renderer.lightSphere[0]->isVisible);
        ImGui::TreePop();
    }
}

void Engine::Render()
{
    renderer.Render();
    renderer.PostRender();

    guiManager.Render();

    renderer.SwapBuffer(); // Present()
}

void Engine::OnMouseMove(int MouseX, int MouseY)
{

    mouseX = MouseX;
    mouseY = MouseY;

    // 마우스 커서의 위치를 NDC로 변환
    // 마우스 커서는 좌측 상단 (0, 0), 우측 하단(width-1, height-1)
    // NDC는 좌측 하단이 (-1, -1), 우측 상단(1, 1)
    //mouseNdcX = mouseX * 2.0f / (resolution.width - guiManager.guiWidth) - 1.0f;
    mouseNdcX = MouseX * 2.0f / resolution.width - 1.0f;
    mouseNdcY = -MouseY * 2.0f / resolution.height + 1.0f;

    // 커서가 화면 밖으로 나갔을 경우 범위 조절
    // 게임에서는 클램프를 안할 수도 있습니다.
    mouseNdcX = std::clamp(mouseNdcX, -1.0f, 1.0f);
    mouseNdcY = std::clamp(mouseNdcY, -1.0f, 1.0f);
     
    // 카메라 시점 회전
    camera.UpdateMouse(mouseNdcX, mouseNdcY);
}

void Engine::OnMouseClick(int MouseX, int MouseY)
{
    mouseX = MouseX;
    mouseY = MouseY;

    //mouseNdcX = mouseX * 2.0f / (resolution.width - guiManager.guiWidth) - 1.0f;
    mouseNdcX = MouseX * 2.0f / resolution.width - 1.0f;
    mouseNdcY = -MouseY * 2.0f / resolution.height + 1.0f;
}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        // Reset Swapchain
        if (renderer.GetSwapChain())
        {
            resolution.width = int(LOWORD(lParam));
            resolution.height = int(HIWORD(lParam));
            renderer.guiWidth = 0;

            // 윈도우 Minimize 모드 screenWidth/Height == 0
            if (resolution.width && resolution.height)
            {

                cout << "Resize SwapChain to " << resolution.width << " "
                     << resolution.height << endl;

                renderer.SetResolution(resolution);
                renderer.SetScreenSize((UINT)LOWORD(lParam),
                                       (UINT)HIWORD(lParam));
                camera.SetAspectRatio(renderer.GetAspectRatio());

                renderer.ResetPostProcess();
            }
        }
        break;
    case WM_SYSCOMMAND:
        break;
    case WM_MOUSEMOVE:
        OnMouseMove(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_LBUTTONDOWN:
        if (!leftButton)
        {
            dragStartFlag = true; // 드래그를 새로 시작하는지 확인
        }
        leftButton = true;
        OnMouseClick(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_LBUTTONUP:
        leftButton = false;
        break;
    case WM_RBUTTONDOWN:
        if (!rightButton)
        {
            dragStartFlag = true; // 드래그를 새로 시작하는지 확인
        }
        rightButton = true;
        break;
    case WM_RBUTTONUP:
        rightButton = false;
        break;

    case WM_KEYDOWN:
        keyPressed[wParam] = true;
        if (wParam == VK_ESCAPE) // ESC키 종료
        {
            DestroyWindow(hwnd);
        }
        break;
    case WM_KEYUP:
        if (wParam == 'F') // F키 1인칭 시점
        {
            camera.useFirstPersonView = !camera.useFirstPersonView;
        }
        if (wParam == 'C') // C키 화면 캡쳐
        {
            renderer.CaptureScreen();
        }
        //if (wParam == 'P') // P키 애니메이션 일시중지
        //{
        //    pauseAnimation = !pauseAnimation;
        //}
        if (wParam == 'Z') // 카메라 설정 출력
        {
            camera.PrintView();
        }

        keyPressed[wParam] = false;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

shared_ptr<Actor> Engine::PickClosest(const Ray &pickingRay, float &minDist)
{
    minDist = 1e5f;
    shared_ptr<Actor> minActor = nullptr;
    for (auto &actor : level->actors)
    {
        float dist = 0.0f;
        if (actor->isPickable &&
            pickingRay.Intersects(actor->boundingSphere, dist) &&
            dist < minDist)
        {
            minActor = actor;
            minDist = dist;
        }
    }
    return minActor;
}

void Engine::ProcessMouseControl()
{
    static shared_ptr<Actor> activeModel = nullptr;
    static float prevRatio = 0.0f;
    static Vector3 prevPos(0.0f);
    static Vector3 prevVector(0.0f);

    // 적용할 회전과 이동 초기화
    Quaternion q =
        Quaternion::CreateFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), 0.0f);
    Vector3 dragTranslation(0.0f);
    Vector3 pickPoint(0.0f);
    float dist = 0.0f;

    // 사용자가 두 버튼 중 하나만 누른다고 가정.
    if (leftButton || rightButton)
    {
        const Matrix viewRow = camera.GetViewRow();
        const Matrix projRow = camera.GetProjRow();
        const Vector3 ndcNear = Vector3(mouseNdcX, mouseNdcY, 0.0f);
        const Vector3 ndcFar = Vector3(mouseNdcX, mouseNdcY, 1.0f);
        const Matrix invProjView = (viewRow * projRow).Invert();
        const Vector3 worldNear = Vector3::Transform(ndcNear, invProjView);
        const Vector3 worldFar = Vector3::Transform(ndcFar, invProjView);
        Vector3 dir = worldFar - worldNear;
        dir.Normalize();
        const Ray curRay = SimpleMath::Ray(worldNear, dir);

        // 이전 프레임에서 아무 물체도 선택되지 않았을 경우에는 새로 선택
        if (!activeModel)
        {
            auto newModel = PickClosest(curRay, dist);
            if (newModel)
            {
                cout << "Newly selected model: " << newModel->name << endl;
                activeModel = newModel;
                pickedModel = newModel; // GUI 조작용 포인터
                pickPoint = curRay.position + dist * curRay.direction;
                if (leftButton)
                { // 왼쪽 버튼 회전 준비
                    prevVector =
                        pickPoint - activeModel->boundingSphere.Center;
                    prevVector.Normalize();
                }
                else
                { // 오른쪽 버튼 이동 준비
                    dragStartFlag = false;
                    prevRatio = dist / (worldFar - worldNear).Length();
                    prevPos = pickPoint;
                }
            }
        }
        else
        { // 이미 선택된 물체가 있었던 경우
            if (leftButton)
            { // 왼쪽 버튼으로 계속 회전
                if (curRay.Intersects(activeModel->boundingSphere, dist))
                {
                    pickPoint = curRay.position + dist * curRay.direction;
                }
                else
                {
                    // 바운딩 스피어에 가장 가까운 점을 찾기
                    Vector3 c =
                        activeModel->boundingSphere.Center - worldNear;
                    Vector3 centerToRay = dir.Dot(c) * dir - c;
                    pickPoint =
                        c +
                        centerToRay *
                            std::clamp(activeModel->boundingSphere.Radius /
                                           centerToRay.Length(),
                                       0.0f, 1.0f);
                    pickPoint += worldNear;
                }

                Vector3 currentVector =
                    pickPoint - activeModel->boundingSphere.Center;
                currentVector.Normalize();
                float theta = acos(prevVector.Dot(currentVector));
                if (theta > 3.141592f / 180.0f * 3.0f)
                {
                    Vector3 axis = prevVector.Cross(currentVector);
                    axis.Normalize();
                    q = SimpleMath::Quaternion::CreateFromAxisAngle(axis,
                                                                    theta);
                    prevVector = currentVector;
                }
            }
            else
            { // 오른쪽 버튼으로 계속 이동
                Vector3 newPos = worldNear + prevRatio * (worldFar - worldNear);
                if ((newPos - prevPos).Length() > 1e-3)
                {
                    dragTranslation = newPos - prevPos;
                    prevPos = newPos;
                }
                pickPoint = newPos; // Cursor sphere 그려질 위치
            }
        }
    }
    else
    {
        // 버튼에서 손을 땠을 경우에는 움직일 모델은 nullptr로 설정
        activeModel = nullptr;

        // pickedModel은 GUI 조작을 위해 마우스에서 손을 떼도 nullptr로
        // 설정하지 않음
    }

    // Cursor sphere 그리기
    if (activeModel)
    {
        Vector3 translation = activeModel->worldRow.Translation();
        activeModel->worldRow.Translation(Vector3(0.0f));
        activeModel->UpdateWorldRow(
            activeModel->worldRow * Matrix::CreateFromQuaternion(q) *
            Matrix::CreateTranslation(dragTranslation + translation));
        activeModel->boundingSphere.Center =
            activeModel->worldRow.Translation();

        // 충돌 지점에 작은 구 그리기
        cursorSphere->isVisible = true;
        cursorSphere->UpdateWorldRow(Matrix::CreateTranslation(pickPoint));
    }
    else
    {
        cursorSphere->isVisible = false;
    }
}

bool Engine::InitMainWindow(const Resolution &res)
{
    WNDCLASSEX wc = {sizeof(WNDCLASSEX),
                     CS_CLASSDC,
                     WndProc,
                     0L,
                     0L,
                     GetModuleHandle(NULL),
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     L"HongLabGraphics", // lpszClassName, L-string
                     NULL};

    if (!RegisterClassEx(&wc))
    {
        cout << "RegisterClassEx() failed." << endl;
        return false;
    }

    RECT wr = {0, 0, res.width, res.height};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, false);
    mainWindow =
        CreateWindow(wc.lpszClassName, L"PadoEngine", WS_OVERLAPPEDWINDOW,
                     100,                // Window upper-left X
                     100,                // Window upper-left Y
                     wr.right - wr.left, // Window width
                     wr.bottom - wr.top, // Window height
                     NULL, NULL, wc.hInstance, NULL);

    if (!mainWindow)
    {
        cout << "CreateWindow() failed." << endl;
        return false;
    }

    ShowWindow(mainWindow, SW_SHOWDEFAULT);
    UpdateWindow(mainWindow);

    return true;
}

bool Engine::InitDirect3D(const Resolution &res)
{
    if (!renderer.Initialize(res, mainWindow))
        return false;

    renderer.SetCurrentLevel(level);

    return true;
}

bool Engine::InitGUI()
{
    if (!guiManager.Initialize(mainWindow, renderer.GetDevice().Get(),
                               renderer.GetContext().Get(), resolution))
        return false;

    return true;
}
