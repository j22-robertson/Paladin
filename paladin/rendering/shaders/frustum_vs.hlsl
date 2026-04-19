
cbuffer Camera : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 view_projection;
    float4x4 inv_view_projection;
}

static const float3 box[8]={
    float3(-1.f, -1.f, 0.f), // Near face
        float3( 1.f, -1.f, 0.f),
        float3( 1.f,  1.f, 0.f),
        float3(-1.f,  1.f, 0.f),
        float3(-1.f, -1.f, 1.f), // Far face
        float3( 1.f, -1.f, 1.f),
        float3( 1.f,  1.f, 1.f),
        float3(-1.f,  1.f, 1.f)
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


struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};
VS_OUTPUT main(uint vertex_id: SV_VertexID)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    float4 ndc = float4(box[indices[vertex_id]],1.0);
    ndc.xy*=0.95;
    float4 world_pos = mul(inv_view_projection,ndc);
    world_pos /= world_pos.w;
    float4 view_pos = mul(view, world_pos);
    view_pos.z += 2.0;
    output.pos = mul(projection, view_pos);
    return output;
}