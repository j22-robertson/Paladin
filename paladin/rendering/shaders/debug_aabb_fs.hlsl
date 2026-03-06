
SamplerState sampler_default : register(s0);
struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};


float4 main(VS_OUTPUT input) : SV_TARGET
{
    return float4(0.9,0.0,0.9,1.0);
}