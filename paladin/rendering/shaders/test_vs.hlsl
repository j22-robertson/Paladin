struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal: NORMAL;
    float3 tangent: TANGENT;
    float3 bitangent: BITANGENT;
    float4 color: COLOR;
    float2 uv : TEXCOORD0;


    //row_major float4x4 transform: TRANSFORM;
};

cbuffer Camera : register(b0)
{
     float4x4 view;
     float4x4 projection;
}

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 normal: NORMAL;
    float3 tangent: TANGENT;
    float3 bitangent: BITANGENT;
    float4 color: COLOR;
    float2 uv : TEXCOORD0;
};

struct InstanceData
{
    uint heap_index;
    uint frame_offset;
};
ConstantBuffer<InstanceData> instance : register(b2);

VS_OUTPUT main(VS_INPUT input, uint instance_id : SV_InstanceID)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    ByteAddressBuffer instance_buffer= ResourceDescriptorHeap[instance.heap_index];
    uint model_address = instance.frame_offset + (instance_id * 128);
    uint inv_model_address = model_address+64;

    //Opposite order because byte addres buffer implicitly transposes to row_major
    float4x4 model = instance_buffer.Load<float4x4>(model_address);
    float4x4 inv_model = instance_buffer.Load<float4x4>(inv_model_address);
    float3x3 inv_transpose_model = (float3x3)transpose(inv_model);
    float3 tangent = normalize(float3(mul(input.tangent,inv_transpose_model)));
    float3 normal =  normalize(float3(mul(input.normal,inv_transpose_model)));
    float3 bitangent =  normalize(float3(mul(input.bitangent,inv_transpose_model)));
    float4 world_pos = mul(float4(input.pos, 1.0f), model);
    //Opposite order because byte addres buffer implicitly transposes to row_major

    float4 view_pos = mul(view, world_pos);
    output.pos = mul(projection,view_pos);

    output.color = input.color;
    output.normal = normal;
    output.tangent = tangent;
    output.bitangent = bitangent;
    output.uv = input.uv;
    return output;

}