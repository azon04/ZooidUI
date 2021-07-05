#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec4 color;
layout (location = 3) in vec3 instancePos;
layout (location = 4) in vec3 dimension;
layout (location = 5) in vec4 instanceColor;
layout (location = 6) in vec4 uvDim;

layout (location=0) out vec4 OutColor;
layout (location=1) out vec2 TexCoord;
layout (location=2) out vec4 FragPos;
layout (location=3) out vec4 NormalizedCropBox;

layout(binding=0) uniform SharedBufferObject {
	vec2 screenDimension;
} sharedParams;

layout(binding=1) uniform UniformBufferObject {
	vec4 CropBox;
} params;

void main()
{
	vec2 screenDimension = sharedParams.screenDimension;
	vec4 CropBox = params.CropBox;
	vec3 pos = position * dimension + instancePos;
	pos.x = 2.0 * (( pos.x ) / screenDimension.x) - 1.0;
	pos.y = 2.0 * ((pos.y + screenDimension.y) / screenDimension.y) - 1.0;
	pos.z = 1.0 - instancePos.z;

	OutColor = color * instanceColor;
	TexCoord = texCoord * uvDim.zw + uvDim.xy;
	FragPos = vec4( pos, 1.0 );
	gl_Position = vec4( pos, 1.0 );

	NormalizedCropBox.x = 2.0 * (( CropBox.x ) / screenDimension.x) - 1.0;
	NormalizedCropBox.y = 2.0 * (( -CropBox.y + screenDimension.y) / screenDimension.y) - 1.0;
	NormalizedCropBox.z = CropBox.z * 2.0 / screenDimension.x;
	NormalizedCropBox.w = CropBox.w * 2.0 / screenDimension.y;
}