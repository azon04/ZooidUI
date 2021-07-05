#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location=0) in vec4 OutColor;
layout (location=1) in vec2 TexCoord;

layout (location=0) out vec4 FragColor;

layout(binding=2) uniform UniformBufferObject
{
	vec2 shapeDimension;
	float roundness;
	bool bCrop;
} fragParams;

void main()
{
	float roundness = fragParams.roundness;
	vec2 shapeDimension = fragParams.shapeDimension;
	vec2 coords = TexCoord * shapeDimension;
	bool toDiscard = (coords.x < roundness && coords.y < roundness && length(vec2(roundness) - coords) > roundness ) ||
		( coords.x > shapeDimension.x - roundness && coords.y < roundness && length(vec2(shapeDimension.x - roundness, roundness) - coords) > roundness ) ||
		( coords.y > shapeDimension.y - roundness && coords.x < roundness && length(vec2(roundness, shapeDimension.y - roundness) - coords) > roundness ) ||
		( coords.x > shapeDimension.x - roundness && coords.y > shapeDimension.y - roundness && length(vec2(shapeDimension.x - roundness, shapeDimension.y - roundness) - coords) > roundness );
	
	float alpha = toDiscard ? 0.0 : 1.0;
	
	if( toDiscard ) { discard; }

	FragColor = OutColor * vec4( 1.0f, 1.0f, 1.0f, alpha );	
}