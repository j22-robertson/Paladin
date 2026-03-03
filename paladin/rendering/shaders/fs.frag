
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

//struct MeshData{uint material_id;};
struct InstanceData
{
    uint heap_index;
    uint frame_offset;
    uint albedo_index;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3x3 TBN = float3x3(input.tangent,input.bitangent,input.normal);

    Texture2D<float4> normal_map = ResourceDescriptorHeap[NonUniformResourceIndex(5+2)];
    float3 t_normal = normal_map.Sample(sampler_default,input.uv).rgb*2.0-1.0;

    float3 world_normal = normalize(mul(t_normal,TBN));

    float3 light_direction = normalize(float3(0.5, 1.0, -0.5));

    float diff = saturate(dot(world_normal, light_direction));
    float3 ambient = 0.2;

    Texture2D<float4> albedo = ResourceDescriptorHeap[NonUniformResourceIndex(5)];

    float3 sample_col = albedo.Sample(sampler_default,input.uv).rgb;
    float3 final_color = sample_col * (diff+ambient);
    return float4(final_color,1.0);
}