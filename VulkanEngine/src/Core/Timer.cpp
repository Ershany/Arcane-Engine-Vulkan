#include "pch.h"
#include "Timer.h"

Timer::Timer()
{
	m_StartTime = glfwGetTime();
}

void Timer::Reset()
{
	m_StartTime = glfwGetTime();
}

double Timer::Elapsed() const
{
	return glfwGetTime() - m_StartTime;
}
