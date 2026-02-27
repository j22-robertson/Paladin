
SamplerState sampler_default : register(s0);



struct Material{
    uint albedo_idx;
    uint normal_idx;
    uint roughness_idx;
    uint metallic_idx;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal: NORMAL;
    float3 tangent: TANGENT;
    float3 bitangent: BITANGENT;
    float4 color: COLOR;
    float2 uv : TEXCOORD0;
};

struct MeshData{uint material_id;};
ConstantBuffer<MeshData> mesh_data : register(b1);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3 norm = normalize(input.normal);
    float3 light_direction = normalize(float3(0.5, 1.0, -0.5));

    float diff = saturate(dot(norm, light_direction));
    float3 ambient = 0.2;

    Texture2D<float4> albedo = ResourceDescriptorHeap[NonUniformResourceIndex(mesh_data.material_id)];

    float3 sample_col = albedo.Sample(sampler_default,input.uv).rgb;
    float3 final_color = sample_col * (diff+ambient);
    return float4(final_color,1.0);
}