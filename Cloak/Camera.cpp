#include "stdafx.h"
#include "Camera.h"


Camera::Camera() : mPosition(0), mDirection(0, 0, 1), mUp(0, 1, 0)
{
}

Camera::Camera(glm::vec3 position) : mPosition(position), mDirection(0, 0, 1), mUp(0, 1, 0)
{
}

Camera::~Camera(void)
{
}

const glm::vec3 & Camera::getPosition() const {
	return mPosition;
}

const glm::vec3 & Camera::getDirection() const {
	return mDirection;
}

void Camera::setPosition(const glm::vec3 &position) {
	mPosition = position;
}

void Camera::setDirection(const glm::vec3 &direction) {
	mDirection = glm::normalize(direction);
}

void Camera::lookAt(const glm::vec3 &targetPosition)
{
	mDirection = glm::normalize(targetPosition - mPosition);
}

void Camera::setPerspective(float fovy, float aspectRatio, float nearPlane, float farPlane)
{
	mFovY = fovy;
	mAspectRatio = aspectRatio;
	mNearPlane = nearPlane;
	mFarPlane = farPlane;
}

void Camera::moveBy(float dx, float dy, float dz)
{
	//x moves side to side, orthogonal to the camera's direction
	//y moves along the up vector
	//z moves along the direction vector
	glm::vec3 xComponent = dx * glm::cross(mUp, mDirection);
	glm::vec3 yComponent = dy * mUp;
	glm::vec3 zComponent = dz * mDirection;

	mPosition = mPosition + xComponent + yComponent + zComponent;
}

void Camera::moveBy(const glm::vec3 &movement) {
	moveBy(movement.x, movement.y, movement.z);
}

void Camera::rotateBy(const glm::quat &rotation) {
	mDirection = glm::normalize(rotation * mDirection);
}

void Camera::rotateBy(float angle, const glm::vec3 &axis) {
	rotateBy(glm::angleAxis(angle, axis));
}

void Camera::rotatePitch(float deltaPitch) {
	rotateBy(deltaPitch, glm::cross(mUp, mDirection));
}

void Camera::rotateYaw(float deltaYaw) {
	rotateBy(deltaYaw, mUp);
}

const glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(mPosition, mPosition + mDirection, mUp);
}

const glm::mat4 Camera::getProjectionMatrix()
{
	glm::mat4 projectionMatrix = glm::perspective(mFovY, mAspectRatio, mNearPlane, mFarPlane);
	//Y-coordinate fixup because this isn't OpenGL
	projectionMatrix[1][1] *= -1;
	return projectionMatrix;
}