#pragma once

#include "graphics.h"

class EventHandler
{
protected:
	D2D1_RECT_F m_rect;

public:
	virtual void LButtonDownEvent(int x, int y, WPARAM flags);
	virtual void RButtonDownEvent(int x, int y, WPARAM flags);
	virtual void LButtonUpEvent(int x, int y, WPARAM flags);
	virtual void RButtonUpEvent(int x, int y, WPARAM flags);
	virtual void MouseMoveEvent(int x, int y, WPARAM flags);

	virtual void SetRect(const Graphics& graphics, D2D1_RECT_F rect);
	inline D2D1_RECT_F Rectangle() const { return m_rect; }
	inline mth::vec2<int> Resolution() const { return mth::vec2<int>(static_cast<int>(std::ceil(m_rect.right - m_rect.left)), static_cast<int>(std::ceil(m_rect.bottom - m_rect.top))); }
};