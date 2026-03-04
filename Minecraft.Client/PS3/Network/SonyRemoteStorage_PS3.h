#pragma once 


#include "Common\Network\Sony\sceRemoteStorage\header\sceRemoteStorage.h"

class SonyRemoteStorage_PS3 : public SonyRemoteStorage
{
public:


	virtual bool init(CallbackFunc cb, LPVOID lpParam);

	virtual bool getRemoteFileInfo(SceRemoteStorageStatus* pInfo, CallbackFunc cb, LPVOID lpParam);
	virtual bool getData(const char* remotePath, const char* localPath, CallbackFunc cb, LPVOID lpParam);

	virtual void abort();
	virtual bool setDataInternal();

	bool isWaitingForTicket() { return m_waitingForTicket; }
	void npauthhandler(int event, int result, void *arg);

	void SetSecureID(char* id) { memcpy(m_secureFileId, id, CELL_SAVEDATA_SECUREFILEID_SIZE); }

	void CompressSaveData();
	bool dataCompressDone() { return (m_compressedSaveState == e_state_Idle);}
private:
	int reqId;
	void * psnTicket;
	size_t psnTicketSize;
	bool m_waitingForTicket;
	bool initialized;
	SceRemoteStorageStatus* outputGetStatus;
	SceRemoteStorageData outputGetData;

	enum CompressSaveState
	{
		e_state_LoadingSave, 
		e_state_CompressedSave, 
		e_state_SavingSave,
		e_state_Idle
	};

	CompressSaveState m_compressedSaveState;
	char m_secureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE];

	int32_t m_lastErrorCode;
	int m_getDataProgress;
	int m_setDataProgress;
	char m_saveFilename[SCE_REMOTE_STORAGE_DATA_NAME_MAX_LEN];
	char m_saveFileDesc[SCE_REMOTE_STORAGE_DATA_DESCRIPTION_MAX_LEN];
	char m_remoteFilename[SCE_REMOTE_STORAGE_DATA_NAME_MAX_LEN];
	int initPreconditions();
	static void staticInternalCallback(const SceRemoteStorageEvent event, int32_t retCode, void * userData);
	void internalCallback(const SceRemoteStorageEvent event, int32_t retCode);

	virtual void runCallback();


	static int SaveCompressCallback(LPVOID lpParam,bool bRes);
	static int LoadCompressCallback(void *pParam,bool bIsCorrupt, bool bIsOwner);


};

