#pragma once

#include "D3D11Renderer.h"
#include "GUIManager.h"

class Engine
{
public:
	Engine();
	virtual ~Engine();

	int Run();

	bool Initialize(int width = 1280, int height = 720);
    virtual bool InitLevel();
	virtual void Update(float dt);
    virtual void UpdateGUI();
	virtual void Render();

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Init
	bool InitMainWindow(const Resolution& resolution);
	bool InitDirect3D(const Resolution& res);
	bool InitGUI();

	// Singleton
	static bool Create(std::unique_ptr<Engine> sample);
	static Engine* Get();

private:
    D3D11Renderer renderer;
    GUIManager guiManager;

	HWND mainWindow;

	Resolution resolution;
	bool quit = false;

	static std::unique_ptr<Engine> instance;
};

