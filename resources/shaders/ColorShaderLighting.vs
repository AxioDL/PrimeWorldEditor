#version 330 core

// Input
layout(location = 0) in vec3 RawPosition;
layout(location = 1) in vec3 RawNormal;

// Output
out vec4 COLOR0A0;

// Uniforms
layout(std140) uniform MVPBlock
{
	mat4 ModelMtx;
	mat4 ViewMtx;
	mat4 ProjMtx;
};

layout(std140) uniform VertexBlock
{
    mat4 TexMtx[10];
    mat4 PostMtx[20];
    vec4 COLOR0_Amb;
    vec4 COLOR0_Mat;
    vec4 COLOR1_Amb;
    vec4 COLOR1_Mat;
};

struct SGXLight
{
    vec4 Position;
    vec4 Direction;
    vec4 Color;
    vec4 DistAtten;
    vec4 AngleAtten;
};
layout(std140) uniform LightBlock {
    SGXLight Lights[8];
};
uniform int NumLights;

// Main
void main()
{
    mat4 MVP = ModelMtx * ViewMtx * ProjMtx;
    mat4 MV = ModelMtx * ViewMtx;
    gl_Position = vec4(RawPosition, 1) * MVP;
    vec3 Normal = normalize(RawNormal.xyz * inverse(transpose(mat3(MV))));

    // Dynamic Lighting
    vec4 Illum = vec4(0.0);
    vec3 PositionMV = vec3(vec4(RawPosition, 1.0) * MV);
    
    for (int iLight = 0; iLight < NumLights; iLight++)
    {
        vec3 LightPosMV = vec3(Lights[iLight].Position * ViewMtx);
        vec3 LightDirMV = normalize(Lights[iLight].Direction.xyz * inverse(transpose(mat3(ViewMtx))));
        vec3 LightDist = LightPosMV.xyz - PositionMV.xyz;
        float DistSquared = dot(LightDist, LightDist);
        float Dist = sqrt(DistSquared);
        LightDist /= Dist;
        vec3 AngleAtten = Lights[iLight].AngleAtten.xyz;
        AngleAtten = vec3(AngleAtten.x, AngleAtten.y, AngleAtten.z);
        float Atten = max(0, dot(LightDist, LightDirMV.xyz));
        Atten = max(0, dot(AngleAtten, vec3(1.0, Atten, Atten * Atten))) / dot(Lights[iLight].DistAtten.xyz, vec3(1.0, Dist, DistSquared));
        float DiffuseAtten = max(0, dot(Normal, LightDist));
        Illum += (Atten * DiffuseAtten * Lights[iLight].Color);
    }
    COLOR0A0 = COLOR0_Mat * (Illum + COLOR0_Amb);
}
