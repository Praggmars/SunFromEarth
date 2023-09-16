#pragma once

#include "slider.h"

class Menu
{
public:
	struct Data
	{
		float latitude;
		float planetTilt;
		float dayAdjust;
	};

private:
	ComPtr<ID2D1SolidColorBrush> m_menuBgBrush;
	ComPtr<ID2D1SolidColorBrush> m_uiBrush;

	Slider m_latitudeSlider;
	Slider m_tiltSlider;
	Slider m_adjustDaySlider;

	Data m_settings;
	mth::vec2<int> m_resolution;
	POINT m_prevCursor;

private:
	D2D1_RECT_F LatitudeTextRect() const;
	D2D1_RECT_F LatitudeSliderRect() const;
	D2D1_RECT_F TiltTextRect() const;
	D2D1_RECT_F TiltSliderRect() const;
	D2D1_RECT_F AdjustDayTextRect() const;
	D2D1_RECT_F AdjustDaySliderRect() const;

public:
	void Init(const Graphics& graphics);
	void Render(const Graphics& graphics) const;
	void Resize(int width, int height);

	void LButtonDownEvent(int x, int y, WPARAM flags);
	void LButtonUpEvent(int x, int y, WPARAM flags);
	void MouseMoveEvent(int x, int y, WPARAM flags);

	void UpdateSettings(std::function<void(Data&)> updater);

	inline const Data& Settings() const { return m_settings; }
};