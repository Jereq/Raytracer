#include "Skeleton.h"

Skeleton::Skeleton()
{
}

Skeleton::Skeleton(cl::Context _context, Pose::c_ptr _bindPose)
	: bindPose(_bindPose),
	currentPose(new Pose(_bindPose)),
	bufferUpdated(false),
	bindToCurrentTransforms(_bindPose->getNumberOfBones()),
	transformBuffer(_context, CL_MEM_READ_ONLY, sizeof(glm::mat4) * bindToCurrentTransforms.size())
{
}

void Skeleton::setWorld(const glm::mat4& _world)
{
	world = _world;
	bufferUpdated = false;
}

Pose::ptr Skeleton::getCurrentPose() const
{
	return currentPose;
}

cl::Buffer Skeleton::getTransformBuffer(cl::CommandQueue _queue) const
{
	if (!bufferUpdated)
	{
		updateBuffer(_queue);
		bufferUpdated = true;
	}

	return transformBuffer;
}

void Skeleton::updateBuffer(cl::CommandQueue _queue) const
{
	bindPose->calculateOffsetTo(currentPose, bindToCurrentTransforms);

	for (auto& transform : bindToCurrentTransforms)
	{
		transform = glm::transpose(world * transform);
	}

	_queue.enqueueWriteBuffer(transformBuffer, false, 0, sizeof(glm::mat4) * bindToCurrentTransforms.size(), bindToCurrentTransforms.data());
}
