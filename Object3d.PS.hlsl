#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t4 enableLighting;
};
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerのregisterはs
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    float32_t4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    if (gMaterial.enableLighting!=0)
    {
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;

    }
   else{
        output.color = gMaterial.color * textureColor;
    }
    
    return output;
}

