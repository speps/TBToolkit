
struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSOutput MainVS(float3 pos : POSITION, float2 uv : TEXCOORD0)
{
    VSOutput vs = (VSOutput)0;
    vs.position = float4(pos, 1);
    vs.uv = uv;
    return vs;
}

cbuffer ScreenConstants : register(c0)
{
    float4 quadColor;
}

Texture2D<float4> mainTexture : register(t0);
sampler mainSampler : register(s0);

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    return mainTexture.Sample(mainSampler, vs.uv) * quadColor;
}
