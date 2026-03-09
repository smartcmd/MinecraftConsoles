#include "stdafx.h"


#include <stdlib.h>
#include <string.h>
#include <cell/l10n.h>
#include <cell/pad.h>
#include <cell/cell_fs.h>
#include <sys/process.h>
#include <sys/ppu_thread.h>
#include <cell/sysmodule.h>
#include <cell/rtc.h>

#include <libsn.h>

#include <sysutil/sysutil_common.h>
#include <sysutil/sysutil_sysparam.h>
#include <sysutil/sysutil_savedata.h>

#include "STO_TitleSmallStorage.h"
#include "../Passphrase/ps3__np_conf.h"



#if 0

int32_t	CTSS::TssSizeCheck(size_t	size)
{
	printf("=======Tss Data size = %d\n", size);

	//E Recommendation: The data's size should be checked against for the server operation mistake.
	//E If it is necessary, implement size check based on your data format, please.

	if (size == 0)
	{
		printf("Tss File size is 0 byte. Maybe, data isn't set on the server.\n\n");
		printf("\tThe data is set with scp. The account is generable in PS3 Developer Network.\n");
		printf("\tThere are three server environments. for development, environment for test, and product.\n");
	}
	return 0;
}

int32_t	CTSS::TssHashCheck(void *data, size_t size)
{
	//E Recommendation: The data format should be include it's hash against for operation mistakes and crack.
	//E If it is necessary, implement hash check based on your data format, please.
	return 0;
}

int32_t	CTSS::TssVersionCheck(void *data, size_t size)
{
	//E Recommendation: The data format should be include it's version for changes of the format.
	//E If it is necessary, implement version check based on your data format, please.
	return 0;
}

int32_t CTSS::doLookupTitleSmallStorage(void)
{
	int32_t ret = 0;
	int32_t transId = 0;
	void *data=nullptr;
	size_t dataSize=0;

	SceNpId npId;
	ret = sceNpManagerGetNpId(&npId);
	if(ret < 0){
		return ret;
	}
	
	ret = sceNpLookupInit();
	if (ret < 0) 
	{
		printf("sceNpLookupInit() failed. ret = 0x%x\n", ret);
		goto error;
	}
	ret = sceNpTusInit(0);
	if (ret < 0) 
	{
		printf("sceNpTusInit() failed. ret = 0x%x\n", ret);
		goto error;
	}

	ret = sceNpLookupCreateTitleCtx(&s_npCommunicationId, &npId);
	if (ret < 0) 
	{
		printf("sceNpLookupCreateTitleCtx() failed. ret = 0x%x\n", ret);
		goto error;
	}
	m_lookupTitleCtxId = ret;

	ret = sceNpTusCreateTitleCtx(&s_npCommunicationId, &s_npCommunicationPassphrase, &npId);
	if (ret < 0) 
	{
		printf("sceNpTusCreateTitleCtx() failed. ret = 0x%x\n", ret);
		goto error;
	}
	printf("sceNpTusCreateTitleCtx() return %x\n", ret);
	m_tusTitleCtxId = ret;

	//memset(&npclient_info, 0x00, sizeof(npclient_info));

	data = malloc(SCE_NET_NP_TSS_MAX_SIZE);
	if (data == nullptr)
	{
		printf("out of memory: can't allocate memory for titleSmallStorage\n");
		ret = -1;
		goto error;
	}
	memset(data, 0x00, SCE_NET_NP_TSS_MAX_SIZE);

	ret = sceNpLookupCreateTransactionCtx(m_lookupTitleCtxId);
	if (ret < 0) 
	{
		printf("sceNpLookupCreateTransactionCtx() failed. ret = 0x%x\n", ret);
		goto error;
	}
	transId = ret;


// 	ret = sceNpLookupTitleSmallStorage(transId, 
// 		data,
// 		SCE_NET_NP_TSS_MAX_SIZE,
// 		&dataSize,
// 		nullptr);
// 	if (ret < 0)
// 	{
// 		printf("sceNpLookupTitleSmallStorage() failed. ret = 0x%x\n", ret);
// 		goto error;
// 	}
// 
// 	ret = TssSizeCheck(dataSize);
// 	if (ret < 0)
// 	{
// 		goto error;
// 	}
// 	ret = TssHashCheck(data, dataSize);
// 	if (ret < 0)
// 	{
// 		goto error;
// 	}
// 	ret = TssVersionCheck(data, dataSize);
// 	if (ret < 0)
// 	{
// 		goto error;
// 	}

/*
	// Process communication
	void *data;
	size_t dataSize;
	SceNpTssSlotId slotId=SLOTID;
	SceNpTssDataStatus dataStatus;
	const char *ptr =nullptr;
	size_t recvdSize=0;
	size_t totalSize=0;
	size_t recvSize=0;

	do {
		ret = sceNpTssGetData(
			transId,
			slotId,
			&dataStatus,
			sizeof(SceNpTssDataStatus),
			ptr,
			recvSize,
			nullptr);
		if (ret < 0) 
		{
			// Error handling
			goto error;
		}
		if (dataStatus.contentLength == 0)
		{
			// Processing when there is no data
			goto finish;
		}
		if (ptr == nullptr)
		{
			ptr = malloc(dataStatus.contentLength);
			if (ptr == nullptr){
				// Error handling
				goto error;
			}
			recvSize = BLOCKSIZE;
		}
		recvedSize += ret;
		ptr += ret;
	} while (recvedSize < dataStatus.contentLength);

*/



error:
	if (transId > 0) 
	{
		ret = sceNpLookupDestroyTransactionCtx(transId);
		printf("sceNpLookupDestroyTransactionCtx() done. ret = 0x%x\n", ret);
	}
	if (data != nullptr)
	{
		free(data);
	}

	sceNpTusTerm();

	return 0;
}

#endif