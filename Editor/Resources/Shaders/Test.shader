Shader "Standard"
{
    VertexShader
    {
        cbuffer CameraBuffer : register(b0)
        {
            float4x4 CB_ViewProj;
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
            float3 color    : COLOR;
            float2 uv       : UV;
        };
    
        VertexOutput main(VertexInput input)
        {
            VertexOutput output;
    
            output.pos = mul(CB_ViewProj, float4(input.pos, 1));
            output.color = input.color;
            output.uv = input.uv;
    
            return output;
        }
    }
    
    PixelShader
    {
		Texture2D AlbedoTexture : register(t0);
		SamplerState AlbedoTextureSampler : register(s0);

        struct VertexOutput
        {
            float4 pos      : SV_POSITION;
            float3 color    : COLOR;
            float2 uv       : UV;
        };
    
        float4 main(VertexOutput input) : SV_TARGET
        {
            return AlbedoTexture.Sample(AlbedoTextureSampler, input.uv);
        }
    }
}