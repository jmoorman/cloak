#pragma once

#include "stdafx.h"

class DrawableObject
{
	friend class GraphicsContext;
public:
	enum DrawableType
	{
		kDrawableTypeStaticMesh = 0,
		kDrawableTypeAnimatedMesh,

		kDrawableTypeCount
	};

	DrawableObject(DrawableType type);
	~DrawableObject();

	//absolute transforms
	void setPosition(const glm::vec3 &position) { mPosition = position; }
	const glm::vec3& getPosition() { return mPosition; }
	void setOrientation(const glm::quat &orientation) { mOrientation = orientation; }
	const glm::quat& getOrientation() { return mOrientation; }
	void setScale(const glm::vec3& scale) { mScale = scale; }
	const glm::vec3& getScale() { return mScale; }

	//relative transforms
	void moveBy(const glm::vec3& deltaXYZ);
	void rotateBy(float angle, const glm::vec3 & axis);
	void scaleBy(const glm::vec3& scale);

	glm::mat4 buildModelMatrix();

	VkBuffer mObjectConstantBuffer;
	VkDeviceMemory mObjectConstantBufferMemory;

protected:
	DrawableType mType;
	glm::vec3 mPosition;
	glm::quat mOrientation;
	glm::vec3 mScale;

	VkCommandBuffer mCommandBuffer;
};

