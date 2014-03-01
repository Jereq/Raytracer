#include "Skeleton.h"

Skeleton::Skeleton()
{
}

Skeleton::Skeleton(const Pose::ptr _bindPose)
	: bindPose(_bindPose),
	currentPose(new Pose(_bindPose)),
	bindToCurrentTransforms(_bindPose->getNumberOfBones())
{
}
