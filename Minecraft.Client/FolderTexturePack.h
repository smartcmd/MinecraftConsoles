#pragma once

#include "AbstractTexturePack.h"

class FolderTexturePack : public AbstractTexturePack
{
private:
	bool bUILoaded;

public:
	FolderTexturePack(DWORD id, const wstring &name, File *folder, TexturePack *fallback);

protected:
	//@Override
	InputStream *getResourceImplementation(const wstring &name); //throws IOException

public:
	//@Override
	bool hasFile(const wstring &name);
	bool isTerrainUpdateCompatible();

	// 4J Added
	virtual wstring getPath(bool bTitleUpdateTexture = false, const char *pchBDPatchFilename=NULL);
	virtual void loadUI();
	virtual void unloadUI();
};