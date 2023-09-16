#include "slider.h"

#undef min
#undef max

constexpr float KNOB_WIDTH = 8.0f;

D2D1_RECT_F Slider::KnobRect() const
{
	const float rectWidth = m_rect.right - m_rect.left;
	return D2D1::RectF(m_rect.left + (rectWidth - KNOB_WIDTH) * m_value, m_rect.top, m_rect.left + (rectWidth - KNOB_WIDTH) * m_value + KNOB_WIDTH, m_rect.bottom);
}

Slider::Slider()
	: m_rect{}
	, m_value{}
	, m_dragOffset{}
	, m_dragged{ false } {}

void Slider::SetRect(D2D1_RECT_F rect)
{
	m_rect = rect;
}

void Slider::Render(const Graphics& graphics, ID2D1Brush* brush) const
{
	const float lineWidth = 4.0f;

	const float rectWidth = m_rect.right - m_rect.left;
	const float rectHeight = m_rect.bottom - m_rect.top;
	const float knobRound = std::min(KNOB_WIDTH, rectHeight) / 3.0f;

	graphics.Context2D()->FillRectangle(D2D1::RectF(m_rect.left, m_rect.top + (rectHeight - lineWidth) * 0.5f, m_rect.right, m_rect.top + (rectHeight + lineWidth) * 0.5f), brush);
	graphics.Context2D()->FillRoundedRectangle(D2D1::RoundedRect(KnobRect(), knobRound, knobRound), brush);
}

void Slider::AssignValueChange(std::function<void(float oldVal, float newVal)> valueChangeCallback)
{
	m_valueChangeCallback = valueChangeCallback;
}

void Slider::LButtonDown(int x, int y, WPARAM flags)
{
	const mth::float2 cursor{ static_cast<float>(x), static_cast<float>(y) };
	if (IsOnRect(cursor, m_rect))
	{
		const float rectWidth = m_rect.right - m_rect.left;
		m_dragged = true;
		if (IsOnRect(cursor, KnobRect()))
		{
			m_dragOffset = m_rect.left + rectWidth * m_value - cursor.x;
		}
		else
		{
			const float oldValue = m_value;
			m_dragOffset = 0.0f;
			m_value = std::max(0.0f, std::min(1.0f, (cursor.x + m_dragOffset - m_rect.left - 0.5f * KNOB_WIDTH) / (rectWidth - KNOB_WIDTH)));
			if (m_valueChangeCallback)
				m_valueChangeCallback(oldValue, m_value);
		}
	}
}

void Slider::LButtonUp(int x, int y, WPARAM flags)
{
	m_dragged = false;
}

void Slider::MouseMove(int x, int y, WPARAM flags)
{
	if (m_dragged)
	{
		const mth::float2 cursor{ static_cast<float>(x), static_cast<float>(y) };
		const float rectWidth = m_rect.right - m_rect.left;
		const float oldValue = m_value;
		m_value = std::max(0.0f, std::min(1.0f, (cursor.x + m_dragOffset - m_rect.left - 0.5f * KNOB_WIDTH) / (rectWidth - KNOB_WIDTH)));
		if (m_valueChangeCallback)
			m_valueChangeCallback(oldValue, m_value);
	}
}

void Slider::SetValue(float value)
{
	const float oldValue = m_value;
	m_value = value;
	if (m_valueChangeCallback)
		m_valueChangeCallback(oldValue, m_value);
}

float Slider::GetValue() const
{
	return m_value;
}
