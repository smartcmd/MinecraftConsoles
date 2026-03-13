#pragma once


#include "../../Common/Network/Sony/sceRemoteStorage/header/sceRemoteStorage.h"

class SonyRemoteStorage
{
public:
	enum Status
	{
		e_idle,
		e_accountLinked,
		e_error,
		e_signInRequired,
		e_compressInProgress,
		e_setDataInProgress,
		e_setDataSucceeded,
		e_getDataInProgress,
		e_getDataSucceeded,
		e_getStatusInProgress,
		e_getStatusSucceeded
	};
	typedef void (*CallbackFunc)(LPVOID lpParam, Status s, int error_code);

	enum GetInfoStatus
	{
		e_gettingInfo,
		e_infoFound,
		e_noInfoFound
	};
	GetInfoStatus m_getInfoStatus;

	enum SetDataStatus
	{
		e_settingData,
		e_settingDataFailed,
		e_settingDataSucceeded
	};
	SetDataStatus m_setDataStatus;

	PSAVE_INFO m_setSaveDataInfo;
	SceRemoteStorageData* m_remoteFileInfo;
	char m_saveFileDesc[128];

	class DescriptionData
	{
		// this stuff is read from a JSON query, so it all has to be text based, max 256 bytes
	public:
		char			m_platform[4];
		char			m_seed[16]; // 8 bytes as hex
		char			m_hostOptions[8]; // 4 bytes as hex
		char			m_texturePack[8]; // 4 bytes as hex
		char			m_saveNameUTF8[128];
	};

	class DescriptionData_V2
	{
		// this stuff is read from a JSON query, so it all has to be text based, max 256 bytes
	public:
		char			m_platformNone[4];  // set to no platform, to indicate we're using the newer version of the data
		char			m_descDataVersion[8]; // 4 bytes as hex	- version number will be 2 in this case
		char			m_platform[4];
		char			m_seed[16]; // 8 bytes as hex
		char			m_hostOptions[8]; // 4 bytes as hex
		char			m_texturePack[8]; // 4 bytes as hex
		char			m_saveVersion[8]; // 4 bytes as hex
		char			m_futureData[64]; // some space for future data in case we need to expand this at all
		char			m_saveNameUTF8[128];
	};

	class DescriptionDataParsed
	{
	public:
		int				m_descDataVersion;
		ESavePlatform	m_savePlatform;
		int64_t			m_seed;
		uint32_t		m_hostOptions;
		uint32_t		m_texturePack;
		uint32_t		m_saveVersion;
		char			m_saveNameUTF8[128];
	};

	static const int sc_CurrentDescDataVersion = 2;

	void GetDescriptionData(char* descData);
	void GetDescriptionData(DescriptionData& descData);
	void GetDescriptionData(DescriptionData_V2& descData);
	uint32_t GetU32FromHexBytes(char* hexBytes);
	uint64_t GetU64FromHexBytes(char* hexBytes);

	void SetU32HexBytes(char* hexBytes, uint32_t u32);
	void SetU64HexBytes(char* hexBytes, uint64_t u64);

	DescriptionDataParsed m_retrievedDescData;
	void SetRetrievedDescData();

	CallbackFunc	m_callbackFunc;
	void*			m_callbackParam;


	CallbackFunc	m_initCallbackFunc;
	void*			m_initCallbackParam;

	void getSaveInfo();
	bool waitingForSaveInfo() { return (m_getInfoStatus == e_gettingInfo); }
	bool saveIsAvailable();
	bool saveVersionSupported();

	int getSaveFilesize();
	bool getSaveData(const char* localDirname, CallbackFunc cb, LPVOID lpParam);

	bool setSaveData(PSAVE_INFO info, CallbackFunc cb, void* lpParam);
	bool waitingForSetData() { return (m_setDataStatus == e_settingData); }

	const char* getLocalFilename();
	const char* getSaveNameUTF8();
	ESavePlatform getSavePlatform();
	int64_t getSaveSeed();
	unsigned int getSaveHostOptions();
	unsigned int getSaveTexturePack();

	void SetServiceID(char *pchServiceID) { m_pchServiceID=pchServiceID; }

	virtual bool init(CallbackFunc cb, LPVOID lpParam) = 0;
	virtual bool getRemoteFileInfo(SceRemoteStorageStatus* pInfo, CallbackFunc cb, LPVOID lpParam) = 0;
	virtual bool getData(const char* remotePath, const char* localPath, CallbackFunc cb, LPVOID lpParam) = 0;
	virtual void abort() = 0;
	virtual bool shutdown();
	virtual bool setDataInternal() = 0;
	virtual void runCallback() = 0;


	Status getStatus() { return m_status; }
	int getDataProgress();
	void waitForStorageManagerIdle();



	bool setData( PSAVE_INFO info, CallbackFunc cb, LPVOID lpParam );
	static int LoadSaveDataThumbnailReturned(LPVOID lpParam,PBYTE pbThumbnail,DWORD dwThumbnailBytes);
	static int setDataThread(void* lpParam);

	SonyRemoteStorage() : m_getInfoStatus(e_noInfoFound), m_bInitialised(false),m_memPoolBuffer(nullptr) {}

protected:
	const char* getRemoteSaveFilename();
	bool m_bInitialised;
	void* m_memPoolBuffer;
	Status m_status;
	int m_dataProgress;
	char *m_pchServiceID;

	PBYTE m_thumbnailData;
	unsigned int m_thumbnailDataSize;
	C4JThread* m_SetDataThread;
	PSAVE_INFO m_setDataSaveInfo;
	int64_t m_startTime;

	bool m_bAborting;
	bool m_bTransferStarted;
	int m_uploadSaveSize;
};

