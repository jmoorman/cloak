#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform SceneConstantBuffer
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec4 lightDirection;
	vec4 lightColor;
} sceneConstantBuffer;

layout(binding = 1) uniform PerObjectConstantBuffer
{
	mat4 modelMatrix;
} perObjectCB;

layout(binding = 2) uniform AnimationConstantBuffer
{
	mat4 boneMatrices[256];
} animationCB;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexcoord;
layout(location = 3) in vec4 inBoneWeights;
layout(location = 4) in uvec4 inBoneIndices;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexcoord;

//This is a hack so I don't have to make another constant buffer in the pixel shader right now. Remove ASAP.
layout(location = 2) out vec4 fragLightDirection;
layout(location = 3) out vec4 fragLightColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	vec4 position = vec4(inPosition, 1.0);
	vec4 skinnedPosition = (animationCB.boneMatrices[inBoneIndices.x] * position) * inBoneWeights.x;
	skinnedPosition += (animationCB.boneMatrices[inBoneIndices.y] * position) * inBoneWeights.y;
	skinnedPosition += (animationCB.boneMatrices[inBoneIndices.z] * position) * inBoneWeights.z;
	skinnedPosition += (animationCB.boneMatrices[inBoneIndices.w] * position) * inBoneWeights.w;
	
	vec4 normal = vec4(inNormal, 0);
	vec4 skinnedNormal = (animationCB.boneMatrices[inBoneIndices.x] * normal) * inBoneWeights.x;
	skinnedNormal += (animationCB.boneMatrices[inBoneIndices.z] * normal) * inBoneWeights.z;
	skinnedNormal += (animationCB.boneMatrices[inBoneIndices.w] * normal) * inBoneWeights.w;
	skinnedNormal += (animationCB.boneMatrices[inBoneIndices.y] * normal) * inBoneWeights.y;

	gl_Position = sceneConstantBuffer.projectionMatrix * sceneConstantBuffer.viewMatrix * perObjectCB.modelMatrix * skinnedPosition;
	fragNormal = normalize(skinnedNormal).xyz;
	fragTexcoord = inTexcoord;

	fragLightDirection = sceneConstantBuffer.lightDirection;
	fragLightColor = sceneConstantBuffer.lightColor;
}