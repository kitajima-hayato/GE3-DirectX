#include "Particle.hlsli"

struct Material
{
    float4 color;
    int enableLighting;
    float4x4 uvTransform;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};


cbuffer MaterialBuffer : register(b0)
{
    Material gMaterial;
};

Texture2D<float4> gTexture : register(t0); // SRVのregisterはt
SamplerState gSampler : register(s0); // Samplerのregisterはs



PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = float4(0.0f, 0.0f, 0.0f, 1.0f); // 初期化

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    output.color = gMaterial.color * textureColor * input.color;
    
   
    if (output.color.a == 0.0f)
    {
        discard;
    }
    //else
    //{
    //    output.color = gMaterial.color * textureColor;
    //}
    return output;
}
