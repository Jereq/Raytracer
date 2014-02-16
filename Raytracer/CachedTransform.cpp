#include "CachedTransform.h"

#include <glm/gtx/transform.hpp>

CachedTransform::CachedTransform() :
	translation(0.f, 0.f, 0.f),
	scale(1.f, 1.f, 1.f),
	orientation(),
	isValid(true),
	transform()
{}

CachedTransform::CachedTransform
	(
		const glm::vec3& _translation,
		const glm::vec3& _scale,
		const glm::quat& _orientation
	)
	: translation(_translation),
	scale(_scale),
	orientation(_orientation),
	isValid(false)
{}

const glm::mat4x4& CachedTransform::getTransform() const
{
	if (!isValid)
		calculateTransform();

	return transform;
}

const glm::vec3& CachedTransform::getTranslation() const
{
	return translation;
}

void CachedTransform::setTranslation(const glm::vec3& _translation)
{
	if (translation != _translation)
	{
		translation = _translation;
		isValid = false;
	}
}

const glm::vec3& CachedTransform::getScale() const
{
	return scale;
}

void CachedTransform::setScale(const glm::vec3& _scale)
{
	if (scale != _scale)
	{
		scale = _scale;
		isValid = false;
	}
}

const glm::quat& CachedTransform::getOrientation() const
{
	return orientation;
}

void CachedTransform::setOrientation(const glm::quat& _orientation)
{
	if (orientation != _orientation)
	{
		orientation = _orientation;
		isValid = false;
	}
}

void CachedTransform::calculateTransform() const
{
	transform = 
		glm::translate(translation) *
		glm::scale(scale) *
		glm::mat4_cast(orientation);

	isValid = true;
}
