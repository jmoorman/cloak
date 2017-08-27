#include "DrawableObject.h"



DrawableObject::DrawableObject(DrawableType type)
	: mType(type), mPosition(0, 0, 0), mOrientation(0, 0, 0, 1)
{
	m_commandBuffer = VK_NULL_HANDLE;
}


DrawableObject::~DrawableObject()
{
}


void DrawableObject::moveBy(const glm::vec3 & deltaXYZ)
{
	mPosition += deltaXYZ;
}

void DrawableObject::rotateBy(float angle, const glm::vec3 & axis)
{
	mOrientation = glm::angleAxis(angle, axis) * mOrientation;
}

void DrawableObject::scaleBy(const glm::vec3 &scale)
{
	mScale *= scale;
}

glm::mat4 DrawableObject::buildModelMatrix()
{
	glm::mat4 modelMatrix = glm::mat4_cast(mOrientation);
	modelMatrix[3][0] = mPosition[0];
	modelMatrix[3][1] = mPosition[1];
	modelMatrix[3][2] = mPosition[2];
	modelMatrix = glm::scale(modelMatrix, mScale);
	return modelMatrix;
}
