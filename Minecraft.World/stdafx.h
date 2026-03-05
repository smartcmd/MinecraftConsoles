// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifdef __PS3__
#else
#endif

#if ( defined _XBOX || defined _WINDOWS64  || defined _DURANGO )
typedef unsigned __int64 __uint64;
#endif

#ifdef _WINDOWS64
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <malloc.h>
#include <tchar.h>
// TODO: reference additional headers your program requires here
#include <d3d11.h>
#endif

#ifdef _DURANGO
#include <xdk.h>
#include <wrl.h>
#include <d3d11_x.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <pix.h>
#include "..\Minecraft.Client\Durango\DurangoExtras\DurangoStubs.h"
#endif

#if (defined __PS3__ || defined _XBOX )
// C RunTime Header Files
#include <stdlib.h>
#endif

#ifdef __ORBIS__
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <scebase.h>
#include <kernel.h>
#include <fios2.h>
#include <message_dialog.h>
#endif

#ifdef _XBOX
#include <xtl.h>
#include <xboxmath.h>
#include <xuiapp.h>
#include <xact3.h>
typedef XINVITE_INFO INVITE_INFO;
typedef XUID PlayerUID;
typedef XNKID SessionID;
typedef XUID GameSessionUID;
#endif

#ifdef __PS3__
#include <cell/l10n.h>
#include <cell/pad.h>
#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <cell/sysmodule.h>
#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_savedata.h>
#include <sysutil/sysutil_sysparam.h>


#include "Ps3Types.h"
#include "Ps3Stubs.h"
#include "PS3Maths.h"

#elif defined __ORBIS__
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "OrbisTypes.h"
#include "OrbisStubs.h"
#include "OrbisMaths.h"
#elif defined __PSVITA__
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <touch.h>
#include "PSVitaTypes.h"
#include "PSVitaStubs.h"
#include "PSVitaMaths.h"
#else
#include <unordered_map>
#include <unordered_set>
#include <sal.h>
#include <vector>
#endif //__PS3__

#include <memory>

#include <list>
#include <map>
#include <set>
#include <queue>
#include <deque>
#include <algorithm>
#include <math.h>
#include <limits>
#include <string>
#include <sstream>
#include <iostream>
#include <exception>

#ifndef __PS3__			// the PS3 lib assert is rubbish, and aborts the code, we define our own in PS3Types.h
#include <assert.h>
#endif

#ifndef _XBOX
#include "extraX64.h"
#else
#include "..\Minecraft.Client\xbox\network\extra.h"
#endif

#include "Definitions.h"
#include "Class.h"
#include "Exceptions.h"
#include "Mth.h"
#include "StringHelpers.h"
#include "ArrayWithLength.h"
#include "Random.h"
#include "TilePos.h"
#include "ChunkPos.h"
#include "compression.h"
#include "PerformanceTimer.h"


#ifdef _FINAL_BUILD
#define printf BREAKTHECOMPILE
#define wprintf BREAKTHECOMPILE
#undef OutputDebugString
#define OutputDebugString BREAKTHECOMPILE
#define OutputDebugStringA BREAKTHECOMPILE
#define OutputDebugStringW BREAKTHECOMPILE
#endif


void MemSect(int sect);

#ifdef _XBOX
#include "..\Minecraft.Client\xbox\4JLibs\inc\4J_Profile.h"
#include "..\Minecraft.Client\xbox\4JLibs\inc\4J_Render.h"
#include "..\Minecraft.Client\xbox\4JLibs\inc\4J_XTMS.h"
#include "..\Minecraft.Client\xbox\4JLibs\inc\4J_Storage.h"
#include "..\Minecraft.Client\xbox\4JLibs\inc\4J_Input.h"
#elif defined (__PS3__)
#include "..\Minecraft.Client\PS3\4JLibs\inc\4J_Profile.h"
#include "..\Minecraft.Client\PS3\4JLibs\inc\4J_Render.h"
#include "..\Minecraft.Client\PS3\4JLibs\inc\4J_Storage.h"
#include "..\Minecraft.Client\PS3\4JLibs\inc\4J_Input.h"
#elif defined _DURANGO
#include "..\Minecraft.Client\Durango\4JLibs\inc\4J_Profile.h"
#include "..\Minecraft.Client\Durango\4JLibs\inc\4J_Render.h"
#include "..\Minecraft.Client\Durango\4JLibs\inc\4J_Storage.h"
#include "..\Minecraft.Client\Durango\4JLibs\inc\4J_Input.h"
#elif defined _WINDOWS64
#include "..\Minecraft.Client\Windows64\4JLibs\inc\4J_Profile.h"
#include "..\Minecraft.Client\Windows64\4JLibs\inc\4J_Render.h"
#include "..\Minecraft.Client\Windows64\4JLibs\inc\4J_Storage.h"
#include "..\Minecraft.Client\Windows64\4JLibs\inc\4J_Input.h"
#elif defined __PSVITA__
#include "..\Minecraft.Client\PSVita\4JLibs\inc\4J_Profile.h"
#include "..\Minecraft.Client\PSVita\4JLibs\inc\4J_Render.h"
#include "..\Minecraft.Client\PSVita\4JLibs\inc\4J_Storage.h"
#include "..\Minecraft.Client\PSVita\4JLibs\inc\4J_Input.h"
#else
#include "..\Minecraft.Client\Orbis\4JLibs\inc\4J_Profile.h"
#include "..\Minecraft.Client\Orbis\4JLibs\inc\4J_Render.h"
#include "..\Minecraft.Client\Orbis\4JLibs\inc\4J_Storage.h"
#include "..\Minecraft.Client\Orbis\4JLibs\inc\4J_Input.h"
#endif

#include "..\Minecraft.Client\Common\Network\GameNetworkManager.h"

// #ifdef _XBOX
#include "..\Minecraft.Client\Common\App_defines.h"
#include "..\Minecraft.Client\Common\UI\UIEnums.h"
#include "..\Minecraft.Client\Common\App_enums.h"
#include "..\Minecraft.Client\Common\Tutorial\TutorialEnum.h"
#include "..\Minecraft.Client\Common\App_structs.h"
//#endif

#ifdef _XBOX
#include "..\Minecraft.Client\Common\XUI\XUI_Helper.h"
#include "..\Minecraft.Client\Common\XUI\XUI_Scene_Base.h"
#endif
#include "..\Minecraft.Client\Common\Consoles_App.h"
#include "..\Minecraft.Client\Common\Minecraft_Macros.h"
#include "..\Minecraft.Client\Common\Colours\ColourTable.h"

#include "..\Minecraft.Client\Common\BuildVer.h"

#ifdef _XBOX
#include "..\Minecraft.Client\Xbox\Xbox_App.h"
#include "..\Minecraft.Client\XboxMedia\strings.h"
#include "..\Minecraft.Client\Xbox\Sentient\SentientTelemetryCommon.h"
#include "..\Minecraft.Client\Xbox\Sentient\MinecraftTelemetry.h"

#elif defined (__PS3__)
#include "..\Minecraft.Client\PS3\PS3_App.h"
#include "..\Minecraft.Client\PS3Media\strings.h"
#include "..\Minecraft.Client\PS3\Sentient\SentientTelemetryCommon.h"
#include "..\Minecraft.Client\PS3\Sentient\MinecraftTelemetry.h"

#elif defined _DURANGO
#include "..\Minecraft.Client\Durango\Durango_App.h"
#include "..\Minecraft.Client\DurangoMedia\strings.h"
//#include "..\Minecraft.Client\Durango\Sentient\SentientManager.h"
#include "..\Minecraft.Client\Durango\Sentient\SentientTelemetryCommon.h"
#include "..\Minecraft.Client\Durango\Sentient\MinecraftTelemetry.h"
#include "..\Minecraft.Client\Durango\Sentient\TelemetryEnum.h"

#elif defined _WINDOWS64
#include "..\Minecraft.Client\Windows64\Windows64_App.h"
#include "..\Minecraft.Client\Windows64Media\strings.h"
#include "..\Minecraft.Client\Windows64\Sentient\SentientTelemetryCommon.h"
#include "..\Minecraft.Client\Windows64\Sentient\MinecraftTelemetry.h"

#elif defined __PSVITA__
#include "..\Minecraft.Client\PSVita\PSVita_App.h"
#include "..\Minecraft.Client\PSVitaMedia\strings.h"		// TODO - create PSVita-specific version of this
#include "..\Minecraft.Client\PSVita\Sentient\SentientManager.h"
#include "..\Minecraft.Client\PSVita\Sentient\MinecraftTelemetry.h"
#else
#include "..\Minecraft.Client\Orbis\Orbis_App.h"
#include "..\Minecraft.Client\OrbisMedia\strings.h"
#include "..\Minecraft.Client\Orbis\Sentient\SentientTelemetryCommon.h"
#include "..\Minecraft.Client\Orbis\Sentient\MinecraftTelemetry.h"
#endif

#include "..\Minecraft.Client\Common\DLC\DLCSkinFile.h"
#include "..\Minecraft.Client\Common\Console_Awards_enum.h"
#include "..\Minecraft.Client\Common\Potion_Macros.h"
#include "..\Minecraft.Client\Common\Console_Debug_enum.h"
#include "..\Minecraft.Client\Common\GameRules\ConsoleGameRulesConstants.h"
#include "..\Minecraft.Client\Common\GameRules\ConsoleGameRules.h"
#include "..\Minecraft.Client\Common\Telemetry\TelemetryManager.h"
