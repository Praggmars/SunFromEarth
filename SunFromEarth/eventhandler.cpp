#include "eventhandler.h"

void EventHandler::LButtonDownEvent(int x, int y, WPARAM flags)
{
}

void EventHandler::RButtonDownEvent(int x, int y, WPARAM flags)
{
}

void EventHandler::LButtonUpEvent(int x, int y, WPARAM flags)
{
}

void EventHandler::RButtonUpEvent(int x, int y, WPARAM flags)
{
}

void EventHandler::MouseMoveEvent(int x, int y, WPARAM flags)
{
}

void EventHandler::SetRect(const Graphics& graphics, D2D1_RECT_F rect)
{
	m_rect = rect;
}
