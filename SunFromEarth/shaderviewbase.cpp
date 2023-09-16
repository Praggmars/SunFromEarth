#include "shaderviewbase.h"

void ShaderViewBase::CreateImageResources(const Graphics& graphics, int width, int height)
{
	ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.Width = max(1, width);
	textureDesc.Height = max(1, height);
	ThrowIfFailed(graphics.Device3D()->CreateTexture2D(&textureDesc, nullptr, &texture));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	ThrowIfFailed(graphics.Device3D()->CreateUnorderedAccessView(texture.Get(), &uavDesc, &m_shaderTarget));

	ComPtr<IDXGISurface2> dxgiSurface;
	ThrowIfFailed(texture.As(&dxgiSurface));
	D2D1_BITMAP_PROPERTIES bmpProp = D2D1::BitmapProperties(D2D1::PixelFormat(textureDesc.Format, D2D1_ALPHA_MODE_PREMULTIPLIED));
	ThrowIfFailed(graphics.Context2D()->CreateSharedBitmap(__uuidof(IDXGISurface2), dxgiSurface.Get(), &bmpProp, &m_outputImage));
}

void ShaderViewBase::LaunchShader(const Graphics& graphics) const
{
	ID3D11UnorderedAccessView* uav = m_shaderTarget.Get();
	graphics.Context3D()->CSSetUnorderedAccessViews(0, 1, &uav, nullptr);

	graphics.Context3D()->CSSetShader(m_shader.Get(), nullptr, 0);

	mth::vec2<int> resolution = Resolution();
	graphics.Context3D()->Dispatch(
		(static_cast<unsigned>(resolution.x) + 15) / 16,
		(static_cast<unsigned>(resolution.y) + 15) / 16,
		1);
}

void ShaderViewBase::ResizeCB()
{
}

void ShaderViewBase::SetRect(const Graphics& graphics, D2D1_RECT_F rect)
{
	m_rect = rect;
	const mth::vec2<int> resolution = Resolution();
	CreateImageResources(graphics, resolution.x, resolution.y);
	ResizeCB();
}
