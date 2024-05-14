float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return pos;
} 
struct VertexShaderOutput
{
    float32_t4 position:SV_POSITION;
};
struct VertexShaderInput
{
    float32_t4 postion : POSTION0;
};
VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = input.postion;
    return output;
}