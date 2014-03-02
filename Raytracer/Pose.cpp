#include "Pose.h"

Pose::Pose(const std::vector<Bone>& _bones)
	: bones(_bones)
{
	Bone::setupSkeleton(bones);
}

Pose::Pose(const c_ptr _pose)
	: bones(_pose->bones)
{
	Bone::setupSkeleton(bones);
}

int Pose::getNumberOfBones() const
{
	return bones.size();
}

std::vector<Bone>& Pose::getBones()
{
	return bones;
}

void Pose::calculateOffsetTo(Pose::c_ptr _other, std::vector<glm::mat4>& _out) const
{
	assert(getNumberOfBones() == _other->getNumberOfBones());

	for (int i = 0; i < getNumberOfBones(); ++i)
	{
		_out[i] = _other->bones[i].getObjectTransform() * glm::inverse(bones[i].getObjectTransform());
	}
}
