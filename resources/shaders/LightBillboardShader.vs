#version 330 core

// Input
layout(location = 0) in vec3 Position;
layout(location = 4) in vec2 Tex0;

// Output
out vec2 TexCoord;

// Uniforms
layout(std140) uniform MVPBlock
{
	mat4 TranslateMtx;
	mat4 ViewMtx;
	mat4 ProjMtx;
};

uniform vec2 BillboardScale;

// Main
void main()
{
	mat4 MV = TranslateMtx * ViewMtx;
	mat4 VP = mat4 (	   1,		 0,		   0, MV[0][3],
						   0,		 1,		   0, MV[1][3],
						   0,		 0,		   1, MV[2][3],
					MV[3][0], MV[3][1], MV[3][2], MV[3][3]) * ProjMtx;
	
	gl_Position = vec4(Position,1) * vec4(BillboardScale.xy, 1, 1) * VP;

	TexCoord = vec2(Tex0.x, -Tex0.y);
}

