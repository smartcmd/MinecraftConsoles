#include "stdafx.h"
#include "Timer.h"
#include "..\Minecraft.World\System.h"

Timer::Timer(float ticksPerSecond)
{
	// 4J - added initialisers
    lastTime = 0;
    ticks = 0;
    a = 0;
    timeScale = 1;
    passedTime = 0;
    accumMs = 0;
    adjustTime = 1.0;

    this->ticksPerSecond = ticksPerSecond;
    lastMs = System::currentTimeMillis();
    lastMsSysTime = System::nanoTime() / 1000000;
}

void Timer::advanceTime()
{
    int64_t nowMs = System::currentTimeMillis();
    int64_t passedMs = nowMs - lastMs;

    // 4J - Use high-resolution timer for 'now' in seconds
    double now = System::nanoTime() / 1000000000.0;


    if (passedMs > 1000)
	{
        lastTime = now;
    }
	else if (passedMs < 0)
	{
        lastTime = now;
    }
	else
	{
        accumMs += passedMs;
        if (accumMs > 1000)
		{
            int64_t msSysTime = (int64_t)(now * 1000.0);
            int64_t passedMsSysTime = msSysTime - lastMsSysTime;

            double adjustTimeT = accumMs / (double) passedMsSysTime;
            adjustTime += (adjustTimeT - adjustTime) * 0.2f;

            lastMsSysTime = msSysTime;
            accumMs = 0;
        }
        if (accumMs < 0)
		{
            lastMsSysTime = (int64_t)(now * 1000.0);
        }
    }
    lastMs = nowMs;

    double passedSeconds = (now - lastTime) * adjustTime;
    lastTime = now;

    if (passedSeconds < 0) passedSeconds = 0;
    if (passedSeconds > 1) passedSeconds = 1;

    passedTime = (float)( passedTime + (passedSeconds * timeScale * ticksPerSecond));

    ticks = (int) passedTime;
    passedTime -= ticks;

    if (ticks > MAX_TICKS_PER_UPDATE) ticks = MAX_TICKS_PER_UPDATE;

    a = passedTime;
}

void Timer::advanceTimeQuickly()
{

    double passedSeconds = (double) MAX_TICKS_PER_UPDATE / (double) ticksPerSecond;

    passedTime = (float)(passedTime + (passedSeconds * timeScale * ticksPerSecond));
    ticks = (int) passedTime;
    passedTime -= ticks;
    a = passedTime;

    lastMs = System::currentTimeMillis();
    lastMsSysTime = System::nanoTime() / 1000000;

}

void Timer::skipTime()
{
    int64_t nowMs = System::currentTimeMillis();
    int64_t passedMs = nowMs - lastMs;
    int64_t msSysTime = System::nanoTime() / 1000000;
    double now = msSysTime / 1000.0;


    if (passedMs > 1000)
	{
        lastTime = now;
    }
	else if (passedMs < 0)
	{
        lastTime = now;
    }
	else
	{
        accumMs += passedMs;
        if (accumMs > 1000)
		{
            int64_t passedMsSysTime = msSysTime - lastMsSysTime;

            double adjustTimeT = accumMs / (double) passedMsSysTime;
            adjustTime += (adjustTimeT - adjustTime) * 0.2f;

            lastMsSysTime = msSysTime;
            accumMs = 0;
        }
        if (accumMs < 0)
		{
            lastMsSysTime = msSysTime;
        }
    }
    lastMs = nowMs;


    double passedSeconds = (now - lastTime) * adjustTime;
    lastTime = now;

    if (passedSeconds < 0) passedSeconds = 0;
    if (passedSeconds > 1) passedSeconds = 1;

    passedTime = (float)(passedTime + (passedSeconds * timeScale * ticksPerSecond));

    ticks = (int) 0;
    if (ticks > MAX_TICKS_PER_UPDATE) ticks = MAX_TICKS_PER_UPDATE;
    passedTime -= ticks;

}