/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client Types
//
// Include this to get access to all basic Sentient Types.

#pragma once

// These guarantees the presence of UINT32 et al, which we will be using 
// for specific bitsized-types until we decide to switch to C++0x-style
// uint32_t (et al) types.

#if defined(_XBOX)

	#include <xtl.h>
	#include <xbox.h>

#else

	#define NOD3D
	#define WIN32_LEAN_AND_MEAN
	#include <winlive.h>
	//#include <windows.h>

#endif

#include <assert.h>


namespace Sentient
{
	//============================//
	//                            //
	//    Sentient Basic Types    //
	//                            //
	//============================//

	/// @brief           A low-resolution timestamp.
	///
	/// @details         This is the timestamp used by UGC.  Its resolution is only ???.  // TODO:
	///                  Full-resolution timestamps generally use SYSTEMTIME structures.
	///
	typedef UINT32 SenSysDateTime;

	/// @brief           A unique identifier per title.
	///
	/// @details         Each title on Xbox Live is given a 32-bit title ID.  We use these
	///                  to help identify title-specific resourced, UGC, etc.
	///                  Note that, except for the two special cases below, all title ID 
	///                  values are considered private and should be obtained only through
	///                  Sentient APIs and structures.  You should never create one manually.
	///
	enum SenSysTitleID : INT32
	{
		// Use this for your own title.  It will be converted to your true
		// title ID internally.
		SenSysTitleID_This   = (SenSysTitleID)0x00000000,

		// Use this if you need to access shared, cross-game entries, which is
		// typically the case.
		SenSysTitleID_Shared = (SenSysTitleID)0xffffffff,  
	};

	/// @brief           A unique identifier for an offer on Xbox/Windows Live Marketplace.
	///
	/// @details         This 64-bit value may be passed off to system routines that prompt
	///                  the user to browse/buy a product, such as a trial/demo or an avatar
	///                  item.
	///                  Note that, except for the the special case below, all offer ID 
	///                  values are considered private and should be obtained only through
	///                  Sentient APIs and structures.  You should never create one manually.
	///
	enum SenSysOfferID : INT64
	{
		// This is the value used when no offerID is available.
		SenSysOfferID_Invalid = (SenSysOfferID)0x0000000000000000LL,
	};

	/// @brief            The size of a resource file.
	///
	/// @details          Note we're going to use this instead of size_t because the value type
	///                   coming down from the server will not necessarily match the client's
	///                   compiler notion of, say, size_t or int, which could be 64 bits.
	///
	typedef INT32 SenSysFileSize;

	/// @brief            A Sentient-specific handle.
	///
	/// @details          These handles are currently used only for tracking and cancelling
	///                   asynchronous tasks.
	///
	/// @related          SentientCancel()
	///
	typedef VOID *SenHandle;
	
	// TODO: this really doesn't belong in SenCore's global types
	typedef UINT64 SenUGCID;

	/// @brief            A unique identifier for a device.
	///
	/// @details          Some routines want to know which device to default to.
	///                   This ID is a way to specify one.  On Xbox, this is equivalent
	///                   to an XCONTENTDEVICEID and can be obtained in several ways 
	///                   through the XDK.  On Windows, this is TBD.
	///
#ifdef _XBOX
	typedef XCONTENTDEVICEID SenContentDeviceID;
	#define SenContentDeviceID_Any XCONTENTDEVICE_ANY
#else
	typedef DWORD SenContentDeviceID;
	#define SenContentDeviceID_Any 0
#endif


	//=========================//
	//                         //
	//    Sentient HRESULTs    //
	//                         //
	//=========================//

	// Sentient HRESULT fields
	#define SENTIENT_FACILITY 0x58
	#define SENTIENT_E_FIRST_ERROR  MAKE_HRESULT( SEVERITY_ERROR, SENTIENT_FACILITY, 0x0000 )
	#define SENTIENT_S_FIRST_STATUS MAKE_HRESULT( SEVERITY_SUCCESS, SENTIENT_FACILITY, 0x0000 )

	// Sentient Error codes
	const HRESULT SENTIENT_E_NOT_INITIALIZED				   = SENTIENT_E_FIRST_ERROR  + 0x0001; // 0x80580001    // title has tried to call a Sentient function when the library is not initialized.
	const HRESULT SENTIENT_E_ALREADY_INITIALIZED			   = SENTIENT_E_FIRST_ERROR  + 0x0002; // 0x80580002    // title has tried to initialize an already-initialized library.
	const HRESULT SENTIENT_E_NO_UGC_PUBLISH_PRIVILEGE          = SENTIENT_E_FIRST_ERROR  + 0x0003; // 0x80580003    // user does not have the proper UGC upload/publish privilege flag set
	const HRESULT SENTIENT_E_NO_UGC_USE_PRIVILEGE              = SENTIENT_E_FIRST_ERROR  + 0x0004; // 0x80580004    // user does not have the proper UGC download/use privilege flag set
	const HRESULT SENTIENT_E_GUEST_ACCESS_VIOLATION            = SENTIENT_E_FIRST_ERROR  + 0x0005; // 0x80580005    // user is a guest, and therefore should not be accessing certain sentient features
	const HRESULT SENTIENT_E_TOO_MANY_CALLS					   = SENTIENT_E_FIRST_ERROR  + 0x0006; // 0x80580006    // title has called this API too frequently.  Call has been discarded to avoid excessive server load.
	const HRESULT SENTIENT_E_SERVER_ERROR					   = SENTIENT_E_FIRST_ERROR  + 0x0007; // 0x80580007    // call generated an unhandled error on the server.  Contact senhelp for assistance.
	const HRESULT SENTIENT_E_TIME_OUT						   = SENTIENT_E_FIRST_ERROR  + 0x0008; // 0x80580008	// the operation took too long to complete and has been abandoned.
	const HRESULT SENTIENT_E_CANCELED					       = SENTIENT_E_FIRST_ERROR  + 0x0009; // 0x80580009	// the operation was canceled by user sign-out or via an explicit call to SentientCancel
	const HRESULT SENTIENT_E_FORBIDDEN					       = SENTIENT_E_FIRST_ERROR  + 0x000a; // 0x8058000a	// the server has indicated that the supplied user is not permitted to perform this operation.
	const HRESULT SENTIENT_E_USERINDEX_IS_ANY                  = SENTIENT_E_FIRST_ERROR  + 0x000b; // 0x8058000b    // userIndex of ANY passed to a routine that requires a specific user.
	const HRESULT SENTIENT_E_USERINDEX_IS_NONE                 = SENTIENT_E_FIRST_ERROR  + 0x000c; // 0x8058000c    // userIndex of NONE passed to a routine that requires a user.
	const HRESULT SENTIENT_E_USERINDEX_IS_INVALID              = SENTIENT_E_FIRST_ERROR  + 0x000d; // 0x8058000d    // userIndex is not a known value
	const HRESULT SENTIENT_E_USERINDEX_IS_FOCUS                = SENTIENT_E_FIRST_ERROR  + 0x000e; // 0x8058000e    // userIndex of FOCUS passed to a routine that requires a specific user.
	const HRESULT SENTIENT_E_BUFFER_EXHAUSTED                  = SENTIENT_E_FIRST_ERROR  + 0x000f; // 0x8058000f    // no more space in buffer, operation was discarded
	const HRESULT SENTIENT_E_RETRY_INVALID_DATA                = SENTIENT_E_FIRST_ERROR  + 0x0010; // 0x80580010    // retry; data was found to be invalid either after step or whole operation finished
	const HRESULT SENTIENT_E_UNREACHABLE					   = SENTIENT_E_FIRST_ERROR  + 0x0011; // 0x80580011    // Server was not reachable when called.  Contact senhelp for assistance.
	const HRESULT SENTIENT_E_RETRY_OVERLOADED				   = SENTIENT_E_FIRST_ERROR  + 0x0012; // 0x80580012    // Server was overloaded when called. Retry later or contact senhelp for assistance.
	const HRESULT SENTIENT_E_NO_UGC_DOWNLOAD_PRIVILEGE		   = SENTIENT_E_FIRST_ERROR  + 0x0013; // 0x80580013    // user does not have the proper UGC download privilege flag set
	const HRESULT SENTIENT_E_UGC_DOESNT_EXIST				   = SENTIENT_E_FIRST_ERROR  + 0x0014; // 0x80580014    // requested action is aborted because the ugcid passed doesn't exist
	const HRESULT SENTIENT_E_INVALID_UGC_PUBLISH_STATE		   = SENTIENT_E_FIRST_ERROR  + 0x0015; // 0x80580015    // requested action is aborted because the UGC is in the wrong publish state

	// Sentient Status/Success codes
	const HRESULT SENTIENT_S_NOT_SIGNED_IN_TO_LIVE             = SENTIENT_S_FIRST_STATUS + 0x0003; // 0x00580003
	const HRESULT SENTIENT_S_INITIALIZING_CONNECTION           = SENTIENT_S_FIRST_STATUS + 0x0004; // 0x00580004
	const HRESULT SENTIENT_S_SERVER_CONNECTION_FAILED          = SENTIENT_S_FIRST_STATUS + 0x0005; // 0x00580005
	const HRESULT SENTIENT_S_OPERATION_IN_PROGRESS             = SENTIENT_S_FIRST_STATUS + 0x0006; // 0x00580006
	const HRESULT SENTIENT_S_NEWER_BLOB_EXISTS				   = SENTIENT_S_FIRST_STATUS + 0x0007; // 0x00580007	// UGC can have different versions of MainDataBlob with Republish API

} // namespace Sentient
