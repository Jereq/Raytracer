#include "Bone.h"
#include "Camera.h"
#include "GLWindow.h"
#include "MovingLight.h"

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include "CL/cl.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "AnimatedObjModel.h"
#include "Model.h"
#include "ModelPaths.h"
#include "ObjModel.h"
#include "TestSettings.h"
#include "TextureManager.h"
#include "TubeGenerator.h"

bool runningTests = false;
const float timePerTest = 10.f;
const float timeBeforeTest = 1.f;
float timeLeftInTest;
unsigned int currentTest = 0;
bool beforeTest;

std::ofstream logFile;
void useSettings(const TestSetting& setting);

void startTests()
{
	runningTests = true;
	currentTest = 0;
	timeLeftInTest = timeBeforeTest;
	beforeTest = true;

	useSettings(testSettings[0]);

	logFile.close();
}

void initCL(cl::Context& _context, std::vector<cl::Device>& _devices, cl::CommandQueue& _queue)
{
	cl_int err = CL_SUCCESS;

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.size() == 0)
	{
		throw std::exception("No OpenCL platform found.");
	}

	HGLRC glCtx = wglGetCurrentContext();

	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties) platforms[0](),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_GL_CONTEXT_KHR, (cl_context_properties)glCtx,
		0
	};

	static clGetGLContextInfoKHR_fn clGetGLContextInfoKHR;
	if (!clGetGLContextInfoKHR)
	{
		clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
		if (!clGetGLContextInfoKHR)
		{
			throw std::exception("Failed to query proc address for clGetGLContextInfoKHR.");
		}
	}

	cl_device_id interopDevice;
	cl_int status = clGetGLContextInfoKHR(properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof(cl_device_id), &interopDevice, nullptr);
	cl::Device dev(interopDevice);

	_devices.push_back(dev);

	_context = cl::Context(_devices, properties);

	_queue = cl::CommandQueue(_context, dev, CL_QUEUE_PROFILING_ENABLE, &err);
}

bool createBuffers(std::vector<float>& a, std::vector<float>& b, std::vector<float>& res, unsigned int size)
{
	a.resize(size);
	b.resize(size);
	res.resize(size);

	for (unsigned int i = 0; i < size; i++)
	{
		a[i] = b[i] = (float) i;
	}

	return true;
}

cl::Program createProgramFromFile(cl::Context& _context, std::vector<cl::Device>& _devices, const std::string& _filename)
{
	std::string kernelString;
	std::ifstream in(_filename, std::ios::in | std::ios::binary);
	if (in)
	{
		in.seekg(0, std::ios::end);
		kernelString.resize((unsigned int) in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&kernelString[0], kernelString.size());
		in.close();
	}
	else
	{
		std::string errMsg("Error opening kernel file: ");
		errMsg += strerror(errno);

		throw std::exception(errMsg.c_str());
	}

	cl::Program::Sources source(1,
		std::make_pair(kernelString.c_str(), kernelString.size()));
	cl::Program program = cl::Program(_context, source);

	//std::cout << "Building program..." << std::endl;

	try
	{
		program.build(_devices, "-Werror -cl-fast-relaxed-math -cl-denorms-are-zero -I ./");
	}
	catch (const cl::Error&)
	{
		for (size_t i = 0; i < _devices.size(); i++)
		{
			std::string log;
			program.getBuildInfo(_devices[i], CL_PROGRAM_BUILD_LOG, &log);
			std::cout << "Build log[" << i << "]:" << std::endl << log << std::endl;
		}

		throw;
	}

	return program;
}

std::ostream& operator<<(std::ostream& _in, const glm::vec4& _vec)
{
	return _in << "(" << _vec.x << ", " << _vec.y << ", " << _vec.z << ", " << _vec.w << ")";
}

struct Ray
{
	glm::vec4 position;
	glm::vec4 direction;
	glm::vec4 diffuseReflectivity;
	glm::vec4 surfaceNormal;
	glm::vec4 reflectDir;
	float distance;
	float shininess;
	float strength;
	float totalStrength;
	int inShadow;
	int collideGroup;
	int collideObject;
	float padding[1];
};

struct Sphere
{
	glm::vec4 position;
	glm::vec4 diffuseReflectivity;
	float radius;
	float reflectFraction;
	float padding[2];
};

glm::vec2 dir;
double prevXPos, prevYPos;
glm::vec2 rotation;

const static unsigned int MAX_LIGHTS = 10;
unsigned int numLights = 1;
unsigned int numBounces = 1;
float cubeReflect = 0.5f;
float cubeReflectStep = 0.1f;

const glm::vec3 modelPositions[NUM_MODELS] = {
	glm::vec3(0.f, 0.f, 0.f),
	glm::vec3(0.f, 0.f, 0.f),
	glm::vec3(-3.f, -0.5f, 4.f),
	glm::vec3(-3.f, -0.5f, 2.f),
	glm::vec3(-3.f, -0.5f, 0.f),
	glm::vec3(-3.f, -0.5f, -2.f),
	glm::vec3(-3.f, -0.5f, -4.f),
	glm::vec3(0.f, 0.f, 0.f),
	glm::vec3(-0.2f, -0.4f, 0.f),
};

const float modelScales[NUM_MODELS] = {
	20.f,
	1.f,
	0.005f,
	0.005f,
	0.005f,
	0.005f,
	0.005f,
	0.04f,
	0.4f,
};

const std::pair<glm::vec3, float> modelRotations[NUM_MODELS] = {
	std::make_pair(glm::normalize(glm::vec3(1.f, 0.f, 0.f)), 0.f),
	std::make_pair(glm::normalize(glm::vec3(1.f, 1.f, 0.f)), 10.f),
	std::make_pair(glm::normalize(glm::vec3(0.f, 1.f, 0.f)), 20.f),
	std::make_pair(glm::normalize(glm::vec3(1.f, 0.f, 1.f)), 30.f),
	std::make_pair(glm::normalize(glm::vec3(0.f, 1.f, 0.f)), 40.f),
	std::make_pair(glm::normalize(glm::vec3(1.f, 0.f, 0.f)), 50.f),
	std::make_pair(glm::normalize(glm::vec3(1.f, 0.f, 0.f)), 60.f),
	std::make_pair(glm::normalize(glm::vec3(1.f, 0.f, 0.f)), 70.f),
	std::make_pair(glm::normalize(glm::vec3(0.f, 1.f, 0.f)), 15.f),
};

bool showModels[NUM_MODELS] = {true};
unsigned int modelTriangleCount[NUM_MODELS];

void updateSetting(const std::string& _name, const std::string& _value);

void updateModelCount()
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

unsigned int threadGroupSize = 32;
cl::NDRange local2D(32, 1);
cl::NDRange linearLocalSize(32);

void useSettings(const TestSetting& setting);
int superSampling = 1;
bool sizeChanged = true;


void keyCallback(GLFWwindow* _window, int _key, int _scanCode, int _action, int _mod)
{
	if (runningTests)
		return;

	float forward;
	if (_action == GLFW_PRESS)
		forward = 1.f;
	else if (_action == GLFW_RELEASE)
		forward = -1.f;
	else
		return;

	switch (_key)
	{
	case GLFW_KEY_A:
		dir.x -= forward;
		break;

	case GLFW_KEY_D:
		dir.x += forward;
		break;

	case GLFW_KEY_W:
		dir.y -= forward;
		break;

	case GLFW_KEY_S:
		dir.y += forward;
		break;

	case GLFW_KEY_R:
		if (_action == GLFW_PRESS)
		{
			numLights++;
			if (numLights > MAX_LIGHTS)
				numLights = MAX_LIGHTS;

			updateSetting("NumLights", std::to_string(numLights));
		}
		break;

	case GLFW_KEY_F:
		if (_action == GLFW_PRESS)
		{
			numLights--;
			if (numLights < 1)
				numLights = 1;

			updateSetting("NumLights", std::to_string(numLights));
		}
		break;

	case GLFW_KEY_T:
		if (_action == GLFW_PRESS)
		{
			numBounces++;

			updateSetting("NumBounces", std::to_string(numBounces));
		}
		break;

	case GLFW_KEY_G:
		if (_action == GLFW_PRESS)
		{
			if (numBounces > 1)
				numBounces--;

			updateSetting("NumBounces", std::to_string(numBounces));
		}
		break;

	case GLFW_KEY_Y:
		if (_action == GLFW_PRESS)
		{
			cubeReflect += cubeReflectStep;
			if (cubeReflect > 1.f)
			{
				cubeReflect = 1.f;
			}
		}
		break;

	case GLFW_KEY_H:
		if (_action == GLFW_PRESS)
		{
			cubeReflect -= cubeReflectStep;
			if (cubeReflect < 0.f)
			{
				cubeReflect = 0.f;
			}
		}
		break;

	case GLFW_KEY_U:
		if (_action == GLFW_PRESS)
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
		break;

	case GLFW_KEY_J:
		if (_action == GLFW_PRESS)
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
		break;

	case GLFW_KEY_M:
		if (_action == GLFW_PRESS)
		{
			startTests();
		}
		break;

	case GLFW_KEY_I:
		if (_action == GLFW_PRESS)
		{
			superSampling++;
			sizeChanged = true;
		}
		break;

	case GLFW_KEY_K:
		if (_action == GLFW_PRESS)
		{
			if(superSampling > 1)
			{
				superSampling--;
				sizeChanged = true;
			}
		}
		break;

	default:
		if (_key >= GLFW_KEY_1 && _key < GLFW_KEY_1 + NUM_MODELS && _action == GLFW_PRESS)
		{
			int model = _key - GLFW_KEY_1;
			showModels[model] = !showModels[model];

			updateModelCount();
		}
	}
}

void cursorPosCallback(GLFWwindow* _window, double _xPos, double _yPos)
{
	if (runningTests)
		return;

	double deltaX = _xPos - prevXPos;
	double deltaY = _yPos - prevYPos;
	prevXPos = _xPos;
	prevYPos = _yPos;

	const static float rotationSpeed = 1.f;
	rotation.x += (float)(rotationSpeed * deltaX);
	rotation.y += (float)(rotationSpeed * -deltaY);
	if(rotation.x > 180.f)
		rotation.x -= 360.f;
	if(rotation.x < -180.f)
		rotation.x += 360.f;

	if (rotation.y > 90.f)
		rotation.y = 90.f;
	if (rotation.y < -90.f)
		rotation.y = -90.f;
}

int windowWidth = 0;
int windowHeight = 0;

void resizeWindowCallback(GLFWwindow* _window, int _width, int _height)
{
	sizeChanged = true;
	windowWidth = _width;
	windowHeight = _height;
}

cl_ulong getExecutionTime(const cl::Event& _event)
{
	cl_ulong start = _event.getProfilingInfo<CL_PROFILING_COMMAND_START>();
	cl_ulong end = _event.getProfilingInfo<CL_PROFILING_COMMAND_END>();
	return end - start;
}

double toSeconds(cl_ulong _nanoSeconds)
{
	return _nanoSeconds / 1000000000.0; // Nanoseconds to seconds
}

void glErr()
{
	GLenum res = glGetError();
	while (res)
	{
		res = glGetError();
	}
}

std::vector<std::pair<std::string, uint64_t>> timers;
std::vector<std::pair<std::string, std::string>> settings;
unsigned int widestName = 0;
std::chrono::high_resolution_clock::time_point prevTimingPoint;

void initTimer()
{
	prevTimingPoint = std::chrono::high_resolution_clock::now();
}

void registerTimer(const std::string& _name, uint64_t _initVal = 0ui64)
{
	if (_name.size() > widestName)
		widestName = _name.size();

	timers.push_back(std::make_pair(_name, 0));
}

void incTime(const std::string& _name, uint64_t _nanoSeconds)
{
	for (auto& val : timers)
	{
		if (val.first == _name)
		{
			val.second += _nanoSeconds;
			return;
		}
	}

	registerTimer(_name, _nanoSeconds);
}

void incTime(const std::string& _name, const std::chrono::system_clock::duration& _duration)
{
	incTime(_name, std::chrono::duration_cast<std::chrono::nanoseconds>(_duration).count());
}

void updateSetting(const std::string& _name, const std::string& _value)
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

void updateSetting(const std::string& _name, float _value)
{
	updateSetting(_name, std::to_string(_value));
}

void incTime(const std::string& _name, const std::vector<cl::Event>& _events)
{
	for (const cl::Event& ev : _events)
	{
		incTime(_name, getExecutionTime(ev));
	}
}

void openLogFile()
{
	if (logFile.is_open())
		logFile.close();
	
	time_t currTime = time(nullptr);
#pragma warning (suppress : 4996)
	tm* currentLocalTime = localtime(&currTime);

	std::ostringstream filename;
	filename << "GPU_usage_log_" << std::put_time(currentLocalTime, "%H+%M+%S") << ".csv"; 
	logFile.open(filename.str());
}

void printLogFileHeader()
{
	if (settings.empty())
		return;

	logFile << settings[0].first;
	for (unsigned int i = 1; i < settings.size(); i++)
	{
		logFile << ',' << settings[i].first;
	}

	for (unsigned int i = 0; i < timers.size(); i++)
	{
		logFile << ',' << timers[i].first;
	}

	logFile << std::endl;
}

void printTimerToConsole(const std::pair<std::string, uint64_t>& _timer, double _deltaTime)
{
	std::cout << std::left << std::setw(widestName) << _timer.first << ": " << std::right << std::setw(5) << toSeconds(_timer.second) * 100.0 / _deltaTime << "%" << std::endl;
}

void printSettingsToLogFile()
{
	if (settings.empty())
		return;

	logFile << settings[0].second;
	for (unsigned int i = 1; i < settings.size(); i++)
	{
		logFile << ',' << settings[i].second;
	}
}

void printTimersAndReset()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	auto deltaTime = currentTime - prevTimingPoint;
	prevTimingPoint = currentTime;
	
	double d_deltaTime = std::chrono::duration<double>(deltaTime).count();

	if (timers.empty())
		return;

	if (!logFile.is_open())
	{
		openLogFile();
		printLogFileHeader();
	}

	printSettingsToLogFile();

	for (unsigned int i = 0; i < timers.size(); i++)
	{
		auto& val = timers[i];

		if (!runningTests)
		{
			printTimerToConsole(timers[i], d_deltaTime);
		}
		logFile << ',' << toSeconds(val.second);
		
		val.second = 0;
	}

	logFile << std::endl;
	
	logFile.flush();
}

void resetTimers()
{
	for (auto& timer : timers)
	{
		timer.second = 0;
	}
}

cl::Event runKernel(const cl::CommandQueue& queue, const cl::Kernel& kernel, const cl::NDRange& globalSize, const cl::NDRange& groupSize, std::vector<cl::Event>& events)
{
	cl::Event event;
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize, groupSize, &events, &event);
	events.push_back(event);
	return event;
}

unsigned int leastMultiple(unsigned int _val, unsigned int _mul)
{
	return ((_val + _mul - 1) / _mul) * _mul;
}

bool shouldChangeWindowSize = false;

void useSettings(const TestSetting& setting)
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

int frames = 0;

void runTests(float _deltaTime)
{
	timeLeftInTest -= _deltaTime;
	if (timeLeftInTest <= 0.f)
	{
		if (beforeTest)
		{
			timeLeftInTest = timePerTest;
			beforeTest = false;

			resetTimers();
			frames = 0;
		}
		else
		{
			currentTest++;
			if (currentTest >= numTests)
			{
				runningTests = false;
				useSettings(defaultSetting);
				logFile.close();

				return;
			}

			printTimersAndReset();
			useSettings(testSettings[currentTest]);
			timeLeftInTest = timeBeforeTest;
			beforeTest = true;
		}
	}
}

int main(int argc, char** argv)
{
	const static int width = 1024;
	const static int height = 768;
	windowWidth = width;
	windowHeight = height;
	const static float speed = 2.f;

	const static int NUM_SPHERES = 10;

	const static std::string WINDOW_TITLE("Raytracing madness");

	const static std::chrono::system_clock::duration MEASURE_TIME(std::chrono::seconds(5));
	const static double MEASURE_TIME_D = std::chrono::duration_cast<std::chrono::duration<double>>(MEASURE_TIME).count();

	float animationTime = 0.f;

	updateSetting("NumBounces", (float)numBounces);
	updateSetting("NumLights", (float)numLights);
	
	try
	{
		GLWindow window(WINDOW_TITLE, windowWidth, windowHeight);
		window.setKeyCallback(&keyCallback);
		window.setMouseCallback(&cursorPosCallback);
		window.setFramebufferSizeCallback(&resizeWindowCallback);

		cl::Context context;
		std::vector<cl::Device> devices;
		cl::CommandQueue queue;
		initCL(context, devices, queue);

		window.createFramebuffer(windowWidth, windowHeight);
		
		cl::Program colorProgram = createProgramFromFile(context, devices, "writeImage.cl");
		cl::Kernel accumulateColorKernel(colorProgram, "accumulateImage");
		cl::Kernel dumpImageKernel(colorProgram, "dumpImage");

		cl::Program rayProgram = createProgramFromFile(context, devices, "rayTracing.cl");
		cl::Kernel primaryRaysKernel(rayProgram, "primaryRays");
		cl::Kernel findClosestSpheresKernel(rayProgram, "findClosestSpheres");
		cl::Kernel findClosestTrianglesKernel(rayProgram, "findClosestTriangles");
		cl::Kernel detectShadowWithSpheres(rayProgram, "detectShadowWithSpheres");
		cl::Kernel detectShadowWithTriangles(rayProgram, "detectShadowWithTriangles");
		cl::Kernel updateRaysToLightKernel(rayProgram, "updateRaysToLight");
		cl::Kernel moveRaysToIntersectionKernel(rayProgram, "moveRaysToIntersection");

		cl::Program transformProgram = createProgramFromFile(context, devices, "Transform.cl");
		cl::Kernel transformVerticesKernel(transformProgram, "transformVertices");
		cl::Kernel transformSkeletalVerticesKernel(transformProgram, "transformSkeletalVertices");

		int numRays;
		cl::Buffer primaryRaysBuffer;
		cl::Buffer accumulationBuffer;
		cl::BufferRenderGL renderbuffer;
		std::vector<cl::Memory> glObjects;

		Camera camera(45.f, (float)windowWidth / (float)windowHeight);
		camera.setViewDirection(glm::vec3(0.f, 0.f, -1.f));
		camera.setPosition(glm::vec3(0.f, 1.f, -2.f));

		primaryRaysKernel.setArg(1, glm::transpose(camera.getInvViewProjectionMatrix()));
		primaryRaysKernel.setArg(2, glm::vec4(camera.getPosition(), 1.f));

		std::vector<Sphere> spheres(NUM_SPHERES);
		for (Sphere& s : spheres)
		{
			s.position = glm::vec4(glm::ballRand(glm::pow((float)NUM_SPHERES, 1.f/3.f) * 3.f), 1.f);
			s.diffuseReflectivity = glm::vec4(glm::abs(glm::sphericalRand(0.5f)), 1.f);
			s.radius = glm::linearRand(0.1f, 2.f);
			s.reflectFraction = glm::linearRand(0.5f, 0.7f);
		}

		cl::Buffer spheresBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(Sphere) * NUM_SPHERES, spheres.data());

		findClosestSpheresKernel.setArg(2, spheresBuffer);
		findClosestSpheresKernel.setArg(3, NUM_SPHERES);
		findClosestSpheresKernel.setArg(4, 0);

		std::vector<MovingLight> movLights;
		for (unsigned int i = 0; i < MAX_LIGHTS; i++)
		{
			movLights.push_back(MovingLight(glm::vec4(50.f, 50.f, 50.f, 0.f),
				glm::vec4(i, 0.f, 9.f, 1.f), glm::vec4(i, 0.f, -9.f, 1.f), 1.f / (i + 1)));
		}

		cl::Buffer lightBuffer(context, CL_MEM_READ_ONLY, sizeof(Light) * movLights.size());

		detectShadowWithSpheres.setArg(2, spheresBuffer);
		detectShadowWithSpheres.setArg(3, NUM_SPHERES);
		detectShadowWithSpheres.setArg(4, 0);

		updateRaysToLightKernel.setArg(2, lightBuffer);
		updateRaysToLightKernel.setArg(3, 0);

		typedef std::chrono::duration<double> dSec;

		auto currentTime = std::chrono::high_resolution_clock::now();
		auto prevTime = currentTime;
		auto prevPrint = currentTime;
		initTimer();

		TextureManager textureManager(context);

		Model models[NUM_MODELS];

		ModelInstance modelInstances[NUM_MODELS];
		for (unsigned int i = 0; i < NUM_MODELS - 1; i++)
		{
			ObjModel obj;

			if (!obj.Initialize(context, modelPaths[i].model.c_str()))
			{
				if (!obj.Initialize(context, fallbackModelPath.c_str()))
				{
					throw std::exception(("Failed to load model: " + modelPaths[i].model).c_str());
				}
				else
				{
					std::cout << "Warning: Failed to load model: " << modelPaths[i].model << ", using fallback model." << std::endl;
				}
			}
			modelTriangleCount[i] = obj.GetVertexCount() / 3;
			models[i].data.reset(new ModelData(obj.getBuffer(), obj.GetVertexCount()));
			models[i].transformedVertices = cl::Buffer(context, CL_MEM_READ_ONLY, obj.GetVertexCount() * sizeof(Vertex));
			models[i].diffuseMap = textureManager.loadTexture(modelPaths[i].diffuseTexture);
			models[i].normalMap = textureManager.loadTexture(modelPaths[i].normalTexture);

			modelInstances[i].model = &models[i];
			modelInstances[i].world.setTranslation(modelPositions[i]);
			modelInstances[i].world.setScale(glm::vec3(modelScales[i]));
		}

		AnimatedObjModel aniModelLoader(context);
		ModelData::ptr modelData = aniModelLoader.loadFromFile("resources/tube.aobj");

		models[NUM_MODELS - 1].data = modelData;
		models[NUM_MODELS - 1].transformedVertices = cl::Buffer(context, CL_MEM_READ_ONLY, modelData->getVertexCount() * sizeof(Vertex));
		models[NUM_MODELS - 1].diffuseMap = textureManager.loadTexture(modelPaths[NUM_MODELS - 1].diffuseTexture);
		models[NUM_MODELS - 1].normalMap = textureManager.loadTexture(modelPaths[NUM_MODELS - 1].normalTexture);

		modelInstances[NUM_MODELS - 1].model = models + NUM_MODELS - 1;
		modelInstances[NUM_MODELS - 1].world.setTranslation(modelPositions[NUM_MODELS - 1]);
		modelInstances[NUM_MODELS - 1].world.setScale(glm::vec3(modelScales[NUM_MODELS - 1]));
		modelInstances[NUM_MODELS - 1].skeleton = Skeleton(context, modelData->getBindPose());

		updateModelCount();

		cl::NDRange global2D;
		cl::NDRange linearGlobalSize;
		
		updateSetting("Local2DSize", std::to_string(local2D[0]) + "x" + std::to_string(local2D[1]));
		updateSetting("LocalLinearSize", std::to_string(linearLocalSize[0]));
		
		accumulateColorKernel.setArg(3, lightBuffer);

		while (!window.shouldClose())
		{
			frames++;
			updateSetting("NumFrames", (float)frames);

			prevTime = currentTime;
			currentTime = std::chrono::high_resolution_clock::now();

			incTime("Duration", currentTime - prevTime);
			double deltaTime = dSec(currentTime - prevTime).count();

			if (runningTests)
			{
				runTests((float)deltaTime);
			}
			else if (currentTime - prevPrint > MEASURE_TIME)
			{
				prevPrint += MEASURE_TIME;
				window.setTitle(WINDOW_TITLE + " | FPS: " + std::to_string((int)(frames / MEASURE_TIME_D)));
				std::cout << "FPS: " << std::fixed << std::setprecision(1) << frames / MEASURE_TIME_D << ", " << std::setprecision(2) << 1000.0 * MEASURE_TIME_D / frames << " ms/F" << std::endl;
				printTimersAndReset();
				std::cout << std::endl;

				frames = 0;
			}

			if (shouldChangeWindowSize)
			{
				window.setWindowSize(windowWidth, windowHeight);
				shouldChangeWindowSize = false;
			}

			if (sizeChanged)
			{
				sizeChanged = false;

				window.updateFramebuffer(windowWidth, windowHeight);

				updateSetting("WindowWidth", (float)windowWidth);
				updateSetting("WindowHeight", (float)windowHeight);

				renderbuffer = cl::BufferRenderGL(context, CL_MEM_READ_WRITE, window.getRenderbuffer());

				glObjects.clear();
				glObjects.push_back(renderbuffer);
		
				numRays = windowWidth * windowHeight * superSampling * superSampling;
				primaryRaysBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, numRays * sizeof(Ray));
				accumulationBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, numRays * sizeof(cl_float4));

				primaryRaysKernel.setArg(0, primaryRaysBuffer);
				primaryRaysKernel.setArg(3, windowWidth * superSampling);
				primaryRaysKernel.setArg(4, windowHeight * superSampling);
				primaryRaysKernel.setArg(5, accumulationBuffer);
				
				findClosestSpheresKernel.setArg(0, primaryRaysBuffer);
				findClosestSpheresKernel.setArg(1, numRays);

				detectShadowWithSpheres.setArg(0, primaryRaysBuffer);
				detectShadowWithSpheres.setArg(1, numRays);

				updateRaysToLightKernel.setArg(0, primaryRaysBuffer);
				updateRaysToLightKernel.setArg(1, numRays);

				moveRaysToIntersectionKernel.setArg(0, primaryRaysBuffer);
				moveRaysToIntersectionKernel.setArg(1, numRays);

				findClosestTrianglesKernel.setArg(0, primaryRaysBuffer);
				findClosestTrianglesKernel.setArg(1, numRays);

				detectShadowWithTriangles.setArg(0, primaryRaysBuffer);
				detectShadowWithTriangles.setArg(1, numRays);

				dumpImageKernel.setArg(0, accumulationBuffer);
				dumpImageKernel.setArg(1, primaryRaysBuffer);
				dumpImageKernel.setArg(2, renderbuffer);
		
				accumulateColorKernel.setArg(0, accumulationBuffer);
				accumulateColorKernel.setArg(1, primaryRaysBuffer);
				accumulateColorKernel.setArg(2, numRays);
				
				camera.setScreenRatio((float)windowWidth / (float)windowHeight);
			}
			
			global2D = cl::NDRange(leastMultiple(windowWidth, local2D[0]), leastMultiple(windowHeight, local2D[1]));
			linearGlobalSize = cl::NDRange(leastMultiple(numRays, linearLocalSize[0]));

			if (dir != glm::vec2(0.f))
			{
				glm::vec2 velocity = glm::normalize(dir) * (float)(speed * deltaTime);
				glm::mat4 rotY = glm::rotate(glm::mat4(), -rotation.x, glm::vec3(0.f, 1.f, 0.f));
				camera.setPosition(camera.getPosition() + glm::vec3(rotY * glm::vec4(velocity.x, 0.f, velocity.y, 0.f)));
			}
			camera.setRotation(glm::vec3(rotation.y, -rotation.x, 0.f));

			animationTime += (float)deltaTime;
			std::vector<Bone>& aniBones = modelInstances[NUM_MODELS - 1].skeleton.getCurrentPose()->getBones();
			aniBones[0].getLocalTransform().setScale(glm::vec3(1.f, 1.f + sinf(animationTime * 3.1f) * 0.2f, 1.f));
			for (size_t i = 1; i < aniBones.size(); ++i)
			{
				auto& bone = aniBones[i];
				bone.getLocalTransform().setOrientation(glm::quat(glm::rotate((sinf(animationTime) + 1.f) * 180.f / (aniBones.size() - 1), glm::vec3(0.f, 0.f, 1.f))));
			}
			
			findClosestTrianglesKernel.setArg(4, cubeReflect);

			std::vector<Light> pointLights;
			for (MovingLight& l : movLights)
			{
				l.onFrame((float)deltaTime);
				pointLights.push_back(l.light);
			}

			for (unsigned int i = 0; i < numLights; i++)
			{
				spheres[i].position = pointLights[i].position;
				spheres[i].radius = 0.1f;
			}

			std::vector<cl::Event> events;

			cl::Event writeLightsEvent, writeSpheresEvent;
			queue.enqueueWriteBuffer(lightBuffer, false, 0, sizeof(Light) * pointLights.size(), pointLights.data(), &events, &writeLightsEvent);
			queue.enqueueWriteBuffer(spheresBuffer, false, 0, sizeof(Sphere) * numLights, spheres.data(), &events, &writeSpheresEvent);

			window.clearFramebuffer(1.f, 0.f, 0.f);
			glFinish();

			auto startCL = std::chrono::high_resolution_clock::now();

			primaryRaysKernel.setArg(1, glm::transpose(camera.getInvViewProjectionMatrix()));
			primaryRaysKernel.setArg(2, glm::vec4(camera.getPosition(), 1.f));

			std::vector<cl::Event> intersectSpheresEvents;
			std::vector<cl::Event> triangleEvents;
			std::vector<cl::Event> updateRaysToLights;
			std::vector<cl::Event> sphereShadowEvents;
			std::vector<cl::Event> triangleShadowEvents;
			std::vector<cl::Event> accumulateColorEvents;
			std::vector<cl::Event> moveRaysEvents;
			std::vector<cl::Event> transformModelEvents;

			cl::NDRange superSampledGlobal2D(global2D[0] * superSampling, global2D[1] * superSampling);
			cl::Event primEvent = runKernel(queue, primaryRaysKernel, superSampledGlobal2D, local2D, events);

			for (unsigned int k = 0; k < NUM_MODELS; k++)
			{
				ModelInstance& model = modelInstances[k];

				if (showModels[k])
				{
					model.world.setOrientation(
						glm::quat(glm::rotate(modelRotations[k].second * (float)deltaTime, modelRotations[k].first)) *
						model.world.getOrientation());

					const glm::mat4& world = model.world.getTransform();

					if (model.model->data->isAnimated())
					{
						model.skeleton.setWorld(world);

						transformSkeletalVerticesKernel.setArg(0, model.model->data->getVertexBuffer());
						transformSkeletalVerticesKernel.setArg(1, model.model->transformedVertices);
						transformSkeletalVerticesKernel.setArg(2, model.skeleton.getTransformBuffer(queue));
						int vertexCount = model.model->data->getVertexCount();
						transformSkeletalVerticesKernel.setArg(3, vertexCount);
						transformModelEvents.push_back(runKernel(queue, transformSkeletalVerticesKernel, cl::NDRange(leastMultiple(vertexCount, linearLocalSize[0])), linearLocalSize, events));
					}
					else
					{
						glm::mat4 invTranspose = glm::inverse(world);

						transformVerticesKernel.setArg(0, model.model->data->getVertexBuffer());
						transformVerticesKernel.setArg(1, model.model->transformedVertices);
						transformVerticesKernel.setArg(2, glm::transpose(world));
						transformVerticesKernel.setArg(3, invTranspose);
						int vertexCount = model.model->data->getVertexCount();
						transformVerticesKernel.setArg(4, vertexCount);
						transformModelEvents.push_back(runKernel(queue, transformVerticesKernel, cl::NDRange(leastMultiple(vertexCount, linearLocalSize[0])), linearLocalSize, events));
					}
				}
			}

			for (unsigned int j = 0; j < numBounces; j++)
			{
				intersectSpheresEvents.push_back(runKernel(queue, findClosestSpheresKernel, linearGlobalSize, linearLocalSize, events));

				for (unsigned int k = 0; k < NUM_MODELS; k++)
				{
					ModelInstance& model = modelInstances[k];

					if (showModels[k])
					{
						findClosestTrianglesKernel.setArg(2, model.model->transformedVertices);
						findClosestTrianglesKernel.setArg(3, model.model->data->getVertexCount() / 3);
						findClosestTrianglesKernel.setArg(5, model.model->diffuseMap);
						findClosestTrianglesKernel.setArg(6, model.model->normalMap);
						findClosestTrianglesKernel.setArg(7, k + 1);
						triangleEvents.push_back(runKernel(queue, findClosestTrianglesKernel, linearGlobalSize, linearLocalSize, events));
					}
				}
				moveRaysEvents.push_back(runKernel(queue, moveRaysToIntersectionKernel, linearGlobalSize, linearLocalSize, events));

				for (unsigned int i = 0; i < numLights; i++)
				{
					updateRaysToLightKernel.setArg(3, i);
					updateRaysToLights.push_back(runKernel(queue, updateRaysToLightKernel, linearGlobalSize, linearLocalSize, events));
					sphereShadowEvents.push_back(runKernel(queue, detectShadowWithSpheres, linearGlobalSize, linearLocalSize, events));
					for (unsigned int k = 0; k < NUM_MODELS; k++)
					{
						ModelInstance& model = modelInstances[k];

						if (showModels[k])
						{
							detectShadowWithTriangles.setArg(2, model.model->transformedVertices);
							detectShadowWithTriangles.setArg(3, model.model->data->getVertexCount() / 3);
							detectShadowWithTriangles.setArg(4, k + 1);
							triangleShadowEvents.push_back(runKernel(queue, detectShadowWithTriangles, linearGlobalSize, linearLocalSize, events));
						}
					}

					accumulateColorKernel.setArg(4, i);
					accumulateColorEvents.push_back(runKernel(queue, accumulateColorKernel, linearGlobalSize, linearLocalSize, events));
				}
			}

			cl::Event aqEvent;
			queue.enqueueAcquireGLObjects(&glObjects, &events, &aqEvent);
			events.push_back(aqEvent);
			
			dumpImageKernel.setArg(3, superSampling);
			cl::Event dumpEvent = runKernel(queue, dumpImageKernel, global2D, local2D, events);

			cl::Event relEvent;
			queue.enqueueReleaseGLObjects(&glObjects, &events, &relEvent);

			auto endCL = std::chrono::high_resolution_clock::now();

			queue.finish();

			auto drawStart = std::chrono::high_resolution_clock::now();
			window.drawFramebuffer();
			auto drawEnd = std::chrono::high_resolution_clock::now();

			incTime("Aquire objects", getExecutionTime(aqEvent));
			incTime("Release objects", getExecutionTime(relEvent));
			incTime("Write lights", getExecutionTime(writeLightsEvent));
			incTime("Write spheres", getExecutionTime(writeLightsEvent));
			incTime("Primary rays", getExecutionTime(primEvent));
			incTime("Intersection Spheres", intersectSpheresEvents);
			incTime("Intersection Triangles", triangleEvents);
			incTime("Move rays", moveRaysEvents);
			incTime("Rays to light", updateRaysToLights);
			incTime("Shadow spheres", sphereShadowEvents);
			incTime("Shadow triangles", triangleShadowEvents);
			incTime("Accumulate colors", accumulateColorEvents);
			incTime("Dump image", getExecutionTime(dumpEvent));

			incTime("Total OpenCL", drawStart - startCL);
			incTime("OpenCL enqueue work", endCL - startCL);
			incTime("OpenGL blit and swap", drawEnd - drawStart);
		}
	}
	catch (const cl::Error& err)
	{
		std::cerr << "Error: (" << err.err() << ") " << err.what() << std::endl;
#ifdef _DEBUG
		throw;
#else
		system("pause");
		return EXIT_FAILURE;
#endif
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
#ifdef _DEBUG
		throw;
#else
		system("pause");
		return EXIT_FAILURE;
#endif
	}

	return EXIT_SUCCESS;
}
