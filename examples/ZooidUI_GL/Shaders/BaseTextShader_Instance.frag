#version 330

in vec4 OutColor;
in vec2 TexCoord;
in vec4 FragPos;
in vec4 NormalizedCropBox;

out vec4 FragColor;

uniform sampler2D InTexture;

uniform bool bCrop;

void main()
{
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