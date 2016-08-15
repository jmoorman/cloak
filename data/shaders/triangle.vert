#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform PerFrameConstantBuffer
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} perFrameCB;

layout(binding = 1) uniform AnimationConstantBuffer
{
	mat4 boneMatrices[256];
} animationCB;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexcoord;
layout(location = 3) in vec4 inBoneWeights;
layout(location = 4) in uvec4 inBoneIndices;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexcoord;
out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = perFrameCB.projectionMatrix * perFrameCB.viewMatrix * perFrameCB.modelMatrix * vec4(inPosition, 1.0);
	fragColor = inColor;
	fragTexcoord = inTexcoord;
}