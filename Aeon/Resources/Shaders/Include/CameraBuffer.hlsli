cbuffer CameraBuffer : register(b0)
{
    float4x4 CB_ViewProj;
    float3 CB_CameraPos;
    float CB_NearPlane;
    float CB_FarPlane;
    float CB_Fov;
    float2 CB_ViewportSize;
}