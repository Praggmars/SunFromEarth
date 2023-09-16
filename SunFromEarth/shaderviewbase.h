#pragma once

#include "graphics.h"
#include "math/position.hpp"

class ShaderViewBase
{
protected:
	ComPtr<ID3D11ComputeShader> m_shader;
	ComPtr<ID3D11UnorderedAccessView> m_shaderTarget;
	ComPtr<ID2D1Bitmap> m_outputImage;

	D2D1_RECT_F m_rect;

protected:
	void CreateImageResources(const Graphics& graphics, int width, int height);
	void LaunchShader(const Graphics& graphics) const;
	virtual void ResizeCB();

public:
	virtual void Init(const Graphics& graphics, int width, int height) = 0;
	void SetRect(const Graphics& graphics, D2D1_RECT_F rect);

	virtual void Render(const Graphics& graphics) const = 0;

	inline D2D1_RECT_F Rectangle() const { return m_rect; }
	inline mth::vec2<int> Resolution() const { return mth::vec2<int>(static_cast<int>(std::ceil(m_rect.right - m_rect.left)), static_cast<int>(std::ceil(m_rect.bottom - m_rect.top))); }
	inline ID2D1Bitmap* OutputImage() const { return m_outputImage.Get(); }
};
