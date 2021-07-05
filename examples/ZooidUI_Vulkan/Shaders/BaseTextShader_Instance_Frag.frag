#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location=0) in vec4 OutColor;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec4 FragPos;
layout (location=3) in vec4 NormalizedCropBox;

layout (location=0) out vec4 FragColor;

layout(binding=2) uniform UniformBufferObject
{
	vec2 shapeDimension;
	float roundness;
	bool bCrop;
} fragParams;

layout(binding=3) uniform sampler2D InTexture;

void main()
{
	bool bCrop = fragParams.bCrop;
	float sampled = texture(InTexture, TexCoord).r;
	if(sampled <= 0.0) { discard; }
	if( bCrop && 
		( FragPos.x < NormalizedCropBox.x ||
			FragPos.x > NormalizedCropBox.x + NormalizedCropBox.z ||
			FragPos.y > NormalizedCropBox.y ||
			FragPos.y < NormalizedCropBox.y - NormalizedCropBox.w ) )
	{
		discard;
	}
	
	FragColor = vec4(OutColor.xyz, OutColor.w * sampled);
}