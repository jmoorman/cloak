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
};

struct ScreenVertex
{
	glm::vec2 position;
	glm::vec3 color;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(ScreenVertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(ScreenVertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(ScreenVertex, color);
		return attributeDescriptions;
	}
};

//This is for demonstration purposes only.
const std::vector<ScreenVertex> gDemoVertices =
{
	{ { 00.0f, -0.5f },	{ 1.0f, 1.0f, 0.0f } },
	{ { 00.5f, 00.0f },	{ 1.0f, 0.0f, 1.0f } },
	{ { -0.5f, 00.0f },	{ 0.0f, 1.0f, 1.0f } },
	{ { 00.0f, 0.5f }, { 1.0f, 1.0f, 0.0f } },
};

const std::vector<U16> gDemoIndices =
{
	0, 1, 2, 2, 1, 3
};