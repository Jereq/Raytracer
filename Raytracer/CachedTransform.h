#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class CachedTransform
{
private:
	glm::vec3 translation;
	glm::vec3 scale;
	glm::quat orientation;

	mutable bool isValid;
	mutable glm::mat4x4 transform;

public:
	CachedTransform();
	CachedTransform(
		const glm::vec3& _translation,
		const glm::vec3& _scale,
		const glm::quat& _orientation);

	const glm::mat4x4& getTransform() const;

	const glm::vec3& getTranslation() const;
	void setTranslation(const glm::vec3& _translation);

	const glm::vec3& getScale() const;
	void setScale(const glm::vec3& _scale);

	const glm::quat& getOrientation() const;
	void setOrientation(const glm::quat& _orientation);

private:
	void calculateTransform() const;
};
