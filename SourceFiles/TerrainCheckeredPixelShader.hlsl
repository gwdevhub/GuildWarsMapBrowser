sampler ss: register(s0);
Texture2D shaderTextures[8] : register(t3);

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
    float pad;
};

cbuffer PerFrameCB: register(b0)
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
};

cbuffer PerTerrainCB : register(b3)
{
    int grid_dim_x;
    int grid_dim_y;
    float min_x;
    float max_x;
    float min_y;
    float max_y;
    float min_z;
    float max_z;
    float water_level;
    float pad[3];
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
};

float4 main(PixelInputType input) : SV_TARGET
{
    float4 finalColor = input.lightingColor;

    // Calculate new texture coordinates by repeating the texture over the terrain
    float2 repeatedTexCoords = input.tex_coords0 * float2(grid_dim_x, grid_dim_y);

    float4 sampledTextureColor = float4(0, 0, 0, 0);
    if (num_uv_texture_pairs == 1) {
        sampledTextureColor = shaderTextures[0].Sample(ss, repeatedTexCoords); 
    }

    float4 outputColor;
    // Multiply the sampled color with the finalColor
    if (input.terrain_height <= water_level) {
        float4 blue_color = float4(0.11, 0.65, 0.81, 1.0); // Water color
        outputColor = finalColor * sampledTextureColor * blue_color;
    }
    else {
        outputColor = finalColor * sampledTextureColor;
    }

    // Return the result
    return outputColor;
}