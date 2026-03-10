#include "stdafx.h"

#include "SonyRemoteStorage_PS3.h"
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



// 4J-PB - ticketing changed to our SCEE product id
#define TICKETING_SERVICE_ID "EP4433-NPEB01899_00" //"EP9009-NPWA00114_00"
#define CLIENT_ID "969e9d21-527c-4c22-b539-f8e479f690bc"




void SonyRemoteStorage_PS3::npauthhandler(int event, int result, void *arg)
{
#ifdef __PS3__
	if (event != SCE_NP_MANAGER_EVENT_GOT_TICKET || result <= 0)
	{
		app.DebugPrintf("Could not retrieve ticket: 0x%x\n", result);
	}
	else
	{
		psnTicketSize = result;
		psnTicket = malloc(psnTicketSize);
<<<<<<< HEAD
		if (psnTicket == nullptr) 
=======
		if (psnTicket == NULL)
>>>>>>> origin/main
		{
			app.DebugPrintf("Failed to allocate for ticket\n");
		}

		int ret = sceNpManagerGetTicket(psnTicket, &psnTicketSize);
		if (ret < 0)
		{
			app.DebugPrintf("Could not retrieve ticket: 0x%x\n", ret);
			free(psnTicket);
			psnTicket = 0;
			return;
		}
	}
	m_waitingForTicket = false;
#endif
}

int SonyRemoteStorage_PS3::initPreconditions()
{
	int ret = 0;

	SceNpId npId;

	ret = sceNpManagerGetNpId(&npId);
	if(ret < 0)
	{
		return ret;
	}
	SceNpTicketVersion ticketVersion;
	ticketVersion.major = 3;
	ticketVersion.minor = 0;
	ret = sceNpManagerRequestTicket2(&npId, &ticketVersion, TICKETING_SERVICE_ID, NULL, 0, NULL, 0);
	if(ret < 0)
	{
		return ret;
	}
	m_waitingForTicket = true;
	while(m_waitingForTicket)
	{
		cellSysutilCheckCallback();
		sys_timer_usleep(50000); //50 milliseconds.
	}
	if(psnTicket == nullptr)
		return -1;

	return 0;
}

void SonyRemoteStorage_PS3::staticInternalCallback(const SceRemoteStorageEvent event, int32_t retCode, void * userData)
{
	static_cast<SonyRemoteStorage_PS3 *>(userData)->internalCallback(event, retCode);
}

void SonyRemoteStorage_PS3::internalCallback(const SceRemoteStorageEvent event, int32_t retCode)
{
	m_lastErrorCode = retCode;

	switch(event)
	{
	case ERROR_OCCURRED:
		app.DebugPrintf("An error occurred with retCode: 0x%x \n", retCode);
// 		shutdown();				// removed, as the remote storage lib now tries to reconnect if an error has occurred
		m_status = e_error;
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

bool SonyRemoteStorage_PS3::init(CallbackFunc cb, LPVOID lpParam)
{
	m_callbackFunc = cb;
	m_callbackParam = lpParam;
	m_bTransferStarted = false;
	m_bAborting = false;

	if(m_bInitialised)
	{
		runCallback();
		return true;
	}


	if(m_bInitialised)
	{
		internalCallback(USER_ACCOUNT_LINKED, 0);
		return true;
	}

	int ret = initPreconditions();
	if(ret < 0)
	{
		return false;
	}

	SceRemoteStorageInitParams params;

	params.callback = staticInternalCallback;
	params.userData = this;
	params.thread.threadAffinity = 0; //Not used in PS3
	params.thread.threadPriority = 1000; //Must be between [0-3071], being 0 the highest.
	params.psnTicket = psnTicket;
	params.psnTicketSize = psnTicketSize;
	strcpy(params.clientId, CLIENT_ID);
	params.timeout.connectMs = 30 * 1000;	//30 seconds is the default
	params.timeout.resolveMs = 30 * 1000;	//30 seconds is the default
	params.timeout.receiveMs = 120 * 1000;	//120 seconds is the default
	params.timeout.sendMs = 120 * 1000;		//120 seconds is the default
	params.pool.memPoolSize = 7 * 1024 * 1024;
	if(m_memPoolBuffer == nullptr)
		m_memPoolBuffer = malloc(params.pool.memPoolSize);
	params.pool.memPoolBuffer = m_memPoolBuffer;

// 	SceRemoteStorageAbortReqParams abortParams;

	ret = sceRemoteStorageInit(params);
	if(ret >= 0 || ret == SCE_REMOTE_STORAGE_ERROR_ALREADY_INITIALISED)
	{
// 		abortParams.requestId = ret;
		//ret = sceRemoteStorageAbort(abortParams);
		app.DebugPrintf("Session will be created \n");
		//if(ret >= 0)
		//{
		//	printf("Session aborted \n");
		//} else
		//{
		//	printf("Error aborting session: 0x%x \n", ret);
		//}
	}
	else
	{
		app.DebugPrintf("Error creating session: 0x%x \n", ret);
		return false;
	}
	return true;
}



bool SonyRemoteStorage_PS3::getRemoteFileInfo(SceRemoteStorageStatus* pInfo, CallbackFunc cb, LPVOID lpParam)
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

void SonyRemoteStorage_PS3::abort()
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



bool SonyRemoteStorage_PS3::setDataInternal()
{
	CompressSaveData();		// check if we need to re-save the file compressed first


	strcpy(m_saveFilename, m_setDataSaveInfo->UTF8SaveFilename);
	strcpy(m_saveFileDesc, m_setDataSaveInfo->UTF8SaveTitle);


	SceRemoteStorageSetDataReqParams params;
	params.visibility = PUBLIC_READ_WRITE;
	strcpy(params.pathLocation, m_saveFilename);
	sprintf(params.fileName, getRemoteSaveFilename());

	DescriptionData descData;
	ZeroMemory(&descData, sizeof(DescriptionData));
	descData.m_platform[0] = SAVE_FILE_PLATFORM_LOCAL & 0xff;
	descData.m_platform[1] = (SAVE_FILE_PLATFORM_LOCAL >> 8) & 0xff;
	descData.m_platform[2] = (SAVE_FILE_PLATFORM_LOCAL >> 16) & 0xff;
	descData.m_platform[3] = (SAVE_FILE_PLATFORM_LOCAL >> 24)& 0xff;

	if(m_thumbnailData)
	{
		unsigned int uiHostOptions;
		bool bHostOptionsRead;
		DWORD uiTexturePack;
		char seed[22];
		app.GetImageTextData(m_thumbnailData, m_thumbnailDataSize,reinterpret_cast<unsigned char *>(seed), uiHostOptions, bHostOptionsRead, uiTexturePack);

		int64_t iSeed = strtoll(seed, nullptr,10);
		char seedHex[17];
		sprintf(seedHex,"%016llx",iSeed);
		memcpy(descData.m_seed,seedHex,16); // Don't copy null

		// Save the host options that this world was last played with
		char hostOptions[9];
		sprintf(hostOptions,"%08x",uiHostOptions);
		memcpy(descData.m_hostOptions,hostOptions,8); // Don't copy null

		// Save the texture pack id
		char texturePack[9];
		sprintf(texturePack,"%08x",uiTexturePack);
		memcpy(descData.m_texturePack,texturePack,8); // Don't copy null
	}

	memcpy(descData.m_saveNameUTF8, m_saveFileDesc, strlen(m_saveFileDesc)+1); // plus null
	memcpy(params.fileDescription, &descData, sizeof(descData));
	strcpy(params.ps3DataFilename, "GAMEDATA");

	params.ps3FileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
	memcpy(params.secureFileId, m_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);


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
		m_status = e_setDataInProgress;
		return true;
	}
	else
	{
		app.DebugPrintf("Error sending Set Data request: 0x%x \n", reqId);
		return false;
	}
}

bool SonyRemoteStorage_PS3::getData( const char* remotePath, const char* localPath, CallbackFunc cb, LPVOID lpParam )
{
	m_callbackFunc = cb;
	m_callbackParam = lpParam;

	SceRemoteStorageGetDataReqParams params;
	strcpy(params.pathLocation, localPath);//"ABCD12345-RS-DATA");
	strcpy(params.fileName, remotePath);//"/test/small.txt");
	strcpy(params.ps3DataFilename, "GAMEDATA");
	params.ps3FileType = CELL_SAVEDATA_FILETYPE_NORMALFILE;
	memcpy(params.secureFileId, m_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);

	reqId = sceRemoteStorageGetData(params, &outputGetData);

	app.DebugPrintf("\n*******************************\n");
	if(reqId >= 0)
	{
		app.DebugPrintf("Get Data request sent \n");
		m_bTransferStarted = true;
		m_status = e_getDataInProgress;
	} else
	{
		app.DebugPrintf("Error sending Get Data request: 0x%x \n", reqId);
	}

}

void SonyRemoteStorage_PS3::runCallback()
{
	assert(m_callbackFunc);
	if(m_callbackFunc)
	{
		m_callbackFunc(m_callbackParam, m_status, m_lastErrorCode);
	}
	m_lastErrorCode = ERROR_SUCCESS;
}

int SonyRemoteStorage_PS3::SaveCompressCallback(LPVOID lpParam,bool bRes)
{
	SonyRemoteStorage_PS3* pRS = static_cast<SonyRemoteStorage_PS3 *>(lpParam);
	pRS->m_compressedSaveState = e_state_Idle;
	return 0;
}

int SonyRemoteStorage_PS3::LoadCompressCallback(void *pParam,bool bIsCorrupt, bool bIsOwner)
{
	SonyRemoteStorage_PS3* pRS = static_cast<SonyRemoteStorage_PS3 *>(pParam);
	int origFilesize = StorageManager.GetSaveSize();
	void* pOrigSaveData = malloc(origFilesize);
	unsigned int retFilesize;
	StorageManager.GetSaveData( pOrigSaveData, &retFilesize );
	// check if this save file is already compressed
	if(*static_cast<int *>(pOrigSaveData) != 0)
	{
		app.DebugPrintf("compressing save data\n");

		// Assume that the compression will make it smaller so initially attempt to allocate the current file size
		// We add 4 bytes to the start so that we can signal compressed data
		// And another 4 bytes to store the decompressed data size
		unsigned int compLength = origFilesize+8;
		byte *compData = static_cast<byte *>(malloc(compLength));
		Compression::UseDefaultThreadStorage();
		Compression::getCompression()->Compress(compData+8,&compLength,pOrigSaveData,origFilesize);
		ZeroMemory(compData,8);
		int saveVer = 0;
		memcpy( compData, &saveVer, sizeof(int) );
		memcpy( compData+4, &origFilesize, sizeof(int) );

		StorageManager.FreeSaveData();
		StorageManager.SetSaveData(compData,compLength+8);
		pRS->m_compressedSaveState = e_state_CompressedSave;
	}
	else
	{
		// already compressed, do nothing
		pRS->m_compressedSaveState = e_state_Idle;
	}

	return 0;
}

void SonyRemoteStorage_PS3::CompressSaveData()
{
	app.DebugPrintf("CompressSaveData\n");
	m_compressedSaveState = e_state_LoadingSave;
	app.DebugPrintf("Loading save\n");
	waitForStorageManagerIdle();
	C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveData(m_setDataSaveInfo, &LoadCompressCallback, this);
	if(eLoadStatus != C4JStorage::ESaveGame_Load)
	{
		app.DebugPrintf("Failed to load savegame for compression!!!!!!\n");
		m_compressedSaveState = e_state_Idle;
		return;
	}
	while(m_compressedSaveState == e_state_LoadingSave)
	{
		Sleep(10);
	}
	if(m_compressedSaveState == e_state_CompressedSave)
	{

		waitForStorageManagerIdle();
		app.DebugPrintf("Saving compressed save\n");
		waitForStorageManagerIdle();
		StorageManager.SetDefaultSaveImage();		// we can't get the save image back to overwrite, so set it to the default
		C4JStorage::ESaveGameState storageState = StorageManager.SaveSaveData( &SaveCompressCallback, this, true );		// only save the data file, so we don't overwrite the icon
		if(storageState == C4JStorage::ESaveGame_Save)
		{
			m_compressedSaveState = e_state_SavingSave;
			while(m_compressedSaveState == e_state_SavingSave)
			{
				Sleep(10);
			}
		}
		else
		{
			app.DebugPrintf("StorageManager.SaveSaveData failed, save is left uncompressed\n");
		}
	}
	waitForStorageManagerIdle();

	app.DebugPrintf("done\n");
	assert(m_compressedSaveState == e_state_Idle);
}

