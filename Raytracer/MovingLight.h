#pragma once

#include <glm\glm.hpp>

struct Light
{
	glm::vec4 position;
	glm::vec4 intensity;
};

class MovingLight
{
public:
	MovingLight(glm::vec4 _intensity, const glm::vec4& _pos1, const glm::vec4& _pos2, float _speed);

	void onFrame(float _deltaTime);
	glm::vec4 getPosition() const;
	
	Light light;
	glm::vec4 position1;
	glm::vec4 direction;
	float speed;
	float length;
	float currentLength;
};
