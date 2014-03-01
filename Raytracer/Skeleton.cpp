#include "Skeleton.h"

Skeleton::Skeleton()
{
}

Skeleton::Skeleton(cl::Context _context, const Pose::ptr _bindPose)
	: bindPose(_bindPose),
	currentPose(new Pose(_bindPose)),
	bindToCurrentTransforms(_bindPose->getNumberOfBones()),
	transformBuffer(_context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		sizeof(glm::mat4) * bindToCurrentTransforms.size(),
		bindToCurrentTransforms.data())
{
}
