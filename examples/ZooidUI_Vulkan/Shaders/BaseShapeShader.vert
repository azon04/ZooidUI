#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec4 color;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec2 TexCoord;

layout(binding=0) uniform UniformBufferObject {
	vec2 screenDimension;
} params;

void main()
{
	vec3 pos = position;
	pos.x = 2.0 * (pos.x / params.screenDimension.x) - 1.0;
	pos.y = 1.0 - 2.0 * (pos.y / params.screenDimension.y);
	pos.z = 1.0 - pos.z;

	gl_Position = vec4( pos, 1.0 );
	OutColor = color;
	TexCoord = texCoord;
}