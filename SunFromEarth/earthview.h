#pragma once

#include "shaderviewbase.h"
#include "math/position.hpp"
#include <chrono>

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

	std::chrono::steady_clock::time_point m_startTime;
	ShaderData m_shaderData;
	float m_tilt;

protected:
	virtual void ResizeCB() override;

public:
	virtual void Init(const Graphics& graphics, int width, int height) override;

	void Update();
	virtual void Render(const Graphics& graphics) const override;

	bool GetLatitude(int x, int y, float& latitude) const;
	void SetTilt(float tilt);
};
