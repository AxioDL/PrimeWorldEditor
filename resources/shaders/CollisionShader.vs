#version 330 core

// Input
layout(location = 0) in vec3 RawPosition;
layout(location = 1) in vec3 RawNormal;

// Output
out vec4 LightColor;

// Uniforms
layout(std140) uniform MVPBlock
{
	mat4 ModelMtx;
	mat4 ViewMtx;
	mat4 ProjMtx;
};

// Main
void main()
{
    mat4 MVP = ModelMtx * ViewMtx * ProjMtx;
	gl_Position = vec4(RawPosition, 1) * MVP;
	
	// Fake lighting; render one white skylight pointing straight down with an ambient 0.5
	float LightDot = dot(RawNormal, vec3(0, 0, -1));
	float Alpha = (-LightDot + 1.0) / 2;
	float LightAlpha = mix(0.5, 0.9, Alpha);
	LightColor = vec4(LightAlpha, LightAlpha, LightAlpha, 1.0);
}
