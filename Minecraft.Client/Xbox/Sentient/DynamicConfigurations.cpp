#include "stdafx.h"
#include "Include\SenClientMain.h"
#include "Include\SenClientDynamicConfig.h"
#include "DynamicConfigurations.h"


MinecraftDynamicConfigurations::Dynamic_Config_Trial_Data MinecraftDynamicConfigurations::trialData;

bool MinecraftDynamicConfigurations::s_bFirstUpdateStarted = false;
bool MinecraftDynamicConfigurations::s_bUpdatedConfigs[MinecraftDynamicConfigurations::eDynamic_Config_Max];
MinecraftDynamicConfigurations::EDynamic_Configs MinecraftDynamicConfigurations::s_eCurrentConfig = MinecraftDynamicConfigurations::eDynamic_Config_Max;
size_t MinecraftDynamicConfigurations::s_currentConfigSize = 0;
size_t MinecraftDynamicConfigurations::s_dataWrittenSize = 0;
byte *MinecraftDynamicConfigurations::s_dataWritten = nullptr;

void MinecraftDynamicConfigurations::Tick()
{
	if(!s_bFirstUpdateStarted)
	{
		UpdateAllConfigurations();
		s_bFirstUpdateStarted = true;
	}
}

DWORD MinecraftDynamicConfigurations::GetTrialTime()
{
	return trialData.trialTimeSeconds;
}

void MinecraftDynamicConfigurations::UpdateAllConfigurations()
{
	for(DWORD i = 0; i < eDynamic_Config_Max; ++i)
	{
		s_bUpdatedConfigs[i] = false;
	}
	UpdateNextConfiguration();
}

void MinecraftDynamicConfigurations::UpdateNextConfiguration()
{
	EDynamic_Configs update = eDynamic_Config_Max;
	for(DWORD i = 0; i < eDynamic_Config_Max; ++i)
	{
		if(!s_bUpdatedConfigs[i])
		{
			update = static_cast<EDynamic_Configs>(i);
			break;
		}
	}
	if( update < eDynamic_Config_Max )
	{
		UpdateConfiguration( update );
	}
}

void MinecraftDynamicConfigurations::UpdateConfiguration(EDynamic_Configs id)
{
	app.DebugPrintf("DynamicConfig: Attempting to update dynamic configuration %d\n", id);

	HRESULT hr = Sentient::SenDynamicConfigGetSize( id,  &s_currentConfigSize, &MinecraftDynamicConfigurations::GetSizeCompletedCallback, nullptr);

	switch(hr)
	{
	case S_OK:
		s_eCurrentConfig = id;
		//The server call was spawned successfully. 
		break;
	case E_FAIL:
		app.DebugPrintf("DynamicConfig: Failed to get size for config\n");
		//Sentient failed to spawn the call to the server.
		//An unknown error occurred. For more information, see the debug log that is available when you compile your application against the debug version of the library (SenCoreD.lib). 
		break;
	case Sentient::SENTIENT_E_NOT_INITIALIZED:
		app.DebugPrintf("DynamicConfig: Failed to get size for config as sentient not initialized\n");
		//Sentient is not initialized. You must call SentientInitialize before you call this function. 
		break;
	case E_POINTER:
		app.DebugPrintf("DynamicConfig: Failed to get size for config as pointer is invalid\n");
		//The out_size pointer is nullptr. 
		break;
	}
	if(FAILED(hr) )
	{
		s_bUpdatedConfigs[s_eCurrentConfig] = true;
		UpdateNextConfiguration();
	}
}

void MinecraftDynamicConfigurations::GetSizeCompletedCallback(HRESULT taskResult,   void *userCallbackData)
{
	if( HRESULT_SUCCEEDED(taskResult) )
	{
		s_dataWritten = new byte[s_currentConfigSize];
		HRESULT hr = Sentient::SenDynamicConfigGetBytes(
			s_eCurrentConfig,
			s_currentConfigSize,
			&s_dataWrittenSize,
			s_dataWritten,
			&MinecraftDynamicConfigurations::GetDataCompletedCallback,
			nullptr
			);

		switch(hr)
		{
		case S_OK:
			//The server call was spawned successfully. 
			break;
		case E_FAIL:
			app.DebugPrintf("DynamicConfig : Failed to get bytes for config\n");
			//Sentient failed to spawn the call to the server.
			//An unknown error occurred. For more information, see the debug log that is available when you compile your application against the debug version of the library (SenCoreD.lib). 
			break;
		case Sentient::SENTIENT_E_NOT_INITIALIZED:
			app.DebugPrintf("DynamicConfig : Failed to get bytes for config as sentient not initialized\n");
			//Sentient is not initialized. You must call SentientInitialize before you call this function. 
			break;
		case E_POINTER:
			app.DebugPrintf("DynamicConfig: Failed to get bytes for config as pointer is nullptr\n");
			//The out_size pointer is nullptr. 
			break;
		}
		if(FAILED(hr) )
		{
			s_bUpdatedConfigs[s_eCurrentConfig] = true;
			UpdateNextConfiguration();
		}
	}
	else
	{
		s_bUpdatedConfigs[s_eCurrentConfig] = true;
		UpdateNextConfiguration();
		app.DebugPrintf("MinecraftDynamicConfigurations::GetSizeCompletedCallback : FAILED\n");
	}
}

void MinecraftDynamicConfigurations::GetDataCompletedCallback(HRESULT   taskResult,   void   *userCallbackData)
{
	if(HRESULT_SUCCEEDED(taskResult) && s_currentConfigSize == s_dataWrittenSize)
	{
		switch(s_eCurrentConfig)
		{
		case eDynamic_Config_Trial:
			{
				int version = *(int *)s_dataWritten;
				switch(version)
				{
				case DYNAMIC_CONFIG_TRIAL_VERSION:
				//case 1:
					memcpy(&trialData,s_dataWritten+4,sizeof(_dynamic_config_trial_data_version1));
					app.DebugPrintf("Updated dynamic config TRIAL: timer is %d\n", trialData.trialTimeSeconds);
					break;
				};
			}
			break;
		};
	}
	else
	{
		app.DebugPrintf("MinecraftDynamicConfigurations::GetDataCompletedCallback : FAILED\n");
	}

	delete [] s_dataWritten;
	s_dataWritten = nullptr;

	s_bUpdatedConfigs[s_eCurrentConfig] = true;
	UpdateNextConfiguration();
}