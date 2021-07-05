#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location=0) in vec4 OutColor;
layout (location=1) in vec2 TexCoord;
layout (location=2) in float Roundness;
layout (location=3) in vec2 ShapeDimension;

layout (location=0) out vec4 FragColor;

layout (binding=3) uniform sampler2D InTexture;

void main()
{
	vec4 sampled = vec4(texture(InTexture, TexCoord).r);
	if(sampled.a <= 0.0) { discard; }
	FragColor = OutColor * sampled;
}