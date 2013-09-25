#include <bitset>
#include <fstream>
#include <iostream>
#include <vector>

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

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

		cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties) (platform)(), 0 };
		context = cl::Context(CL_DEVICE_TYPE_GPU, properties);

		if (printInfo)
		{
			std::cout << "Context:" << std::endl;
			printContextInfo(context);
		}

		devices = context.getInfo<CL_CONTEXT_DEVICES>();
		if (printInfo)
		{
			c = 0;
			for (auto& dev : devices)
			{
				std::cout << "Device " << c++ << ":" << std::endl;
				printDeviceInfo(dev);
			}
		}
		
		//std::cout << "Creating queue..." << std::endl;

		queue = cl::CommandQueue(context, devices[0], 0, &err);
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

bool runKernel(size_t size, cl::Buffer& a, cl::Buffer& b, cl::Buffer& res)
{
	cl_int err;
	try
	{
		std::cout << "Reading kernel file..." << std::endl;

		std::string kernelString;
		std::ifstream in("vector_add_gpu.cl", std::ios::in | std::ios::binary);
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

		cl::Kernel kernel(program_, "vector_add_gpu", &err);

		std::cout << "Setting kernel arguments..." << std::endl;

		kernel.setArg(0, a);
		kernel.setArg(1, b);
		kernel.setArg(2, res);
		kernel.setArg(3, size);
		
		std::cout << "Starting kernel..." << std::endl;
		
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

		std::cout << "Waiting..." << std::endl;

		event.wait();
	}
	catch (const cl::Error& err)
	{
		std::cerr << "Error: " << err.what() << "(" << err.err() << ")" << std::endl;
		return false;
	}

	return true;
}

int main(int argc, char** argv)
{
	if (!initCL(false))
	{
		system("pause");
		return EXIT_FAILURE;
	}

	std::vector<float> src_a_h;
	std::vector<float> src_b_h;
	std::vector<float> res_h;

	const static int size = 1234567;
	createBuffers(src_a_h, src_b_h, res_h, size);

	cl::Buffer src_a_d(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(float), src_a_h.data());
	cl::Buffer src_b_d(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size * sizeof(float), src_b_h.data());
	cl::Buffer res_d(context, CL_MEM_WRITE_ONLY, size * sizeof(float));

	if (!runKernel(size, src_a_d, src_b_d, res_d))
	{
		system("pause");
		return EXIT_FAILURE;
	}

	queue.enqueueReadBuffer(res_d, CL_TRUE, 0, size * sizeof(float), res_h.data());
	bool success = true;
	for (unsigned int i = 0; i < size; i++)
	{
		if (res_h[i] != src_a_h[i] + src_b_h[i])
		{
			std::cout << "Found invalid number: " << res_h[i] << std::endl;
			success = false;
		}
	}

	if (success)
	{
		std::cout << "Program finished successfully." << std::endl;
	}

	system("pause");

	return EXIT_SUCCESS;
}
