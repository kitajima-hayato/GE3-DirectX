#include "Object3d.hlsli"


struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
struct Material
{
    float32_t color;
    int32_t enableLighting;
};
struct TransformationMatrix
{
    float32_t4x4 WVP;
    float32_t4x4 World;
};
ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerのregisterはs

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
   
    output.color = gMaterial.color * textureColor;
    return output;
}

