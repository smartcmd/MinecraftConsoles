#include "stdafx.h"
#include "TexturePack.h"

wstring TexturePack::getPath(bool bTitleUpdateTexture /*= false*/,const char *pchBDPatchFileName /*= NULL*/)
{
	wstring wDrive;
#ifdef _XBOX
	if(bTitleUpdateTexture)
	{
		// Make the content package point to to the UPDATE: drive is needed
#ifdef _TU_BUILD
		wDrive=L"UPDATE:\\";
#else

		wDrive=L"GAME:\\res\\TitleUpdate\\";
#endif
	}
	else
	{
		wDrive=L"GAME:\\";
	}
#else

#ifdef __PS3__

	// 4J-PB - we need to check for a BD patch - this is going to be an issue for full DLC texture packs (Halloween)
	char *pchUsrDir=NULL;
	if(app.GetBootedFromDiscPatch() && pchBDPatchFileName!=NULL)
	{
		pchUsrDir = app.GetBDUsrDirPath(pchBDPatchFileName);
		wstring wstr (pchUsrDir, pchUsrDir+strlen(pchUsrDir));

		if(bTitleUpdateTexture)
		{
			wDrive= wstr + L"\\Common\\res\\TitleUpdate\\";

		}
		else
		{
			wDrive= wstr + L"/Common/";
		}
	}
	else
	{
		pchUsrDir=getUsrDirPath();
	
		wstring wstr (pchUsrDir, pchUsrDir+strlen(pchUsrDir));

		if(bTitleUpdateTexture)
		{
			// Make the content package point to to the UPDATE: drive is needed
			wDrive= wstr + L"\\Common\\res\\TitleUpdate\\";
		}
		else
		{
			wDrive= wstr + L"/Common/";
		}			
	}

#elif __PSVITA__
	char *pchUsrDir="";//getUsrDirPath();
	wstring wstr (pchUsrDir, pchUsrDir+strlen(pchUsrDir));

	if(bTitleUpdateTexture)
	{
		// Make the content package point to to the UPDATE: drive is needed
		wDrive= wstr + L"Common\\res\\TitleUpdate\\";
	}
	else
	{
		wDrive= wstr + L"Common\\";
	}
#else
	if(bTitleUpdateTexture)
	{
		// Make the content package point to to the UPDATE: drive is needed
		wDrive=L"Common\\res\\TitleUpdate\\";
	}
	else
	{
		wDrive=L"Common/";
	}
#endif
#endif

	return wDrive;
}
