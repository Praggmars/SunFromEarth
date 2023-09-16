#pragma once

#include <Windows.h>
#include "earthview.h"
#include "daycycleview.h"
#include "menu.h"

class Application
{
	HWND m_mainWindow;
	POINT m_prevCursor;
	mth::vec2<int> m_resolution;
	Graphics m_graphics;
	Menu m_menu;
	EarthView m_earthView;
	DayCycleView m_dayCycleView;

private:
	void ChangeLatitudeFromMouse(int x, int y);
	void ChangeTiltFromMouse(int x, int y);

	void PaintEvent();
	void Resize(int width, int height);
	void MouseLButtonDownEvent(int x, int y, WPARAM flags);
	void MouseRButtonDownEvent(int x, int y, WPARAM flags);
	void MouseLButtonUpEvent(int x, int y, WPARAM flags);
	void MouseRButtonUpEvent(int x, int y, WPARAM flags);
	void MouseMoveEvent(int x, int y, WPARAM flags);

public:
	Application();
	~Application();
	void Init(const wchar_t* title, int width, int height);
	void Run();
	LRESULT MessageHandler(UINT msg, WPARAM wparam, LPARAM lparam);
};