#include "stdafx.h"
#include "Consoles_SoundEngine.h"


bool ConsoleSoundEngine::GetIsPlayingStreamingCDMusic()				
{ 
	return m_bIsPlayingStreamingCDMusic;
}
bool ConsoleSoundEngine::GetIsPlayingStreamingGameMusic()			
{
	return m_bIsPlayingStreamingGameMusic;
}
void ConsoleSoundEngine::SetIsPlayingStreamingCDMusic(bool bVal)	
{
	m_bIsPlayingStreamingCDMusic=bVal;
}
void ConsoleSoundEngine::SetIsPlayingStreamingGameMusic(bool bVal)	
{
	m_bIsPlayingStreamingGameMusic=bVal;
}
bool ConsoleSoundEngine::GetIsPlayingEndMusic()						
{ 
	return m_bIsPlayingEndMusic;
}
bool ConsoleSoundEngine::GetIsPlayingNetherMusic()					
{ 
	return m_bIsPlayingNetherMusic;
}
void ConsoleSoundEngine::SetIsPlayingEndMusic(bool bVal)			
{
	m_bIsPlayingEndMusic=bVal;
}
void ConsoleSoundEngine::SetIsPlayingNetherMusic(bool bVal)			
{
	m_bIsPlayingNetherMusic=bVal;
}

void ConsoleSoundEngine::tick()
{
	if (scheduledSounds.empty())
	{
		return;
	}

	for (auto it = scheduledSounds.begin(); it != scheduledSounds.end();)
	{
		SoundEngine::ScheduledSound *next = *it;
		next->delay--;

		if (next->delay <= 0)
		{
			play(next->iSound, next->x, next->y, next->z, next->volume, next->pitch);
			it =scheduledSounds.erase(it);
			delete next;
		}
		else
		{
			++it;
		}
	}
}

void ConsoleSoundEngine::schedule(int iSound, float x, float y, float z, float volume, float pitch, int delayTicks)
{
	scheduledSounds.push_back(new SoundEngine::ScheduledSound(iSound, x, y, z, volume, pitch, delayTicks));
}

ConsoleSoundEngine::ScheduledSound::ScheduledSound(int iSound, float x, float y, float z, float volume, float pitch, int delay)
{
	this->iSound = iSound;
	this->x = x;
	this->y = y;
	this->z = z;
	this->volume = volume;
	this->pitch = pitch;
	this->delay = delay;
}