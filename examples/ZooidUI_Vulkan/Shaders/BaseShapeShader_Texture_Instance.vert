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
layout (location=2) out float Roundness;
layout (location=3) out vec2 ShapeDimension;

layout(binding=0) uniform UniformBufferObject {
	vec2 screenDimension;
} params;

void main()
{
	vec2 screenDimension = params.screenDimension;
	vec3 pos = position * dimension + instancePos;
	pos.x = 2.0 * ( pos.x / screenDimension.x) - 1.0;
	pos.y = 1.0 - 2.0 * ( pos.y / screenDimension.y);
	pos.z = 1.0 - instancePos.z;
	ShapeDimension = dimension.xy;
	Roundness = dimension.z;

	gl_Position = vec4( pos, 1.0 );
	OutColor = color * instanceColor;
	TexCoord = texCoord * uvDim.zw + uvDim.xy;
}