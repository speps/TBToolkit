#include <Content/FXAA311.hlsl>

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
sampler mainSampler : register(s0);

float4 MainPS(in VSOutput vs) : SV_TARGET0
{
    FxaaTex inputFXAATex = { mainSampler, mainTexture };
    return FxaaPixelShader(
        vs.uv,                                  // FxaaFloat2 pos,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),     // FxaaFloat4 fxaaConsolePosPos,
        inputFXAATex,                           // FxaaTex tex,
        inputFXAATex,                           // FxaaTex fxaaConsole360TexExpBiasNegOne,
        inputFXAATex,                           // FxaaTex fxaaConsole360TexExpBiasNegTwo,
        invTexel,                               // FxaaFloat2 fxaaQualityRcpFrame,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),     // FxaaFloat4 fxaaConsoleRcpFrameOpt,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),     // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),     // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
        0.75f,                                  // FxaaFloat fxaaQualitySubpix,
        0.166f,                                 // FxaaFloat fxaaQualityEdgeThreshold,
        0.0833f,                                // FxaaFloat fxaaQualityEdgeThresholdMin,
        0.0f,                                   // FxaaFloat fxaaConsoleEdgeSharpness,
        0.0f,                                   // FxaaFloat fxaaConsoleEdgeThreshold,
        0.0f,                                   // FxaaFloat fxaaConsoleEdgeThresholdMin,
        FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)      // FxaaFloat fxaaConsole360ConstDir,
    );
}
