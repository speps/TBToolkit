
struct VSOutput
{
    float4 position : SV_POSITION;
};

VSOutput MainVS(float3 pos : POSITION)
{
    VSOutput vs = (VSOutput)0;
    vs.position = float4(pos, 1);
    return vs;
}

cbuffer ScreenConstants : register(c0)
{
    float4 quadColor;
}

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    return quadColor;
}
