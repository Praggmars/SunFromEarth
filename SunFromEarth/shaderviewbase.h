#pragma once

#include "graphics.h"
#include "math/position.hpp"
#include "eventhandler.h"

class ShaderViewBase : public EventHandler
{
protected:
	ComPtr<ID3D11ComputeShader> m_shader;
	ComPtr<ID3D11UnorderedAccessView> m_shaderTarget;
	ComPtr<ID2D1Bitmap> m_outputImage;

protected:
	void CreateImageResources(const Graphics& graphics, int width, int height);
	void LaunchShader(const Graphics& graphics) const;

public:
	virtual void Init(const Graphics& graphics, int width, int height) = 0;
	virtual void SetRect(const Graphics& graphics, D2D1_RECT_F rect) override;

	virtual void Render(const Graphics& graphics) const = 0;

	inline ID2D1Bitmap* OutputImage() const { return m_outputImage.Get(); }
};
