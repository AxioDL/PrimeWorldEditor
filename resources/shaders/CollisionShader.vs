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

uniform bool IsFloor;
uniform bool IsUnstandable;

// Main
void main()
{
	const vec3 kLightDir = normalize( vec3(0.3, 0, -1) );	// Sets the direction of the light
	const float kLightColorMin = 0.5;						// Sets the minimum light color (color of a vertex facing away from the light)
	const float kLightColorMax = 0.9;						// Sets the maximum light color (color of a vertex facing towards the light)
	
	// Calculate vertex position
    mat4 MVP = ModelMtx * ViewMtx * ProjMtx;
	gl_Position = vec4(RawPosition, 1) * MVP;
	
	// Apply some simple lighting
	float LightDot = dot(RawNormal, kLightDir);
	float Alpha = (-LightDot + 1.0) / 2;
	float LightAlpha = mix(kLightColorMin, kLightColorMax, Alpha);
	LightColor = vec4(LightAlpha, LightAlpha, LightAlpha, 1.0);
	
	// If this is not a floor, make the surface significantly darker
	// The surface is considered a floor if IsFloor is true OR if the floor normal Z is greater than 0.85
	float FloorMul = (!IsUnstandable && (IsFloor || RawNormal.z > 0.85)) ? 1.0 : 0.5;
	LightColor *= FloorMul;
}
