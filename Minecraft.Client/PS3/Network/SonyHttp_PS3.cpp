#include "stdafx.h"
#include "SonyHttp_PS3.h"



// static const int sc_HTTPPoolSize = (32 * 1024);
// static const int sc_SSLPoolSize   = (256 * 1024U);
// static const int sc_CookiePoolSize   = (256 * 1024U);
 static const int sc_HTTPPoolSize = (32 * 1024);
 static const int sc_SSLPoolSize   = (512 * 1024U);
 static const int sc_CookiePoolSize   = (256 * 1024U);

void* SonyHttp_PS3::uriPool = nullptr;
void* SonyHttp_PS3::httpPool = nullptr;
void* SonyHttp_PS3::sslPool = nullptr;
void* SonyHttp_PS3::cookiePool = nullptr;
CellHttpClientId SonyHttp_PS3::client;
CellHttpTransId SonyHttp_PS3::trans;
bool SonyHttp_PS3:: bInitialised = false;



bool SonyHttp_PS3::loadCerts(size_t *numBufPtr, CellHttpsData **caListPtr)
{
	CellHttpsData *caList = nullptr;
	size_t size = 0;
	int ret = 0;
	char *buf = nullptr;

	caList = static_cast<CellHttpsData *>(malloc(sizeof(CellHttpsData) * 2));
	if (nullptr == caList) {
		app.DebugPrintf("failed to malloc cert data");
		return false;
	}

	ret = cellSslCertificateLoader(CELL_SSL_LOAD_CERT_ALL, nullptr, 0, &size);
	if (ret < 0) {
		app.DebugPrintf("cellSslCertifacateLoader() failed(1): 0x%08x", ret);
		return ret;
	}

	buf = static_cast<char *>(malloc(size));
	if (nullptr == buf) {
		app.DebugPrintf("failed to malloc cert buffer");
		free(caList);
		return false;
	}

	ret = cellSslCertificateLoader(CELL_SSL_LOAD_CERT_ALL, buf, size, nullptr);
	if (ret < 0) {
		app.DebugPrintf("cellSslCertifacateLoader() failed(2): 0x%08x", ret);
		free(buf);
		free(caList);
		return false;
	}

	(&caList[0])->ptr = buf;
	(&caList[0])->size = size;

	*caListPtr = caList;
	*numBufPtr = 1;

	return true;
}


bool SonyHttp_PS3::init()
{
	int ret;
	client = 0;
	trans = 0;

	/*E startup procedures */
	httpPool = malloc(sc_HTTPPoolSize);
	if (httpPool == nullptr) {
		app.DebugPrintf("failed to malloc libhttp memory pool\n");
		return false;
	}
	ret = cellHttpInit(httpPool, sc_HTTPPoolSize);
	if (0 > ret) 
	{
		app.DebugPrintf("unable to start libhttp... (0x%x)\n", ret);
		return false;
	}

	cookiePool = malloc(sc_CookiePoolSize);
	if (cookiePool == nullptr) {
		app.DebugPrintf("failed to malloc ssl memory pool\n");
		return false;
	}
	ret = cellHttpInitCookie(cookiePool, sc_CookiePoolSize);
	if (ret < 0 && ret != CELL_HTTP_ERROR_ALREADY_INITIALIZED) 
	{
		app.DebugPrintf("cellHttpInitCookie failed... (0x%x)\n", ret);
		return false;
	}


	sslPool = malloc(sc_SSLPoolSize);
	if (sslPool == nullptr) {
		app.DebugPrintf("failed to malloc ssl memory pool\n");
		return false;
	}
	ret = cellSslInit(sslPool, sc_SSLPoolSize);
	if (0 > ret) 
	{
		app.DebugPrintf("unable to start cellSslInit... (0x%x)\n", ret);
		return false;
	}

	size_t numBuf = 0;
	CellHttpsData *caList = nullptr;
	if(!loadCerts(&numBuf, &caList))
		return false;

	ret = cellHttpsInit(numBuf, caList);
	free(caList->ptr);
	free(caList);
	if (ret < 0 && ret != CELL_HTTP_ERROR_ALREADY_INITIALIZED) 
	{
		app.DebugPrintf("unable to start https: 0x%08x", ret);
		return false;
	}

	ret = cellHttpCreateClient(&client);
	if (0 > ret) 
	{
		app.DebugPrintf("unable to create http client... (0x%x)\n", ret);
		return false;
	}
	bInitialised = true;
	return true;
}

void SonyHttp_PS3::shutdown()
{
	if (trans) 
	{
		cellHttpDestroyTransaction(trans);
		trans = 0;
	}
	if (client) 
	{
		cellHttpDestroyClient(client);
		client = 0;
	}
	cellHttpEnd();
	free(httpPool);
}


bool SonyHttp_PS3::parseUri(const char* szUri, CellHttpUri& parsedUri)
{
	/*E the main part */
	size_t poolSize = 0;
	int ret = cellHttpUtilParseUri(nullptr, szUri, nullptr, 0, &poolSize);
	if (0 > ret) 
	{
		app.DebugPrintf("error parsing URI... (0x%x)\n\n", ret);
		return false;
	}
	if (nullptr == (uriPool = malloc(poolSize))) 
	{
		app.DebugPrintf("error mallocing uriPool (%d)\n", poolSize);
		return false;
	}
	ret = cellHttpUtilParseUri(&parsedUri, szUri, uriPool, poolSize, nullptr);
	if (0 > ret) 
	{
		free(uriPool);
		app.DebugPrintf("error parsing URI... (0x%x)\n\n", ret);
		return false;
	}
	return true;
}

void* SonyHttp_PS3::getData(const char* url, int* pDataSize)
{
	CellHttpUri uri;
	int ret;
	bool has_cl = true;
	uint64_t length = 0;
	uint64_t recvd;
	const char *serverName;
	size_t localRecv = 0;

	if(!parseUri(url, uri))
		return nullptr;

	app.DebugPrintf("  scheme:   %s\n", uri.scheme);
	app.DebugPrintf("  hostname: %s\n", uri.hostname);
	app.DebugPrintf("  port:     %d\n", uri.port);
	app.DebugPrintf("  path:     %s\n", uri.path);
	app.DebugPrintf("  username: %s\n", uri.username);
	app.DebugPrintf("  password: %s\n", uri.password);

	ret = cellHttpCreateTransaction(&trans, client, CELL_HTTP_METHOD_GET, &uri);
	free(uriPool);
	if (0 > ret) 
	{
		app.DebugPrintf("failed to create http transaction... (0x%x)\n\n", ret);
		return nullptr;
	}

	ret = cellHttpSendRequest(trans, nullptr, 0, nullptr);
	if (0 > ret) 
	{
		app.DebugPrintf("failed to complete http transaction... (0x%x)\n\n", ret);
		/*E continue down to check status code, just in case */
		cellHttpDestroyTransaction(trans);
		trans = 0;
	}

	{
		int code = 0;
		ret = cellHttpResponseGetStatusCode(trans, &code);
		if (0 > ret)
		{
			app.DebugPrintf("failed to receive http response... (0x%x)\n\n", ret);
			cellHttpDestroyTransaction(trans);
			trans = 0;
			return nullptr;
		}
		app.DebugPrintf("Status Code is %d\n", code);
	}

	ret = cellHttpResponseGetContentLength(trans, &length);
	if (0 > ret) 
	{
		if (ret == CELL_HTTP_ERROR_NO_CONTENT_LENGTH) 
		{
			app.DebugPrintf("Only supporting data that has a content length : CELL_HTTP_ERROR_NO_CONTENT_LENGTH\n", ret);
			cellHttpDestroyTransaction(trans);
			trans = 0;
			return nullptr;
		} 
		else 
		{
			app.DebugPrintf("error in receiving content length... (0x%x)\n\n", ret);
			cellHttpDestroyTransaction(trans);
			trans = 0;
			return nullptr;
		}
	}

	int bufferSize = length;
	char* buffer = static_cast<char *>(malloc(bufferSize + 1));

	recvd = 0;

	// 4J-PB - seems to be some limit to a read with cellHttpRecvResponse, even though it claims it's read all the data, it hasn't
	// reducing this to read 10k at a time.
	int iMaxRead=10240;
	app.DebugPrintf("\n===BEGINNING OF TRANSMISSION===\n");
	while(recvd<length)
	{	
		ret = cellHttpRecvResponse(trans, &buffer[recvd], iMaxRead, &localRecv);
		if (0 > ret) 
		{
			app.DebugPrintf("error receiving body... (0x%x)\n\n", ret);
			free(buffer);
			cellHttpDestroyTransaction(trans);
			trans = 0;
			return nullptr;
		} 
		else
		{
			if(localRecv==0) break;
		}
		recvd += localRecv;
	}
	buffer[bufferSize] = '\0';
	ret = 0;
	app.DebugPrintf("\n===END OF TRANSMISSION===\n\n");
	*pDataSize = bufferSize;
	cellHttpDestroyTransaction(trans);
	trans = 0;
	return buffer;
}

bool SonyHttp_PS3::getDataFromURL( const char* szURL, void** ppOutData, int* pDataSize)
{
	if(!bInitialised)
		return false;
	*ppOutData = getData(szURL, pDataSize);
	if(*ppOutData)
		return true;
	return false;
}
