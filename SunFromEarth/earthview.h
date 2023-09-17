#pragma once

#include "shaderviewbase.h"
#include "math/position.hpp"
#include <chrono>
#include <functional>

class EarthView : public ShaderViewBase
{
public:
	struct ShaderData
	{
		mth::float2 coordOffset;
		mth::float2 coordScaler;
		mth::float3 lightDir;
		float planeDist;
		mth::float3 planetPos;
		float planetRad;
		mth::float3 eyePosition;
		float latitude;
		mth::float4x4 viewRotation;
		mth::float4x4 planetTranform;

		ShaderData();
	};

private:
	ComPtr<ID3D11ShaderResourceView> m_dayMap;
	ComPtr<ID3D11ShaderResourceView> m_nightMap;
	ComPtr<ID3D11ShaderResourceView> m_cloudMap;
	ComPtr<ID3D11ShaderResourceView> m_normalMap;
	ComPtr<ID3D11ShaderResourceView> m_specularMap;
	ComPtr<ID3D11Buffer> m_shaderParamBuffer;

	mth::float3 m_eyePosition;
	mth::float3x3 m_viewRotation;
	float m_tilt;
	float m_latitude;
	bool m_latitudeShowing;
	mth::float2 m_prevCursor;
	std::chrono::steady_clock::time_point m_startTime;
	std::function<void(float latitude)> m_onLatitudeChange;

private:
	mth::float2 CoordScaler() const;
	mth::float2 CoordOffset() const;
	bool GetLatitude(int x, int y, float& latitude) const;
	void HandleLatitudeSelector(int x, int y);

public:
	EarthView();
	virtual void Init(const Graphics& graphics, int width, int height) override;

	virtual void LButtonDownEvent(int x, int y, WPARAM flags) override;
	virtual void LButtonUpEvent(int x, int y, WPARAM flags) override;
	virtual void MouseMoveEvent(int x, int y, WPARAM flags) override;

	virtual void Render(const Graphics& graphics) const override;

	void SetTilt(float tilt);
	void SetLatitude(float latitude);
	void AssignLatitudeChange(std::function<void(float latitude)> onLatitudeChange);
};
