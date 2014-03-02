#pragma once

#include "Bone.h"

#include <memory>

class Pose
{
public:
	typedef std::shared_ptr<Pose> ptr;
	typedef std::shared_ptr<const Pose> c_ptr;

private:
	std::vector<Bone> bones;

public:
	explicit Pose(const std::vector<Bone>& _bones);
	explicit Pose(const c_ptr _pose);

	int getNumberOfBones() const;
	std::vector<Bone>& getBones();

	void calculateOffsetTo(Pose::c_ptr _other, std::vector<glm::mat4>& _out) const;
};
