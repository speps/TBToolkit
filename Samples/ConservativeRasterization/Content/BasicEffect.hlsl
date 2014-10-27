
cbuffer ViewBuffer : register(b0)
{
    float4x4 worldToView;
    float4x4 viewToClip;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 localToWorld;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

VSOutput MainVS(float3 pos : POSITION, float4 color : COLOR0)
{
    VSOutput vs = (VSOutput)0;
    vs.position = mul(mul(mul(float4(pos, 1), localToWorld), worldToView), viewToClip);
    vs.color = color;
    return vs;
}

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    return vs.color;
}
