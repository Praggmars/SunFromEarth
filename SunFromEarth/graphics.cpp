#include "graphics.h"
#include <wincodec.h>
#include <vector>
#include <stdexcept>
#include <fstream>

#pragma comment (lib, "windowscodecs.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dwrite.lib")

static std::vector<char> LoadFile(const wchar_t* filename)
{
	std::ifstream infile(filename, std::ios::binary | std::ios::ate);
	std::vector<char> buffer(infile.tellg());
	infile.seekg(0);
	infile.read(buffer.data(), buffer.size());
	return buffer;
}

bool IsOnRect(mth::float2 point, D2D1_RECT_F rect)
{
	return rect.right >= point.x && rect.left <= point.x &&
		rect.bottom >= point.y && rect.top <= point.y;
}

void ThrowIfFailed(HRESULT result, const char* message)
{
	if (FAILED(result))
		throw std::runtime_error(message);
}

void Graphics::Init(HWND target)
{
	ThrowIfFailed(CoInitialize(nullptr));

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0
	};
	ThrowIfFailed(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &m_device3D, nullptr, &m_context3D), "Failed to create D3D device");
	ComPtr<IDXGIDevice> dxgiDevice;
	ThrowIfFailed(m_device3D.As(&dxgiDevice), "Failed to query DXGI device");
	ComPtr<ID2D1Factory1> d2dFactory;
	ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, IID_PPV_ARGS(&d2dFactory)), "Failed to create D2D factory");
	ThrowIfFailed(d2dFactory->CreateDevice(dxgiDevice.Get(), &m_device2D), "Failed to create D2D device");
	ThrowIfFailed(m_device2D->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_context2D), "Failed to create D2D device context");

	RECT rect;
	GetClientRect(target, &rect);

	ComPtr<IDXGIFactory> factory;
	ThrowIfFailed(CreateDXGIFactory(IID_PPV_ARGS(&factory)), "Failed to greate DXGI factory");

	DXGI_SWAP_CHAIN_DESC swapChainDesc{};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = max(1, rect.right);
	swapChainDesc.BufferDesc.Height = max(1, rect.bottom);
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = target;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;
	ThrowIfFailed(factory->CreateSwapChain(m_device3D.Get(), &swapChainDesc, &m_swapChain), "Failed to create swap chain");

	ComPtr<IDXGISurface> backBufferSurface;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferSurface)), "Failed to get back buffer surface");
	ThrowIfFailed(m_context2D->CreateBitmapFromDxgiSurface(backBufferSurface.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_targetBitmap), "Failed to create render tarteg bitmap");
	m_context2D->SetTarget(m_targetBitmap.Get());

	ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory7), static_cast<IUnknown**>(&m_writeFactory)));
	ComPtr<IDWriteTextFormat> textFormat;
	ThrowIfFailed(m_writeFactory->CreateTextFormat(L"Consolas", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"en", &textFormat));
	ThrowIfFailed(textFormat.As(&m_font));

	ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)(&m_imagingFactory)));
}

void Graphics::Resize(int width, int height)
{
	m_context2D->SetTarget(nullptr);
	m_targetBitmap.Reset();
	ThrowIfFailed(m_swapChain->ResizeBuffers(2, max(1, width), max(1, height), DXGI_FORMAT_UNKNOWN, 0), "Failed to resize swapchain buffer");

	ComPtr<IDXGISurface> backBufferSurface;
	ThrowIfFailed(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferSurface)), "Failed to get back buffer surface");
	ThrowIfFailed(m_context2D->CreateBitmapFromDxgiSurface(backBufferSurface.Get(), D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &m_targetBitmap), "Failed to create render tarteg bitmap");
	m_context2D->SetTarget(m_targetBitmap.Get());
}

void Graphics::BeginDraw() const
{
	m_context2D->BeginDraw();
}

void Graphics::EndDraw() const
{
	m_context2D->EndDraw();
	m_swapChain->Present(1, 0);
}

ComPtr<ID3D11Buffer> Graphics::CreateConstBuffer(unsigned size) const
{
	ComPtr<ID3D11Buffer> buffer;

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.ByteWidth = size;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = 0;
	ThrowIfFailed(m_device3D->CreateBuffer(&bufferDesc, nullptr, &buffer), "Failed to create shader const buffer");

	return buffer;
}

ComPtr<ID3D11ShaderResourceView> Graphics::LoadTexture(const wchar_t* filename) const
{
	ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	ComPtr<IWICBitmapDecoder> decoder;
	ComPtr<IWICBitmapFrameDecode> frame;
	ComPtr<IWICFormatConverter> converter;
	std::vector<BYTE> pixels;
	UINT width, height;

	ThrowIfFailed(m_imagingFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder));
	ThrowIfFailed(decoder->GetFrame(0, &frame));
	ThrowIfFailed(m_imagingFactory->CreateFormatConverter(&converter));
	ThrowIfFailed(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom));
	ThrowIfFailed(converter->GetSize(&width, &height));
	pixels.resize(4ull * width * height);
	ThrowIfFailed(converter->CopyPixels(nullptr, width * 4, width * height * 4, pixels.data()));

	ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = pixels.data();
	subresourceData.SysMemPitch = width * 4;
	subresourceData.SysMemSlicePitch = 0;
	ThrowIfFailed(m_device3D->CreateTexture2D(&textureDesc, &subresourceData, &texture));

	D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc{};
	resourceViewDesc.Format = textureDesc.Format;
	resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	resourceViewDesc.Texture2D.MostDetailedMip = 0;
	resourceViewDesc.Texture2D.MipLevels = 1;
	ThrowIfFailed(m_device3D->CreateShaderResourceView(texture.Get(), &resourceViewDesc, &shaderResourceView));

	return shaderResourceView;
}

ComPtr<ID3D11ComputeShader> Graphics::CreateComputeShader(const wchar_t* filename) const
{
	ComPtr<ID3D11ComputeShader> shader;
	const std::vector<char> shaderByteCode = LoadFile(filename);
	ThrowIfFailed(m_device3D->CreateComputeShader(shaderByteCode.data(), shaderByteCode.size(), nullptr, &shader));
	return shader;
}

ComPtr<ID2D1SolidColorBrush> Graphics::CreateBrush(mth::float4 color) const
{
	ComPtr<ID2D1SolidColorBrush> brush;
	ThrowIfFailed(m_context2D->CreateSolidColorBrush(D2D1::ColorF(color.x, color.y, color.z, color.w), &brush));
	return brush;
}

void Graphics::UpdateConstBuffer(ID3D11Buffer* buffer, const void* data, unsigned size) const
{
	D3D11_MAPPED_SUBRESOURCE resource;
	if (SUCCEEDED(m_context3D->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource)))
	{
		std::memcpy(resource.pData, data, size);
		m_context3D->Unmap(buffer, 0);
	}
}

void Graphics::RenderText(const wchar_t* text, D2D1_RECT_F rect, ID2D1Brush* brush) const
{
	m_context2D->DrawTextW(text, static_cast<UINT32>(std::wcslen(text)), m_font.Get(), rect, brush);
}
