#pragma once
#include "4JLibs\inc\4J_Render.h"

class CConsoleMinecraftApp : public CMinecraftApp
{
	ImageFileBuffer m_ThumbnailBuffer;
public:
	CConsoleMinecraftApp();

	virtual void SetRichPresenceContext(int iPad, int contextId);

	virtual void StoreLaunchData();
	virtual void ExitGame();
	virtual void FatalLoadError();

	virtual void CaptureSaveThumbnail();
	virtual void GetSaveThumbnail(PBYTE*,DWORD*);
	virtual void ReleaseSaveThumbnail();
	virtual void GetScreenshot(int iPad,PBYTE *pbData,DWORD *pdwSize);

	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile);
	virtual int LoadLocalTMSFile(WCHAR *wchTMSFile, eFileExtensionType eExt);

	virtual void FreeLocalTMSFiles(eTMSFileType eType);
	virtual int GetLocalTMSFileIndex(WCHAR *wchTMSFile,bool bFilenameIncludesExtension,eFileExtensionType eEXT=eFileExtensionType_PNG);

	// BANNED LEVEL LIST
	virtual void ReadBannedList(int iPad, eTMSAction action=static_cast<eTMSAction>(0), bool bCallback=false) {}

	C4JStringTable *GetStringTable()																									{ return nullptr;}

	// original code
	virtual void TemporaryCreateGameStart();

	bool m_bShutdown;
};

extern CConsoleMinecraftApp app;
