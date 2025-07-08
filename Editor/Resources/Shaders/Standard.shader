Shader "Standard"
{
    Properties
    {
        _BaseColor ("Base Color", Color) = (1, 1, 1, 1)
        _MainTex ("Main Texture", Tex2D) = "white"
        _Multiplier ("Multiplier", Range(0.0, 1.0)) = 0.5
    }

    VertexShader
    {
        struct VertexOutput
        {
            float4 pos : SV_POSITION;
            float2 uv : UV;
        };
        
        VertexOutput main(unsigned int aVertexIndex : SV_VertexID)
        {
            const float4 pos[3] =
            {
                float4(-0.5, -0.5, 0, 1),
                float4(0, 0.5, 0, 1),
                float4(0.5, -0.5, 0, 1)
            };
            
            const float4 uvs[3] =
            {
                float4(0.0f, 0.0f, 0.0f, 1.0f),
                float4(0.5f, 1.0f, 0.0f, 1.0f),
                float4(1.0f, 0.0f, 0.0f, 1.0f)
            };
            
            VertexOutput output;
            
            output.pos = pos[aVertexIndex];
            output.uv = uvs[aVertexIndex];
            
            return output;
        }
    }
    
    PixelShader
    {
        struct VertexOutput
        {
            float4 pos : SV_POSITION;
            float2 uv : UV;
        };
        
        float4 main(VertexOutput input) : SV_TARGET
        {
            float4 baseTexColor = _MainTex.Sample(_MainTexSampler, input.uv);
            float3 finalColor = baseTexColor.rgb * _BaseColor.rgb;
        
            return float4(finalColor, 1.0f);
        }
    }
}