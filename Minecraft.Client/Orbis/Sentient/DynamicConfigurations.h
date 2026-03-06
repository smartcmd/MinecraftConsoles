#pragma once

// 4J Stu - This file defines the id's for the dynamic configurations that we are currently using
// as well as the format of the data in them

/***********************
*
* TRIAL TIMER
*
************************/

#define DYNAMIC_CONFIG_TRIAL_ID 0
#define DYNAMIC_CONFIG_TRIAL_VERSION 1
#define DYNAMIC_CONFIG_DEFAULT_TRIAL_TIME 2400 //40 mins 1200 // 20 mins //300; // 5 minutes

class MinecraftDynamicConfigurations
{
private:
	enum EDynamic_Configs
	{
		eDynamic_Config_Trial,

		eDynamic_Config_Max,
	};

	/***********************
	*
	* TRIAL TIMER
	*
	************************/

	// 4J Stu - The first 4 bytes define a version number, that defines the structure of the data
	// After reading those bytes into a DWORD, the remainder of the data should be the size of the
	// relevant struct and can be cast to the struct
	struct _dynamic_config_trial_data_version1
	{
		// The time in seconds that the player can play the trial for
		DWORD trialTimeSeconds;

		_dynamic_config_trial_data_version1() { trialTimeSeconds = DYNAMIC_CONFIG_DEFAULT_TRIAL_TIME; }
	};

	typedef _dynamic_config_trial_data_version1 Dynamic_Config_Trial_Data;

	// Stored configurations
	static Dynamic_Config_Trial_Data trialData;
	
	static bool s_bFirstUpdateStarted;
	static bool s_bUpdatedConfigs[eDynamic_Config_Max];
	static EDynamic_Configs s_eCurrentConfig;
	static size_t s_currentConfigSize;

	static size_t s_dataWrittenSize;
	static uint8_t *s_dataWritten;

public:
	static void Tick();

	static DWORD GetTrialTime();

private:
	static void UpdateAllConfigurations();
	static void UpdateNextConfiguration();
	static void UpdateConfiguration(EDynamic_Configs id);

	static void GetSizeCompletedCallback(HRESULT   taskResult,   void   *userCallbackData);
	static void GetDataCompletedCallback(HRESULT   taskResult,   void   *userCallbackData);
};