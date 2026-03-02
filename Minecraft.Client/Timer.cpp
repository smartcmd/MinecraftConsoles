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
    __int64 nowMs = System::currentTimeMillis();
    __int64 passedMs = nowMs - lastMs;
    __int64 msSysTime = System::nanoTime() / 1000000;
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
            __int64 passedMsSysTime = msSysTime - lastMsSysTime;

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
    __int64 nowMs = System::currentTimeMillis();
    __int64 passedMs = nowMs - lastMs;
    __int64 msSysTime = System::nanoTime() / 1000000;
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
            __int64 passedMsSysTime = msSysTime - lastMsSysTime;

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