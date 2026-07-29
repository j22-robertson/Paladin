struct Vertex
{
    float3 pos;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float4 color;
    float2 uv;
};

cbuffer Camera : register(b0)
{
	float4x4 view;
	float4x4 projection;
	float4x4 view_projection;
	float4x4 inv_view_projection;
}

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
	uint index_start_offset;
	uint index_count;

	uint albedo_id;
	uint normal_map_id;
	uint roughess_id;
	uint metal_id;
	uint ao_id;
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

struct InstanceOffset{
	uint offset;
};

struct DrawIdentifier{
	uint current;
};

Vertex LoadVertex(ByteAddressBuffer buffer, uint byteOffset)
{
    Vertex v;
    v.pos       = asfloat(buffer.Load3(byteOffset + 0));   // Bytes 0..11
    v.normal    = asfloat(buffer.Load3(byteOffset + 12));  // Bytes 12..23
    v.tangent   = asfloat(buffer.Load3(byteOffset + 24));  // Bytes 24..35
    v.bitangent = asfloat(buffer.Load3(byteOffset + 36));  // Bytes 36..47
    v.color     = asfloat(buffer.Load4(byteOffset + 48));  // Bytes 48..63
    v.uv        = asfloat(buffer.Load2(byteOffset + 64));  // Bytes 64..71
    return v;
}

ConstantBuffer<DrawIdentifier> draw_id : register(b1);
ConstantBuffer<InstanceOffset> instance_offset : register(b2);
ConstantBuffer<PerFrameData> frame_data : register(b3);


VS_OUTPUT main(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

	ByteAddressBuffer mesh_descriptor_heap = ResourceDescriptorHeap[frame_data.mesh_descriptor_heap_index];
    uint mesh_descriptor_offset = draw_id.current*32;

    MeshDescriptor mesh_descriptor = mesh_descriptor_heap.Load<MeshDescriptor>(mesh_descriptor_offset);

    ByteAddressBuffer vertex_heap = ResourceDescriptorHeap[frame_data.global_vertices_heap_index];
    ByteAddressBuffer index_heap = ResourceDescriptorHeap[frame_data.global_indices_heap_index];

    uint index_location = mesh_descriptor.index_start_offset + vertex_id;
    uint index_byte_offset = index_location * 4;

    uint real_vertex_index = index_heap.Load(index_byte_offset);
    uint vertex_location = mesh_descriptor.vertex_offset + real_vertex_index;
    uint vertex_byte_offset = vertex_location * 72;

    Vertex vertex = LoadVertex(vertex_heap, vertex_byte_offset);

    ByteAddressBuffer instance_buffer= ResourceDescriptorHeap[frame_data.ib_heap_index];
	ByteAddressBuffer identifier_buffer= ResourceDescriptorHeap[frame_data.draw_id_heap_index];
	// Get address of identifier for current instance (instance id(Current instance being drawn, based on instance count) + offset(Start location) * 4 bytes for uint step size)
	uint id_address = frame_data.draw_id_buffer_bytes*frame_data.frame_index+((instance_id+instance_offset.offset)*4);
	// Load identifier
	uint transform_id = identifier_buffer.Load(id_address);
	// Use identifier as an offset to get correct model matrix
    uint model_address = frame_data.ib_buffer_bytes*frame_data.frame_index + (transform_id * 128);
	// Get Inverse model matrix using model address + matrix offset of 64 bytes
    uint inv_model_address = model_address+64;

    //Opposite order because byte address buffer implicitly transposes to row_major
    float4x4 model = instance_buffer.Load<float4x4>(model_address);
	// Classical transformations for tangent/normal/bitangent
    float4x4 inv_model = instance_buffer.Load<float4x4>(inv_model_address);

    float3x3 inv_transpose_model = (float3x3)transpose(inv_model);
    float3 tangent = normalize(float3(mul(vertex.tangent,inv_transpose_model)));
    float3 normal =  normalize(float3(mul(vertex.normal,inv_transpose_model)));
    float3 bitangent =  normalize(float3(mul(vertex.bitangent,inv_transpose_model)));
    float4 world_pos = mul(float4(vertex.pos, 1.0f), model);
    //Opposite order because byte address buffer implicitly transposes to row_major

	output.pos = mul(view_projection,world_pos);
    output.color = vertex.color;
    output.normal = normal;
    output.tangent = tangent;
    output.bitangent = bitangent;
    output.uv = vertex.uv;
    return output;

}