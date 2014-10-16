
struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

VSOutput MainVS(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD0)
{
    VSOutput vs = (VSOutput)0;
    vs.position = pos;
    vs.normal = normal;
    vs.uv = uv;
    return vs;
}

cbuffer SSAABuffer : register(b0)
{
    float2 invTexel;
}

Texture2D<float4> mainTexture : register(t0);
Texture2D<float4> offsetTexture : register(t1);
sampler mainSampler : register(s0);

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    float4 main = mainTexture.Sample(mainSampler, vs.uv);
    float4 offset = offsetTexture.Sample(mainSampler, vs.uv + float2(-0.5f, -0.5f) * invTexel);
    return (main + offset) * 0.5f;
}
