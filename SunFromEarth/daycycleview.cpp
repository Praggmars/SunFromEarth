#include "daycycleview.h"
#include <iomanip>
#include <sstream>
#include <algorithm>

DayCycleView::ShaderData::ShaderData()
	: latitude{}
	, tilt{}
	, dayAdjust{}
	, padding{} {}

DayCycleView::DayCycleView()
	: m_tilt{}
	, m_latitude{}
	, m_dayAdjust{}
	, m_cursorShowing{} {}

void DayCycleView::Init(const Graphics& graphics, int width, int height)
{
	m_tilt = -23.5f / 180.0f * mth::pi;
	m_latitude = 45.0f / 180.0f * mth::pi;
	m_dayAdjust = 0.0f;
	m_shader = graphics.CreateComputeShader(L"data/daycycleshader.cso");
	CreateImageResources(graphics, width, height);
	m_shaderParamBuffer = graphics.CreateConstBuffer(sizeof(ShaderData));
	m_timePresentBrush = graphics.CreateBrush(mth::float4(1.0f));
}

void DayCycleView::LButtonDownEvent(int x, int y, WPARAM flags)
{
	m_cursor = mth::float2(static_cast<float>(x), static_cast<float>(y));
	m_cursorShowing = true;
}

void DayCycleView::RButtonDownEvent(int x, int y, WPARAM flags)
{
	m_cursorShowing = false;
}

void DayCycleView::MouseMoveEvent(int x, int y, WPARAM flags)
{
	if (flags & MK_LBUTTON)
	{
		const mth::float2 cursor = mth::float2(static_cast<float>(x), static_cast<float>(y));
		if (IsOnRect(cursor, m_rect))
			m_cursor = cursor;
	}
}

void DayCycleView::Render(const Graphics& graphics) const
{
	ShaderData shaderData{};
	shaderData.dayColor = mth::float4(0.6902f, 0.8549f, 0.9098f, 1.0f);
	shaderData.civilTwilightColor = mth::float4(0.5882f, 0.7333f, 0.7882f, 1.0f);
	shaderData.nauticalTwilightColor = mth::float4(0.4353f, 0.5608f, 0.6039f, 1.0f);
	shaderData.astronomicalTwilightColor = mth::float4(0.3451f, 0.4157f, 0.4392f, 1.0f);
	shaderData.nightColor = mth::float4(0.1843f, 0.2706f, 0.3020f, 1.0f);
	shaderData.latitude = m_latitude;
	shaderData.tilt = m_tilt;
	shaderData.resolution = mth::float2(m_rect.right - m_rect.left, m_rect.bottom - m_rect.top);
	shaderData.dayAdjust = m_dayAdjust;
	ID3D11Buffer* cBuffer = m_shaderParamBuffer.Get();
	graphics.UpdateConstBuffer(cBuffer, &shaderData, sizeof(shaderData));
	graphics.Context3D()->CSSetConstantBuffers(0, 1, &cBuffer);
	LaunchShader(graphics);
	graphics.Context2D()->DrawBitmap(m_outputImage.Get(), m_rect);
	if (m_cursorShowing)
		RenderTimePresenter(graphics, m_cursor);
}

void DayCycleView::RenderTimePresenter(const Graphics& graphics, mth::float2 cursor) const
{
	const int monthLengths[] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
	const char* monthNames[] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	int month = 0;
	int day = std::clamp(static_cast<int>((cursor.x - m_rect.left) / (m_rect.right - m_rect.left) * 365.0f) + 1, 1, 365);
	for (; month < 12; day -= monthLengths[month], ++month)
		if (day <= monthLengths[month])
			break;

	int second = std::clamp(static_cast<int>((1.0f - (cursor.y - m_rect.top) / (m_rect.bottom - m_rect.top + 1.0f)) * 24.0f * 60.0f * 60.0f), 0, 24 * 60 * 60 - 1);
	int minute = second / 60;
	int hour = minute / 60;
	minute %= 60;
	second %= 60;

	std::wstringstream ss;
	ss << monthNames[month] << ' '
		<< std::setfill(L'0') << std::setw(2) << day << ' ' 
		<< std::setfill(L'0') << std::setw(2) << hour << ':'
		<< std::setfill(L'0') << std::setw(2) << minute << ':'
		<< std::setfill(L'0') << std::setw(2) << second;
	const std::wstring str = ss.str();
	const float strHeight = 20.0f;
	const float strWidth = strHeight * 0.55f * static_cast<float>(str.length());
	D2D1_RECT_F rect;
	rect.left = m_rect.right - cursor.x < strWidth + 10.0f ? cursor.x - strWidth - 5.0f : cursor.x + 5.0f;
	rect.right = rect.left + strWidth;
	rect.top = cursor.y - m_rect.top < strHeight + 10.0f ? cursor.y + 5.0f + strHeight : cursor.y - 5.0f;
	rect.bottom = rect.top - strHeight;

	graphics.Context2D()->DrawLine(D2D1::Point2F(m_rect.right, cursor.y), D2D1::Point2F(m_rect.left, cursor.y), m_timePresentBrush.Get(), 2.0f);
	graphics.Context2D()->DrawLine(D2D1::Point2F(cursor.x, m_rect.top), D2D1::Point2F(cursor.x, m_rect.bottom), m_timePresentBrush.Get(), 2.0f);
	graphics.RenderText(str.c_str(), rect, m_timePresentBrush.Get());
}

void DayCycleView::SetTilt(float tilt)
{
	m_tilt = tilt;
}

void DayCycleView::SetLatitude(float latitude)
{
	m_latitude = latitude;
}

void DayCycleView::SetDayAdjust(float dayAdjust)
{
	m_dayAdjust = dayAdjust;
}
