#pragma once

#include <d2d1_3.h>
#include <d3d11_4.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include "math/position.hpp"

#if 0
template <typename T>
class ComPtr
{
	T* m_ptr;

public:
	ComPtr() : m_ptr{} {}
	ComPtr(const ComPtr<T>& ptr) : m_ptr{ptr.m_ptr} 
	{
		if (m_ptr)
			m_ptr->AddRef();
	}
	ComPtr(ComPtr<T>&& ptr) noexcept: m_ptr{ptr.m_ptr}
	{
		ptr.m_ptr = nullptr;
	}
	~ComPtr() { Reset(); }

	void Reset()
	{
		if (m_ptr)
		{
			m_ptr->Release();
			m_ptr = nullptr;
		}
	}

	ComPtr<T>& operator=(const ComPtr<T>& ptr)
	{
		Reset();
		m_ptr = ptr.m_ptr;
		if (m_ptr)
			m_ptr->AddRef();
		return *this;
	}

	ComPtr<T>& operator=(ComPtr<T>&& ptr) noexcept
	{
		Reset();
		m_ptr = ptr.m_ptr;
		ptr.m_ptr = nullptr;
		return *this;
	}

	template <typename T2>
	HRESULT As(T2 ptr) const
	{
		return m_ptr->QueryInterface(IID_PPV_ARGS(ptr));
	}

	T* Get() const { return m_ptr; }
	T** GetAddressOf() { return &m_ptr; }
	T** operator&() { Reset(); return &m_ptr; }
	T* operator->() const { return m_ptr; }
};
#else
#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif

bool IsOnRect(mth::float2 point, D2D1_RECT_F rect);

void ThrowIfFailed(HRESULT result, const char* message = "Unknown error");

class Graphics
{
	ComPtr<ID3D11Device> m_device3D;
	ComPtr<ID3D11DeviceContext> m_context3D;
	ComPtr<ID2D1Device> m_device2D;
	ComPtr<ID2D1DeviceContext> m_context2D;
	ComPtr<IDXGISwapChain> m_swapChain;
	ComPtr<ID2D1Bitmap1> m_targetBitmap;
	ComPtr<IDWriteFactory7> m_writeFactory;
	ComPtr<IDWriteTextFormat3> m_font;
	ComPtr<IWICImagingFactory> m_imagingFactory;

public:
	void Init(HWND target);
	void Resize(int width, int height);
	void BeginDraw() const;
	void EndDraw() const;

	ComPtr<ID3D11Buffer> CreateConstBuffer(unsigned size) const;
	ComPtr<ID3D11ShaderResourceView> LoadTexture(const wchar_t* filename) const;
	ComPtr<ID3D11ComputeShader> CreateComputeShader(const wchar_t* filename) const;

	ComPtr<ID2D1SolidColorBrush> CreateBrush(mth::float4 color) const;

	void UpdateConstBuffer(ID3D11Buffer* buffer, const void* data, unsigned size) const;
	void RenderText(const wchar_t* text, D2D1_RECT_F rect, ID2D1Brush* brush) const;

	inline ID3D11Device* Device3D() const { return m_device3D.Get(); }
	inline ID3D11DeviceContext* Context3D() const { return m_context3D.Get(); }
	inline ID2D1Device* Device2D() const { return m_device2D.Get(); }
	inline ID2D1DeviceContext* Context2D() const { return m_context2D.Get(); }
};