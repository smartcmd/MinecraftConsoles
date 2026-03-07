

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




void SonyRemoteStorage::SetRetrievedDescData()
{
	DescriptionData* pDescDataTest = (DescriptionData*)m_remoteFileInfo->fileDescription;
	ESavePlatform testPlatform = (ESavePlatform)MAKE_FOURCC(pDescDataTest->m_platform[0], pDescDataTest->m_platform[1], pDescDataTest->m_platform[2], pDescDataTest->m_platform[3]);
	if(testPlatform == SAVE_FILE_PLATFORM_NONE)
	{
		// new version of the descData
		DescriptionData_V2* pDescData2 = (DescriptionData_V2*)m_remoteFileInfo->fileDescription;
		m_retrievedDescData.m_descDataVersion = GetU32FromHexBytes(pDescData2->m_descDataVersion);
		m_retrievedDescData.m_savePlatform = (ESavePlatform)MAKE_FOURCC(pDescData2->m_platform[0], pDescData2->m_platform[1], pDescData2->m_platform[2], pDescData2->m_platform[3]);
		m_retrievedDescData.m_seed = GetU64FromHexBytes(pDescData2->m_seed);
		m_retrievedDescData.m_hostOptions = GetU32FromHexBytes(pDescData2->m_hostOptions);
		m_retrievedDescData.m_texturePack = GetU32FromHexBytes(pDescData2->m_texturePack);
		m_retrievedDescData.m_saveVersion = GetU32FromHexBytes(pDescData2->m_saveVersion);
		memcpy(m_retrievedDescData.m_saveNameUTF8, pDescData2->m_saveNameUTF8, sizeof(pDescData2->m_saveNameUTF8));
		assert(m_retrievedDescData.m_descDataVersion > 1 &&  m_retrievedDescData.m_descDataVersion <= sc_CurrentDescDataVersion);
	}
	else
	{
		// old version,copy the data across to the new version
		DescriptionData* pDescData = (DescriptionData*)m_remoteFileInfo->fileDescription;
		m_retrievedDescData.m_descDataVersion = 1;
		m_retrievedDescData.m_savePlatform = (ESavePlatform)MAKE_FOURCC(pDescData->m_platform[0], pDescData->m_platform[1], pDescData->m_platform[2], pDescData->m_platform[3]);
		m_retrievedDescData.m_seed = GetU64FromHexBytes(pDescData->m_seed);
		m_retrievedDescData.m_hostOptions = GetU32FromHexBytes(pDescData->m_hostOptions);
		m_retrievedDescData.m_texturePack = GetU32FromHexBytes(pDescData->m_texturePack);
		m_retrievedDescData.m_saveVersion = SAVE_FILE_VERSION_COMPRESSED_CHUNK_STORAGE;		// the last save version before we added it to this data
		memcpy(m_retrievedDescData.m_saveNameUTF8, pDescData->m_saveNameUTF8, sizeof(pDescData->m_saveNameUTF8));
	}

}




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
				pRemoteStorage->m_remoteFileInfo = &statParams.data[i];
				pRemoteStorage->SetRetrievedDescData();
				pRemoteStorage->m_getInfoStatus = SonyRemoteStorage::e_infoFound;
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
	m_dataProgress = -1;
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
	m_dataProgress = -1;
	m_uploadSaveSize = 0;
	m_startTime = System::currentTimeMillis();
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
	return m_retrievedDescData.m_saveNameUTF8;
}

ESavePlatform SonyRemoteStorage::getSavePlatform()
{
	if(m_getInfoStatus != e_infoFound)
		return SAVE_FILE_PLATFORM_NONE;
	return m_retrievedDescData.m_savePlatform;

}

int64_t SonyRemoteStorage::getSaveSeed()
{
	if(m_getInfoStatus != e_infoFound)
		return 0;

	return m_retrievedDescData.m_seed;
}

unsigned int SonyRemoteStorage::getSaveHostOptions()
{
	if(m_getInfoStatus != e_infoFound)
		return 0;
	return m_retrievedDescData.m_hostOptions;
}

unsigned int SonyRemoteStorage::getSaveTexturePack()
{
	if(m_getInfoStatus != e_infoFound)
		return 0;

	return m_retrievedDescData.m_texturePack;
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

bool SonyRemoteStorage::saveVersionSupported()
{
	return (m_retrievedDescData.m_saveVersion <= SAVE_FILE_VERSION_NUMBER);
}



int SonyRemoteStorage::getDataProgress()
{
	if(m_dataProgress < 0)
		return 0;
	int chunkSize = 1024*1024; // 1mb chunks when downloading
	int totalSize = getSaveFilesize();
	int transferRatePerSec = 300*1024; // a pessimistic download transfer rate
	if(getStatus() == e_setDataInProgress)
	{
		chunkSize = 5 * 1024 * 1024; // 5mb chunks when uploading
		totalSize = m_uploadSaveSize;
		transferRatePerSec = 20*1024; // a pessimistic upload transfer rate
	}
	int sizeTransferred = (totalSize * m_dataProgress) / 100;
	int nextChunk = ((sizeTransferred + chunkSize) * 100) / totalSize;


	int64_t time = System::currentTimeMillis();
	int elapsedSecs = (time - m_startTime) / 1000;
	float estimatedTransfered = float(elapsedSecs * transferRatePerSec);
	int progVal = m_dataProgress + (estimatedTransfered / float(totalSize)) * 100;
	if(progVal > nextChunk)
		return nextChunk;
	if(progVal > 99)
	{
		if(m_dataProgress > 99)
			return m_dataProgress;
		return 99;
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
void SonyRemoteStorage::GetDescriptionData(char* descData)
{
	switch(sc_CurrentDescDataVersion)
	{
	case 1:
	{
		DescriptionData descData_V1;
		GetDescriptionData(descData_V1);
		memcpy(descData, &descData_V1, sizeof(descData_V1));
	}
	break;
	case 2:
	{
		DescriptionData_V2 descData_V2;
		GetDescriptionData(descData_V2);
		memcpy(descData, &descData_V2, sizeof(descData_V2));
	}
	break;
	default:
		assert(0);
		break;
	}
}

void SonyRemoteStorage::GetDescriptionData( DescriptionData& descData)
{
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
		app.GetImageTextData(m_thumbnailData, m_thumbnailDataSize,(unsigned char *)seed, uiHostOptions, bHostOptionsRead, uiTexturePack);

		int64_t iSeed = strtoll(seed,NULL,10);
		SetU64HexBytes(descData.m_seed, iSeed);
		// Save the host options that this world was last played with
		SetU32HexBytes(descData.m_hostOptions, uiHostOptions);
		// Save the texture pack id
		SetU32HexBytes(descData.m_texturePack, uiTexturePack);
	}

	memcpy(descData.m_saveNameUTF8, m_saveFileDesc, strlen(m_saveFileDesc));

}

void SonyRemoteStorage::GetDescriptionData( DescriptionData_V2& descData)
{
	ZeroMemory(&descData, sizeof(DescriptionData_V2));
	descData.m_platformNone[0] = SAVE_FILE_PLATFORM_NONE & 0xff;
	descData.m_platformNone[1] = (SAVE_FILE_PLATFORM_NONE >> 8) & 0xff;
	descData.m_platformNone[2] = (SAVE_FILE_PLATFORM_NONE >> 16) & 0xff;
	descData.m_platformNone[3] = (SAVE_FILE_PLATFORM_NONE >> 24)& 0xff;

	// Save descData version
	char descDataVersion[9];
	sprintf(descDataVersion,"%08x",sc_CurrentDescDataVersion);
	memcpy(descData.m_descDataVersion,descDataVersion,8); // Don't copy null


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
		app.GetImageTextData(m_thumbnailData, m_thumbnailDataSize,(unsigned char *)seed, uiHostOptions, bHostOptionsRead, uiTexturePack);

		int64_t iSeed = strtoll(seed,NULL,10);
		SetU64HexBytes(descData.m_seed, iSeed);
		// Save the host options that this world was last played with
		SetU32HexBytes(descData.m_hostOptions, uiHostOptions);
		// Save the texture pack id
		SetU32HexBytes(descData.m_texturePack, uiTexturePack);
		// Save the savefile version
		SetU32HexBytes(descData.m_saveVersion, SAVE_FILE_VERSION_NUMBER);
		// clear out the future data with underscores
		memset(descData.m_futureData, '_', sizeof(descData.m_futureData));
	}

	memcpy(descData.m_saveNameUTF8, m_saveFileDesc, strlen(m_saveFileDesc));

}


uint32_t SonyRemoteStorage::GetU32FromHexBytes(char* hexBytes)
{
	char hexString[9];
	ZeroMemory(hexString,9);
	memcpy(hexString, hexBytes,8);

	uint32_t u32Val = 0;
	std::stringstream ss;
	ss << hexString;
	ss >> std::hex >> u32Val;
	return u32Val;
}

uint64_t SonyRemoteStorage::GetU64FromHexBytes(char* hexBytes)
{
	char hexString[17];
	ZeroMemory(hexString,17);
	memcpy(hexString, hexBytes,16);

	uint64_t u64Val = 0;
	std::stringstream ss;
	ss << hexString;
	ss >> std::hex >> u64Val;
	return u64Val;

}

void SonyRemoteStorage::SetU32HexBytes(char* hexBytes, uint32_t u32)
{
	char hexString[9];
	sprintf(hexString,"%08x",u32);
	memcpy(hexBytes,hexString,8); // Don't copy null
}

void SonyRemoteStorage::SetU64HexBytes(char* hexBytes, uint64_t u64)
{
	char hexString[17];
	sprintf(hexString,"%016llx",u64);
	memcpy(hexBytes,hexString,16); // Don't copy null
}
