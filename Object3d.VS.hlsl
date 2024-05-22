struct TransformationMatrix
{
    float32_t4x4 WVP;
};
ConstantBuffer<TransformationMatrix> gTransformationMatrix : register(b0);
struct VertexShaderOutput
{
    float32_t4 position : SV_POSITION;
};
struct VertexShaderInput
{
    float32_t4 postion : POSITION;
};
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.postion, gTransformationMatrix.WVP);
    return output;
}