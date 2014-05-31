#pragma once

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include "CL/cl.hpp"

#include "TestSettings.h"

namespace Settings
{
	extern unsigned int threadGroupSize;
	extern cl::NDRange local2D;
	extern cl::NDRange linearLocalSize;
	
	extern int windowWidth;
	extern int windowHeight;
	
	extern bool shouldChangeWindowSize;
	
	extern const unsigned int MAX_LIGHTS;
	extern unsigned int numLights;
	extern unsigned int numBounces;
	
	extern bool showModels[NUM_MODELS];
	extern unsigned int modelTriangleCount[NUM_MODELS];
	
	extern float cubeReflect;
	
	extern int superSampling;
	extern bool sizeChanged;

	extern std::vector<std::pair<std::string, std::string>> settings;

	void updateModelCount();

	void useSettings(const TestSetting& setting);
	void updateSetting(const std::string& _name, const std::string& _value);
	void updateSetting(const std::string& _name, float _value);

	void increaseLights();
	void decreaseLights();

	void increaseBounces();
	void decreaseBounces();

	void increaseCubeReflect();
	void decreaseCubeReflect();

	void increaseThreadGroupSize();
	void decreaseThreadGroupSize();

	void increaseSuperSampling();
	void decreaseSuperSampling();

	void toggleShowModel(int _modelIndex);

	void updateWindowSize(int _width, int _height);
}
