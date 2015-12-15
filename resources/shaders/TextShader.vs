#version 330 core

// Input
layout(location = 0) in vec3 Position;
layout(location = 4) in vec2 Tex0;

// Output
out vec2 TexCoord;

// Uniforms
uniform mat4 ModelMtx;

// Main
void main()
{
	gl_Position = vec4(Position, 1) * ModelMtx;
	TexCoord = Tex0;
}
