#pragma once

#include "CachedTransform.h"
#include "ObjModel.h"
#include "Skeleton.h"

class Model
{
public:
	ObjModel model;
	cl::Buffer transformedVertices;
	cl::Image2D diffuseMap;
	cl::Image2D normalMap;
};

class ModelInstance
{
public:
	Model* model;
	CachedTransform world;
	Skeleton skeleton;
};
