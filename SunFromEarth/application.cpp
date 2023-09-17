#include "application.h"
#include <windowsx.h>

void Application::PaintEvent()
{
	m_graphics.BeginDraw();

	m_earthView.SetLatitude(m_menu.Settings().latitude);
	m_earthView.SetTilt(m_menu.Settings().planetTilt);
	m_dayCycleView.SetLatitude(m_menu.Settings().latitude);
	m_dayCycleView.SetTilt(m_menu.Settings().planetTilt);
	m_dayCycleView.SetDayAdjust(m_menu.Settings().dayAdjust);

	m_earthView.Render(m_graphics);
	m_dayCycleView.Render(m_graphics);
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

	m_menu.SetRect(m_graphics, D2D1::RectF(
		0.0f,
		0.0f,
		static_cast<float>(m_resolution.x - m_dayCycleView.Resolution().x),
		static_cast<float>(m_resolution.y)));
}

void Application::MouseLButtonDownEvent(int x, int y, WPARAM flags)
{
	SetCapture(m_mainWindow);
	if (m_activeEventHandler)
		m_activeEventHandler->LButtonDownEvent(x, y, flags);
	else
	{
		for (EventHandler* eh : m_eventHandlers)
			if (IsOnRect(mth::float2(static_cast<float>(x), static_cast<float>(y)), eh->Rectangle()))
			{
				m_activeEventHandler = eh;
				eh->LButtonDownEvent(x, y, flags);
				break;
			}
	}
	m_prevCursor = { x, y };
}

void Application::MouseRButtonDownEvent(int x, int y, WPARAM flags)
{
	SetCapture(m_mainWindow);
	if (m_activeEventHandler)
		m_activeEventHandler->RButtonDownEvent(x, y, flags);
	else
	{
		for (EventHandler* eh : m_eventHandlers)
			if (IsOnRect(mth::float2(static_cast<float>(x), static_cast<float>(y)), eh->Rectangle()))
			{
				m_activeEventHandler = eh;
				eh->RButtonDownEvent(x, y, flags);
				break;
			}
	}
	m_prevCursor = { x, y };
}

void Application::MouseLButtonUpEvent(int x, int y, WPARAM flags)
{
	if (0 == (flags & (MK_LBUTTON | MK_RBUTTON)))
	{
		m_activeEventHandler = nullptr;
		ReleaseCapture();
	}
	if (m_activeEventHandler)
		m_activeEventHandler->LButtonUpEvent(x, y, flags);
	else
	{
		for (EventHandler* eh : m_eventHandlers)
			if (IsOnRect(mth::float2(static_cast<float>(x), static_cast<float>(y)), eh->Rectangle()))
			{
				eh->LButtonUpEvent(x, y, flags);
				break;
			}
	}
	m_prevCursor = { x, y };
}

void Application::MouseRButtonUpEvent(int x, int y, WPARAM flags)
{
	if (0 == (flags & (MK_LBUTTON | MK_RBUTTON)))
	{
		m_activeEventHandler = nullptr;
		ReleaseCapture();
	}
	if (m_activeEventHandler)
		m_activeEventHandler->RButtonUpEvent(x, y, flags);
	else
	{
		for (EventHandler* eh : m_eventHandlers)
			if (IsOnRect(mth::float2(static_cast<float>(x), static_cast<float>(y)), eh->Rectangle()))
			{
				eh->RButtonUpEvent(x, y, flags);
				break;
			}
	}
	m_prevCursor = { x, y };
}

void Application::MouseMoveEvent(int x, int y, WPARAM flags)
{
	if (m_activeEventHandler)
		m_activeEventHandler->MouseMoveEvent(x, y, flags);
	else
	{
		for (EventHandler* eh : m_eventHandlers)
			if (IsOnRect(mth::float2(static_cast<float>(x), static_cast<float>(y)), eh->Rectangle()))
			{
				eh->MouseMoveEvent(x, y, flags);
				break;
			}
	}
	m_prevCursor = { x, y };
}

Application::Application()
	: m_mainWindow{}
	, m_prevCursor{}
	, m_activeEventHandler{} {}

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
	m_dayCycleView.Init(m_graphics, 1, 1);
	m_earthView.Init(m_graphics, 1, 1);
	m_eventHandlers.push_back(&m_menu);
	m_eventHandlers.push_back(&m_dayCycleView);
	m_eventHandlers.push_back(&m_earthView);

	m_earthView.AssignLatitudeChange([this](float latitude) {
		m_menu.UpdateSettings([latitude](Menu::Data& settings) {
			settings.latitude = latitude;
			});
		});

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