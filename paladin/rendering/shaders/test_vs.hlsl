struct VS_INPUT
{
    float3 pos : POSITION;
    //float3 normal: NORMAL;
    //float3 tangent: TANGENT;
    //float3 bitangent: BITANGENT;
    //float4 color: COLOR;
    //float2 uv : TEXCOORD;


    //row_major float4x4 transform: TRANSFORM;
};

cbuffer Camera : register(b0)
{
    row_major float4x4 view;
    row_major float4x4 projection;
}

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;


    float4x4 model = float4x4(
    1,0,0,0.0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    );

   // float4 world_pos = mul(model, float4(input.pos, 1.0f));
    float4x4 view_proj = mul(projection,view);
    output.pos = mul(model,float4(input.pos,1.0));
    output.color = float4(abs(input.pos), 1.0f);
    return output;

}