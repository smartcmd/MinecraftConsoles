/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client ConfigFile API
//
// Include this to get access to all ConfigFile-related Sentient features.

#pragma once

#include "SenClientRawData.h"
#include "SenClientResource.h"
#include "SenClientSys.h"


namespace Sentient
{
	//==============================//
	//                              //
	//    Game Config File Types    //
	//                              //
	//==============================//

	/// @brief           Basic config file information.
	///
	/// @details         This structure contains the original uploaded info plus any info implied by the raw data that was uploaded (e.g. data sizes).
	///
	struct SenConfigInfo : public SenResourceInfo
	{
		SenRawDataTransferInfo config;  ///< Points to a generic data block, whose format is determined by the game.
	};


	//==================================//
	//                                  //
	//    Game Config File Functions    //
	//                                  //
	//==================================//

	/// @brief           Find a config file by ID.
	///
	/// @param[in]       resourceID
	///                  Which config file to find, e.g. "SenResourceID_Config_0 + 4".
	///
	/// @param[out]      out_configInfo
	///                  The structure to fill in with the found information.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_POINTER: out_configInfo is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Search the database for a specific config file at the current time.
	///
	/// @deprecated		 This function is deprecated.  Use SenDynamicConfigGetSize() instead
	///
	/// @related         SenConfigFileDownload()
	///
	__declspec(deprecated("Use SenDynamicConfigGetSize() instead"))
		HRESULT SenConfigFileFind(
		SenResourceID resourceID,
		SenConfigInfo *out_configInfo,
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

	/// @brief           Download the raw config file data to the client.
	///
	/// @param[in]       configInfo
	///                  The info describing the attributes and data of the config file.
	///                  This is obtained from SenConfigFileFind().
	///
	/// @param[in]       dataSizeMax
	///                  Used to indicate the size of the buffer pointed to by @a out_data.
	///                  If the actual size of the data exceeds this, you will receive an error.
	///                  It is assumed that this is at least @a configInfo.config.GetBufferSize() bytes.
	///
	/// @param[out]      out_data
	///                  The buffer to fill in with the raw image data.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_INVALIDARG: configInfo.resourceID or configInfo.config is invalid.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         The data format is determined entirely by the game code and is never examined or processed by Sentient.
	///
	/// @deprecated		 This function is deprecated.  Use SenDynamicConfigGetSize() instead
	///
	/// @related         SenConfigFileFind()
	///
	__declspec(deprecated("Use SenDynamicConfigGetBytes() instead")) 
		HRESULT SenConfigFileDownload(
		const SenConfigInfo &configInfo,
		size_t dataSizeMax,
		void *out_data, 
		SenSysCompletedCallback userCallback,
		void *userCallbackData );

} // namespace Sentient
