#include "Time.h"

#include <iomanip>
#include <iostream>

#include "CLHelper.h"

std::vector<std::pair<std::string, uint64_t>> Time::timers;
std::chrono::high_resolution_clock::time_point Time::prevTimingPoint;

static unsigned int widestName = 0;

void Time::initTimer()
{
	prevTimingPoint = std::chrono::high_resolution_clock::now();
}

void Time::registerTimer(const std::string& _name, uint64_t _initVal)
{
	if (_name.size() > widestName)
		widestName = _name.size();

	timers.push_back(std::make_pair(_name, 0));
}

void Time::incTime(const std::string& _name, uint64_t _nanoSeconds)
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

void Time::incTime(const std::string& _name, const std::chrono::system_clock::duration& _duration)
{
	incTime(_name, std::chrono::duration_cast<std::chrono::nanoseconds>(_duration).count());
}

void Time::incTime(const std::string& _name, const cl::Event& _event)
{
	incTime(_name, getExecutionTime(_event));
}

void Time::incTime(const std::string& _name, const std::vector<cl::Event>& _events)
{
	for (const cl::Event& ev : _events)
	{
		incTime(_name, ev);
	}
}

void Time::printTimerToConsole(const std::pair<std::string, uint64_t>& _timer, double _deltaTime)
{
	std::cout << std::left << std::setw(widestName) << _timer.first << ": " << std::right << std::setw(5) << toSeconds(_timer.second) * 100.0 / _deltaTime << "%" << std::endl;
}

void Time::resetTimers()
{
	for (auto& timer : timers)
	{
		timer.second = 0;
	}
}
