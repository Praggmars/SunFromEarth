#include "menu.h"
#include <sstream>

D2D1_RECT_F Menu::LatitudeTextRect() const
{
	return D2D1::RectF(10.0f, 10.0f, static_cast<float>(m_resolution.x - 10), 30.0f);
}

D2D1_RECT_F Menu::LatitudeSliderRect() const
{
	return D2D1::RectF(10.0f, 40.0f, static_cast<float>(m_resolution.x - 10), 70.0f);
}

D2D1_RECT_F Menu::TiltTextRect() const
{
	return D2D1::RectF(10.0f, 80.0f, static_cast<float>(m_resolution.x - 10), 100.0f);
}

D2D1_RECT_F Menu::TiltSliderRect() const
{
	return D2D1::RectF(10.0f, 110.0f, static_cast<float>(m_resolution.x - 10), 140.0f);
}

D2D1_RECT_F Menu::AdjustDayTextRect() const
{
	return D2D1::RectF(10.0f, 150.0f, static_cast<float>(m_resolution.x - 10), 170.0f);
}

D2D1_RECT_F Menu::AdjustDaySliderRect() const
{
	return D2D1::RectF(10.0f, 180.0f, static_cast<float>(m_resolution.x - 10), 210.0f);
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

	graphics.Context2D()->FillRectangle(D2D1::RectF(0.0f, 0.0f, static_cast<float>(m_resolution.x), static_cast<float>(m_resolution.y)), m_menuBgBrush.Get());
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

void Menu::Resize(int width, int height)
{
	m_resolution.x = width;
	m_resolution.y = height;

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
