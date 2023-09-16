RWTexture2D<float4> target : register(u0);

cbuffer ShaderData : register(b0)
{
    float4 dayColor;
    float4 civilTwilightColor;
    float4 nauticalTwilightColor;
    float4 astronomicalTwilightColor;
    float4 nightColor;
    float latitude;
    float tilt;
    float2 resolution;
    float dayAdjust;
};

#define PI 3.141592654f

float3 RotateX(float3 v, float a)
{
    float ca = cos(a);
    float sa = sin(a);
    return float3(v.x, ca * v.y - sa * v.z, sa * v.y + ca * v.z);
}

float3 RotateY(float3 v, float a)
{
    float ca = cos(a);
    float sa = sin(a);
    return float3(ca * v.x - sa * v.z, v.y, sa * v.x + ca * v.z);
}

float3 RotateZ(float3 v, float a)
{
    float ca = cos(a);
    float sa = sin(a);
    return float3(ca * v.x - sa * v.y, sa * v.x + ca * v.y, v.z);
}

float4 Color(float2 coord)
{
    const float yearRad = (coord.x + 11.0f / 365.0f) * PI * 2.0f;
    const float dayRotation = coord.y * PI * 2.0f + dayAdjust * yearRad;
	
    const float3 up = float3(-1.0f, 0.0f, 0.0f);
    const float3 latitudeUp = RotateZ(up, latitude);
    const float3 dayUp = RotateY(latitudeUp, dayRotation);
    const float3 tiltUp = RotateZ(dayUp, tilt);
        
    const float sun = dot(tiltUp, float3(cos(yearRad), 0.0f, sin(yearRad)));
    if (sun > 0.0f)
        return dayColor;
    
    const float cosSun = cos(sun);
    if (cosSun > cos(6.0f / 180.0f * PI))
        return civilTwilightColor;
    if (cosSun > cos(12.0f / 180.0f * PI))
        return nauticalTwilightColor;
    if (cosSun > cos(18.0f / 180.0f * PI))
        return astronomicalTwilightColor;
    return nightColor;
}

[numthreads(16, 16, 1)]
void main(int3 dtID : SV_DispatchThreadID)
{
    target[dtID.xy] = Color(float2(float(dtID.x) / (resolution.x - 1.0f), 1.0f - float(dtID.y) / (resolution.y - 1.0f)));
}