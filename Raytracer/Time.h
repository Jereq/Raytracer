#pragma once

#define __CL_ENABLE_EXCEPTIONS
#define CL_GL_INTEROP
#include "CL/cl.hpp"

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace Time
{
	extern std::vector<std::pair<std::string, uint64_t>> timers;
	extern std::chrono::high_resolution_clock::time_point prevTimingPoint;

	void initTimer();
	void registerTimer(const std::string& _name, uint64_t _initVal = 0ui64);
	void incTime(const std::string& _name, uint64_t _nanoSeconds);
	void incTime(const std::string& _name, const std::chrono::system_clock::duration& _duration);
	void incTime(const std::string& _name, const cl::Event& _event);
	void incTime(const std::string& _name, const std::vector<cl::Event>& _events);
	void printTimerToConsole(const std::pair<std::string, uint64_t>& _timer, double _deltaTime);
	void resetTimers();
}
