#include "Object3d.hlsli"


struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
struct Material
{
    float color;
    float enableLighting;
};

struct DirectionalLight
{
    float4 color;       //ライトの色
    float3 direction;   //ライトの向き
    float intensity;        //輝度
};
ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerのregisterはs
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gMaterial.color;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
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

