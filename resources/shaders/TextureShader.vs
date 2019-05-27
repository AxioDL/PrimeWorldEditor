#version 330 core

// Input
layout(location = 0) in vec3 Position;
layout(location = 4) in vec2 Tex0;

// Output
out vec2 TexCoord;

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
	gl_Position = vec4(Position, 1) * MVP;
	TexCoord = Tex0;
}
