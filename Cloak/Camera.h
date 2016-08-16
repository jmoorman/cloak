#pragma once

#include "stdafx.h"

class Camera
{
public:
	Camera();
	Camera(glm::vec3 position);
	~Camera();

	const glm::vec3 &getPosition() const;
	const glm::vec3 &getDirection() const;

	//absolute position/rotation
	void setPosition(const glm::vec3 &position);
	void setDirection(const glm::vec3 &direction);
	void lookAt(const glm::vec3 &position);
	void setPerspective(float fovy, float aspectRatio, float nearPlane, float farPlane);

	//relative movement/rotation
	void move(float dx, float dy, float dz);
	void move(const glm::vec3 &movement);
	void rotate(float angle, const glm::vec3 &axis);
	void rotate(const glm::quat &rotation);

	void rotatePitch(float deltaPitch);
	void rotateYaw(float deltaYaw);

	const glm::mat4 getViewMatrix();
	const glm::mat4 getProjectionMatrix();
private:
	glm::vec3 mPosition;
	glm::vec3 mDirection;
	glm::vec3 mUp;

	float mNearPlane;
	float mFarPlane;
	float mAspectRatio;
	float mFovY;
};