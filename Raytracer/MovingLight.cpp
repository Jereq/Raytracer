#include "MovingLight.h"

MovingLight::MovingLight(glm::vec4 _intensity, const glm::vec4& _pos1, const glm::vec4& _pos2, float _speed)
	: position1(_pos1),
	  speed(_speed),
	  currentLength(0.f)
{
	glm::vec4 distance = _pos2 - _pos1;
	length = glm::length(distance);
	direction = distance / length;

	light.intensity = _intensity;
	light.position = position1;
}

void MovingLight::onFrame(float _deltaTime)
{
	currentLength += speed * _deltaTime;
	if (speed < 0.f && currentLength < 0.f)
	{
		speed = -speed;
	}
	else if (speed > 0.f && currentLength > length)
	{
		speed = -speed;
	}

	light.position = getPosition();
}

glm::vec4 MovingLight::getPosition() const
{
	return position1 + direction * currentLength;
}