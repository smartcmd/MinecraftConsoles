#include "stdafx.h"
#include "SonyHttp_Orbis.h"

static const int sc_SSLHeapSize   = (304 * 1024U);
static const int sc_HTTPHeapSize = (48 * 1024);
static const int sc_NetHeapSize = (16 * 1024);

#define TEST_USER_AGENT	"SimpleSample/1.00"


int SonyHttp_Orbis::libnetMemId = 0;
int SonyHttp_Orbis::libsslCtxId = 0;
int SonyHttp_Orbis::libhttpCtxId = 0;
bool SonyHttp_Orbis:: bInitialised = false;





bool SonyHttp_Orbis::init()
{
	int ret = sceNetPoolCreate("simple", sc_NetHeapSize, 0);
	assert(ret >= 0);
	libnetMemId = ret;

	ret = sceSslInit(sc_SSLHeapSize);
	assert(ret >= 0);
	libsslCtxId = ret;

	ret = sceHttpInit(libnetMemId, libsslCtxId, sc_HTTPHeapSize);
	assert(ret >= 0);
	libhttpCtxId = ret;

	bInitialised = true;
	return true;
}

void SonyHttp_Orbis::shutdown()
{
	int ret = sceHttpTerm(libhttpCtxId);
	assert(ret == SCE_OK);

	ret = sceSslTerm(libsslCtxId);
	assert(ret == SCE_OK);

	/* libnet */
	ret = sceNetPoolDestroy(libnetMemId);
	assert(ret == SCE_OK);

}
void SonyHttp_Orbis::printSslError(SceInt32 sslErr, SceUInt32 sslErrDetail)
{
	switch (sslErr)
	{
	case (SCE_HTTPS_ERROR_CERT):			/* Verify error */
		/* Internal error at verifying certificate*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_INTERNAL){
			app.DebugPrintf("ssl verify error: unexpcted error\n");
		}
		/* Error of server certificate or CA certificate */
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_INVALID_CERT){
			app.DebugPrintf("ssl verify error: invalid server cert or CA cert\n");
		}
		/* Server hostname and server certificate are mismatched*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_CN_CHECK){
			app.DebugPrintf("ssl verify error: invalid server hostname\n");
		}
		/* Server certificate or CA certificate is expired.*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_NOT_AFTER_CHECK){
			app.DebugPrintf("ssl verify error: server cert or CA cert had expired\n");
		}
		/* Server certificate or CA certificate is before validated.*/
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_NOT_BEFORE_CHECK){
			app.DebugPrintf("ssl verify error: server cert or CA cert isn't validated yet.\n");
		}
		/* Unknown CA error */
		if (sslErrDetail & SCE_HTTPS_ERROR_SSL_UNKNOWN_CA){
			app.DebugPrintf("ssl verify error: unknown CA\n");
		}
		break;
	case (SCE_HTTPS_ERROR_HANDSHAKE):		/* fail to ssl-handshake  */
		app.DebugPrintf("ssl error: handshake error\n");
		break;
	case (SCE_HTTPS_ERROR_IO):				/* Error of Socket IO */
		app.DebugPrintf("ssl error: io error\n");
		break;
	case (SCE_HTTP_ERROR_OUT_OF_MEMORY):	/* Out of memory*/
		app.DebugPrintf("ssl error: out of memory\n");
		break;
	case (SCE_HTTPS_ERROR_INTERNAL):		/* Unexpected Internal Error*/
		app.DebugPrintf("ssl error: unexpcted error\n");
		break;
	default:
		break;
	}
	return;
}


void SonyHttp_Orbis::printSslCertInfo(int libsslCtxId,SceSslCert *sslCert)
{
	SceInt32 ret;
	SceUChar8 *sboData = nullptr ;
	SceSize sboLen, counter;

	ret = sceSslGetSerialNumber(libsslCtxId, sslCert, nullptr, &sboLen);
	if (ret < 0){
		app.DebugPrintf("sceSslGetSerialNumber() returns 0x%x\n", ret);
	}
	else {
		sboData = static_cast<SceUChar8 *>(malloc(sboLen));
		if ( sboData != nullptr ) {
			ret = sceSslGetSerialNumber(libsslCtxId, sslCert, sboData, &sboLen);
			if (ret < 0){
				app.DebugPrintf ("sceSslGetSerialNumber() returns 0x%x\n", ret);
			}
			else {
				app.DebugPrintf("Serial number=");
				for (counter = 0; counter < sboLen; counter++){
					app.DebugPrintf("%02X", sboData[counter]);
				}
				app.DebugPrintf("\n");
			}
			free(sboData);
		}
	}
}


bool SonyHttp_Orbis::getDataFromURL( const char* szURL, void** ppOutData, int* pDataSize)
{
	if(!bInitialised)
		return false;
	return http_get(szURL, ppOutData, pDataSize);
}


SceInt32 SonyHttp_Orbis::sslCallback(int libsslCtxId,unsigned int verifyErr,SceSslCert * const sslCert[],int certNum,void *userArg)
{
	SceInt32 i;
	(void)userArg;

	app.DebugPrintf("Ssl callback:\n");
	app.DebugPrintf("\tbase tmpl[%x]\n", (*static_cast<SceInt32 *>(userArg)) );

	if (verifyErr != 0){
		printSslError(static_cast<SceInt32>(SCE_HTTPS_ERROR_CERT), verifyErr);
	}
	for (i = 0; i < certNum; i++){
		printSslCertInfo(libsslCtxId,sslCert[i]);
	}
	if (verifyErr == 0){
		return SCE_OK;
	} else {
		return -1;
	}
}

bool SonyHttp_Orbis::http_get_close(bool bOK, SceInt32 tmplId, SceInt32 connId, SceInt32 reqId)
{
	SceInt32 ret;
	if (reqId > 0)
	{
		ret = sceHttpDeleteRequest(reqId);
		assert(ret >= 0);
	}
	if (connId > 0)
	{
		ret = sceHttpDeleteConnection(connId);
		assert(ret >= 0);
	}
	if (tmplId > 0)
	{
		ret = sceHttpDeleteTemplate(tmplId);
		assert(ret >= 0);
	}
	assert(bOK);
	return bOK;
}

bool SonyHttp_Orbis::http_get(const char *targetUrl, void** ppOutData, int* pDataSize)
{
	SceInt32 ret, tmplId=0, connId=0, reqId=0, statusCode;
	SceULong64	contentLength=0;
	SceBool		finFlag=SCE_FALSE;
	SceUChar8* recvBuf;

	ret = sceHttpCreateTemplate(libhttpCtxId, TEST_USER_AGENT, SCE_HTTP_VERSION_1_1, SCE_TRUE);
	if (ret < 0)
	{
		app.DebugPrintf("sceHttpCreateTemplate() error: 0x%08X\n", ret);
		return http_get_close(false, tmplId, connId, reqId);
	}
	tmplId = ret;

	/* Perform http_get without server verification */
	ret = sceHttpsDisableOption(tmplId,SCE_HTTPS_FLAG_SERVER_VERIFY);
	if (ret < 0)
	{
		app.DebugPrintf("sceHttpsDisableOption() error: 0x%08X\n", ret);
		return http_get_close(false, tmplId, connId, reqId);
	}

	/* Register SSL callback */
	ret = sceHttpsSetSslCallback(tmplId, sslCallback, static_cast<void *>(&tmplId));
	if (ret < 0) 
	{
		app.DebugPrintf("sceHttpsSetSslCallback() error: 0x%08X\n", ret);
		return http_get_close(false, tmplId, connId, reqId);
	}

	ret = sceHttpCreateConnectionWithURL(tmplId, targetUrl, SCE_TRUE);
	if (ret < 0)
	{
		app.DebugPrintf("sceHttpCreateConnectionWithURL() error: 0x%08X\n", ret);
		return http_get_close(false, tmplId, connId, reqId);
	}
	connId = ret;

	ret = sceHttpCreateRequestWithURL(connId, SCE_HTTP_METHOD_GET, targetUrl, 0);
	if (ret < 0)
	{
		app.DebugPrintf("sceHttpCreateRequestWithURL() error: 0x%08X\n", ret);
		return http_get_close(false, tmplId, connId, reqId);
	}
	reqId = ret;

	ret = sceHttpSendRequest(reqId, nullptr, 0);
	if (ret < 0)
	{
		app.DebugPrintf("sceHttpSendRequest() error: 0x%08X\n", ret);
		return http_get_close(false, tmplId, connId, reqId);
	}

	ret = sceHttpGetStatusCode(reqId, &statusCode);
	if (ret < 0)
	{
		app.DebugPrintf("sceHttpGetStatusCode() error: 0x%08X\n", ret);
		return http_get_close(false, tmplId, connId, reqId);
	}
	app.DebugPrintf("response code = %d\n", statusCode);

	if(statusCode == 200)
	{
		int contentLengthType;
		ret = sceHttpGetResponseContentLength(reqId, &contentLengthType, &contentLength);
		if(ret < 0)
		{
			app.DebugPrintf("sceHttpGetContentLength() error: 0x%08X\n", ret);
			return http_get_close(false, tmplId, connId, reqId);
		}
		else
		{
			if (contentLengthType == SCE_HTTP_CONTENTLEN_EXIST)
			{
				app.DebugPrintf("Content-Length = %lu\n", contentLength);
			}
		}
		recvBuf = new SceUChar8[contentLength+1];
		int bufferLeft = contentLength+1;
		SceUChar8* pCurrBuffPos = recvBuf;
		int totalBytesRead = 0;
		while(finFlag != SCE_TRUE)
		{
			ret = sceHttpReadData(reqId, pCurrBuffPos, bufferLeft);
			if (ret < 0)
			{
				app.DebugPrintf("\n sceHttpReadData() failed 0x%08X\n", ret);
				return http_get_close(false, tmplId, connId, reqId);
			} 
			else if (ret == 0)
			{
				finFlag = SCE_TRUE;
			}
			app.DebugPrintf("\n sceHttpReadData() read %d bytes\n", ret);
			pCurrBuffPos += ret;
			totalBytesRead += ret;
			bufferLeft -= ret;
		}
	}

	*ppOutData = recvBuf;
	*pDataSize = contentLength;
	return http_get_close(true, tmplId, connId, reqId);
}
