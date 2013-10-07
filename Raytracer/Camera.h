#pragma once

#include <glm/glm.hpp>

class Camera
{
private:
	glm::vec3 position;
	glm::vec3 viewDirection;
	glm::vec3 up;
	float fieldOfViewY;
	float ratio; // Width / Height

	mutable bool matrixUpdated;
	mutable glm::mat4 viewProjectionMatrix;
	mutable bool invMatrixUpdated;
	mutable glm::mat4 invMatrix;

	void updateMatrix() const;
	void updateInvMatrix() const;

public:
	Camera();
	Camera(float _fovY, float _ratio);

	glm::mat4 getViewProjectionMatrix() const;
	glm::mat4 getInvViewProjectionMatrix() const;

	void setPosition(const glm::vec3& _position);
	void setViewDirection(const glm::vec3& _viewDirection);
	void setUpDirection(const glm::vec3& _upDirection);
	void setRotation(const glm::vec3& _rot);	// Yaw, pitch, roll
	void setFieldOfView(float _fovY);
	void setScreenRatio(float _ratio);

	glm::vec3 getPosition() const;
};