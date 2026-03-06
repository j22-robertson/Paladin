
cbuffer Camera : register(b0)
{
     float4x4 view;
     float4x4 projection;
}

static const float3 box[8]={
    float3(-0.5f,-0.5f,-0.5f),
    float3(0.5f, -0.5f,-0.5f),
    float3(0.5f, 0.5f, -0.5f),
    float3(-0.5f, 0.5f, -0.5f),
    float3(-0.5f, -0.5f, 0.5f),
    float3(0.5f, -0.5f, 0.5f),
    float3(0.5f, 0.5f, 0.5f),
    float3(-0.5f, 0.5f, 0.5f)
};

static const uint indices[24]={
    0,1,
    1,2,
    2,3,
    3,0,

    4,5,
    5,6,
    6,7,
    7,4,

    0,4,
    1,5,
    2,6,
    3,7
};


struct InstanceData
{
    uint heap_index;
    uint frame_offset;
    uint instance_offset;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};

struct BoundingBoxData
{
    uint aabb_index;
};
ConstantBuffer<InstanceData> instance : register(b2);
ConstantBuffer<BoundingBoxData> bounding_box : register(b3);

VS_OUTPUT main(uint vertex_id: SV_VertexID,uint instance_id : SV_InstanceID)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    ByteAddressBuffer instance_buffer= ResourceDescriptorHeap[instance.heap_index];
    uint model_address = instance.frame_offset + ((instance_id+instance.instance_offset) * 128);

    float3 position = box[indices[vertex_id]];
    //Opposite order because byte addres buffer implicitly transposes to row_major
    float4x4 model = instance_buffer.Load<float4x4>(model_address);
    float4 world_pos = mul(float4(position, 1.0f), model);
    //Opposite order because byte addres buffer implicitly transposes to row_major

    float4 view_pos = mul(view, world_pos);
    output.pos = mul(projection,view_pos);
    return output;
}