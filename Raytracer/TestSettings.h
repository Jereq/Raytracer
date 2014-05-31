#pragma once

#include "ModelPaths.h"

struct TestSetting
{
	unsigned int threads;
	unsigned int width;
	unsigned int height;
	unsigned int bounces;
	unsigned int lights;
	unsigned int triangles;
};

static const bool modelSetups[][NUM_MODELS] =
{
	{true, false, false, true, true},
	{false},
	{true},
	{true, true},
	{true, true, true},
	{true, false, false, false, true},
	{true, false, false, false, false, true},
	{true, false, false, false, false, false, true},
	{true, false, false, false, false, false, false, true},
	{true, false, false, true},
};

static const TestSetting defaultSetting = { 32, 1024, 768, 1, 1, 0 };

static const TestSetting testSettings[] =
{
	{32, 1024, 768, 4, 2, 0},
	{64, 1024, 768, 4, 2, 0},
	{128, 1024, 768, 4, 2, 0},
	{256, 1024, 768, 4, 2, 0},

	{32, 640,  480, 4, 2, 0},
	{64,  640, 480, 4, 2, 0},
	{128, 640, 480, 4, 2, 0},
	{256, 640, 480, 4, 2, 0},

	{128, 128, 128, 4, 2, 0},
	{128, 800, 600, 4, 2, 0},
	{128, 1024, 768, 4, 2, 0},
	{128, 1280, 1024, 4, 2, 0},

	{128, 1024, 768, 0, 2, 0},
	{128, 1024, 768, 1, 2, 0},
	{128, 1024, 768, 2, 2, 0},
	{128, 1024, 768, 3, 2, 0},
	{128, 1024, 768, 4, 2, 0},
	{128, 1024, 768, 5, 2, 0},
	{128, 1024, 768, 6, 2, 0},
	{128, 1024, 768, 7, 2, 0},
	{128, 1024, 768, 8, 2, 0},
	{128, 1024, 768, 9, 2, 0},
	{128, 1024, 768, 10, 2, 0},
	{128, 1024, 768, 100, 2, 0},

	{128, 1024, 768, 4, 1, 0},
	{128, 1024, 768, 4, 2, 0},
	{128, 1024, 768, 4, 3, 0},
	{128, 1024, 768, 4, 4, 0},
	{128, 1024, 768, 4, 7, 0},
	{128, 1024, 768, 4, 10, 0},

	{128, 1024, 768, 4, 2, 1},
	{128, 1024, 768, 4, 2, 2},
	{128, 1024, 768, 4, 2, 3},
	{128, 1024, 768, 4, 2, 4},
	{128, 1024, 768, 4, 2, 5},
	{128, 1024, 768, 4, 2, 6},
	{128, 320, 240, 4, 2, 2},
	{128, 320, 240, 4, 2, 9},
	{128, 320, 240, 4, 2, 5},
	{128, 320, 240, 4, 2, 6},
	{128, 320, 240, 4, 2, 7},
	{128, 320, 240, 4, 2, 8},
};
static const unsigned int numTests = sizeof(testSettings) / sizeof(TestSetting);
