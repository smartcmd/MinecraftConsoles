#pragma once

enum eDebugSetting
{
	eDebugSetting_LoadSavesFromDisk,
	eDebugSetting_WriteSavesToDisk,
	eDebugSetting_FreezePlayers,	//eDebugSetting_InterfaceOff,
	eDebugSetting_Safearea,
	eDebugSetting_MobsDontAttack,
	eDebugSetting_FreezeTime,
	eDebugSetting_DisableWeather,
	eDebugSetting_CraftAnything,
	eDebugSetting_UseDpadForDebug,
	eDebugSetting_MobsDontTick,	
	eDebugSetting_ArtTools, //eDebugSetting_InstantDestroy,
	eDebugSetting_ShowUIConsole,
	eDebugSetting_DistributableSave,
	eDebugSetting_DebugLeaderboards,
	eDebugSetting_EnableHeightWaterOverride, //eDebugSetting_TipsAlwaysOn,
	eDebugSetting_SuperflatNether,
	//eDebugSetting_LightDarkBackground,
	eDebugSetting_RegularLightning,
	eDebugSetting_EnableBiomeOverride, //eDebugSetting_GoToNether,
	//eDebugSetting_GoToEnd,
	eDebugSetting_GoToOverworld,
	eDebugSetting_UnlockAllDLC, // eDebugSetting_ToggleFont,
	eDebugSetting_ShowUIMarketingGuide,
	eDebugSetting_Max,
};

enum eDebugButton
{
	eDebugButton_Theme=0,
	eDebugButton_Avatar_Item_1,
	eDebugButton_Avatar_Item_2,
	eDebugButton_Avatar_Item_3,
	eDebugButton_Gamerpic_1,
	eDebugButton_Gamerpic_2,
	eDebugButton_CheckTips,
	eDebugButton_WipeLeaderboards,
	eDebugButton_Max,
};
