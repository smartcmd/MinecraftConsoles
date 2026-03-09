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
    int64_t lastMs;
    int64_t lastMsSysTime;
    int64_t accumMs;

	double adjustTime;

public:
	Timer(float ticksPerSecond);
    void advanceTime();
    void advanceTimeQuickly();
    void skipTime();
};