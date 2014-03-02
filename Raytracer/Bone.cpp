#include "Bone.h"

#include <glm/gtc/matrix_transform.hpp>

Bone::Bone()
	: transform(),
	name(),
	parent(nullptr),
	firstChild(nullptr),
	nextSibling(nullptr),
	index(-1),
	parentIndex(-1),
	isObjectValid(false)
{}

Bone::Bone(
	const glm::vec3& _translation,
	const glm::vec3& _scale,
	const glm::quat& _orientation,
	const std::string& _name,
	int _index,
	int _parentIndex)
	: transform(_translation, _scale, _orientation),
	name(_name),
	parent(nullptr),
	firstChild(nullptr),
	nextSibling(nullptr),
	index(_index),
	parentIndex(_parentIndex),
	isObjectValid(false)
{}

const std::string& Bone::getName() const
{
	return name;
}

CachedTransform& Bone::getLocalTransform()
{
	return transform;
}

const CachedTransform& Bone::getLocalTransform() const
{
	return transform;
}

glm::mat4x4 Bone::getObjectTransform() const
{
	if (parent)
	{
		return parent->getObjectTransform() * getLocalTransform().getTransform();
	}
	else
	{
		return getLocalTransform().getTransform();
	}
}

const Bone* Bone::getParent() const
{
	return parent;
}

int Bone::getIndex() const
{
	return index;
}

void Bone::setIndex(int _index)
{
	index = _index;
}

void Bone::addChild(Bone* _newChild)
{
	if (_newChild == nullptr || this == _newChild)
		return;

	if (_newChild->parent != nullptr)
	{
		_newChild->parent->removeChild(_newChild);
	}

	_newChild->setParent(this);
	_newChild->setNextSibling(firstChild);
	firstChild = _newChild;
}

void Bone::removeChild(Bone* _child)
{
	if (firstChild == nullptr || _child == nullptr)
	{
		return;
	}

	Bone* childsSibling = firstChild->nextSibling;

	firstChild->removeSibling(_child);

	if (_child == firstChild)
	{
		firstChild = childsSibling;
	}
}

void Bone::setupSkeleton(std::vector<Bone>& _bones)
{
	assert(!_bones.empty());
	assert(_bones[0].parentIndex == -1);

	_bones[0].firstChild = nullptr;
	_bones[0].nextSibling = nullptr;
	_bones[0].parent = nullptr;
	_bones[0].index = 0;

	for (unsigned int i = 1; i < _bones.size(); ++i)
	{
		Bone& bone = _bones[i];
		bone.index = i;

		bone.firstChild = nullptr;
		bone.nextSibling = nullptr;
		bone.parent = nullptr;

		assert(bone.parentIndex < (int)i);

		_bones[bone.parentIndex].addChild(&bone);
	}
}

static std::istream& operator>>(std::istream& _stream, glm::vec3& _vec)
{
	return _stream >> _vec.x >> _vec.y >> _vec.z;
}

static std::istream& operator>>(std::istream& _stream, glm::quat& _quat)
{
	return _stream >> _quat.x >> _quat.y >> _quat.z >> _quat.w;
}

std::istream& operator>>(std::istream& _stream, Bone& _bone)
{
	glm::vec3 translation;
	glm::vec3 scale;
	glm::quat orientation;

	_stream >> _bone.name >>
		_bone.parentIndex >>
		translation >>
		scale >>
		orientation;

	--_bone.parentIndex;

	_bone.transform.setTranslation(translation);
	_bone.transform.setScale(scale);
	_bone.transform.setOrientation(orientation);

	return _stream;
}

static std::ostream& operator<<(std::ostream& _stream, const glm::vec3& _vec)
{
	return _stream << _vec.x << ' ' << _vec.y << ' ' << _vec.z;
}

static std::ostream& operator<<(std::ostream& _stream, const glm::quat& _quat)
{
	return _stream << _quat.x << ' ' << _quat.y << ' ' << _quat.z << ' ' << _quat.w;
}

std::ostream& operator<<(std::ostream& _stream, const Bone& _bone)
{
	return _stream <<
		_bone.getName() << ' ' <<
		_bone.parentIndex + 1 << ' ' <<
		_bone.transform.getTranslation() << ' ' <<
		_bone.transform.getScale() << ' ' <<
		_bone.transform.getOrientation();
}

void Bone::setParent(Bone* _newParent)
{
	parent = _newParent;
	if (_newParent)
	{
		parentIndex = _newParent->index;
	}
	else
	{
		parentIndex = -1;
	}
}

void Bone::setNextSibling(Bone* _newSibling)
{
	nextSibling = _newSibling;
}

void Bone::removeSibling(Bone* _sibling)
{
	if (this == _sibling)
	{
		nextSibling = nullptr;
		parent = nullptr;
		return;
	}

	if (nextSibling == nullptr)
	{
		return;
	}

	Bone* tempNext = nextSibling;
	if (nextSibling == _sibling)
	{
		nextSibling = nextSibling->nextSibling;
	}

	tempNext->removeSibling(_sibling);
}
