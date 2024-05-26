
struct VertexShaderOutput
{
    float32_t4 position : SV_POSION;
    float32_t2 texcoord : TEXCOORD0;
};

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};
