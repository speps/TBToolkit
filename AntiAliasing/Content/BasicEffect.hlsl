
cbuffer ViewBuffer : register(b0)
{
    float4x4 worldToView;
    float4x4 viewToClip;
};

cbuffer WorldBuffer : register(b1)
{
    float3 mainLightDir;
};

cbuffer ObjectBuffer : register(b2)
{
    float4x4 localToWorld;
};

Texture2D<float4> mainTexture : register(t0);
sampler mainSampler : register(s0);

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
};

VSOutput MainVS(float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD0)
{
    VSOutput vs = (VSOutput)0;
    vs.position = mul(mul(mul(pos, localToWorld), worldToView), viewToClip);
    vs.normal = mul(float4(normal, 0), localToWorld).xyz;
    vs.uv = uv;
    return vs;
}

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    float light = max(dot(vs.normal, mainLightDir), 0.0f);
    float4 color = mainTexture.Sample(mainSampler, vs.uv);
    return float4(color.rgb, 1);
}
