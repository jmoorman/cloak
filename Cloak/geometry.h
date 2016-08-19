#pragma once

#include "stdafx.h"

struct PrimitiveVertex
{
	glm::vec3 position;
	glm::vec3 normal;
};

struct MeshVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(MeshVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(MeshVertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(MeshVertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(MeshVertex, texcoord);

		return attributeDescriptions;
	}
};

struct AnimatedMeshVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
	glm::vec4 bone_weights;
	glm::uvec4 bone_indices;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(AnimatedMeshVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(AnimatedMeshVertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(AnimatedMeshVertex, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(AnimatedMeshVertex, texcoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(AnimatedMeshVertex, bone_weights);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_UINT;
		attributeDescriptions[4].offset = offsetof(AnimatedMeshVertex, bone_indices);
		return attributeDescriptions;
	}
};

struct ScreenVertex
{
	glm::vec2 position;
	glm::vec3 color;
	glm::vec2 texcoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(ScreenVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(ScreenVertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(ScreenVertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(ScreenVertex, texcoord);

		return attributeDescriptions;
	}
};

//This is for demonstration purposes only.
const std::vector<MeshVertex> gDemoVertices =
{
	{ { 0.f, 5.f, 0.f },{ 1.0f, 1.0f, 0.0f }, { 0.5f, 0.5f } },
	{ { -3.0f, 0.f, 3.f },	{ 1.0f, 0.0f, 1.0f }, { 1.f, 0.f } },
	{ { 3.f, 0.f, 3.f },	{ 0.0f, 1.0f, 1.0f }, { 1.f, 1.f } },
	{ { 3.f, 0.f, -3.f }, { 1.0f, 1.0f, 1.0f }, { 0.f, 1.f } },
	{ { -3.f, 0.f, -3.f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
};

const std::vector<U16> gDemoIndices =
{
	0, 1, 2, 0, 3, 4,
	0, 4, 1, 0, 2, 3,
	2, 1, 3, 4, 3, 1,
};

struct SceneConstantBuffer
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec4 lightDirection;
	glm::vec4 lightColor;
};

struct ObjectConstantBuffer
{
	glm::mat4 modelMatrix;
};

struct AnimationConstantBuffer
{
	glm::mat4 boneMatrices[256];
};