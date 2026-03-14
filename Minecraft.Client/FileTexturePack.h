#pragma once
#include "AbstractTexturePack.h"
#include "../Minecraft.World/ZipFile.h"
//class ZipFile;
class BufferedImage;
class File;
class Textures;
using namespace std;

class FileTexturePack : public AbstractTexturePack
{
private:
	std::unique_ptr<ZipFile> zipFile;
	std::vector<BYTE> _iconData;
	DWORD id;
	bool _hasAudio;

public:
	FileTexturePack(DWORD id, File *file, TexturePack *fallback);

	//@Override
	void unload(Textures *textures);
	DLCPack* getDLCPack();
	bool hasAudio();
	bool hasData();
	void loadIcon();
	PBYTE getPackIcon(DWORD& dwImageBytes);
	void loadName(File *file);
	bool isLoadingData();
	wstring getAnimationString(const wstring& textureName, const wstring& path, bool allowFallback);

protected:
	InputStream* getResourceImplementation(const wstring& name);

public:
	//@Override
	bool hasFile(const wstring &name);
	BufferedImage* getImageResource(const wstring& File, bool filenameHasExtension = false, bool bTitleUpdateTexture = false, const wstring& drive = L"");

private:
	void loadZipFile(File* file);

public:
	bool isTerrainUpdateCompatible();
};
