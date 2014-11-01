
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
    float2 invTexel;
    float2 offset;
    float invTexelDiag;
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
    float4 color : COLOR0;
};

#define XFORM(p) (mul(mul(mul(float4(p, 1), localToWorld), worldToView), viewToClip))

float2 cross2D(float2 v1, float2 v2)
{
    return (v1.x * v2.y) - (v1.y * v2.x);
}

float2 intersect(float2 p1, float2 p2, float2 p3, float2 p4)
{
    float2 a = p2 - p1;
    float2 b = p4 - p3;
    float2 c = p3 - p1;

    float2 ab = cross2D(a, b);
    float t = dot(cross2D(c, b), ab) / dot(ab, ab);

    return p1 + a * t;
}

VSOutput MainVS(VSInput input)
{
    VSOutput vs = (VSOutput)0;

    float4 curPos = XFORM(input.curPos);
    float4 nextPos = XFORM(input.nextPos);
    float4 prevPos = XFORM(input.prevPos);

    float2 vNext = curPos.xy / curPos.w - nextPos.xy / nextPos.w;
    float2 vPrev = prevPos.xy / prevPos.w - curPos.xy / curPos.w;

    float area = 0.5 * cross2D(vPrev, vNext);

    float2 nNext = normalize(vNext.yx * float2(-1, 1)) * invTexel * sign(area) * invTexelDiag;
    float2 nPrev = normalize(vPrev.yx * float2(-1, 1)) * invTexel * sign(area) * invTexelDiag;

    float2 curOffsetN = curPos.xy / curPos.w + nNext;
    float2 nextOffset = nextPos.xy / nextPos.w + nNext;
    float2 curOffsetP = curPos.xy / curPos.w + nPrev;
    float2 prevOffset = prevPos.xy / prevPos.w + nPrev;

    float2 pt = intersect(curOffsetN, nextOffset, curOffsetP, prevOffset);

    vs.position = curPos;
    vs.position.xy = pt * curPos.w;
    vs.color = meshColor;

    return vs;
}

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    return vs.color;
}
