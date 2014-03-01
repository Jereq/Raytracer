#include "Pose.h"

Pose::Pose(const std::vector<Bone>& _bones)
	: bones(_bones)
{
	Bone::setupSkeleton(bones);
}

Pose::Pose(const ptr _pose)
	: bones(_pose->bones)
{
	Bone::setupSkeleton(bones);
}

int Pose::getNumberOfBones() const
{
	return bones.size();
}
