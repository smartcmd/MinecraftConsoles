#pragma once 


#include "Common/Network/Sony/SonyRemoteStorage.h"

class SonyRemoteStorage_Orbis : public SonyRemoteStorage
{
public:


	virtual bool init(CallbackFunc cb, LPVOID lpParam);
	virtual bool setData(PSAVE_INFO info, CallbackFunc cb, LPVOID lpParam);

	virtual bool getRemoteFileInfo(SceRemoteStorageStatus* pInfo, CallbackFunc cb, LPVOID lpParam);
	virtual bool getData(const char* remotePath, const char* localPath, CallbackFunc cb, LPVOID lpParam);

	virtual void abort();
	virtual bool setDataInternal(){ assert(0); }

private:
	int reqId;
	void * psnTicket;
	size_t psnTicketSize;
	bool m_waitingForTicket;
	bool initialized;
	SceRemoteStorageStatus* outputGetStatus;
	SceRemoteStorageData outputGetData;

	int32_t m_lastErrorCode;
	int m_getDataProgress;
	int m_setDataProgress;
	char m_saveFilename[SCE_REMOTE_STORAGE_DATA_NAME_MAX_LEN];
	char m_saveFileDesc[SCE_REMOTE_STORAGE_DATA_DESCRIPTION_MAX_LEN];
	char m_remoteFilename[SCE_REMOTE_STORAGE_DATA_NAME_MAX_LEN];
	char m_mountPoint[SCE_SAVE_DATA_MOUNT_POINT_DATA_MAXSIZE];

	static void staticInternalCallback(const SceRemoteStorageEvent event, int32_t retCode, void * userData);
	void internalCallback(const SceRemoteStorageEvent event, int32_t retCode);

	void runCallback();
};

