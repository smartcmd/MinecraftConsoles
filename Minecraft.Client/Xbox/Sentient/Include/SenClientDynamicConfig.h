/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client DynamicConfig API
//
// Include this to get access to all DynamicConfig-related Sentient features

#pragma once

#include "SenClientSys.h"

namespace Sentient
{

	/// @brief           Gets the size of a config file, so memory can be allocated
	///
	/// @param[in]       titleID
	///                  Which title's config file to search for.
	///
	/// @param[in]       resourceID
	///                  Which config file to find.
	///
	/// @param[out]      out_size
	///                  The size of the file, in bytes.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_POINTER: size is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Search the service for a specific config file at the current time.
	///
	/// @related         SenDynamicConfigFileGetBytes()
	///
	HRESULT SenDynamicConfigGetSize(
		__in unsigned int titleID,
		__in unsigned int resourceID,
		__out size_t* out_size,
		__in SenSysCompletedCallback userCallback,
		__in void *userCallbackData );


	/// @brief           Gets the size of a config file, so memory can be allocated
	///
	/// @param[in]       resourceID
	///                  Which config file to find.
	///
	/// @param[out]      out_size
	///                  The size of the file, in bytes.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_POINTER: size is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Search the service for a specific config file at the current time.
	///
	/// @related         SenDynamicConfigFileGetBytes()
	///
	__inline HRESULT SenDynamicConfigGetSize(
		__in unsigned int resourceID,
		__out size_t* out_size,
		__in SenSysCompletedCallback userCallback,
		__in void *userCallbackData )
	{
		return SenDynamicConfigGetSize(SenSysTitleID_This, resourceID, out_size, userCallback, userCallbackData);
	}


	/// @brief           Gets the contents of a config file
	///
	/// @param[in]       titleID
	///                  Which title's config file to search for.
	///
	/// @param[in]       resourceID
	///                  Which config file to return.
	///
	/// @param[in]       dataSizeMax
	///                  Maximum number of bytes to return.
	///
	/// @param[out]      out_bytesWritten
	///                  The number of bytes written into the buffer.
	///
	/// @param[out]      out_data
	///                  The buffer that receives the contents of the file.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Search the service for a specific config file at the current time.
	///
	/// @related         SenDynamicConfigFileGetSize()
	///
	HRESULT SenDynamicConfigGetBytes(
		__in unsigned int titleID,
		__in unsigned int resourceID,
		__in size_t dataSizeMax,
		__out_opt size_t* out_bytesWritten,
		__out void *out_data,
		__in SenSysCompletedCallback userCallback,
		__in void *userCallbackData );


	/// @brief           Gets the contents of a config file
	///
	/// @param[in]       resourceID
	///                  Which config file to return.
	///
	/// @param[in]       dataSizeMax
	///                  Maximum number of bytes to return.
	///
	/// @param[out]      out_bytesWritten
	///                  The number of bytes written into the buffer.
	///
	/// @param[out]      out_data
	///                  The buffer that receives the contents of the file.
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_POINTER: out_data is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	/// @details         Search the service for a specific config file at the current time.
	///
	/// @related         SenDynamicConfigFileGetSize()
	///
	__inline HRESULT SenDynamicConfigGetBytes(
		__in unsigned int resourceID,
		__in size_t dataSizeMax,
		__out_opt size_t* out_bytesWritten,
		__out void *out_data,
		__in SenSysCompletedCallback userCallback,
		__in void *userCallbackData )
	{
		return SenDynamicConfigGetBytes(SenSysTitleID_This, resourceID, dataSizeMax, out_bytesWritten, out_data, userCallback, userCallbackData);
	}
}
