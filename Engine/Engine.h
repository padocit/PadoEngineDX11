#pragma once

#include "D3D11Renderer.h"
#include "GUIManager.h"
#include "Camera.h"
#include "Level.h"
#include "Actor.h"

class Engine
{
public:
	Engine(int Width = 1600, int Height = 900);
	virtual ~Engine();

	int Run();

	bool Initialize();
    virtual bool InitLevel();
	virtual void Update(float dt);
    virtual void UpdateGUI();
	virtual void Render();

    virtual void OnMouseMove(int MouseX, int MouseY);
    virtual void OnMouseClick(int MouseX, int MouseY);
    virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    shared_ptr<Actor> PickClosest(const DirectX::SimpleMath::Ray &pickingRay,
                                  float &minDist);
    void ProcessMouseControl();

	// Init
	bool InitMainWindow(const Resolution& resolution);
	bool InitDirect3D(const Resolution& res);
	bool InitGUI();

	D3D11Renderer &GetRenderer()
    {
        return renderer;
    }

	// Singleton
	static bool Create(std::unique_ptr<Engine> sample);
	static Engine* Get();

public:
    shared_ptr<Actor> pickedModel; // GUI 조작용
    shared_ptr<Actor> cursorSphere;

protected:
    D3D11Renderer renderer;
    GUIManager guiManager;
	HWND mainWindow;

    shared_ptr<Level> level;

	Camera camera;
    Camera camera_debug; // 디버깅용 카메라 (ex. culling)

	Resolution resolution;
    float aspect;
	bool quit = false;

	bool keyPressed[256] = {
        false,
    };

    bool leftButton = false;
    bool rightButton = false;
    bool dragStartFlag = false;

    // 마우스 커서 위치 저장 (Picking에 사용)
    float mouseNdcX = 0.0f;
    float mouseNdcY = 0.0f;
    float wheelDelta = 0.0f;
    int mouseX = -1;
    int mouseY = -1;


	bool pauseAnimation = false;


	static std::unique_ptr<Engine> instance;
};

