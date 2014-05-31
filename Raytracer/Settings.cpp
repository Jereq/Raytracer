#include "Settings.h"

unsigned int Settings::threadGroupSize = 32;
cl::NDRange Settings::local2D(32, 1);
cl::NDRange Settings::linearLocalSize(32);

int Settings::windowWidth = 0;
int Settings::windowHeight = 0;

bool Settings::shouldChangeWindowSize = false;

const static unsigned int Settings::MAX_LIGHTS = 10;
unsigned int Settings::numLights = 1;
unsigned int Settings::numBounces = 1;

bool Settings::showModels[NUM_MODELS] = {true};
unsigned int Settings::modelTriangleCount[NUM_MODELS];

float Settings::cubeReflect = 0.5f;
static const float cubeReflectStep = 0.1f;

int Settings::superSampling = 1;
bool Settings::sizeChanged = true;

std::vector<std::pair<std::string, std::string>> Settings::settings;

void Settings::updateModelCount()
{
	unsigned int numModels = 0;
	unsigned int numTriangles = 0;

	for (unsigned int i = 0; i < NUM_MODELS; i++)
	{
		if (showModels[i])
		{
			numModels++;
			numTriangles += modelTriangleCount[i];
		}
	}

	updateSetting("NumModels", std::to_string(numModels));
	updateSetting("NumTriangles", std::to_string(numTriangles));
}

void Settings::useSettings(const TestSetting& setting)
{
	threadGroupSize = setting.threads;
	local2D = cl::NDRange(32, threadGroupSize / 32);
	linearLocalSize = cl::NDRange(threadGroupSize);
			
	updateSetting("Local2DSize", std::to_string(local2D[0]) + "x" + std::to_string(local2D[1]));
	updateSetting("LocalLinearSize", std::to_string(linearLocalSize[0]));

	windowWidth = setting.width;
	windowHeight = setting.height;
	shouldChangeWindowSize = true;

	numBounces = setting.bounces;
	updateSetting("NumBounces", std::to_string(numBounces));

	numLights = setting.lights;
	updateSetting("NumLights", std::to_string(numLights));

	const bool* modelSetup = modelSetups[setting.triangles];
	for (unsigned int i = 0; i < NUM_MODELS; ++i)
	{
		showModels[i] = modelSetup[i];
	}

	updateModelCount();
}

void Settings::updateSetting(const std::string& _name, const std::string& _value)
{
	for (auto& val : settings)
	{
		if (val.first == _name)
		{
			val.second = _value;
			return;
		}
	}

	settings.push_back(std::make_pair(_name, _value));
}

void Settings::updateSetting(const std::string& _name, float _value)
{
	updateSetting(_name, std::to_string(_value));
}

void Settings::increaseLights()
{
	numLights++;
	if (numLights > MAX_LIGHTS)
		numLights = MAX_LIGHTS;

	updateSetting("NumLights", std::to_string(numLights));
}

void Settings::decreaseLights()
{
	numLights--;
	if (numLights < 1)
		numLights = 1;

	updateSetting("NumLights", std::to_string(numLights));
}

void Settings::increaseBounces()
{
	numBounces++;

	updateSetting("NumBounces", std::to_string(numBounces));
}

void Settings::decreaseBounces()
{
	if (numBounces > 1)
		numBounces--;

	updateSetting("NumBounces", std::to_string(numBounces));
}

void Settings::increaseCubeReflect()
{
	cubeReflect += cubeReflectStep;
	if (cubeReflect > 1.f)
	{
		cubeReflect = 1.f;
	}
}

void Settings::decreaseCubeReflect()
{
	cubeReflect -= cubeReflectStep;
	if (cubeReflect < 0.f)
	{
		cubeReflect = 0.f;
	}
}

void Settings::increaseThreadGroupSize()
{
	static const unsigned int maxThreadGroupSize = 256;
	if (threadGroupSize < maxThreadGroupSize)
	{
		threadGroupSize *= 2;
		local2D = cl::NDRange(32, threadGroupSize / 32);
		linearLocalSize = cl::NDRange(threadGroupSize);
			
		updateSetting("Local2DSize", std::to_string(local2D[0]) + "x" + std::to_string(local2D[1]));
		updateSetting("LocalLinearSize", std::to_string(linearLocalSize[0]));
	}
}

void Settings::decreaseThreadGroupSize()
{
	static const unsigned int minThreadGroupSize = 32;
	if (threadGroupSize > minThreadGroupSize)
	{
		threadGroupSize /= 2;
		local2D = cl::NDRange(32, threadGroupSize / 32);
		linearLocalSize = cl::NDRange(threadGroupSize);
			
		updateSetting("Local2DSize", std::to_string(local2D[0]) + "x" + std::to_string(local2D[1]));
		updateSetting("LocalLinearSize", std::to_string(linearLocalSize[0]));
	}
}

void Settings::increaseSuperSampling()
{
	superSampling++;
	sizeChanged = true;
}

void Settings::decreaseSuperSampling()
{
	if(superSampling > 1)
	{
		superSampling--;
		sizeChanged = true;
	}
}

void Settings::toggleShowModel(int _modelIndex)
{
	if (_modelIndex >= NUM_MODELS)
		return;

	showModels[_modelIndex] = !showModels[_modelIndex];

	updateModelCount();
}

void Settings::updateWindowSize(int _width, int _height)
{
	sizeChanged = true;
	windowWidth = _width;
	windowHeight = _height;
}
