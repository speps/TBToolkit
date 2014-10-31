
cbuffer ViewBuffer : register(b0)
{
    float4x4 worldToView;
    float4x4 viewToClip;
};

cbuffer ObjectBuffer : register(b1)
{
    float4x4 localToWorld;
};

cbuffer WorldBuffer : register(b2)
{
    float3 mainLightDir;
};

Texture2D<float4> mainTexture : register(t0);
sampler mainSampler : register(s0);

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 nextPosition : POSITION1;
    float3 prevPosition : POSITION2;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 color : COLOR0;
};

float4 getWorldPos(float3 pos)
{
    return mul(float4(pos, 1), localToWorld);
}

VSOutput MainVS(VSInput input)
{
    float4 worldPos = getWorldPos(input.position);
    float4 worldPos1 = getWorldPos(input.nextPosition);
    float4 worldPos2 = getWorldPos(input.prevPosition);

    float minLen = min(min(length(worldPos1.xyz - worldPos.xyz), length(worldPos2.xyz - worldPos1.xyz)), length(worldPos.xyz - worldPos2.xyz));
    float alpha = saturate(minLen / 50.0f);

    VSOutput vs = (VSOutput)0;
    vs.position = mul(mul(worldPos, worldToView), viewToClip);
    vs.normal = mul(float4(input.normal, 0), localToWorld).xyz;
    vs.uv = input.uv;
    vs.color = alpha.xxx;
    return vs;
}

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    float light = max(dot(vs.normal, mainLightDir), 0.0f);
    float4 color = mainTexture.Sample(mainSampler, vs.uv);
    //return float4(color.rgb, 1);
    return float4(vs.color, 1);
}
