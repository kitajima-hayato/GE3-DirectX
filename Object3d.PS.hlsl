
struct PixelShaderOutput
{
    float32_t4 color : SV_Target0;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;
    output.color = float32_t4(1.0, 1.0, 1.0, 1.0);
    return output;
}