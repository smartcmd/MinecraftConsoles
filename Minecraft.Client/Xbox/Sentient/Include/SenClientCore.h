/********************************************************
*                                                       *
* Copyright (C) Microsoft. All rights reserved.         *
*                                                       *
********************************************************/

// Sentient Client Core API
//
// Include this to get access to ALL core Sentient features.

#pragma once

#include "SenClientConfig.h"
#include "SenClientCulture.h"
#include "SenClientFile.h"
#include "SenClientMain.h"
#include "SenClientPackage.h"
#include "SenClientRawData.h"
#include "SenClientResource.h"
#include "SenClientStats.h"
#include "SenClientSys.h"
#include "SenClientUtil.h"
#include "SenClientXML.h"
#include "SenClientUser.h"


// REVIEW: is this block of text still relevant?

/* API Design considerations

	1. All functions will return S_OK if they succeed or either E_FAIL or an HRESULT error code
		if they fail.  Callers should use the standard SUCCEEDED(hr) or FAILED(hr) macros to
		test the results, as it eliminates errors due to misconceptions about what exactly is
		an HRESULT error vs. success/status.

		PLEASE NOTE: S_FALSE is for successfully returning a false, and does *not* qualify as an
		error code.  It should not be used for fail cases.  Use only codes like E_FAIL, 
		E_OUTOFMEMORY, E_INVALIDARG, E_POINTER, etc. for failures.  Most error codes contain "E_"
		at or near the start of the name, but you can always tell by whether the top bit is set.
	    
	2. The API is not thread safe at this time, so all calls must be made from the same thread.
	    
	3. Many functions are designed for asynchronous use, and will internally create a task that
		will only finish after feedback from the server.
		a. These functions can be either blocking or asynchronous. If the callback pointer is nullptr
			then it is a blocking call.
		b. If the call is asynchronous, then it will always return instantly, and will return S_OK
			if a task has been created.
			1. If the task is created then at some point the callback will be called once the task
				finishes, regardless of whether the task succeeds or fails. The task result is then
				passed into the callback
			2. However, if the call fails to create a task (due to bad parameters or something else)
				and returns something other than S_OK, it will *not* call the callback. 
		c. If the call is blocking, the return value will be the task result value.
		d. The recommended usage is to use async calls, but if a game run all Sentient calls from a 
			worker thread then using blocking calls could work.

	4. The order in which async calls complete is not guaranteed. 
		a. Similarly, if one or more async calls are started, and a blocking call is performed, the 
			async calls may or may not finish before the blocking call returns ( as the blocking call
			just calls SentientUpdate() until it completes ).

	5. All async functions that take pointers will assume that the pointers point valid memory for the 
		duration of the task, and that the game does not free or overwrite the memory until the task
			is completed.
	    
	6. All output variables (but not output buffers) are cleared even if the function call fails.
	    
	7. Functions that return buffers of data, will have to have the buffer allocated before being called. 
		NOTE: This is changed from the first release (october 2010)
	    
	8. There are multiple sets of functionality that relies on "blobs" - a blob is a custom (usually
		game-specific) chunk of binary data that the Sentient does not know anything about.
*/
