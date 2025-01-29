#include "Particle.hlsli"
struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};
struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};
struct ParticleForGPU
{
    float4x4 WVP;
    float4x4 World;
    float4 color;
};

StructuredBuffer<ParticleForGPU> gPatricle : register(t0);


VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gPatricle[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.color = gPatricle[instanceId].color;
    return output;
}

