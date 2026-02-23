//Texture2D texture_array : register(t0);
//SamplerState sampler_default : register(s0)

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
    float3 normal: NORMAL;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3 norm = normalize(input.normal);
    float3 light_direction = normalize(float3(0.5, 1.0, -0.5));

    float diff = saturate(dot(norm, light_direction));
    float3 ambient = 0.2;
    float3 final_color = input.color.rgb * (diff + ambient);
    return float4(final_color,1.0);
}