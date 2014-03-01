#include "ModelData.h"

ModelData::ModelData(cl::Buffer _vertexBuffer, int _vertexCount)
	: vertexCount(_vertexCount),
	vertexBuffer(_vertexBuffer),
	_isAnimated(false)
{
}

bool ModelData::isAnimated() const
{
	return _isAnimated;
}

void ModelData::isAnimated(bool _val)
{
	_isAnimated = _val;
}

int ModelData::getVertexCount() const
{
	return vertexCount;
}

cl::Buffer ModelData::getVertexBuffer() const
{
	return vertexBuffer;
}

Pose::c_ptr ModelData::getBindPose() const
{
	return bindPose;
}

void ModelData::setBindPose(const std::vector<Bone>& _bones)
{
	setBindPose(Pose::c_ptr(new Pose(_bones)));
}

void ModelData::setBindPose(Pose::c_ptr _pose)
{
	bindPose = _pose;
	_isAnimated = true;
}
