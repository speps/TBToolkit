
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

Texture2D<float4> mainTexture : register(t0);
sampler mainSampler : register(s0);

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    return mainTexture.Sample(mainSampler, vs.uv);
}
