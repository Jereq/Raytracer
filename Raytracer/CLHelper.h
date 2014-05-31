#pragma once

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include "CL/cl.hpp"

void initCL(cl::Context& _context, std::vector<cl::Device>& _devices, cl::CommandQueue& _queue);
cl::Program createProgramFromFile(cl::Context& _context, std::vector<cl::Device>& _devices, const std::string& _filename);
cl_ulong getExecutionTime(const cl::Event& _event);
double toSeconds(cl_ulong _nanoSeconds);
cl::Event runKernel(const cl::CommandQueue& queue, const cl::Kernel& kernel, const cl::NDRange& globalSize, const cl::NDRange& groupSize, std::vector<cl::Event>& events);
