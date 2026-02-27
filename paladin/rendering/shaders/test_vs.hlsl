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

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;

    float4 view_pos = mul(view, float4(input.pos,1.0));


   // float4 world_pos = mul(model, float4(input.pos, 1.0f));
    output.pos = mul(projection,view_pos);
    output.color = input.color;
    output.normal = input.normal;
    output.tangent = input.tangent;
    output.bitangent = input.bitangent;
    output.uv = input.uv;
    return output;

}