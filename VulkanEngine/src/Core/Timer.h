#pragma once

class Timer
{
public:
	Timer();

	void Reset();

	double Elapsed() const;
private:
	double m_StartTime;
};
