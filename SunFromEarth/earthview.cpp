#include "earthview.h"
#include <future>

static const mth::float3 LIGHT_DIRECTION = mth::float3(-1.0f, 0.0f, 0.0f);
static const float PLANE_DISTANCE = 70.0f;
static const mth::float3 PLANET_POSITION = mth::float3(0.0f, 0.0f, 0.0f);
static const float PLANET_RADIUS = 1.0f;

EarthView::ShaderData::ShaderData()
	: planeDist{}
	, planetRad{} {}

mth::float2 EarthView::CoordScaler() const
{
	return mth::float2(2.0f / (m_rect.bottom - m_rect.top), -2.0f / (m_rect.bottom - m_rect.top));
}

mth::float2 EarthView::CoordOffset() const
{
	return mth::float2((m_rect.left - m_rect.right) / (m_rect.bottom - m_rect.top), 1.0f);
}

bool EarthView::GetLatitude(int x, int y, float& latitude) const
{
	mth::float3 src = m_eyePosition;
	mth::float3 dir = mth::float3(mth::float2(static_cast<float>(x) - m_rect.left, static_cast<float>(y) - m_rect.top) * CoordScaler() + CoordOffset(), PLANE_DISTANCE).Normalized() * m_viewRotation;
	mth::float3 p = src - PLANET_POSITION;
	float a = dir.LengthSquare();
	float b = p.Dot(dir);
	float c = p.LengthSquare() - PLANET_RADIUS * PLANET_RADIUS;
	float d = b * b - a * c;
	if (d < 0.0f)
		return false;

	float t = (-std::sqrt(d) - b) / a;
	mth::float3 normal = (src + dir * t - PLANET_POSITION).Normalized();
	mth::float3 sphereCoord = mth::float3x3::RotationZ(-m_tilt) * normal;
	latitude = std::atan2(sphereCoord.y, mth::float2(sphereCoord.x, sphereCoord.z).Length());

	return true;
}

void EarthView::HandleLatitudeSelector(int x, int y)
{
	float latitude = 0.0f;
	if (GetLatitude(x, y, latitude))
	{
		if (m_onLatitudeChange)
		{
			m_latitude = latitude;
			m_onLatitudeChange(latitude);
		}
	}
}

EarthView::EarthView()
	: m_viewRotation{}
	, m_tilt{}
	, m_latitude{}
	, m_latitudeShowing{} {}

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
	m_shaderParamBuffer = graphics.CreateConstBuffer(sizeof(ShaderData));

	m_dayMap = dayMapFuture.get();
	m_nightMap = nightMapFuture.get();
	m_cloudMap = cloudMapFuture.get();
	m_normalMap = normalMapFuture.get();
	m_specularMap = specularMapFuture.get();

	m_startTime = std::chrono::steady_clock::now();
	m_eyePosition = mth::float3(0.0f, 0.0f, -75.0f);
	m_viewRotation = mth::float3x3::Identity();
}

void EarthView::LButtonDownEvent(int x, int y, WPARAM flags)
{
	HandleLatitudeSelector(x, y);
	m_latitudeShowing = true;
}

void EarthView::LButtonUpEvent(int x, int y, WPARAM flags)
{
	m_latitudeShowing = false;
}

void EarthView::MouseMoveEvent(int x, int y, WPARAM flags)
{
	const mth::float2 cursor = mth::float2(static_cast<float>(x), static_cast<float>(y));

	if (flags & MK_LBUTTON)
		HandleLatitudeSelector(x, y);
	if (flags & MK_RBUTTON)
	{
		const float sensitivity = 0.008f;
		m_viewRotation = 
			mth::float3x3::RotationX((m_prevCursor.y - cursor.y) * sensitivity) *
			mth::float3x3::RotationY((m_prevCursor.x - cursor.x) * sensitivity) *
			m_viewRotation;
		m_eyePosition = mth::float3(0.0f, 0.0f, -75.0f) * m_viewRotation;
	}
	m_prevCursor = cursor;
}

void EarthView::Render(const Graphics& graphics) const
{
	ShaderData shaderData{};
	shaderData.lightDir = LIGHT_DIRECTION;
	shaderData.planeDist = PLANE_DISTANCE;
	shaderData.planetPos = PLANET_POSITION;
	shaderData.planetRad = PLANET_RADIUS;
	shaderData.coordOffset = CoordOffset();
	shaderData.coordScaler = CoordScaler();
	shaderData.eyePosition = m_eyePosition;
	shaderData.latitude = m_latitudeShowing ? m_latitude : std::numeric_limits<float>::quiet_NaN();
	shaderData.viewRotation = mth::float4x4(m_viewRotation);
	shaderData.planetTranform = mth::float4x4::RotationZ(m_tilt) * mth::float4x4::RotationY(std::chrono::duration<float>(std::chrono::steady_clock::now() - m_startTime).count() * -0.25f);
	ID3D11Buffer* cBuffer = m_shaderParamBuffer.Get();
	graphics.UpdateConstBuffer(cBuffer, &shaderData, sizeof(shaderData));
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

void EarthView::SetTilt(float tilt)
{
	m_tilt = tilt;
}

void EarthView::SetLatitude(float latitude)
{
	m_latitude = latitude;
}

void EarthView::AssignLatitudeChange(std::function<void(float latitude)> onLatitudeChange)
{
	m_onLatitudeChange = onLatitudeChange;
}
