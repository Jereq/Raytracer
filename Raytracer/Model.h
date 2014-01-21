#pragma once

#include "ObjModel.h"

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
	glm::vec3 position;
	glm::vec3 rotation;
	float scale;
};
