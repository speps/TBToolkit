
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
    float2 invPixel;
    float2 offset;
};

struct VSInput
{
    float3 curPos : POSITION0;
    float3 nextPos : POSITION1;
    float3 prevPos : POSITION2;
};

struct VSOutput
{
    float4 position : SV_POSITION;
};

static const float Diag = 0.70710678f;

float4 xform(float3 p)
{
    return mul(mul(mul(float4(p, 1), localToWorld), worldToView), viewToClip);
}

float cross2D(float2 v1, float2 v2)
{
    return (v1.x * v2.y) - (v1.y * v2.x);
}

VSOutput MainVS(VSInput input)
{
    VSOutput vs = (VSOutput)0;

    // Transform positions to projective space and then to raster space
    float4 curPos = xform(input.curPos);
    curPos.xy /= curPos.w;
    curPos.xy /= invPixel;
    float4 nextPos = xform(input.nextPos);
    nextPos.xy /= nextPos.w;
    nextPos.xy /= invPixel;
    float4 prevPos = xform(input.prevPos);
    prevPos.xy /= prevPos.w;
    prevPos.xy /= invPixel;

    float2 vNext = nextPos.xy - curPos.xy;
    float2 vPrev = curPos.xy - prevPos.xy;

    float areaSign = sign(cross2D(vPrev, vNext));

    float2 nNext = normalize(float2(vNext.y, -vNext.x)) * areaSign / Diag;
    float2 nPrev = normalize(float2(vPrev.y, -vPrev.x)) * areaSign / Diag;

    float2 pt = curPos.xy + (vPrev / dot(vPrev, nNext) + vNext / dot(vNext, nPrev));
    vs.position = float4(pt * curPos.w * invPixel, curPos.zw);

    return vs;
}

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    return meshColor;
}
