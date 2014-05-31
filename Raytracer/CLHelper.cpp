#include "CLHelper.h"

#include <fstream>
#include <iostream>

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

cl::Event runKernel(const cl::CommandQueue& queue, const cl::Kernel& kernel, const cl::NDRange& globalSize, const cl::NDRange& groupSize, std::vector<cl::Event>& events)
{
	cl::Event event;
	queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalSize, groupSize, &events, &event);
	events.push_back(event);
	return event;
}
