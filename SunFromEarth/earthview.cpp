#include "earthview.h"
#include <future>

EarthView::ShaderData::ShaderData()
	: planeDist{}
	, planetRad{} {}

void EarthView::ResizeCB()
{
	m_shaderData.coordOffset.x = (m_rect.left - m_rect.right) / (m_rect.bottom - m_rect.top);
	m_shaderData.coordOffset.y = 1.0f;
	m_shaderData.coordScaler.x = 2.0f / (m_rect.bottom - m_rect.top);
	m_shaderData.coordScaler.y = -2.0f / (m_rect.bottom - m_rect.top);
}

void EarthView::Init(const Graphics& graphics, int width, int height)
{
	ComPtr<ID3D11ShaderResourceView>(*loader)(const Graphics&, const wchar_t*) = [](const Graphics& graphics, const wchar_t* filename)->ComPtr<ID3D11ShaderResourceView> {
		return graphics.LoadTexture(filename);
		};
	std::future<ComPtr<ID3D11ShaderResourceView>> dayMapFuture = std::async(loader, graphics, L"data/8k_earth_daymap.jpg");
	std::future<ComPtr<ID3D11ShaderResourceView>> nightMapFuture = std::async(loader, graphics, L"data/8k_earth_nightmap.jpg");
	std::future<ComPtr<ID3D11ShaderResourceView>> cloudMapFuture = std::async(loader, graphics, L"data/8k_earth_clouds.jpg");
	std::future<ComPtr<ID3D11ShaderResourceView>> normalMapFuture = std::async(loader, graphics, L"data/8k_earth_normal_map.tif");
	std::future<ComPtr<ID3D11ShaderResourceView>> specularMapFuture = std::async(loader, graphics, L"data/8k_earth_specular_map.tif");

	m_shader = graphics.CreateComputeShader(L"data/earthshader.cso");
	CreateImageResources(graphics, width, height);
	m_shaderParamBuffer = graphics.CreateConstBuffer(sizeof(m_shaderData));

	m_tilt = -23.5f / 180.0f * mth::pi;

	m_shaderData.lightDir = mth::float3(-3.0f, 0.0f, -2.0f).Normalized();
	m_shaderData.planeDist = 70.0f;
	m_shaderData.planetPos = mth::float3(0.0f, 0.0f, 75.0f);
	m_shaderData.planetRad = 1.0f;

	m_dayMap = dayMapFuture.get();
	m_nightMap = nightMapFuture.get();
	m_cloudMap = cloudMapFuture.get();
	m_normalMap = normalMapFuture.get();
	m_specularMap = specularMapFuture.get();

	m_startTime = std::chrono::steady_clock::now();
}

void EarthView::Update()
{
	const float totalTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - m_startTime).count();
	m_shaderData.planetTranform = mth::float4x4::RotationZ(m_tilt) * mth::float4x4::RotationY(totalTime * -0.25f);
}

void EarthView::Render(const Graphics& graphics) const
{
	ID3D11Buffer* cBuffer = m_shaderParamBuffer.Get();
	graphics.UpdateConstBuffer(cBuffer, &m_shaderData, sizeof(m_shaderData));
	graphics.Context3D()->CSSetConstantBuffers(0, 1, &cBuffer);

	ID3D11ShaderResourceView* textures[] = {
		m_dayMap.Get(),
		m_nightMap.Get(),
		m_cloudMap.Get(),
		m_normalMap.Get(),
		m_specularMap.Get()
	};
	graphics.Context3D()->CSSetShaderResources(0, ARRAYSIZE(textures), textures);
	
	LaunchShader(graphics);

	graphics.Context2D()->DrawBitmap(m_outputImage.Get(), m_rect);
}

bool EarthView::GetLatitude(int x, int y, float& latitude) const
{
	mth::float3 src = mth::float3(0.0f, 0.0f, 0.0f);
	mth::float3 dir = mth::float3(mth::float2(static_cast<float>(x), static_cast<float>(y)) * m_shaderData.coordScaler + m_shaderData.coordOffset, m_shaderData.planeDist).Normalized();
	mth::float3 p = src - m_shaderData.planetPos;
	float a = dir.LengthSquare();
	float b = p.Dot(dir);
	float c = p.LengthSquare() - m_shaderData.planetRad * m_shaderData.planetRad;
	float d = b * b - a * c;
	if (d < 0.0f)
		return false;

	float t = (-sqrt(d) - b) / a;
	mth::float3 normal = (src + dir * t - m_shaderData.planetPos).Normalized();
	mth::float3 sphereCoord = mth::float3x3::RotationZ(-m_tilt) * normal;
	latitude = atan2(sphereCoord.y, mth::float2(sphereCoord.x, sphereCoord.z).Length());

	return true;
}

void EarthView::SetTilt(float tilt)
{
	m_tilt = tilt;
}
