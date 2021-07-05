#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec4 OutColor;
layout(location=1) in vec2 TexCoord;
layout(location=2) in float Roundness;
layout(location=3) in vec2 ShapeDimension;

layout(location=0) out vec4 FragColor;

void main()
{
	vec2 coords = TexCoord * ShapeDimension;
	bool toDiscard = (coords.x < Roundness && coords.y < Roundness && length(vec2(Roundness) - coords) > Roundness ) ||
		( coords.x > ShapeDimension.x - Roundness && coords.y < Roundness && length(vec2(ShapeDimension.x - Roundness, Roundness) - coords) > Roundness ) ||
		( coords.y > ShapeDimension.y - Roundness && coords.x < Roundness && length(vec2(Roundness, ShapeDimension.y - Roundness) - coords) > Roundness ) ||
		( coords.x > ShapeDimension.x - Roundness && coords.y > ShapeDimension.y - Roundness && length(vec2(ShapeDimension.x - Roundness, ShapeDimension.y - Roundness) - coords) > Roundness );
	
	float alpha = toDiscard ? 0.0 : 1.0;
	
	if( toDiscard ) { discard; }

	FragColor = OutColor * vec4( 1.0f, 1.0f, 1.0f, alpha );	
}