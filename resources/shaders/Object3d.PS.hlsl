#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
    float32_t shininess;
};
struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};
struct DirectionalLight
{
    float32_t4 color; //ライトの色
    float32_t3 direction; //ライトの向き
    float intensity; //輝度
};
ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float32_t4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerのregisterはs
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<Camera> gCamera : register(b2);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    
    float32_t3 toEye = normalize(gCamera.worldPosition - input.worldPosition);
    float32_t3 reflectLight = reflect(normalize(gDirectionalLight.direction), normalize(normalize(input.normal)));
    
    float RdotE = dot(reflectLight, toEye);
    float specularPow = pow(saturate(RdotE), gMaterial.shininess);
    
    float cos = saturate(dot(normalize(input.normal), normalize(-gDirectionalLight.direction)));
    
    // 拡散反射
    float32_t3 diffuse = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
    // 鏡面反射
    float32_t3 specular = gDirectionalLight.color.rgb * gDirectionalLight.intensity * specularPow * float32_t3(1.0f, 1.0f, 1.0f);
    // 拡散反射 + 鏡面反射
    output.color.rgb = diffuse + specular;
    // 透明度
    output.color.a = gMaterial.color.a * textureColor.a;
    
    if (gMaterial.enableLighting != 0)
    {
        if (gMaterial.enableLighting == 1)
        {
            // 通常
            output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
            output.color.a = gMaterial.color.a * textureColor.a;
        }
        // if (gMaterial.enableLighting == 2)
        // {
        //     // HalfLambert
        //     float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
        //     cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        //     output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
        // }
    }
    else
    {
        output.color = gMaterial.color * textureColor;
    }
    return output;
}
