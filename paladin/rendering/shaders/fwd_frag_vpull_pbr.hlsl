SamplerState sampler_default : register(s0);

cbuffer Camera : register(b0)
{
	float4x4 view;
	float4x4 projection;
	float4x4 view_projection;
	float4x4 inv_view_projection;
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
    uint roughness_id;
    uint metal_id;
    uint ao_id;
};

struct DrawIdentifier{
    uint current;
};

float TrowbridgeReitzD(float roughness, float NdotH){
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH2 = NdotH*NdotH;
    float denominator = 3.14159265359 * pow(NdotH2*(a2 - 1.0) + 1.0,2.0);
    denominator = max(denominator,0.0000001);
    return (a2/denominator);
}

float SBG(float roughness,float3 N, float3 X){
    float NdotX = max(dot(N,X),0.0);
    float r = (roughness + 1.0);
    float k = (r*r)/8.0;
    float denominator = NdotX*(1.0-k)+k;
    denominator= max(denominator,0.0000001);
    return NdotX/denominator;
}

float SmithG(float roughness, float3 normal, float3 view_direction, float3 light_direction){
    return SBG(roughness,normal,view_direction)*SBG(roughness,normal,light_direction);
}

ConstantBuffer<DrawIdentifier> draw_id : register(b1);
ConstantBuffer<PerFrameData> frame_data : register(b3);

float4 main(VS_OUTPUT input) : SV_TARGET
{
    float3x3 TBN = float3x3(input.tangent,input.bitangent,input.normal);

    float3 camera_position = inv_view_projection[3].xyz;
    float3 view_direction = normalize(camera_position.xyz-input.pos.xyz);

    ByteAddressBuffer mesh_descriptor_heap = ResourceDescriptorHeap[frame_data.mesh_descriptor_heap_index];
    MeshDescriptor descriptor = mesh_descriptor_heap.Load<MeshDescriptor>(draw_id.current*32);

    Texture2D<float4> normal_map = ResourceDescriptorHeap[NonUniformResourceIndex(descriptor.normal_map_id)];
    float3 t_normal = normal_map.Sample(sampler_default,input.uv).rgb *2.0-1.0;
    float3 world_normal = normalize(mul(t_normal,TBN));


    float3 light_direction = normalize(float3(0.5, 1.0, -0.5));


    Texture2D<float4> roughness_text = ResourceDescriptorHeap[NonUniformResourceIndex(descriptor.roughness_id)];
    float roughness = roughness_text.Sample(sampler_default,input.uv).g;
    Texture2D<float4> metallic_text = ResourceDescriptorHeap[NonUniformResourceIndex(descriptor.metal_id)];
    float metallic = metallic_text.Sample(sampler_default,input.uv).b;

    Texture2D<float4> albedo_tex = ResourceDescriptorHeap[NonUniformResourceIndex(descriptor.albedo_id)];
    float3 albedo = albedo_tex.Sample(sampler_default,input.uv).rgb;
    //float diff = saturate(dot(world_normal, light_direction));
    float3 ambient = 0.1 * albedo.rgb;

    float3 halfway = normalize(view_direction + light_direction);
    float NdotH = max(0.0, dot(world_normal, halfway));

    float NdotV = max(0.0, dot(world_normal, view_direction));
    float NdotL = saturate(dot(world_normal, light_direction));



    float3 lambertian = albedo.xyz/3.14159265359;

    float3 f0 = float3(0.04,0.04,0.04);

    f0 = lerp(f0,albedo,float3(metallic,metallic,metallic));
    float3 fresnel_schlick = f0+(float3(1.0,1.0,1.0)-f0)*pow(1.0-max(dot(halfway,view_direction),0.0),5.0);

    float D = TrowbridgeReitzD(roughness,NdotH);
    float G = SmithG(roughness,world_normal, view_direction, light_direction);
    float3 numerator = D*G*fresnel_schlick;
    float denominator = 4.0*NdotV * NdotL;
    denominator = max(denominator,0.0000001);
    float3 specular = numerator/denominator;
    float3 kd = 1.0 - fresnel_schlick;
    kd *= 1.0-metallic;

   float3 brdf = kd *  albedo.xyz/3.14159265359 * specular;
   float3 Lo = brdf * (float3(1.0,1.0,1.0)*15.0 *NdotL) ;
   float3 cl = ambient + Lo;

   float3 cl2 = cl / (cl + float3(1.0, 1.0, 1.0));
   float3 cl3 = pow(cl2, (float3(1.0/2.2, 1.0/2.2, 1.0/2.2)));

   //
    //float3 final_color = sample_col * (diff+ambient);
    return float4(cl3,1.0);
}