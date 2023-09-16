#include "application.h"
#include <windowsx.h>

void Application::ChangeLatitudeFromMouse(int x, int y)
{
	float latitude = 0.0f;
	if (m_earthView.GetLatitude(
		x - (m_resolution.x - m_earthView.Resolution().x),
		y - (m_resolution.y - m_earthView.Resolution().y),
		latitude))
	{
		m_menu.UpdateSettings([latitude](Menu::Data& settings) {
			settings.latitude = latitude;
			});
	}
}

void Application::ChangeTiltFromMouse(int x, int y)
{
	const float tilt = (mth::float2(static_cast<float>(x), static_cast<float>(-y)) - mth::float2(
		static_cast<float>(m_resolution.x) - static_cast<float>(m_earthView.Resolution().x) * 0.5f,
		static_cast<float>(m_earthView.Resolution().y) * 0.5f - static_cast<float>(m_resolution.y))).Angle();
	m_earthView.SetTilt(tilt);
	m_dayCycleView.SetTilt(tilt);
}

void Application::PaintEvent()
{
	m_graphics.BeginDraw();

	m_earthView.SetTilt(m_menu.Settings().planetTilt);
	m_dayCycleView.SetLatitude(m_menu.Settings().latitude);
	m_dayCycleView.SetTilt(m_menu.Settings().planetTilt);
	m_dayCycleView.SetDayAdjust(m_menu.Settings().dayAdjust);

	m_earthView.Update();

	m_earthView.Render(m_graphics);
	m_dayCycleView.Render(m_graphics);
	const mth::float2 cursor(static_cast<float>(m_prevCursor.x), static_cast<float>(m_prevCursor.y));
	if (IsOnRect(cursor, m_dayCycleView.Rectangle()))
		m_dayCycleView.RenderTimePresenter(m_graphics, cursor);
	m_menu.Render(m_graphics);

	m_graphics.EndDraw();
	//ValidateRect(m_mainWindow, nullptr);
}

void Application::Resize(int width, int height)
{
	m_resolution.x = width;
	m_resolution.y = height;
	m_graphics.Resize(width, height);
	const int earthMapSize = min(width * 3 / 4, height * 3 / 4);
	const int dayCycleHeight = earthMapSize / 3;

	m_earthView.SetRect(m_graphics, D2D1::RectF(
			static_cast<float>(m_resolution.x - earthMapSize),
			static_cast<float>(dayCycleHeight),
			static_cast<float>(m_resolution.x),
			static_cast<float>(dayCycleHeight + earthMapSize)));

	m_dayCycleView.SetRect(m_graphics, D2D1::RectF(
			static_cast<float>(m_resolution.x - earthMapSize),
			0.0f,
			static_cast<float>(m_resolution.x),
			static_cast<float>(dayCycleHeight)));

	m_menu.Resize(m_resolution.x - m_dayCycleView.Resolution().x, m_resolution.y);
}

void Application::MouseLButtonDownEvent(int x, int y, WPARAM flags)
{
	SetCapture(m_mainWindow);
	ChangeLatitudeFromMouse(x, y);
	m_menu.LButtonDownEvent(x, y, flags);
	m_prevCursor = { x, y };
}

void Application::MouseRButtonDownEvent(int x, int y, WPARAM flags)
{
	//ChangeTiltFromMouse(x, y);
	m_prevCursor = { x, y };
}

void Application::MouseLButtonUpEvent(int x, int y, WPARAM flags)
{
	ReleaseCapture();
	m_menu.LButtonUpEvent(x, y, flags);
	m_prevCursor = { x, y };
}

void Application::MouseRButtonUpEvent(int x, int y, WPARAM flags)
{
	m_prevCursor = { x, y };
}

void Application::MouseMoveEvent(int x, int y, WPARAM flags)
{
	if (flags & MK_LBUTTON && IsOnRect(mth::float2(static_cast<float>(x), static_cast<float>(y)), m_earthView.Rectangle()))
		ChangeLatitudeFromMouse(x, y);
	/*if (flags & MK_RBUTTON)
		ChangeTiltFromMouse(x, y);*/
	m_menu.MouseMoveEvent(x, y, flags);
	m_prevCursor = { x, y };
}

Application::Application()
	: m_mainWindow{}
	, m_prevCursor{} {}

Application::~Application() 
{
}

void Application::Init(const wchar_t* title, int width, int height)
{
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(wc);
	wc.hInstance = GetModuleHandleW(nullptr);
	wc.lpszClassName = title;
	wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = DefWindowProcW;
	RegisterClassExW(&wc);
	RECT rect{ 0, 0, width, height };
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, 0);
	m_resolution.x = rect.right - rect.left;
	m_resolution.y = rect.bottom - rect.top;
	m_mainWindow = CreateWindowExW(0, title, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, m_resolution.x, m_resolution.y, nullptr, nullptr, nullptr, nullptr);
	SetWindowLongPtrW(m_mainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	SetWindowLongPtrW(m_mainWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(static_cast<LRESULT(*)(HWND, UINT, WPARAM, LPARAM)>(
		[](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)->LRESULT {
			return reinterpret_cast<Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))->MessageHandler(msg, wparam, lparam);
	})));

	m_graphics.Init(m_mainWindow);
	m_menu.Init(m_graphics);
	m_earthView.Init(m_graphics, 1, 1);
	m_dayCycleView.Init(m_graphics, 1, 1);

	ShowWindow(m_mainWindow, SW_SHOWDEFAULT);
	UpdateWindow(m_mainWindow);
}

void Application::Run()
{
	MSG message{};
	while (GetMessageW(&message, nullptr, 0, 0))
	{
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}
}

LRESULT Application::MessageHandler(UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_MOUSEMOVE:
		MouseMoveEvent(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_NCMOUSEMOVE:
		MouseMoveEvent(-1, -1, 0);
		return 0;
	case WM_LBUTTONDOWN:
		MouseLButtonDownEvent(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_RBUTTONDOWN:
		MouseRButtonDownEvent(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_LBUTTONUP:
		MouseLButtonUpEvent(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_RBUTTONUP:
		MouseRButtonUpEvent(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), wparam);
		return 0;
	case WM_PAINT:
		PaintEvent();
		return 0;
	case WM_SIZE:
		Resize(LOWORD(lparam), HIWORD(lparam));
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(m_mainWindow, msg, wparam, lparam);
}