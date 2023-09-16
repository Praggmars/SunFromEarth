Texture2D daymap : register(t0);
Texture2D nightmap : register(t1);
Texture2D cloudmap : register(t2);
Texture2D normalmap : register(t3);
Texture2D specularmap : register(t4);
SamplerState ss : register(s0);
RWTexture2D<float4> target : register(u0);

cbuffer ShaderData : register(b0)
{
    float2 coordOffset;
    float2 coordScaler;
    float3 lightDir;
    float planeDist;
    float3 planetPos;
    float planetRad;
    float4x4 planetTranform;
};

#define PI 3.141592654f

bool ApproachSphere(float3 src, float3 dir, float3 sphCenter, float sphRadius, out float t)
{
    const float3 p = src - sphCenter;
    const float a = dot(dir, dir);
    const float b = dot(p, dir);
    const float c = dot(p, p) - sphRadius * sphRadius;
    const float d = b * b - a * c;
    if (d < 0.0f)
        return false;
    
    t = (-sqrt(d) - b) / a;
    return true;
}

float4 EarthColor(float3 sphereCoord)
{
    float3 normal = sphereCoord;
    float brightness = saturate(dot(normal, lightDir));
    
    sphereCoord = mul((float3x3) planetTranform, sphereCoord);
    float2 uv = float2(
        atan2(sphereCoord.x, -sphereCoord.z) / (2.0f * PI) + 0.5f,
        atan2(sphereCoord.y, length(sphereCoord.xz)) / -PI + 0.5f
    );
        
    float3 dayColor = daymap.SampleLevel(ss, uv, 0).rgb;
    float3 nightColor = nightmap.SampleLevel(ss, uv, 0).rgb;
    return float4(dayColor * brightness + nightColor * saturate(1.0f - 4.0f * brightness), 1.0f);
}

[numthreads(16, 16, 1)]
void main(int3 dtID : SV_DispatchThreadID)
{   
    float3 src = float3(0.0f, 0.0f, 0.0f);
    float3 dir = normalize(float3(float2(dtID.x, dtID.y) * coordScaler + coordOffset, planeDist));
    
    float4 color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    float t;
    if (ApproachSphere(src, dir, planetPos, planetRad, t))
        color = EarthColor(normalize(src + dir * t - planetPos));
    
    target[dtID.xy] = color;
}