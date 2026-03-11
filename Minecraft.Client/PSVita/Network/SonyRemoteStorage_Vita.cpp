#include "stdafx.h"

#include "SonyRemoteStorage_Vita.h"
#include "SonyHttp_Vita.h"
#include <stdio.h>
#include <string>
#include <stdlib.h>
// #include <cell/sysmodule.h>
// #include <cell/http.h>
// #include <cell/ssl.h>
// #include <netex/net.h>
// #include <netex/libnetctl.h>
// #include <np.h>
// #include <sysutil/sysutil_common.h>
// #include <sys/timer.h>
// #include <sys/paths.h>
// #include <sysutil\sysutil_savedata.h>




#define	AUTH_SCOPE		"psn:s2s"
#define CLIENT_ID		"969e9d21-527c-4c22-b539-f8e479f690bc"
static 	SceRemoteStorageData s_getDataOutput;


void SonyRemoteStorage_Vita::staticInternalCallback(const SceRemoteStorageEvent event, int32_t retCode, void * userData)
{
	static_cast<SonyRemoteStorage_Vita *>(userData)->internalCallback(event, retCode);
}

void SonyRemoteStorage_Vita::internalCallback(const SceRemoteStorageEvent event, int32_t retCode)
{
	m_lastErrorCode = retCode;

	switch(event)
	{
	case ERROR_OCCURRED:
		app.DebugPrintf("An error occurred with retCode: 0x%x \n", retCode);
		m_status = e_error;
// 		shutdown();				// removed, as the remote storage lib now tries to reconnect if an error has occurred
		runCallback();
		m_bTransferStarted = false;
		break;

	case GET_DATA_RESULT:
		if(retCode >= 0) 
		{
			app.DebugPrintf("Get Data success \n");
			m_status = e_getDataSucceeded;
		} 
		else 
		{
			app.DebugPrintf("An error occurred while Get Data was being processed. retCode: 0x%x \n", retCode);
			m_status = e_error;
		}
		runCallback();
		m_bTransferStarted = false;
		break;

	case GET_DATA_PROGRESS:
		app.DebugPrintf("Get data progress: %i%%\n", retCode);
		m_status = e_getDataInProgress;
		m_dataProgress = retCode;
		m_startTime = System::currentTimeMillis();
		break;

	case GET_STATUS_RESULT:
		if(retCode >= 0) 
		{
			app.DebugPrintf("Get Status success \n");
			app.DebugPrintf("Remaining Syncs for this user: %llu\n", outputGetStatus->remainingSyncs);
			app.DebugPrintf("Number of files on the cloud: %d\n", outputGetStatus->numFiles);
			for(int i = 0; i < outputGetStatus->numFiles; i++) 
			{
				app.DebugPrintf("\n*** File %d information: ***\n", (i + 1));
				app.DebugPrintf("File name: %s \n", outputGetStatus->data[i].fileName);
				app.DebugPrintf("File description: %s \n", outputGetStatus->data[i].fileDescription);
				app.DebugPrintf("MD5 Checksum: %s \n", outputGetStatus->data[i].md5Checksum);
				app.DebugPrintf("Size of the file: %u bytes \n", outputGetStatus->data[i].fileSize);
				app.DebugPrintf("Timestamp: %s \n", outputGetStatus->data[i].timeStamp);
				app.DebugPrintf("Visibility: \"%s\" \n", (outputGetStatus->data[i].visibility ==  0)?"Private":((outputGetStatus->data[i].visibility ==  1)?"Public read only":"Public read and write"));
			}
			m_status = e_getStatusSucceeded;
		} 
		else 
		{
			app.DebugPrintf("An error occurred while Get Status was being processed. retCode: 0x%x \n", retCode);
			m_status = e_error;
		}
		runCallback();
		break;

	case PSN_SIGN_IN_REQUIRED:
		app.DebugPrintf("User's PSN sign-in through web browser is required \n");
		m_status = e_signInRequired;
		runCallback();
		break;

	case SET_DATA_RESULT:
		if(retCode >= 0) 
		{
			app.DebugPrintf("Set Data success \n");
			m_status = e_setDataSucceeded;
		} 
		else 
		{
			app.DebugPrintf("An error occurred while Set Data was being processed. retCode: 0x%x \n", retCode);
			m_status = e_error;
		}
		runCallback();
		m_bTransferStarted = false;
		break;

	case SET_DATA_PROGRESS:
		app.DebugPrintf("Set data progress: %i%%\n", retCode);
		m_status = e_setDataInProgress;
		m_dataProgress = retCode;
		m_startTime = System::currentTimeMillis();
		break;

	case USER_ACCOUNT_LINKED:
		app.DebugPrintf("User's account has been linked with PSN \n");
		m_bInitialised = true;
		m_status = e_accountLinked;
		runCallback();
		break;

	case WEB_BROWSER_RESULT:
		app.DebugPrintf("This function is not used on PS Vita, as the account will be linked, it is not needed to open a browser to link it \n");
		assert(0);
		break;

	default:
		app.DebugPrintf("This should never happen \n");
		assert(0);
		break;

	}
}

bool SonyRemoteStorage_Vita::init(CallbackFunc cb, LPVOID lpParam)
{
	int ret = 0;
	int reqId = 0;

	m_callbackFunc = cb;
	m_callbackParam = lpParam;
	m_bTransferStarted = false;
	m_bAborting = false;

	m_lastErrorCode = SCE_OK;


	if(m_bInitialised)
	{
		internalCallback(USER_ACCOUNT_LINKED, 0);
		return true;
	}

	ret = sceNpAuthInit();
	if(ret < 0 && ret != SCE_NP_AUTH_ERROR_ALREADY_INITIALIZED)
	{
		app.DebugPrintf("sceNpAuthInit failed  0x%x\n", ret);
		return false;
	}

	ret = sceNpAuthCreateOAuthRequest();
	if (ret < 0)
	{
		app.DebugPrintf("Couldn't create auth request 0x%x\n", ret);
		return false;
	}

	reqId = ret;

	SceNpClientId clientId;
	memset(&clientId, 0x0, sizeof(clientId));

// 	SceNpAuthorizationCode authCode;
// 	memset(&authCode, 0x0, sizeof(authCode));
 
	SceNpAuthGetAuthorizationCodeParameter authParams;
	memset(&authParams, 0x0, sizeof(authParams));	

	authParams.size = sizeof(authParams);
	authParams.pScope = AUTH_SCOPE;	

	memcpy(clientId.id, CLIENT_ID, strlen(CLIENT_ID));
	authParams.pClientId = &clientId;	

	int issuerId = 0;
// 	ret = sceNpAuthGetAuthorizationCode(reqId, &authParams, &authCode, &issuerId);
// 	if (ret < 0) 
// 	{
// 		app.DebugPrintf("Failed to get auth code 0x%x\n", ret);
// 		sceNpAuthDeleteOAuthRequest(reqId);
// 		return false;
// 	}

	ret = sceNpAuthDeleteOAuthRequest(reqId);
	if (ret < 0) 
	{
		app.DebugPrintf("Couldn't delete auth request 0x%x\n", ret);
		return false;
	}

	SceRemoteStorageInitParams params;

	params.callback = SonyRemoteStorage_Vita::staticInternalCallback;
	params.userData = this;
	params.thread.threadAffinity = SCE_KERNEL_THREAD_CPU_AFFINITY_MASK_DEFAULT;
	params.thread.threadPriority = SCE_KERNEL_DEFAULT_PRIORITY_USER;
// 	memcpy(params.authCode, authCode.code, SCE_NP_AUTHORIZATION_CODE_MAX_LEN);
	strcpy(params.clientId, CLIENT_ID);
	params.timeout.connectMs = 30 * 1000;	//30 seconds is the default
	params.timeout.resolveMs = 30 * 1000;	//30 seconds is the default
	params.timeout.receiveMs = 120 * 1000;	//120 seconds is the default
	params.timeout.sendMs = 120 * 1000;		//120 seconds is the default
	params.pool.memPoolSize = 7 * 1024 * 1024;
	if(m_memPoolBuffer == nullptr)
		m_memPoolBuffer = malloc(params.pool.memPoolSize);
	params.pool.memPoolBuffer = m_memPoolBuffer;

	SceRemoteStorageAbortReqParams abortParams;

	ret = sceRemoteStorageInit(params);
	if(ret >= 0) 
	{
		abortParams.requestId = ret;
		app.DebugPrintf("Session will be created \n");
	} 
	else if(ret == SCE_REMOTE_STORAGE_ERROR_ALREADY_INITIALISED)
	{
		app.DebugPrintf("Session already created \n");
		runCallback();
	}
	else
	{
		app.DebugPrintf("Error creating session: 0x%x \n", ret);
		return false;
	}
	return true;
}



bool SonyRemoteStorage_Vita::getRemoteFileInfo(SceRemoteStorageStatus* pInfo, CallbackFunc cb, LPVOID lpParam)
{
	m_callbackFunc = cb;
	m_callbackParam = lpParam;
	outputGetStatus = pInfo;

	SceRemoteStorageStatusReqParams params;
	reqId = sceRemoteStorageGetStatus(params, outputGetStatus);
	m_status = e_getStatusInProgress;

	if(reqId >= 0) 
	{
		app.DebugPrintf("Get Status request sent \n");
		return true;
	} 
	else 
	{
		app.DebugPrintf("Error sending Get Status request: 0x%x \n", reqId);
		return false;
	}
}

void SonyRemoteStorage_Vita::abort()
{
	m_bAborting = true;
	app.DebugPrintf("Aborting...\n");
	if(m_bTransferStarted)
	{
		app.DebugPrintf("transfer has started so we'll call sceRemoteStorageAbort...\n");

		SceRemoteStorageAbortReqParams params;
		params.requestId = reqId;
		int ret = sceRemoteStorageAbort(params);

		if(ret >= 0) 
		{
			app.DebugPrintf("Abort request done \n");
		} 
		else 
		{
			app.DebugPrintf("Error in Abort request: 0x%x \n", ret);
		}
	}
}



bool SonyRemoteStorage_Vita::setDataInternal()
{
	//	CompressSaveData();		// check if we need to re-save the file compressed first

	snprintf(m_saveFilename, sizeof(m_saveFilename), "%s:%s/GAMEDATA.bin", "savedata0", m_setDataSaveInfo->UTF8SaveFilename);

	SceFiosSize outSize = sceFiosFileGetSizeSync(nullptr, m_saveFilename);
	m_uploadSaveSize = static_cast<int>(outSize);
	
	strcpy(m_saveFileDesc, m_setDataSaveInfo->UTF8SaveTitle);
	m_status = e_setDataInProgress;


	SceRemoteStorageSetDataReqParams params;
	params.visibility = PUBLIC_READ_WRITE;
	strcpy(params.pathLocation, m_saveFilename);
	sprintf(params.fileName, getRemoteSaveFilename());

	GetDescriptionData(params.fileDescription);


	if(m_bAborting)
	{
		runCallback();
		return false;
	}
	reqId = sceRemoteStorageSetData(params);

	app.DebugPrintf("\n*******************************\n");
	if(reqId >= 0) 
	{
		app.DebugPrintf("Set Data request sent \n");
		m_bTransferStarted = true;
		return true;
	} 
	else 
	{
		app.DebugPrintf("Error sending Set Data request: 0x%x \n", reqId);
		return false;
	}
}


bool SonyRemoteStorage_Vita::getData( const char* remotePath, const char* localPath, CallbackFunc cb, LPVOID lpParam )
{
	m_callbackFunc = cb;
	m_callbackParam = lpParam;

	SceRemoteStorageGetDataReqParams params;
	sprintf(params.pathLocation, "savedata0:%s/GAMEDATA.bin", localPath);
// 	strcpy(params.pathLocation, localPath);
	// 	strcpy(params.fileName, "/test/small.txt");
	strcpy(params.fileName, remotePath);
	memset(&params.psVitaSaveDataSlot, 0, sizeof(params.psVitaSaveDataSlot));
	SceRemoteStorageData s_getDataOutput;
	reqId = sceRemoteStorageGetData(params, &s_getDataOutput);

	app.DebugPrintf("\n*******************************\n");
	if(reqId >= 0) 
	{
		app.DebugPrintf("Get Data request sent \n");
		m_bTransferStarted = true;
		return true;
	} 
	else 
	{
		app.DebugPrintf("Error sending Get Data request: 0x%x \n", reqId);
		return false;
	}
}

void SonyRemoteStorage_Vita::runCallback()
{
	assert(m_callbackFunc);
	if(m_callbackFunc)
	{
		m_callbackFunc(m_callbackParam, m_status, m_lastErrorCode);
	}
		m_lastErrorCode = SCE_OK;
}
