#include "Timer.h"

using namespace std;
using namespace std::chrono;

Timer::Timer(const milliseconds& duration) : Timer(system_clock::now(), duration)
{
}

Timer::Timer(system_clock::time_point now, const milliseconds& duration) noexcept :
	now{now}, deadline{now + duration}
{
}

bool Timer::IsRunning() const noexcept
{
	return now <= deadline;
}

void Timer::Update()
{
	now = system_clock::now();
}