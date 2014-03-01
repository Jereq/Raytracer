#pragma once

#include "Bone.h"
#include "Pose.h"

class Skeleton
{
private:
	const Pose::ptr bindPose;
	const Pose::ptr currentPose;
	std::vector<glm::mat4> bindToCurrentTransforms;

public:
	Skeleton();
	Skeleton(const Pose::ptr _bindPose);
};
