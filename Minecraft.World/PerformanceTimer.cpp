#include "stdafx.h"
#include "PerformanceTimer.h"

PerformanceTimer::PerformanceTimer()
{
#ifndef _CONTENT_PACKAGE
	// Get the frequency of the timer
	LARGE_INTEGER qwTicksPerSec;
	QueryPerformanceFrequency( &qwTicksPerSec );
	m_fSecsPerTick = 1.0f / static_cast<float>(qwTicksPerSec.QuadPart);
	
	Reset();
#endif
}

void PerformanceTimer::Reset()
{
#ifndef _CONTENT_PACKAGE
	QueryPerformanceCounter( &m_qwStartTime );
#endif
}

void PerformanceTimer::PrintElapsedTime(const wstring &description)
{
#ifndef _CONTENT_PACKAGE
	LARGE_INTEGER qwNewTime, qwDeltaTime;

	QueryPerformanceCounter( &qwNewTime );

	qwDeltaTime.QuadPart = qwNewTime.QuadPart - m_qwStartTime.QuadPart;
	float fElapsedTime = m_fSecsPerTick * static_cast<FLOAT>(qwDeltaTime.QuadPart);

	app.DebugPrintf("TIMER: %ls: Elapsed time %f\n", description.c_str(), fElapsedTime);
#endif
}