#pragma once

#include "CachedTransform.h"
#include "ModelData.h"
#include "Skeleton.h"

class Model
{
public:
	ModelData::ptr data;
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
