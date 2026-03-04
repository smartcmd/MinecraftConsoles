#include "stdafx.h"

#include "SonyRemoteStorage_Orbis.h"
#include "SonyHttp_Orbis.h"
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


void SonyRemoteStorage_Orbis::staticInternalCallback(const SceRemoteStorageEvent event, int32_t retCode, void * userData)
{
	((SonyRemoteStorage_Orbis*)userData)->internalCallback(event, retCode);
}
void SonyRemoteStorage_Orbis::internalCallback(const SceRemoteStorageEvent event, int32_t retCode)
{
	m_lastErrorCode = retCode;

	switch(event)
	{
	case ERROR_OCCURRED:
		app.DebugPrintf("SonyRemoteStorage_Orbis: An error occurred with retCode: 0x%x \n", retCode);
		m_status = e_error;
// 		shutdown();				// removed, as the remote storage lib now tries to reconnect if an error has occurred
		runCallback();
		break;

	case GET_DATA_RESULT:
		if(retCode >= 0) 
		{
			app.DebugPrintf("SonyRemoteStorage_Orbis: Get Data success \n");
			m_status = e_getDataSucceeded;
		} 
		else 
		{
			app.DebugPrintf("SonyRemoteStorage_Orbis: An error occurred while Get Data was being processed. retCode: 0x%x \n", retCode);
			m_status = e_error;
		}
		if(StorageManager.SaveGameDirUnMount(m_mountPoint) == false)
		{
			app.DebugPrintf("Failed to unmount %s\n", m_mountPoint);
		}

		m_mountPoint[0] = 0;
		runCallback();
		break;

	case GET_DATA_PROGRESS:
		app.DebugPrintf("SonyRemoteStorage_Orbis: Get data progress: %i%%\n", retCode);
		m_status = e_getDataInProgress;
		m_dataProgress = retCode;
		m_startTime = System::currentTimeMillis();
		break;

	case GET_STATUS_RESULT:
		if(retCode >= 0) 
		{
			app.DebugPrintf("SonyRemoteStorage_Orbis: Get Status success \n");
			app.DebugPrintf("SonyRemoteStorage_Orbis: Remaining Syncs for this user: %llu\n", outputGetStatus->remainingSyncs);
			app.DebugPrintf("SonyRemoteStorage_Orbis: Number of files on the cloud: %d\n", outputGetStatus->numFiles);
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
			app.DebugPrintf("SonyRemoteStorage_Orbis: An error occurred while Get Status was being processed. retCode: 0x%x \n", retCode);
			m_status = e_error;
		}
		runCallback();
		break;

	case PSN_SIGN_IN_REQUIRED:
		app.DebugPrintf("SonyRemoteStorage_Orbis: User's PSN sign-in through web browser is required \n");
		m_status = e_signInRequired;
		runCallback();
		break;

	case SET_DATA_RESULT:
		if(retCode >= 0) 
		{
			app.DebugPrintf("SonyRemoteStorage_Orbis: Set Data success \n");
			m_status = e_setDataSucceeded;
		} 
		else 
		{
			app.DebugPrintf("SonyRemoteStorage_Orbis: An error occurred while Set Data was being processed. retCode: 0x%x \n", retCode);
			m_status = e_error;
		}
		runCallback();
		break;

	case SET_DATA_PROGRESS:
		app.DebugPrintf("SonyRemoteStorage_Orbis: Set data progress: %i%%\n", retCode);
		m_status = e_setDataInProgress;
		m_dataProgress = retCode;

		break;

	case USER_ACCOUNT_LINKED:
		app.DebugPrintf("SonyRemoteStorage_Orbis: User's account has been linked with PSN \n");
		m_bInitialised = true;
		m_status = e_accountLinked;
		runCallback();
		break;

	case WEB_BROWSER_RESULT:
		app.DebugPrintf("SonyRemoteStorage_Orbis: This function is not used on PS Vita, as the account will be linked, it is not needed to open a browser to link it \n");
		assert(0);
		break;

	default:
		app.DebugPrintf("SonyRemoteStorage_Orbis: This should never happen \n");
		assert(0);
		break;

	}
}

bool SonyRemoteStorage_Orbis::init(CallbackFunc cb, LPVOID lpParam)
{

	int ret = 0;
	int reqId = 0;
	SceNpId npId;
	SceUserServiceUserId userId;
	SceRemoteStorageInitParams params;

	m_callbackFunc = cb;
	m_callbackParam = lpParam;

	if(m_bInitialised)
	{
		runCallback();
		return true;
	}

	ret = sceUserServiceGetInitialUser(&userId); 
	if (ret < 0) 
	{
		app.DebugPrintf("Couldn't retrieve user ID 0x%x\n", ret);
		return false;
	}

	ret = sceNpGetNpId(userId, &npId);
	if (ret < 0) {
		app.DebugPrintf("Couldn't retrieve NP ID 0x%x\n", ret);
		return false;
	}

// 	ret = sceNpAuthCreateRequest();
// 	if (ret < 0) {
// 		app.DebugPrintf("Couldn't create auth request 0x%x\n", ret);
// 		return false;
// 	}
// 
// 	reqId = ret;
// 
// 	SceNpClientId clientId;
// 	memset(&clientId, 0x0, sizeof(clientId));

// 	SceNpAuthorizationCode authCode;
// 	memset(&authCode, 0x0, sizeof(authCode));

// 	SceNpAuthGetAuthorizationCodeParameter authParams;
// 	memset(&authParams, 0x0, sizeof(authParams));	
// 
// 	authParams.size = sizeof(authParams);
// 	authParams.pOnlineId = &npId.handle;
// 	authParams.pScope = AUTH_SCOPE;	

// 	memcpy(clientId.id, CLIENT_ID, strlen(CLIENT_ID));
// 	authParams.pClientId = &clientId;	

// 	ret = sceNpAuthGetAuthorizationCode(reqId, &authParams, &authCode, NULL);
// 	if (ret < 0) {
// 		app.DebugPrintf("Failed to get auth code 0x%x\n", ret);
// 	}

// 	ret = sceNpAuthDeleteRequest(reqId);
// 	if (ret < 0) {
// 		app.DebugPrintf("Couldn't delete auth request 0x%x\n", ret);
// 	}

	params.callback = staticInternalCallback;
	params.userData = this;
// 	memcpy(params.authCode, authCode.code, SCE_NP_AUTHORIZATION_CODE_MAX_LEN);
	params.userId = userId;
	params.environment = DEVELOPMENT;
	params.httpContextId = SonyHttp_Orbis::getHTTPContextID();
	strcpy(params.clientId, CLIENT_ID);

	params.thread.threadAffinity = SCE_KERNEL_CPUMASK_USER_ALL;
	params.thread.threadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;	

	params.timeout.connectMs = 30 * 1000;	//30 seconds is the default
	params.timeout.resolveMs = 30 * 1000;	//30 seconds is the default
	params.timeout.receiveMs = 120 * 1000;	//120 seconds is the default
	params.timeout.sendMs = 120 * 1000;		//120 seconds is the default

	params.pool.memPoolSize = 7 * 1024 * 1024;
	if(m_memPoolBuffer == NULL)
		m_memPoolBuffer = malloc(params.pool.memPoolSize);

	params.pool.memPoolBuffer = m_memPoolBuffer;	

	ret = sceRemoteStorageInit(params);
	if (ret >= 0) 
	{
		app.DebugPrintf("Session will be created \n");
	} 
	else if(ret == SCE_REMOTE_STORAGE_ERROR_ALREADY_INITIALISED)
	{
		app.DebugPrintf("Already initialised: 0x%x \n", ret);
		runCallback();
	}
	else
	{
		app.DebugPrintf("Error creating session: 0x%x \n", ret);
		return false;
	}
	return true;
}



bool SonyRemoteStorage_Orbis::getRemoteFileInfo(SceRemoteStorageStatus* pInfo, CallbackFunc cb, LPVOID lpParam)
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

void SonyRemoteStorage_Orbis::abort()
{
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


bool SonyRemoteStorage_Orbis::setData( PSAVE_INFO info, CallbackFunc cb, LPVOID lpParam )
{
	assert(0);
	return false;

// 	m_callbackFunc = cb;
// 	m_callbackParam = lpParam;
// 
// 	strcpy(m_saveFilename, savePath);
// 	strcpy(m_saveFileDesc, saveDesc);
// 	m_status = e_setDataInProgress;
// 
// 
// 	SceRemoteStorageSetDataReqParams params;
// 	params.visibility = PUBLIC_READ_WRITE;
// 	strcpy(params.pathLocation, m_saveFilename);
// 	sprintf(params.fileName, "/%s/GAMEDATA", m_saveFilename);
// //	strcpy(params.fileName, "/test/small.txt");
// 	strcpy(params.fileDescription, m_saveFileDesc);
// 	strcpy(params.ps3DataFilename, "GAMEDATA");
// 
// 	params.ps3FileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
// 	memcpy(params.secureFileId, m_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);
// 
// 
// 
// 	reqId = sceRemoteStorageSetData(params);
// 
// 	app.DebugPrintf("\n*******************************\n");
// 	if(reqId >= 0) 
// 	{
// 		app.DebugPrintf("Set Data request sent \n");
// 		return true;
// 	} 
// 	else 
// 	{
// 		app.DebugPrintf("Error sending Set Data request: 0x%x \n", reqId);
// 		return false;
// 	}
}

bool SonyRemoteStorage_Orbis::getData( const char* remotePath, const char* localPath, CallbackFunc cb, LPVOID lpParam )
{
	m_callbackFunc = cb;
	m_callbackParam = lpParam;

	if(StorageManager.SaveGameDirMountExisting(m_mountPoint) == false)
	{
		app.DebugPrintf("Error mounting save dir \n");
		m_mountPoint[0] = 0;
		return false;
	}

	SceRemoteStorageGetDataReqParams params;
	sprintf(params.pathLocation, "%s/GAMEDATA", m_mountPoint);
	// 	strcpy(params.fileName, "/test/small.txt");
	strcpy(params.fileName, remotePath);
	SceRemoteStorageData s_getDataOutput;
	reqId = sceRemoteStorageGetData(params, &s_getDataOutput);

	app.DebugPrintf("\n*******************************\n");
	if(reqId >= 0) 
	{
		app.DebugPrintf("Get Data request sent \n");
		return true;
	} 
	else 
	{
		app.DebugPrintf("Error sending Get Data request: 0x%x \n", reqId);
		return false;
	}
}

void SonyRemoteStorage_Orbis::runCallback()
{
	assert(m_callbackFunc);
	if(m_callbackFunc)
	{
		m_callbackFunc(m_callbackParam, m_status, m_lastErrorCode);
	}
	m_lastErrorCode = SCE_OK;
}
