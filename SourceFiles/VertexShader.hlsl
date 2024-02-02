struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float pad;
};

cbuffer PerFrameCB : register(b0)
{
    DirectionalLight directionalLight;
};

cbuffer PerObjectCB : register(b1)
{
    matrix World;
    uint4 uv_indices[8];
    uint4 texture_indices[8];
    uint4 blend_flags[8];
    uint4 texture_types[8];
    uint num_uv_texture_pairs;
    uint object_id;
    float pad1[2];
};

cbuffer PerCameraCB : register(b2)
{
    matrix View;
    matrix Projection;
    float3 cam_position;
    float pad[1];
};

struct VertexInputType
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 tex_coords0 : TEXCOORD0;
    float2 tex_coords1 : TEXCOORD1;
    float2 tex_coords2 : TEXCOORD2;
    float2 tex_coords3 : TEXCOORD3;
    float2 tex_coords4 : TEXCOORD4;
    float2 tex_coords5 : TEXCOORD5;
    float2 tex_coords6 : TEXCOORD6;
    float2 tex_coords7 : TEXCOORD7;
    float3 tangent : TANGENT;
    float3 bitangent : TANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float4 lightingColor : COLOR0;
    float2 tex_coords0 : TEXCOORD0;
    float2 tex_coords1 : TEXCOORD1;
    float2 tex_coords2 : TEXCOORD2;
    float2 tex_coords3 : TEXCOORD3;
    float2 tex_coords4 : TEXCOORD4;
    float2 tex_coords5 : TEXCOORD5;
    float2 tex_coords6 : TEXCOORD6;
    float2 tex_coords7 : TEXCOORD7;
    float terrain_height : TEXCOORD8;
    float3x3 TBN : TEXCOORD9;
};


PixelInputType main(VertexInputType input)
{
    PixelInputType output;

    // Transform the vertex position to clip space
    float4 worldPosition = mul(float4(input.position, 1.0f), World);
    float4 viewPosition = mul(worldPosition, View);
    output.position = mul(viewPosition, Projection);

    // Transform the normal using the inverse transpose of the world matrix
    output.normal = normalize(mul(input.normal, World));

    // Pass the texture coordinates to the pixel shader
    output.tex_coords0 = input.tex_coords0;
    output.tex_coords1 = input.tex_coords1;
    output.tex_coords2 = input.tex_coords2;
    output.tex_coords3 = input.tex_coords3;
    output.tex_coords4 = input.tex_coords4;
    output.tex_coords5 = input.tex_coords5;
    output.tex_coords6 = input.tex_coords6;
    output.tex_coords7 = input.tex_coords7;
    output.terrain_height = worldPosition.y;

    // Lighting computation
    if (input.tangent.x == 0.0f && input.tangent.y == 0.0f && input.tangent.z == 0.0f ||
		input.bitangent.x == 0.0f && input.bitangent.y == 0.0f && input.bitangent.z == 0.0f)
    {
        float3 normal = normalize(mul(input.normal, (float3x3) World)); // Normalize normal after transformation

        // Ensure directionalLight.direction is normalized
        float3 lightDir = normalize(-directionalLight.direction);
        float NdotL = max(dot(normal, lightDir), 0.0);

        float4 ambientComponent = directionalLight.ambient;
        float4 diffuseComponent = directionalLight.diffuse * NdotL;

        // Calculate view direction and ensure normalization
        float3 viewDirection = normalize(cam_position - worldPosition.xyz);

        // Compute half vector and ensure normalization
        float3 halfVector = normalize(lightDir + viewDirection);
        float NdotH = max(dot(normal, halfVector), 0.0);

        float shininess = 80.0; // Shininess factor
        float specularIntensity = pow(NdotH, shininess);
        float4 specularComponent = directionalLight.specular * specularIntensity;

        // Combine lighting components
        output.lightingColor = ambientComponent + diffuseComponent + specularComponent;

    }
    else
    {
		// Calculate the TBN matrix using direct tangent and bitangent
        float3 T = normalize(mul(input.tangent, (float3x3) World)); // Transform tangent
        float3 B = normalize(mul(input.bitangent, (float3x3) World)); // Transform bitangent
        float3 N = normalize(mul(input.normal, (float3x3) World)); // Transform normal

		// Set the TBN matrix
        output.TBN = float3x3(T, B, N);

        output.lightingColor = float4(1, 1, 1, 1);
    }

    return output;
}