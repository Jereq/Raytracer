#pragma once

#include "Bone.h"
#include "Pose.h"

#include "CL/cl.hpp"

#include <memory>

class ModelData
{
public:
	typedef std::shared_ptr<ModelData> ptr;

private:
	bool _isAnimated;
	int vertexCount;
	cl::Buffer vertexBuffer;
	Pose::c_ptr bindPose;

public:
	ModelData(cl::Buffer _vertexBuffer, int _vertexCount);

	bool isAnimated() const;
	void isAnimated(bool _val);

	int getVertexCount() const;
	cl::Buffer getVertexBuffer() const;

	Pose::c_ptr getBindPose() const;
	void setBindPose(const std::vector<Bone>& _bones);
	void setBindPose(Pose::c_ptr _pose);
};
