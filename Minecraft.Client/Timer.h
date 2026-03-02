#pragma once

class Timer
{
private:
	static const int MAX_TICKS_PER_UPDATE = 10;

public:
    float ticksPerSecond;

private:
    double lastTime;
public:
	int ticks;
    float a;
    float timeScale;
    float passedTime;

private:
    __int64 lastMs;
    __int64 lastMsSysTime;
    __int64 accumMs;

	double adjustTime;

public:
	Timer(float ticksPerSecond);
    void advanceTime();
    void advanceTimeQuickly();
    void skipTime();
};