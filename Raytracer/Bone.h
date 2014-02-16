#pragma once

#include "CachedTransform.h"

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Bone
{
private:
	CachedTransform transform;

	mutable bool isObjectValid;
	mutable glm::mat4x4 objectTransform;

	Bone* parent;
	Bone* firstChild;
	Bone* nextSibling;
	std::string name;

public:
	Bone();
	Bone(
		const glm::vec3& _translation,
		const glm::vec3& _scale,
		const glm::quat& _orientation,
		const std::string& _name);

	const std::string& getName() const;
	glm::mat4x4 getLocalTransform() const;
	glm::mat4x4 getObjectTransform() const;

	void addChild(Bone* _newChild);
	void removeChild(Bone* _child);

private:
	void setParent(Bone* _newParent);
	void setNextSibling(Bone* _newSibling);
	void removeSibling(Bone* _sibling);
};
