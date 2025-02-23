#include "Particle.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float32_t4x4 uvTransform;
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
Texture2D<float4> gTexture : register(t0); //SRVのregisterはt
SamplerState gSampler : register(s0); //Samplerのregisterはs
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
    

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
 
    output.color = gMaterial.color * textureColor*input.color;
    
    if (output.color.a == 0.0)
    {
        discard;
    }
    if (textureColor.a <= 0.5)
    {
        discard;
    }

       
            
    return output;
    
    /*if (gMaterial.enableLighting != 0)
   {
    
       if (gMaterial.enableLighting == 1)
       {
           //通常
           float cos = saturate(dot(normalize(input.normal), -gDirectionalLight.direction));
           //output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
           output.color.rgb = gMaterial.color.rgb * textureColor.rgb * gDirectionalLight.color.rgb * cos * gDirectionalLight.intensity;
           output.color.a = gMaterial.color.a * textureColor.a;
        

       }
       if (gMaterial.enableLighting == 2)
       {
           //HalfLambert
           float NdotL = dot(normalize(input.normal), -gDirectionalLight.direction);
           float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
           output.color = gMaterial.color * textureColor * gDirectionalLight.color * cos * gDirectionalLight.intensity;
     
         
       }
    
        
   }
   else
   {
       output.color = gMaterial.color * textureColor;
   }
   if (textureColor.a <= 0.5)
   {
       discard;
   }
   if (textureColor.a == 0.0)
   {
       discard;
   }
    
   if (output.color.a == 0.0)
   {
       discard;
   }
         
   return output;*/
    
}

