#pragma once

#include <functional>
#include "graphics.h"

class Slider
{
	D2D1_RECT_F m_rect;
	std::function<void(float oldVal, float newVal)> m_valueChangeCallback;
	float m_value;
	float m_dragOffset;
	bool m_dragged;

private:
	D2D1_RECT_F KnobRect() const;

public:
	Slider();
	void SetRect(D2D1_RECT_F rect);
	void Render(const Graphics& graphics, ID2D1Brush* brush) const;

	void AssignValueChange(std::function<void(float oldVal, float newVal)> valueChangeCallback);

	void LButtonDown(int x, int y, WPARAM flags);
	void LButtonUp(int x, int y, WPARAM flags);
	void MouseMove(int x, int y, WPARAM flags);

	void SetValue(float value);
	float GetValue() const;
};