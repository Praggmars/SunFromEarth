#include "menu.h"
#include <sstream>

D2D1_RECT_F Menu::LatitudeTextRect() const
{
	return D2D1::RectF(m_rect.left + 10.0f, m_rect.top + 10.0f, m_rect.right - 10.0f, m_rect.top + 30.0f);
}

D2D1_RECT_F Menu::LatitudeSliderRect() const
{
	return D2D1::RectF(m_rect.left + 10.0f, m_rect.top + 40.0f, m_rect.right - 10.0f, m_rect.top + 70.0f);
}

D2D1_RECT_F Menu::TiltTextRect() const
{
	return D2D1::RectF(m_rect.left + 10.0f, m_rect.top + 80.0f, m_rect.right - 10.0f, m_rect.top + 100.0f);
}

D2D1_RECT_F Menu::TiltSliderRect() const
{
	return D2D1::RectF(m_rect.left + 10.0f, m_rect.top + 110.0f, m_rect.right - 10.0f, m_rect.top + 140.0f);
}

D2D1_RECT_F Menu::AdjustDayTextRect() const
{
	return D2D1::RectF(m_rect.left + 10.0f, m_rect.top + 150.0f, m_rect.right - 10.0f, m_rect.top + 170.0f);
}

D2D1_RECT_F Menu::AdjustDaySliderRect() const
{
	return D2D1::RectF(m_rect.left + 10.0f, m_rect.top + 180.0f, m_rect.right - 10.0f, m_rect.top + 210.0f);
}

void Menu::Init(const Graphics& graphics)
{
	m_menuBgBrush = graphics.CreateBrush(mth::float4(0.2f, 0.2f, 0.2f, 1.0f));
	m_uiBrush = graphics.CreateBrush(mth::float4(1.0f, 1.0f, 1.0f, 1.0f));

	m_settings.latitude = 0.25f * mth::pi;
	m_settings.planetTilt = -23.5f * mth::pi / 180.0f;
	m_settings.dayAdjust = 1.0f;

	m_latitudeSlider.SetValue(m_settings.latitude / mth::pi + 0.5f);
	m_latitudeSlider.AssignValueChange([this](float oldValue, float newValue) {
		m_settings.latitude = (newValue - 0.5f) * mth::pi;
		});

	m_tiltSlider.SetValue(m_settings.planetTilt / (2.0f * mth::pi) + 0.5f);
	m_tiltSlider.AssignValueChange([this](float oldValue, float newValue) {
		m_settings.planetTilt = (newValue - 0.5f) * 2.0f * mth::pi;
		});

	m_adjustDaySlider.SetValue(m_settings.dayAdjust * 0.5f + 0.5f);
	m_adjustDaySlider.AssignValueChange([this](float oldValue, float newValue) {
		m_settings.dayAdjust = (newValue - 0.5f) * 2.0f;
		});
}

void Menu::Render(const Graphics& graphics) const
{
	std::wstringstream latss, tiltss, adjss;

	graphics.Context2D()->FillRectangle(m_rect, m_menuBgBrush.Get());
	latss << L"Latitude: " << (m_latitudeSlider.GetValue() - 0.5f) * 180.0f << '°';
	graphics.RenderText(latss.str().c_str(), LatitudeTextRect(), m_uiBrush.Get());
	m_latitudeSlider.Render(graphics, m_uiBrush.Get());
	tiltss << L"Tilt: " << (m_tiltSlider.GetValue() - 0.5f) * 360.0f << '°';
	graphics.RenderText(tiltss.str().c_str(), TiltTextRect(), m_uiBrush.Get());
	m_tiltSlider.Render(graphics, m_uiBrush.Get());
	adjss << L"Adjust day: " << (m_adjustDaySlider.GetValue() - 0.5f) * 2.0f;
	graphics.RenderText(adjss.str().c_str(), AdjustDayTextRect(), m_uiBrush.Get());
	m_adjustDaySlider.Render(graphics, m_uiBrush.Get());
}

void Menu::SetRect(const Graphics& graphics, D2D1_RECT_F rect)
{
	m_rect = rect;
	m_latitudeSlider.SetRect(LatitudeSliderRect());
	m_tiltSlider.SetRect(TiltSliderRect());
	m_adjustDaySlider.SetRect(AdjustDaySliderRect());
}

void Menu::LButtonDownEvent(int x, int y, WPARAM flags)
{
	m_latitudeSlider.LButtonDown(x, y, flags);
	m_tiltSlider.LButtonDown(x, y, flags);
	m_adjustDaySlider.LButtonDown(x, y, flags);
}

void Menu::LButtonUpEvent(int x, int y, WPARAM flags)
{
	m_latitudeSlider.LButtonUp(x, y, flags);
	m_tiltSlider.LButtonUp(x, y, flags);
	m_adjustDaySlider.LButtonUp(x, y, flags);
}

void Menu::MouseMoveEvent(int x, int y, WPARAM flags)
{
	m_latitudeSlider.MouseMove(x, y, flags);
	m_tiltSlider.MouseMove(x, y, flags);
	m_adjustDaySlider.MouseMove(x, y, flags);
}

void Menu::UpdateSettings(std::function<void(Data&)> updater)
{
	if (updater)
	{
		updater(m_settings);
		m_latitudeSlider.SetValue(m_settings.latitude / mth::pi + 0.5f);
		m_tiltSlider.SetValue(m_settings.planetTilt / (2.0f * mth::pi) + 0.5f);
		m_adjustDaySlider.SetValue(m_settings.dayAdjust * 0.5f + 0.5f);
	}
}
