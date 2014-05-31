#pragma once

#include <string>

static const std::string fallbackModelPath = "resources/cube.obj";

struct ModelResourcePaths
{
	std::string model;
	std::string diffuseTexture;
	std::string normalTexture;
};

static const ModelResourcePaths modelPaths[] = {
	{
		"resources/cubeInv.obj",
		"resources/bthcolor.dds",
		"resources/Bump.png",
	},
	{
		"resources/cube.obj",
		"resources/CubeMap_COLOR.png",
		"resources/CubeMap_NRM.png",
	},
	{
		"resources/12 tri.obj",
		"resources/bthcolor.dds",
		"resources/Default_NRM.png",
	},
	{
		"resources/48 tri.obj",
		"resources/bthcolor.dds",
		"resources/Default_NRM.png",
	},
	{
		"resources/192 tri.obj",
		"resources/bthcolor.dds",
		"resources/Default_NRM.png",
	},
	{
		"resources/768 tri.obj",
		"resources/bthcolor.dds",
		"resources/Default_NRM.png",
	},
	{
		"resources/3072 tri.obj",
		"resources/bthcolor.dds",
		"resources/Default_NRM.png",
	},
	{
		"resources/bth.obj",
		"resources/bthcolor.dds",
		"resources/Default_NRM.png",
	},
	{
		"resources/tube.obj",
		"resources/CubeMap_COLOR.png",
		"resources/CubeMap_NRM.png",
	},
};

static const int NUM_MODELS = sizeof(modelPaths) / sizeof(ModelResourcePaths);
