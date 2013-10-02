#include <GL/glew.h>
#include <GL/wglew.h>

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include <CL/cl.hpp>

#include <GLFW/glfw3.h>

#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>

cl::Platform platform;
cl::Context context;
std::vector<cl::Device> devices;
cl::CommandQueue queue;

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

bool initCL(bool printInfo)
{
	cl_int err = CL_SUCCESS;
	try
	{
		//std::cout << "Getting platforms..." << std::endl;

		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);
		if (platforms.size() == 0)
		{
			std::cout << "Platform size 0" << std::endl;
			return false;
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
		platform = platforms[0];

		//std::cout << "Creating context..." << std::endl;

		HGLRC glCtx = wglGetCurrentContext();

		cl_context_properties properties[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties) (platform)(),
			CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
			CL_GL_CONTEXT_KHR, (cl_context_properties)glCtx,
			0
		};

		static clGetGLContextInfoKHR_fn clGetGLContextInfoKHR;
		if (!clGetGLContextInfoKHR)
		{
			clGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddressForPlatform(platform(), "clGetGLContextInfoKHR");
			if (!clGetGLContextInfoKHR)
			{
				std::cout << "Failed to query proc address for clGetGLContextInfoKHR" << std::endl;
				return false;
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

		devices.push_back(dev);

		context = cl::Context(devices, properties);

		if (printInfo)
		{
			std::cout << "Context:" << std::endl;
			printContextInfo(context);
		}

		//std::cout << "Creating queue..." << std::endl;

		queue = cl::CommandQueue(context, dev, 0, &err);
	}
	catch (const cl::Error& err)
	{
		std::cerr << "Error: " << err.what() << "(" << err.err() << ")" << std::endl;
		return false;
	}

	return true;
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

cl::Kernel kernel;

bool compileKernel()
{
	cl_int err;
	try
	{
		std::cout << "Reading kernel file..." << std::endl;

		std::string kernelString;
		std::ifstream in("testImage.cl", std::ios::in | std::ios::binary);
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
			std::cout << "Error opening kernel file: " << strerror(errno) << std::endl;
			return false;
		}

		std::cout << "Creating program..." << std::endl;

		cl::Program::Sources source(1,
			std::make_pair(kernelString.c_str(), kernelString.size()));
		cl::Program program_ = cl::Program(context, source);

		std::cout << "Building program..." << std::endl;

		try
		{
			program_.build(devices, "-Werror");
		}
		catch (const cl::Error& err)
		{
			std::string log;
			program_.getBuildInfo(devices[0], CL_PROGRAM_BUILD_LOG, &log);
			std::cout << "Build log:" << std::endl << log << std::endl;

			throw err;
		}

		std::cout << "Creating kernel..." << std::endl;

		kernel = cl::Kernel(program_, "testImage", &err);
	}
	catch (const cl::Error& err)
	{
		std::cerr << "Error: " << err.what() << "(" << err.err() << ")" << std::endl;
		return false;
	}

	return true;
}

bool runKernel(size_t size, cl::Buffer& a, cl::Buffer& b, cl::Buffer& res)
{
	try
	{
		//std::cout << "Setting kernel arguments..." << std::endl;

		kernel.setArg(0, a);
		kernel.setArg(1, b);
		kernel.setArg(2, res);
		kernel.setArg(3, size);
		
		//std::cout << "Starting kernel..." << std::endl;
		
		size_t local_ws = 256;
		size_t global_ws = ((size + local_ws - 1) / local_ws) * local_ws;
		cl::Event event;
		queue.enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(global_ws),
			cl::NDRange(local_ws),
			nullptr,
			&event);

		//std::cout << "Waiting..." << std::endl;

		event.wait();
	}
	catch (const cl::Error& err)
	{
		std::cerr << "Error: " << err.what() << "(" << err.err() << ")" << std::endl;
		return false;
	}

	return true;
}

bool runImageKernel(cl::ImageGL& image, int width, int height)
{
	try
	{
		//std::cout << "Setting kernel arguments..." << std::endl;
		
		kernel.setArg(0, image);
		
		//std::cout << "Starting kernel..." << std::endl;
		
		size_t local_ws = 16;
		size_t global_wsx = ((width + local_ws - 1) / local_ws) * local_ws;
		size_t global_wsy = ((height + local_ws - 1) / local_ws) * local_ws;
		cl::Event event;
		queue.enqueueNDRangeKernel(
			kernel,
			cl::NullRange,
			cl::NDRange(global_wsx, global_wsy),
			cl::NDRange(local_ws, local_ws),
			nullptr,
			&event);

		//std::cout << "Waiting..." << std::endl;

		event.wait();
	}
	catch (const cl::Error& err)
	{
		std::cerr << "Error: " << err.what() << "(" << err.err() << ")" << std::endl;
		return false;
	}

	return true;
}

GLFWwindow* window;
int windowWidth;
int windowHeight;

bool initOpenGL(int _windowWidth, int _windowHeight)
{
	windowWidth = _windowWidth;
	windowHeight = _windowHeight;

	if (!glfwInit())
	{
		std::cout << "Failed to initialize GLFW." << std::endl;
		return false;
	}

	window = glfwCreateWindow(windowWidth, windowHeight, "Hello World", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();

		std::cout << "Failed to create window." << std::endl;
		return false;
	}

	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err)
	{
		std::cout << "Failed to initialize GLEW(" << err << ")." << std::endl;
		return false;
	}

	return true;
}

void destroyOpenGL()
{
	glfwTerminate();
}

GLuint framebuffer;
GLuint framebufferTexture;
GLuint framebufferWidth;
GLuint framebufferHeight;

bool createFramebuffer(GLuint _framebufferWidth, GLuint _framebufferHeight)
{
	framebufferWidth = _framebufferWidth;
	framebufferHeight = _framebufferHeight;

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D, framebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebufferWidth, framebufferHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
	
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return true;
}

void destroyFramebuffer()
{
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteTextures(1, &framebufferTexture);
}

void blitFramebuffer()
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
	glBlitFramebuffer(0, 0, framebufferWidth, framebufferHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char** argv)
{
	const static int width = 640;
	const static int height = 480;

	if (!initOpenGL(width, height))
	{
		system("pause");
		return EXIT_FAILURE;
	}

	if (!initCL(false))
	{
		destroyOpenGL();
		system("pause");
		return EXIT_FAILURE;
	}

	createFramebuffer(width, height);
	
	if (!compileKernel())
	{
		destroyFramebuffer();
		destroyOpenGL();
		system("pause");
		return EXIT_FAILURE;
	}

	cl::ImageGL clImage;
	try
	{
		clImage = cl::ImageGL(context, CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, framebufferTexture);
	}
	catch (cl::Error& err)
	{
		std::cerr << "Error: " << err.what() << "(" << err.err() << ")" << std::endl;

		destroyFramebuffer();
		destroyOpenGL();

		system("pause");
		return EXIT_FAILURE;
	}

	std::vector<cl::Memory> glObjects;
	glObjects.push_back(clImage);

	while (!glfwWindowShouldClose(window))
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClearColor(1.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		glFinish();
		queue.enqueueAcquireGLObjects(&glObjects);
		if (!runImageKernel(clImage, framebufferWidth, framebufferHeight))
		{
			std::cout << "Failed to run kernel." << std::endl;
			system("pause");
			break;
		}
		queue.enqueueReleaseGLObjects(&glObjects);
		queue.finish();

		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Render

		glClearColor(1.f, 0.f, 1.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		blitFramebuffer();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	destroyFramebuffer();
	destroyOpenGL();

	return EXIT_SUCCESS;
}
