#include "stdafx.h"
#include "SonyHttp.h"


#ifdef __PS3__
#include "PS3/Network/SonyHttp_PS3.h"
SonyHttp_PS3 g_SonyHttp;

#elif defined __ORBIS__
#include "Orbis/Network/SonyHttp_Orbis.h"
SonyHttp_Orbis g_SonyHttp;

#elif defined __PSVITA__
#include "PSVita/Network/SonyHttp_Vita.h"
SonyHttp_Vita g_SonyHttp;

#endif



bool SonyHttp::init()
{
	return g_SonyHttp.init();
}

void SonyHttp::shutdown()
{
	g_SonyHttp.shutdown();
}

bool SonyHttp::getDataFromURL(const char* szURL, void** ppOutData, int* pDataSize)
{
	return g_SonyHttp.getDataFromURL(szURL, ppOutData, pDataSize);
}
