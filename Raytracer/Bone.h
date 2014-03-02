#pragma once

#include "CachedTransform.h"

#include <string>
#include <vector>

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
	int index;
	int parentIndex;

public:
	Bone();
	Bone(
		const glm::vec3& _translation,
		const glm::vec3& _scale,
		const glm::quat& _orientation,
		const std::string& _name,
		int _index = -1,
		int _parentIndex = -1);

	const std::string& getName() const;
	CachedTransform& getLocalTransform();
	const CachedTransform& getLocalTransform() const;
	glm::mat4x4 getObjectTransform() const;

	const Bone* getParent() const;
	int getIndex() const;
	void setIndex(int _index);

	void addChild(Bone* _newChild);
	void removeChild(Bone* _child);

	static void setupSkeleton(std::vector<Bone>& _bones);

	friend std::istream& operator>>(std::istream& _stream, Bone& _bone);
	friend std::ostream& operator<<(std::ostream& _stream, const Bone& _bone);

private:
	void setParent(Bone* _newParent);
	void setNextSibling(Bone* _newSibling);
	void removeSibling(Bone* _sibling);
};
