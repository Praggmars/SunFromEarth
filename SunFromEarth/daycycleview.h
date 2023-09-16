#pragma once

#include "shaderviewbase.h"
#include "math/position.hpp"

class DayCycleView : public ShaderViewBase
{
public:
	struct ShaderData
	{
		mth::float4 dayColor;
		mth::float4 civilTwilightColor;
		mth::float4 nauticalTwilightColor;
		mth::float4 astronomicalTwilightColor;
		mth::float4 nightColor;
		float latitude;
		float tilt;
		mth::float2 resolution;
		float dayAdjust;
		float padding[3];

		ShaderData();
	};

private:
	ComPtr<ID3D11Buffer> m_shaderParamBuffer;
	ComPtr<ID2D1SolidColorBrush> m_timePresentBrush;

	float m_tilt;
	float m_latitude;
	float m_dayAdjust;

public:
	virtual void Init(const Graphics& graphics, int width, int height) override;

	virtual void Render(const Graphics& graphics) const override;
	void RenderTimePresenter(const Graphics& graphics, mth::float2 cursor) const;

	void SetTilt(float tilt);
	void SetLatitude(float latitude);
	void SetDayAdjust(float dayAdjust);
};
