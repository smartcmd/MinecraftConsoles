/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client UGC API - Backwards compat file for SenCulture removal
//
// Include this to get access to all UGC related backwards compatibility with the old SenCulture

#pragma once

#include "SenClientUGC.h"
#include "SenClientCultureBackCompat_SenCore.h"


namespace Sentient
{
	/// @brief			Retrieves a collection of feeds that are viewable by the
	///					current user.
	///
	/// @param[in]		userIndex
	///                 The index of the initiating user on the console.  
	///					Note: This is NOT a XUID.
	///
	/// @param[in]      culture
	///                 This is the result of a call to SenCultureFind() or SenCultureGet*().
	///                 You may also pass nullptr to use the culture set with SenCultureSetCurrent().
	///					May be nullptr for default culture.
	///
	/// @param[in]		maxResults
	///					Used to indicate the number of items to be returned by @a out_feedInfo.
	///					If the actual number of items exceeds this, you will receive an error.
	///
	/// @param[out]		out_feedInfo
	///					Pointer to a collection of structures to fill with SenUGCFeedInfo data.
	///					
	/// @param[out]		out_resultCount
	///					The number of entries actually enumerated by the call.
	///
	/// @param[in]      userCallback
	///                 If this call returns a success code, 
	///					the userCallback will be called at the end of the 
	///					asynchronous process.
	///
	/// @param[in]		userCallbackData
	///                 Data to be passed to the @a userCallback on completion.
	///
	/// @return			Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.
	///					Specific values include:
	///					E_POINTER: out_feedInfo or out_resultCount are null.
	///
	/// @details		<Insert detailed method documentation>
	///
	/// @related		SenUGCGetFeed()
	///
	/// @deprecated      Use SenUGCEnumerateFeeds() without SenCulture
	///
	__declspec(deprecated("This function is deprecated. See the function body for an example of using the new API.")) 
	__inline HRESULT SenUGCEnumerateFeeds(
		int userIndex,
		const SenCultureInfo *culture,
		size_t maxResults,
		SenUGCFeedInfo *out_feedInfo,
		size_t *out_resultCount,
		SenSysCompletedCallback userCallback,
		void *userCallbackData)
	{
		return SenUGCEnumerateFeeds(userIndex, maxResults, out_feedInfo, out_resultCount, userCallback, userCallbackData);
	}

} // namespace Sentient
