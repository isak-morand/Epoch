Shader "Standard"
{
    Properties
    {
        _BaseColor ("Base Color", Color) = (1, 1, 1, 1)
        _ColorTex ("Color Texture", Tex2D) = "white"
        _NormalTex ("Normal Texture", Tex2D) = "flat"
        _Multiplier ("Multiplier", Range(0.0, 1.0)) = 0.5
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
           float4 baseTexColor = _ColorTex.Sample(_ColorTexSampler, input.uv);
           float3 finalColor = baseTexColor.rgb * _BaseColor.rgb * input.color;
       
           return float4(finalColor, 1.0f);
       }
    }
}