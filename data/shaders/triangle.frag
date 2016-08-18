#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexcoord;
layout(location = 2) in vec4 fragLightDirection;
layout(location = 3) in vec4 fragLightColor;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 unlitColor = texture(texSampler, fragTexcoord).xyz;
	float diffuseIntensity = dot(normalize(fragNormal).xyz, -normalize(fragLightDirection).xyz);
	vec3 diffuseLighting = unlitColor * fragLightColor.xyz * diffuseIntensity;
	vec3 ambientLighting = unlitColor * fragLightColor.w;
	outColor = vec4(diffuseLighting + ambientLighting, 1.f);
}