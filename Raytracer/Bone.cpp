#include "Bone.h"

#include <glm/gtc/matrix_transform.hpp>

Bone::Bone()
	: transform(),
	name(),
	parent(nullptr),
	firstChild(nullptr),
	nextSibling(nullptr)
{}

Bone::Bone(
	const glm::vec3& _translation,
	const glm::vec3& _scale,
	const glm::quat& _orientation,
	const std::string& _name
	)
	: transform(_translation, _scale, _orientation),
	name(_name),
	parent(nullptr),
	firstChild(nullptr),
	nextSibling(nullptr)
{}

const std::string& Bone::getName() const
{
	return name;
}

glm::mat4x4 Bone::getLocalTransform() const
{
	return transform.getTransform();
}

glm::mat4x4 Bone::getObjectTransform() const
{
	if (parent)
	{
		return parent->getObjectTransform() * getLocalTransform();
	}
	else
	{
		return getLocalTransform();
	}
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

void Bone::setParent(Bone* _newParent)
{
	parent = _newParent;
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
