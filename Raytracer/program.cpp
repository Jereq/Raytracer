#include "Camera.h"
#include "GLWindow.h"
#include "MovingLight.h"

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include "CL/cl.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

#include <IL/il.h>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "ObjModel.h"

void printPlatformInfo(cl::Platform& plat)
{
	std::string profile = plat.getInfo<CL_PLATFORM_PROFILE>();
	std::string version = plat.getInfo<CL_PLATFORM_VERSION>();
	std::string name = plat.getInfo<CL_PLATFORM_NAME>();
	std::string vendor = plat.getInfo<CL_PLATFORM_VENDOR>();
	std::string extensions = plat.getInfo<CL_PLATFORM_EXTENSIONS>();

	std::cout << "  Profile    : " << profile << std::endl;
	std::cout << "  Version    : " << version << std::endl;
	std::cout << "  Name       : " << name << std::endl;
	std::cout << "  Vendor     : " << vendor << std::endl;
	std::cout << "  Extensions : " << extensions << std::endl;
}

void printContextInfo(cl::Context& con)
{
	std::cout << "  Reference count : " << con.getInfo<CL_CONTEXT_REFERENCE_COUNT>() << std::endl;
	std::cout << "  Num devices     : " << con.getInfo<CL_CONTEXT_NUM_DEVICES>() << std::endl;
}

void printDeviceInfo(cl::Device& dev)
{
	//std::cout << "  Address bits              : " << dev.getInfo<CL_DEVICE_ADDRESS_BITS>() << std::endl;
	std::cout << "  Available                 : " << dev.getInfo<CL_DEVICE_AVAILABLE>() << std::endl;
	//std::cout << "  Compiler available        : " << dev.getInfo<CL_DEVICE_COMPILER_AVAILABLE>() << std::endl;
	//std::cout << "  Double fp config          : " << dev.getInfo<CL_DEVICE_DOUBLE_FP_CONFIG>() << std::endl;
	//std::cout << "  Endian little             : " << dev.getInfo<CL_DEVICE_ENDIAN_LITTLE>() << std::endl;
	//std::cout << "  Error correction support  : " << dev.getInfo<CL_DEVICE_ERROR_CORRECTION_SUPPORT>() << std::endl;
	//std::cout << "  Execution capabilities    : " << dev.getInfo<CL_DEVICE_EXECUTION_CAPABILITIES>() << std::endl;
	//std::cout << "  Extensions                : " << dev.getInfo<CL_DEVICE_EXTENSIONS>() << std::endl;
	std::cout << "  Global mem cache size     : " << dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>() << std::endl;
	//std::cout << "  Global mem cache type     : " << dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>() << std::endl;
	std::cout << "  Global mem cacheline size : " << dev.getInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>() << std::endl;
	std::cout << "  Global mem size           : " << dev.getInfo<CL_DEVICE_GLOBAL_MEM_SIZE>() << std::endl;
	std::cout << "  Host unified memory       : " << dev.getInfo<CL_DEVICE_HOST_UNIFIED_MEMORY>() << std::endl;
	//std::cout << "  Image support             : " << dev.getInfo<CL_DEVICE_IMAGE_SUPPORT>() << std::endl;
	//std::cout << "  Image 2D max height       : " << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>() << std::endl;
	//std::cout << "  Image 2D max width        : " << dev.getInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>() << std::endl;
	std::cout << "  Local mem size            : " << dev.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>() << std::endl;
	std::cout << "  Local mem type            : " << dev.getInfo<CL_DEVICE_LOCAL_MEM_TYPE>() << std::endl;
	std::cout << "  Max clock frequency       : " << dev.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>() << std::endl;
	std::cout << "  Max compute units         : " << dev.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << std::endl;
	std::cout << "  Max constant args         : " << dev.getInfo<CL_DEVICE_MAX_CONSTANT_ARGS>() << std::endl;
	std::cout << "  Max constant buffer size  : " << dev.getInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>() << std::endl;
	std::cout << "  Max mem alloc size        : " << dev.getInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>() << std::endl;
	std::cout << "  Max parameter size        : " << dev.getInfo<CL_DEVICE_MAX_PARAMETER_SIZE>() << std::endl;
	std::cout << "  Max read image args       : " << dev.getInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>() << std::endl;
	std::cout << "  Max samplers              : " << dev.getInfo<CL_DEVICE_MAX_SAMPLERS>() << std::endl;
	std::cout << "  Max work group size       : " << dev.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;
	std::cout << "  Max work item dimensions  : " << dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>() << std::endl;

	std::vector<size_t> maxItemSizes = dev.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
	for (size_t i = 0; i < maxItemSizes.size(); i++)
	{
		std::cout << "  Max work item sizes[" << i << "]    : " << maxItemSizes[i] << std::endl;
	}
	std::cout << "  Max write image args      : " << dev.getInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>() << std::endl;
	std::cout << "  Name                      : " << dev.getInfo<CL_DEVICE_NAME>() << std::endl;
	std::cout << "  Version                   : " << dev.getInfo<CL_DEVICE_VERSION>() << std::endl;
	std::cout << "  Profiling timer resolution: " << dev.getInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>() << std::endl;
}

void initCL(cl::Context& _context, std::vector<cl::Device>& _devices, cl::CommandQueue& _queue, bool printInfo)
{
	cl_int err = CL_SUCCESS;

	//std::cout << "Getting platforms..." << std::endl;

	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);
	if (platforms.size() == 0)
	{
		throw std::exception("No OpenCL platform found.");
	}

	unsigned int c = 0;
	if (printInfo)
	{
		for (auto p : platforms)
		{
			std::cout << "Platform " << c++ << ":" << std::endl;
			printPlatformInfo(p);
		}

		std::cout << "Using platform 0" << std::endl;
	}

	//std::cout << "Creating context..." << std::endl;

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

	if (printInfo)
	{
		std::cout << "Device:" << std::endl;
		printDeviceInfo(dev);
	}

	_devices.push_back(dev);

	_context = cl::Context(_devices, properties);

	if (printInfo)
	{
		std::cout << "Context:" << std::endl;
		printContextInfo(_context);
	}

	//std::cout << "Creating queue..." << std::endl;

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
		program.build(_devices, "-Werror -cl-fast-relaxed-math -cl-denorms-are-zero");
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

void runImageKernel(cl::CommandQueue& _queue, cl::Kernel& _kernel, cl::BufferRenderGL& _renderbuffer, cl::Buffer& _rays,
					cl::Buffer& _lights, int _lightIdx, int _width, int _height, std::vector<cl::Event>* _events, cl::Event* _outEvent)
{
	//std::cout << "Setting kernel arguments..." << std::endl;

	_kernel.setArg(0, _renderbuffer);
	_kernel.setArg(1, _rays);
	_kernel.setArg(2, _lights);
	_kernel.setArg(3, _lightIdx);

	//std::cout << "Starting kernel..." << std::endl;

	size_t local_ws = 16;
	size_t global_wsx = ((_width + local_ws - 1) / local_ws) * local_ws;
	size_t global_wsy = ((_height + local_ws - 1) / local_ws) * local_ws;

	_queue.enqueueNDRangeKernel(
		_kernel,
		cl::NullRange,
		cl::NDRange(global_wsx, global_wsy),
		cl::NDRange(local_ws, local_ws),
		_events,
		_outEvent);
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
std::string modelNames[] = {
	"resources/cubeInv.obj",
	"resources/cube.obj",
	"resources/12 tri.obj",
	"resources/48 tri.obj",
	"resources/192 tri.obj",
	"resources/768 tri.obj",
	"resources/3072 tri.obj",
	"resources/bth.obj",
};
const int NUM_MODELS = sizeof(modelNames) / sizeof(std::string);

const glm::vec3 modelPositions[NUM_MODELS] = {
	glm::vec3(0.f, 0.f, 0.f),
	glm::vec3(0.f, 0.f, 0.f),
	glm::vec3(-3.f, -0.5f, 4.f),
	glm::vec3(-3.f, -0.5f, 2.f),
	glm::vec3(-3.f, -0.5f, 0.f),
	glm::vec3(-3.f, -0.5f, -2.f),
	glm::vec3(-3.f, -0.5f, -4.f),
	glm::vec3(0.f, 0.f, 0.f),
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
};

const glm::vec3 modelRotationSpeeds[NUM_MODELS] = {
	glm::vec3(0.f, 0.f, 0.f),
	glm::vec3(5.f, 10.f, 13.f),
	glm::vec3(10.f, 1.f, 1.f),
	glm::vec3(15.f, 1.f, 1.f),
	glm::vec3(20.f, 1.f, 1.f),
	glm::vec3(30.f, 1.f, 1.f),
	glm::vec3(90.f, 1.f, 1.f),
	glm::vec3(12.f, 6.7f, 42.f),
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

void keyCallback(GLFWwindow* _window, int _key, int _scanCode, int _action, int _mod)
{
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

bool sizeChanged = true;
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

std::ofstream logFile;

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

		printTimerToConsole(timers[i], d_deltaTime);
		logFile << ',' << toSeconds(val.second);
		
		val.second = 0;
	}

	logFile << std::endl;
	
	logFile.flush();
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
		initCL(context, devices, queue, false);

		window.createFramebuffer(windowWidth, windowHeight);

		ilInit();
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

		ILuint image = ilGenImage();
		ilBindImage(image);
		ilLoadImage("resources/bthcolor.dds");
		
		static const std::pair<int, int> formatOrders[] =
		{
			std::make_pair(IL_RGBA, CL_RGBA),
		};

		static const std::pair<int, int> formatTypes[] =
		{
			std::make_pair(IL_FLOAT, CL_FLOAT),
			std::make_pair(IL_UNSIGNED_BYTE, CL_UNORM_INT8),
		};

		cl::ImageFormat format(0, 0);
		ILint imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
		ILint imageType = ilGetInteger(IL_IMAGE_TYPE);
		for (auto& f : formatOrders)
		{
			if (f.first == imageFormat)
			{
				format.image_channel_order = f.second;
				break;
			}
		}

		for (auto& f : formatTypes)
		{
			if (f.first == imageType)
			{
				format.image_channel_data_type = f.second;
				break;
			}
		}

		if (format.image_channel_data_type == 0 || format.image_channel_order == 0)
		{
			throw std::exception("Invalid image format");
		}

		cl_int err = CL_SUCCESS;
		cl::Image2D diffuseTexture(context, CL_MEM_COPY_HOST_PTR | CL_MEM_READ_ONLY, format, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
			0, ilGetData(), &err);

		ilDeleteImage(image);

		ilShutDown();

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
				glm::vec4(i, 0.f, 10.f, 1.f), glm::vec4(i, 0.f, -10.f, 1.f), 1.f / (i + 1)));
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
		int frames = 0;
		initTimer();

		ObjModel objModels[NUM_MODELS];
		cl::Buffer objTransformedModels[NUM_MODELS];
		glm::vec3 objRotations[NUM_MODELS];
		for (unsigned int i = 0; i < NUM_MODELS; i++)
		{
			objModels[i].Initialize(context, modelNames[i].c_str());
			modelTriangleCount[i] = objModels[i].GetVertexCount() / 3;
			objTransformedModels[i] = cl::Buffer(context, CL_MEM_READ_ONLY, objModels[i].GetVertexCount() * sizeof(ObjModel::VertexType));
		}
		updateModelCount();

		findClosestTrianglesKernel.setArg(5, diffuseTexture);
		
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

			if (currentTime - prevPrint > MEASURE_TIME)
			{
				prevPrint += MEASURE_TIME;
				window.setTitle(WINDOW_TITLE + " | FPS: " + std::to_string((int)(frames / MEASURE_TIME_D)));
				std::cout << "FPS: " << std::fixed << std::setprecision(1) << frames / MEASURE_TIME_D << ", " << std::setprecision(2) << 1000.0 * MEASURE_TIME_D / frames << " ms/F" << std::endl;
				printTimersAndReset();
				std::cout << std::endl;

				frames = 0;
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
		
				numRays = windowWidth * windowHeight;
				primaryRaysBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, numRays * sizeof(Ray));
				accumulationBuffer = cl::Buffer(context, CL_MEM_READ_WRITE, numRays * sizeof(cl_float4));

				primaryRaysKernel.setArg(0, primaryRaysBuffer);
				primaryRaysKernel.setArg(3, windowWidth);
				primaryRaysKernel.setArg(4, windowHeight);
				primaryRaysKernel.setArg(5, accumulationBuffer);
				
				findClosestSpheresKernel.setArg(0, primaryRaysBuffer);
				findClosestSpheresKernel.setArg(1, numRays);

				detectShadowWithSpheres.setArg(0, primaryRaysBuffer);
				detectShadowWithSpheres.setArg(1, numRays);
				
				global2D = cl::NDRange(leastMultiple(windowWidth, local2D[0]), leastMultiple(windowHeight, local2D[1]));

				linearGlobalSize = cl::NDRange(leastMultiple(numRays, linearLocalSize[0]));

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

			incTime("Duration", currentTime - prevTime);
			double deltaTime = dSec(currentTime - prevTime).count();
			if (dir != glm::vec2(0.f))
			{
				glm::vec2 velocity = glm::normalize(dir) * (float)(speed * deltaTime);
				glm::mat4 rotY = glm::rotate(glm::mat4(), -rotation.x, glm::vec3(0.f, 1.f, 0.f));
				camera.setPosition(camera.getPosition() + glm::vec3(rotY * glm::vec4(velocity.x, 0.f, velocity.y, 0.f)));
			}
			camera.setRotation(glm::vec3(rotation.y, -rotation.x, 0.f));
			
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

			cl::Event primEvent = runKernel(queue, primaryRaysKernel, global2D, local2D, events);

			for (unsigned int k = 0; k < NUM_MODELS; k++)
			{
				if (showModels[k])
				{
					objRotations[k] += modelRotationSpeeds[k] * (float)deltaTime;

					glm::mat4 translation = glm::transpose(glm::translate(modelPositions[k]));
					glm::mat4 rotationYaw = glm::transpose(glm::rotate(objRotations[k].x, glm::vec3(0.f, 1.f, 0.f)));
					glm::mat4 rotationPitch = glm::transpose(glm::rotate(objRotations[k].y, glm::vec3(1.f, 0.f, 0.f)));
					glm::mat4 rotationRoll = glm::transpose(glm::rotate(objRotations[k].z, glm::vec3(0.f, 0.f, 1.f)));
					glm::mat4 scaling = glm::scale(glm::vec3(modelScales[k]));

					glm::mat4 world = scaling * rotationYaw * rotationPitch * rotationRoll * translation;

					glm::mat4 invTranspose = glm::transpose(glm::inverse(scaling * rotationYaw * rotationPitch * rotationRoll));

					transformVerticesKernel.setArg(0, objModels[k].getBuffer());
					transformVerticesKernel.setArg(1, objTransformedModels[k]);
					transformVerticesKernel.setArg(2, world);
					transformVerticesKernel.setArg(3, invTranspose);
					transformVerticesKernel.setArg(4, objModels[k].GetVertexCount());
					transformModelEvents.push_back(runKernel(queue, transformVerticesKernel, cl::NDRange(leastMultiple(objModels[k].GetVertexCount(), linearLocalSize[0])), linearLocalSize, events));
				}
			}

			for (unsigned int j = 0; j < numBounces; j++)
			{
				intersectSpheresEvents.push_back(runKernel(queue, findClosestSpheresKernel, linearGlobalSize, linearLocalSize, events));

				for (unsigned int k = 0; k < NUM_MODELS; k++)
				{
					if (showModels[k])
					{
						findClosestTrianglesKernel.setArg(2, objTransformedModels[k]);
						findClosestTrianglesKernel.setArg(3, objModels[k].GetVertexCount() / 3);
						findClosestTrianglesKernel.setArg(6, k + 1);
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
						if (showModels[k])
						{
							detectShadowWithTriangles.setArg(2, objTransformedModels[k]);
							detectShadowWithTriangles.setArg(3, objModels[k].GetVertexCount() / 3);
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
		system("pause");
		return EXIT_FAILURE;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		system("pause");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
