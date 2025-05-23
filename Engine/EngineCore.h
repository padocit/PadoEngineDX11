#pragma once

#include "Renderer.h"
#include "GUIManager.h"

class EngineCore
{
public:
	EngineCore();
	virtual ~EngineCore();

	int Run();

	bool Initialize(int width = 1280, int height = 720);
	void Update();
    virtual void UpdateGUI();
	void Render();

	virtual LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Init
	bool InitMainWindow(const Resolution& resolution);
	bool InitDirect3D(const Resolution& res);
	bool InitGUI();

	static EngineCore& Get();

private:
    Renderer renderer;
    GUIManager guiManager;

	HWND mainWindow;

	Resolution resolution;
	bool quit = false;

	// юс╫ц
    Mesh triangle;

	static std::unique_ptr<EngineCore> instance;
};

