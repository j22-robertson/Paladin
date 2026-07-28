SamplerState sampler_default : register(s0);


struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal: NORMAL;
    float3 tangent: TANGENT;
    float3 bitangent: BITANGENT;
    float4 color: COLOR;
    float2 uv : TEXCOORD0;
};

struct PerFrameData{
    uint frame_index;

    uint ib_buffer_bytes;
    uint ib_heap_index;
    uint draw_id_buffer_bytes;
    uint draw_id_heap_index;

    uint global_vertices_heap_index;
    uint global_indices_heap_index;
    uint mesh_descriptor_heap_index;
};

struct MeshDescriptor{
    uint vertex_offset;
    uint instance_start_offset;
    uint index_count;


    uint albedo_id;
    uint normal_map_id;
    uint roughess_id;
    uint metal_id;
    uint ao_id;
};

struct DrawIdentifier{
    uint current;
};
//struct MeshData{uint material_id;};
ConstantBuffer<DrawIdentifier> draw_id : register(b1);
ConstantBuffer<PerFrameData> frame_data : register(b3);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3x3 TBN = float3x3(input.tangent,input.bitangent,input.normal);

    ByteAddressBuffer mesh_descriptor_heap = ResourceDescriptorHeap[frame_data.mesh_descriptor_heap_index];
    MeshDescriptor descriptor = mesh_descriptor_heap.Load<MeshDescriptor>(draw_id.current*32);


    Texture2D<float4> normal_map = ResourceDescriptorHeap[NonUniformResourceIndex(descriptor.normal_map_id)];
    float3 t_normal = normal_map.Sample(sampler_default,input.uv).rgb*2.0-1.0;

    float3 world_normal = normalize(mul(t_normal,TBN));

    float3 light_direction = normalize(float3(0.5, 1.0, -0.5));

    float diff = saturate(dot(world_normal, light_direction));
    float3 ambient = 0.2;

    Texture2D<float4> albedo = ResourceDescriptorHeap[NonUniformResourceIndex(descriptor.albedo_id)];

    float3 sample_col = albedo.Sample(sampler_default,input.uv).rgb;
    float3 final_color = sample_col * (diff+ambient);
    return float4(final_color,1.0);
}