#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
	: position(0.f),
	viewDirection(0.f),
	up(0.f, 1.f, 0.f),
	fieldOfViewY(90.f),
	ratio(1.f),
	matrixUpdated(false),
	viewProjectionMatrix(1.f)
{
}

Camera::Camera(float _fovY, float _ratio)
	: position(0.f),
	viewDirection(0.f),
	up(0.f, 1.f, 0.f),
	fieldOfViewY(_fovY),
	ratio(_ratio),
	matrixUpdated(false),
	viewProjectionMatrix(1.f)
{
}

void Camera::updateMatrix() const
{
	glm::mat4 projectionMatrix = glm::perspective(fieldOfViewY, ratio, 0.01f, 100.f);
	glm::mat4 viewMatrix = glm::lookAt(position, position + viewDirection, up);

	viewProjectionMatrix = projectionMatrix * viewMatrix;
}

void Camera::updateInvMatrix() const
{
	invMatrix = glm::inverse(viewProjectionMatrix);
}

glm::mat4 Camera::getViewProjectionMatrix() const
{
	if (!matrixUpdated)
	{
		updateMatrix();
		matrixUpdated = true;
		invMatrixUpdated = false;
	}

	return viewProjectionMatrix;
}

glm::mat4 Camera::getInvViewProjectionMatrix() const
{
	if (!matrixUpdated)
	{
		updateMatrix();
		updateInvMatrix();
		matrixUpdated = true;
		invMatrixUpdated = true;
	}
	else if (!invMatrixUpdated)
	{
		updateInvMatrix();
		invMatrixUpdated = true;
	}

	return invMatrix;
}

void Camera::setPosition(const glm::vec3& _position)
{
	if (_position != position)
	{
		position = _position;
		matrixUpdated = false;
	}
}

void Camera::setViewDirection(const glm::vec3& _viewDirection)
{
	if (_viewDirection != viewDirection)
	{
		viewDirection = _viewDirection;
		matrixUpdated = false;
	}
}

void Camera::setUpDirection(const glm::vec3& _upDirection)
{
	if (_upDirection != up)
	{
		up = _upDirection;
		matrixUpdated = false;
	}
}

void Camera::setRotation(const glm::vec3& _rot)
{
	glm::mat4 rotMat = glm::rotate(glm::mat4(), _rot.y, glm::vec3(0.f, 1.f, 0.f));
	rotMat = glm::rotate(rotMat, _rot.x, glm::vec3(1.f, 0.f, 0.f));
	rotMat = glm::rotate(rotMat, _rot.z, glm::vec3(0.f, 0.f, 1.f));

	setViewDirection(glm::vec3(rotMat * glm::vec4(0.f, 0.f, -1.f, 0.f)));
	setUpDirection(glm::vec3(rotMat * glm::vec4(0.f, 1.f, 0.f, 0.f)));
}

void Camera::setFieldOfView(float _fovY)
{
	if (_fovY != fieldOfViewY)
	{
		fieldOfViewY = _fovY;
		matrixUpdated = false;
	}
}

void Camera::setScreenRatio(float _ratio)
{
	if (_ratio != ratio)
	{
		ratio = _ratio;
		matrixUpdated = false;
	}
}

glm::vec3 Camera::getPosition() const
{
	return position;
}
