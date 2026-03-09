/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client User API
//
// Include this to get access to all user-related Sentient features.

#pragma once

#include "SenClientSys.h"


namespace Sentient
{
	//======================//
	//                      //
	//    User Data Types   //
	//                      //
	//======================//

	/// @brief           Roles a user can have in Sentient
	///
	/// @details         Roles are not necessarily mutually exclusive. 
	///				     A single user may be in multiple roles simultaneously.
	///
	enum SenUserRole
	{
		/// The user is a UGC moderator.
		SenUserRole_UGC_Moderator,

		/// The user has been banned from UGC participation.
		SenUserRole_UGC_Banned,
	};


	//======================//
	//                      //
	//    User Functions    //
	//                      //
	//======================//

	/// @brief           Ask Sentient whether a user belongs to the enumerated roles
	///
	/// @param[in]       userIndex
	///                  Local index [0-3] of the user whose role is being checked
	///
	/// @param[in]       role
	///                  Role to check
	///
	/// @param[out]      out_isInRole
	///                  Location to store the output role membership state
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_POINTER: out_isInRole is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	HRESULT SenUserIsInRole(
		int userIndex, 
		SenUserRole role,
		bool *out_isInRole, 
		SenSysCompletedCallback userCallback, 
		void *userCallbackData );

	/// @brief           Ask Sentient whether a user belongs to the enumerated roles
	///
	/// @param[in]       xuid
	///                  XUID of user whose role is being checked
	///
	/// @param[in]       role
	///                  Role to check
	///
	/// @param[out]      out_isInRole
	///                  Location to store the output role membership state
	///
	/// @param[in]       userCallback
	///                  If this call returns a success code, the userCallback will be called at the end of the asynchronous process.
	///
	/// @param[in]       userCallbackData
	///                  Data to be passed to the @a userCallback on completion.
	///
	/// @return          Check SUCCEEDED( hresult ) or FAILED( hresult ) to determine success.  Specific values include:
	///                  SENTIENT_E_NOT_INITIALIZED: You did not call SentientInitialize() first.
	///                  E_POINTER: out_isInRole is nullptr.
	///                  E_FAIL: Failed to spawn server call.
	///                  S_OK: Server call spawned successfully.
	///
	HRESULT SenUserIsInRole(
		PlayerUID xuid,
		SenUserRole role,
		bool *out_isInRole, 
		SenSysCompletedCallback userCallback, 
		void *userCallbackData );
}