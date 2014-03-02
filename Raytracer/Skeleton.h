#pragma once

#include "Bone.h"
#include "Pose.h"

#include <CL/cl.hpp>

class Skeleton
{
private:
	Pose::c_ptr bindPose;
	Pose::ptr currentPose;
	glm::mat4 world;
	
	mutable bool bufferUpdated;
	mutable std::vector<glm::mat4> bindToCurrentTransforms;
	mutable cl::Buffer transformBuffer;

public:
	Skeleton();
	Skeleton(cl::Context _context, Pose::c_ptr _bindPose);

	void setWorld(const glm::mat4& _world);
	Pose::ptr getCurrentPose() const;

	cl::Buffer getTransformBuffer(cl::CommandQueue _queue) const;

private:
	void updateBuffer(cl::CommandQueue _queue) const;
};
