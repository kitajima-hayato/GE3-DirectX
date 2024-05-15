
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
    output.position = input.postion;
    return output;
}