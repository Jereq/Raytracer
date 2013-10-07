#include "Camera.h"
#include "GLWindow.h"

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include <CL/cl.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <iostream>
#include <vector>

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
		clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddressForPlatform(platforms[0](), "clGetGLContextInfoKHR");
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

	_queue = cl::CommandQueue(_context, dev, 0, &err);
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
		program.build(_devices, "-Werror");
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

cl::Kernel compileKernel(cl::Context& _context, std::vector<cl::Device>& _devices, const std::string& _filename, const std::string& _kernelName)
{
	cl::Program program = createProgramFromFile(_context, _devices, _filename);

	//std::cout << "Creating kernel..." << std::endl;

	return cl::Kernel(program, _kernelName.c_str());
}

void runKernel(cl::CommandQueue& _queue, cl::Kernel& _kernel, size_t size, cl::Buffer& a, cl::Buffer& b, cl::Buffer& res)
{
	//std::cout << "Setting kernel arguments..." << std::endl;

	_kernel.setArg(0, a);
	_kernel.setArg(1, b);
	_kernel.setArg(2, res);
	_kernel.setArg(3, size);
		
	//std::cout << "Starting kernel..." << std::endl;
		
	size_t local_ws = 256;
	size_t global_ws = ((size + local_ws - 1) / local_ws) * local_ws;
	cl::Event event;
	_queue.enqueueNDRangeKernel(
		_kernel,
		cl::NullRange,
		cl::NDRange(global_ws),
		cl::NDRange(local_ws),
		nullptr,
		&event);

	//std::cout << "Waiting..." << std::endl;

	event.wait();
}

void runImageKernel(cl::CommandQueue& _queue, cl::Kernel& _kernel, cl::ImageGL& _image, cl::Buffer& _rays, int _width, int _height)
{
	//std::cout << "Setting kernel arguments..." << std::endl;

	_kernel.setArg(0, _image);
	_kernel.setArg(1, _rays);

	//std::cout << "Starting kernel..." << std::endl;

	size_t local_ws = 16;
	size_t global_wsx = ((_width + local_ws - 1) / local_ws) * local_ws;
	size_t global_wsy = ((_height + local_ws - 1) / local_ws) * local_ws;
	cl::Event event;
	_queue.enqueueNDRangeKernel(
		_kernel,
		cl::NullRange,
		cl::NDRange(global_wsx, global_wsy),
		cl::NDRange(local_ws, local_ws),
		nullptr,
		&event);

	//std::cout << "Waiting..." << std::endl;

	event.wait();
}

std::ostream& operator<<(std::ostream& _in, const glm::vec4& _vec)
{
	return _in << "(" << _vec.x << ", " << _vec.y << ", " << _vec.z << ", " << _vec.w << ")";
}

int toMultiple(int _val, int _mul)
{
	return ((_val + _mul - 1) / _mul) * _mul;
}

struct ray
{
	glm::vec4 position;
	glm::vec4 direction;
};

glm::vec2 dir;
double prevXPos, prevYPos;
glm::vec2 rotation;

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
	}
}

void cursorPosCallback(GLFWwindow* _window, double _xPos, double _yPos)
{
	double deltaX = _xPos - prevXPos;
	double deltaY = _yPos - prevYPos;
	prevXPos = _xPos;
	prevYPos = _yPos;

	const static float rotationSpeed = 1.f;
	rotation.x += rotationSpeed * deltaX;
	rotation.y += rotationSpeed * -deltaY;
	if(rotation.x > 180.f)
		rotation.x -= 360.f;
	if(rotation.x < -180.f)
		rotation.x += 360.f;

	if (rotation.y > 90.f)
		rotation.y = 90.f;
	if (rotation.y < -90.f)
		rotation.y = -90.f;
}

int main(int argc, char** argv)
{
	const static int width = 640;
	const static int height = 480;
	const static float speed = 2.f;

	try
	{
		GLWindow window("Raytracing madness", width, height);
		window.setKeyCallback(&keyCallback);
		window.setMouseCallback(&cursorPosCallback);

		cl::Context context;
		std::vector<cl::Device> devices;
		cl::CommandQueue queue;
		initCL(context, devices, queue, true);

		window.createFramebuffer(width, height);

		cl::Kernel kernel = compileKernel(context, devices, "testImage.cl", "testImage");
		cl::Program rayProgram = createProgramFromFile(context, devices, "rayTracing.cl");
		cl::Kernel primaryRaysKernel(rayProgram, "primaryRays");

		cl::ImageGL clImage = cl::ImageGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, window.getFramebufferTexture());
		int imageWidth = window.getFramebufferWidth();
		int imageHeight = window.getFramebufferHeight();

		std::vector<cl::Memory> glObjects;
		glObjects.push_back(clImage);

		Camera camera(45.f, (float)width / (float)height);
		camera.setViewDirection(glm::vec3(0.f, 0.f, -1.f));
		camera.setPosition(glm::vec3(0.f, 0.f, 0.f));

		std::vector<ray> primaryRays(width * height);
		cl::Buffer primaryRaysBuffer(context, CL_MEM_READ_WRITE, primaryRays.size() * sizeof(ray));

		primaryRaysKernel.setArg(0, primaryRaysBuffer);
		primaryRaysKernel.setArg(1, glm::transpose(camera.getInvViewProjectionMatrix()));
		primaryRaysKernel.setArg(2, glm::vec4(camera.getPosition(), 1.f));
		primaryRaysKernel.setArg(3, width);
		primaryRaysKernel.setArg(4, height);

		cl::NDRange local(32, 8);
		cl::NDRange global(toMultiple(width, local[0]), toMultiple(height, local[1]));
		queue.enqueueReadBuffer(primaryRaysBuffer, true, 0, width * height * sizeof(ray), primaryRays.data());
		
		int dummy = 42;

		/*glm::vec4 tests[] = {
			glm::vec4(0.f, 0.f, -0.01f, 1.f),
			glm::vec4(0.f, 0.f, 1.f, 1.f),
			glm::vec4(0.f, 0.f, -1.f, 1.f),
			glm::vec4(1.f, 0.f, 0.f, 1.f),
			glm::vec4(-1.f, 0.f, 0.f, 1.f),
			glm::vec4(0.f, 1.f, 0.f, 1.f),
			glm::vec4(0.f, 0.f, 0.f, 1.f)
		};

		glm::mat4 m = camera.getViewProjectionMatrix();
		glm::mat4 invM = camera.getInvViewProjectionMatrix();

		for (auto v : tests)
		{
			glm::vec4 mVec = m * v;
			if (mVec.w != 0.f)
			{
				mVec *= 1.f / mVec.w;
			}

			std::cout << "m    * " << v << " => " << mVec << std::endl;
			glm::vec4 nVec = invM * v;
			std::cout << "invM * " << v << " => " << nVec / nVec.w << std::endl;
		}*/

		while (!window.shouldClose())
		{
			float deltaTime = 0.001f;
			if (dir != glm::vec2(0.f))
			{
				glm::vec2 velocity = glm::normalize(dir) * speed * deltaTime;
				glm::mat4 rotY = glm::rotate(glm::mat4(), -rotation.x, glm::vec3(0.f, 1.f, 0.f));
				camera.setPosition(camera.getPosition() + glm::vec3(rotY * glm::vec4(velocity.x, 0.f, velocity.y, 0.f)));
			}
			camera.setRotation(glm::vec3(rotation.y, -rotation.x, 0.f));

			window.clearFramebuffer(1.f, 0.f, 0.f);
			glFinish();

			primaryRaysKernel.setArg(1, glm::transpose(camera.getInvViewProjectionMatrix()));
			primaryRaysKernel.setArg(2, glm::vec4(camera.getPosition(), 1.f));

			cl::Event tEv;
			queue.enqueueNDRangeKernel(primaryRaysKernel, cl::NullRange, global, local, nullptr, &tEv);
			tEv.wait();

			queue.enqueueAcquireGLObjects(&glObjects);

			// Render
			runImageKernel(queue, kernel, clImage, primaryRaysBuffer, imageWidth, imageHeight);

			queue.enqueueReleaseGLObjects(&glObjects);
			queue.finish();

			window.drawFramebuffer();
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
