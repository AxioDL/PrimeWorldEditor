// This shader will be obsoleted soon when the collision rendering is improved
#version 330 core

// Input
layout(location = 0) in vec3 Position;

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
	
	// UV Generation
	float avg = (Position.x + Position.z) / 2;
	TexCoord.x = avg;
	TexCoord.y = Position.y + (avg / 2);
}
