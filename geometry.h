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