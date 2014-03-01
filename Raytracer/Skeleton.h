#pragma once

#include "Bone.h"
#include "Pose.h"

#include <CL/cl.hpp>

class Skeleton
{
private:
	const Pose::ptr bindPose;
	const Pose::ptr currentPose;
	std::vector<glm::mat4> bindToCurrentTransforms;

	cl::Buffer transformBuffer;

public:
	Skeleton();
	Skeleton(cl::Context _context, const Pose::ptr _bindPose);
};
