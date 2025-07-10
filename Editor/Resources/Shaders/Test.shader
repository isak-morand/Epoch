Shader "Standard"
{
    Properties
    {
        _AlbedoTex ("Albedo Texture", Tex2D) = "white"
    }

    VertexShader
    {
        cbuffer CameraBuffer : register(b1)
        {
            float4x4 CB_ViewProj;
        }

        cbuffer ObjectBuffer : register(b2)
        {
            float4x4 OB_Transform;
        }
    
        struct VertexInput
        {
            float3 pos      : POSITION;
            float3 normal   : NORMAL;
            float3 tangent  : TANGENT;
            float2 uv       : UV;
            float3 color    : COLOR;
        };
    
        struct VertexOutput
        {
            float4 pos      : SV_POSITION;
            float4 worldPos : WORLDPOSITION;
            float3 color    : COLOR;
            float2 uv       : UV;
        };
    
        VertexOutput main(VertexInput input)
        {
            VertexOutput output;
    
            output.worldPos = mul(OB_Transform, float4(input.pos, 1));
            output.pos = mul(CB_ViewProj, output.worldPos);
            output.color = input.color;
            output.uv = input.uv;
    
            return output;
        }
    }
    
    PixelShader
    {
        struct VertexOutput
        {
            float4 pos      : SV_POSITION;
            float4 worldPos : WORLDPOSITION;
            float3 color    : COLOR;
            float2 uv       : UV;
        };
    
        float4 main(VertexOutput input) : SV_TARGET
        {
            return _AlbedoTex.Sample(_AlbedoTexSampler, input.uv);
        }
    }
}