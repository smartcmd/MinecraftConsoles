#include "../stdafx.h"
#include <sysutil/sysutil_sysparam.h>
#include <cell/sysmodule.h>
#include <sys/spu_initialize.h>
#include <fcntl.h>
#include <unistd.h>
#include <cell/audio.h>
#include "../../Common/Audio/SoundEngine.h"
#include "../../Common/Consoles_App.h"
#include "../../PS3/Miles/include/mss.h"

// This file has the platform specific functions required for PS3 audio

//
// this function configures the audio hardware.
//   you specify the minimum number of channels that you need, and it 
//   returns the number of output channels that will be used (which will
//   be at least your minimum count, but possibly more).  it will return 0
//   for any failure cases.

int SoundEngine::initAudioHardware( int minimum_chans )
{
	int ret;
	int ch_pcm;
	int ch_bit;
	CellAudioOutConfiguration a_config;

	memset( &a_config, 0, sizeof( CellAudioOutConfiguration ) );

	// first lets see how many pcm output channels we have
	ch_pcm = cellAudioOutGetSoundAvailability( CELL_AUDIO_OUT_PRIMARY,
		CELL_AUDIO_OUT_CODING_TYPE_LPCM,
		CELL_AUDIO_OUT_FS_48KHZ, 0 );

	if ( ch_pcm >= minimum_chans )
	{
		a_config.channel = ch_pcm;
		a_config.encoder = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
		a_config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_NONE; /* No downmixer is used */
		cellAudioOutConfigure( CELL_AUDIO_OUT_PRIMARY, &a_config, NULL, 0 );

		ret = ch_pcm;
	}
	else
	{
		switch ( ch_pcm )
		{
		case 6:
			// this means we asked for 8 channels, but only 6 are available
			//   so, we'll turn on the 7.1 to 5.1 downmixer.
			a_config.channel = 6; 
			a_config.encoder = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
			a_config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_B; 
			if ( cellAudioOutConfigure( CELL_AUDIO_OUT_PRIMARY, &a_config, NULL, 0 ) != CELL_OK )
			{
				return 0; // error - the downmixer didn't init
			}

			ret = 8;
			break;

		case 2:
			// ok, this means they asked for multi-channel out, but only stereo
			//   is supported.  we'll try dolby digital first and then the downmixer

			ch_bit = cellAudioOutGetSoundAvailability( CELL_AUDIO_OUT_PRIMARY,
				CELL_AUDIO_OUT_CODING_TYPE_AC3,
				CELL_AUDIO_OUT_FS_48KHZ, 0 );
			if ( ch_bit > 0 ) 
			{
				a_config.channel = ch_bit;
				a_config.encoder = CELL_AUDIO_OUT_CODING_TYPE_AC3;
				if ( ch_bit >= minimum_chans )
				{
					// we have enough channels to support their minimum
					a_config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_NONE;
					ret = ch_bit;
				}
				else
				{
					// we don't have enough channels to support their minimum, so use the downmixer
					a_config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_B;
					ret = 8;
				}

				if ( cellAudioOutConfigure( CELL_AUDIO_OUT_PRIMARY, &a_config, NULL, 0 ) == CELL_OK )
				{
					break;
				}

				// if we got here the dolby encoder, didn't init, so fall through to downmixing to stereo
			}

			a_config.channel = 2;
			a_config.encoder   = CELL_AUDIO_OUT_CODING_TYPE_LPCM;
			a_config.downMixer = CELL_AUDIO_OUT_DOWNMIXER_TYPE_A;
			if ( cellAudioOutConfigure( CELL_AUDIO_OUT_PRIMARY, &a_config, NULL, 0 ) != CELL_OK )
			{
				return 0; // error - downmixer didn't work
			}

			ret = 2; // downmixer does 7.0 to 2.0 downmixing...
			break;

		default:
			// some other weird case that we don't understand
			return 0;
		}
	}

	// turn off copy protection stupidness
	cellAudioOutSetCopyControl( CELL_AUDIO_OUT_PRIMARY,
		CELL_AUDIO_OUT_COPY_CONTROL_COPY_FREE );

	return( ret );
}

