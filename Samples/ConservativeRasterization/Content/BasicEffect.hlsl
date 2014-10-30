
cbuffer ViewBuffer : register(b0)
{
    float4x4 worldToView;
    float4x4 viewToClip;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 localToWorld;
};

cbuffer TempBuffer : register(b2)
{
    float4 meshColor;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput MainVS(float3 pos : POSITION)
{
    VSOutput vs = (VSOutput)0;
    vs.position = mul(mul(mul(float4(pos, 1), localToWorld), worldToView), viewToClip);
    return vs;
}

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    return meshColor;
}
