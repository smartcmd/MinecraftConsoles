

#include "stdafx.h"
#include "SonyRemoteStorage.h"


static const char sc_remoteSaveFilename[] = "/minecraft_save/gamedata.rs";
#ifdef __PSVITA__
static const char sc_localSaveFilename[] = "CloudSave_Vita.bin";
static const char sc_localSaveFullPath[] = "savedata0:CloudSave_Vita.bin";
#elif defined __PS3__
static const char sc_localSaveFilename[] = "CloudSave_PS3.bin";
static const char sc_localSaveFullPath[] = "NPEB01899--140720203552";
#else
static const char sc_localSaveFilename[] = "CloudSave_Orbis.bin";
static const char sc_localSaveFullPath[] = "/app0/CloudSave_Orbis.bin";
#endif

static SceRemoteStorageStatus statParams;




// void remoteStorageGetCallback(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
// {
// 	app.DebugPrintf("remoteStorageGetCallback err : 0x%08x\n");
// }
// 
// void remoteStorageCallback(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
// {
// 	app.DebugPrintf("remoteStorageCallback err : 0x%08x\n");
// 
// 	app.getRemoteStorage()->getRemoteFileInfo(&statParams, remoteStorageGetInfoCallback, NULL);
// }








void getSaveInfoReturnCallback(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
{
	SonyRemoteStorage* pRemoteStorage = (SonyRemoteStorage*)lpParam;
	app.DebugPrintf("remoteStorageGetInfoCallback err : 0x%08x\n", error_code);
	if(error_code == 0)
	{
		for(int i=0;i<statParams.numFiles;i++)
		{
			if(strcmp(statParams.data[i].fileName, sc_remoteSaveFilename) == 0)
			{
				// found the file we need in the cloud
				pRemoteStorage->m_getInfoStatus = SonyRemoteStorage::e_infoFound;
				pRemoteStorage->m_remoteFileInfo = &statParams.data[i];
			}
		}
	}
	if(pRemoteStorage->m_getInfoStatus != SonyRemoteStorage::e_infoFound)
		pRemoteStorage->m_getInfoStatus = SonyRemoteStorage::e_noInfoFound;
}






static void getSaveInfoInitCallback(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
{
	SonyRemoteStorage* pRemoteStorage = (SonyRemoteStorage*)lpParam;
	if(error_code != 0)
	{
		app.DebugPrintf("getSaveInfoInitCallback err : 0x%08x\n", error_code);
		pRemoteStorage->m_getInfoStatus = SonyRemoteStorage::e_noInfoFound;
	}
	else
	{
		app.DebugPrintf("getSaveInfoInitCallback calling getRemoteFileInfo\n");
		app.getRemoteStorage()->getRemoteFileInfo(&statParams, getSaveInfoReturnCallback, pRemoteStorage);
	}
}

void SonyRemoteStorage::getSaveInfo()
{
	if(m_getInfoStatus == e_gettingInfo)
	{
		app.DebugPrintf("SonyRemoteStorage::getSaveInfo already running!!!\n");
		return;
	}

	m_getInfoStatus = e_gettingInfo;
	if(!ProfileManager.IsSignedInLive(ProfileManager.GetPrimaryPad()))
	{
		m_getInfoStatus = e_noInfoFound;
		return;
	}
	app.DebugPrintf("SonyRemoteStorage::getSaveInfo calling init\n");

	bool bOK = init(getSaveInfoInitCallback, this);
	if(!bOK)
		m_getInfoStatus = e_noInfoFound;
}

bool SonyRemoteStorage::getSaveData( const char* localDirname, CallbackFunc cb, LPVOID lpParam )
{
	m_startTime = System::currentTimeMillis();
	m_dataProgress = 0;
	return getData(sc_remoteSaveFilename, localDirname, cb, lpParam);
}


static void setSaveDataInitCallback(LPVOID lpParam, SonyRemoteStorage::Status s, int error_code)
{
	SonyRemoteStorage* pRemoteStorage = (SonyRemoteStorage*)lpParam;
	if(error_code != 0)
	{
		app.DebugPrintf("setSaveDataInitCallback err : 0x%08x\n", error_code);
		pRemoteStorage->m_setDataStatus = SonyRemoteStorage::e_settingDataFailed;
		if(pRemoteStorage->m_initCallbackFunc)
			pRemoteStorage->m_initCallbackFunc(pRemoteStorage->m_initCallbackParam, s, error_code);
	}
	else
	{
		app.getRemoteStorage()->setData(pRemoteStorage->m_setSaveDataInfo, pRemoteStorage->m_initCallbackFunc, pRemoteStorage->m_initCallbackParam);
	}

}
bool SonyRemoteStorage::setSaveData(PSAVE_INFO info, CallbackFunc cb, void* lpParam)
{
	m_setSaveDataInfo = info;
	m_setDataStatus = e_settingData;
	m_initCallbackFunc = cb;
	m_initCallbackParam = lpParam;
	m_dataProgress = 0;
	bool bOK = init(setSaveDataInitCallback, this);
	if(!bOK)
		m_setDataStatus = e_settingDataFailed;

	return bOK;
}

const char* SonyRemoteStorage::getLocalFilename()
{
	return sc_localSaveFullPath;
}

const char* SonyRemoteStorage::getSaveNameUTF8()
{
	if(m_getInfoStatus != e_infoFound)
		return NULL;
	DescriptionData* pDescData = (DescriptionData*)m_remoteFileInfo->fileDescription;
	return pDescData->m_saveNameUTF8;
}

ESavePlatform SonyRemoteStorage::getSavePlatform()
{
	if(m_getInfoStatus != e_infoFound)
		return SAVE_FILE_PLATFORM_NONE;
	DescriptionData* pDescData = (DescriptionData*)m_remoteFileInfo->fileDescription;
	return (ESavePlatform)MAKE_FOURCC(pDescData->m_platform[0], pDescData->m_platform[1], pDescData->m_platform[2], pDescData->m_platform[3]);

}

__int64 SonyRemoteStorage::getSaveSeed()
{
	if(m_getInfoStatus != e_infoFound)
		return 0;
	DescriptionData* pDescData = (DescriptionData*)m_remoteFileInfo->fileDescription;

	char seedString[17];
	ZeroMemory(seedString,17);	
	memcpy(seedString, pDescData->m_seed,16);

	__uint64 seed = 0;
	std::stringstream ss;
	ss << seedString;
	ss >> std::hex >> seed;
	return seed;
}

unsigned int SonyRemoteStorage::getSaveHostOptions()
{
	if(m_getInfoStatus != e_infoFound)
		return 0;
	DescriptionData* pDescData = (DescriptionData*)m_remoteFileInfo->fileDescription;

	char optionsString[9];
	ZeroMemory(optionsString,9);	
	memcpy(optionsString, pDescData->m_hostOptions,8);

	unsigned int uiHostOptions = 0;
	std::stringstream ss;
	ss << optionsString;
	ss >> std::hex >> uiHostOptions;
	return uiHostOptions;
}

unsigned int SonyRemoteStorage::getSaveTexturePack()
{
	if(m_getInfoStatus != e_infoFound)
		return 0;
	DescriptionData* pDescData = (DescriptionData*)m_remoteFileInfo->fileDescription;

	char textureString[9];
	ZeroMemory(textureString,9);	
	memcpy(textureString, pDescData->m_texturePack,8);

	unsigned int uiTexturePack = 0;
	std::stringstream ss;
	ss << textureString;
	ss >> std::hex >> uiTexturePack;
	return uiTexturePack;
}

const char* SonyRemoteStorage::getRemoteSaveFilename()
{
	return sc_remoteSaveFilename;
}

int SonyRemoteStorage::getSaveFilesize()
{
	if(m_getInfoStatus == e_infoFound) 
	{
		return m_remoteFileInfo->fileSize; 
	}
	return 0;
}


bool SonyRemoteStorage::setData( PSAVE_INFO info, CallbackFunc cb, LPVOID lpParam )
{
	m_setDataSaveInfo = info;
	m_callbackFunc = cb;
	m_callbackParam = lpParam;
	m_status = e_setDataInProgress;

	C4JStorage::ESaveGameState eLoadStatus=StorageManager.LoadSaveDataThumbnail(info,&LoadSaveDataThumbnailReturned,this);
	return true;
}

int SonyRemoteStorage::LoadSaveDataThumbnailReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes)
{
	SonyRemoteStorage *pClass= (SonyRemoteStorage *)lpParam;

	if(pClass->m_bAborting)
	{
		pClass->runCallback();
		return 0;
	}

	app.DebugPrintf("Received data for a thumbnail\n");

	if(pbThumbnail && dwThumbnailBytes)
	{
		pClass->m_thumbnailData = pbThumbnail;
		pClass->m_thumbnailDataSize = dwThumbnailBytes;
	}
	else
	{
		app.DebugPrintf("Thumbnail data is NULL, or has size 0\n");
		pClass->m_thumbnailData = NULL;
		pClass->m_thumbnailDataSize = 0;
	}

	if(pClass->m_SetDataThread != NULL)
		delete pClass->m_SetDataThread;

	pClass->m_SetDataThread = new C4JThread(setDataThread, pClass, "setDataThread");
	pClass->m_SetDataThread->Run();

	return 0;
}

int SonyRemoteStorage::setDataThread(void* lpParam)
{
	SonyRemoteStorage* pClass = (SonyRemoteStorage*)lpParam;
	pClass->m_startTime = System::currentTimeMillis();
	pClass->setDataInternal();
	return 0;
}

bool SonyRemoteStorage::saveIsAvailable()
{
	if(m_getInfoStatus != e_infoFound)
		return false;
#ifdef __PS3__
	return (getSavePlatform() == SAVE_FILE_PLATFORM_PSVITA); 
#elif defined __PSVITA__
	return (getSavePlatform() == SAVE_FILE_PLATFORM_PS3); 
#else // __ORBIS__
	return true;
#endif
}

int SonyRemoteStorage::getDataProgress()
{
	__int64 time = System::currentTimeMillis();
	int elapsedSecs = (time - m_startTime) / 1000;
	int progVal = m_dataProgress + (elapsedSecs/3);
	if(progVal > 95)
	{
		return m_dataProgress;
	}
	return progVal;
}


bool SonyRemoteStorage::shutdown()
{
	if(m_bInitialised)
	{
		int ret = sceRemoteStorageTerm();
		if(ret >= 0) 
		{
			app.DebugPrintf("Term request done \n");
			m_bInitialised = false;
			free(m_memPoolBuffer);
			m_memPoolBuffer = NULL;
			return true;
		} 
		else 
		{
			app.DebugPrintf("Error in Term request: 0x%x \n", ret);
			return false;
		}
	}
	return true;
}


void SonyRemoteStorage::waitForStorageManagerIdle()
{
	C4JStorage::ESaveGameState storageState = StorageManager.GetSaveState();
	while(storageState != C4JStorage::ESaveGame_Idle)
	{
		Sleep(10);
// 		app.DebugPrintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>           >>>>>     storageState = %d\n", storageState);
		storageState = StorageManager.GetSaveState();
	}
}
