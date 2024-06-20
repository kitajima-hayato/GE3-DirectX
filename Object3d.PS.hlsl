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
struct DirectionalLight
{
    float32_t4 color;//ライトの色
    float32_t3 direction;//ライトの向き
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
    if (gMaterial.enableLighting != 0)//Lightingをする場合
    {
        float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
        output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
    }else//Lightingしない場合前回までと同じ処理
    {
        output.color = gMaterial.color * textureColor;
    }
    return output;
}

