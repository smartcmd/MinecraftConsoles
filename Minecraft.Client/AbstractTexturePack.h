#pragma once
using namespace std;

#include "TexturePack.h"

class BufferedImage;

class AbstractTexturePack : public TexturePack
{
private:
	const DWORD id;
	const wstring name;

protected:
	File *file;
	wstring texname;
	wstring m_wsWorldName;

	wstring desc1;
	wstring desc2;

	PBYTE m_iconData;
	DWORD m_iconSize;

	PBYTE m_comparisonData;
	DWORD m_comparisonSize;

	TexturePack *fallback;

	ColourTable *m_colourTable;

protected:
	BufferedImage *iconImage;

private:
	int textureId;

protected:
	AbstractTexturePack(DWORD id, File *file, const wstring &name, TexturePack *fallback);

private:
	static wstring trim(wstring line);
	
protected:
	virtual void loadIcon();
	virtual void loadComparison();
	virtual void loadDescription();
	virtual void loadName();

public:
	virtual InputStream *getResource(const wstring &name, bool allowFallback); //throws IOException
	// 4J Removed do to current override in TexturePack class
	//virtual InputStream *getResource(const wstring &name); //throws IOException
	virtual DLCPack * getDLCPack() =0;


protected:
	virtual InputStream *getResourceImplementation(const wstring &name) = 0; // throws IOException;
public:
	virtual void unload(Textures *textures);
	virtual void load(Textures *textures);
	virtual bool hasFile(const wstring &name, bool allowFallback);
	virtual bool hasFile(const wstring &name) = 0;
	virtual DWORD getId();
	virtual wstring getName();
	virtual wstring getDesc1();
	virtual wstring getDesc2();
	virtual wstring getWorldName();
	
	virtual wstring getAnimationString(const wstring &textureName, const wstring &path, bool allowFallback);

protected:
	virtual wstring getAnimationString(const wstring &textureName, const wstring &path);
	void loadDefaultUI();
	void loadDefaultColourTable();
	void loadDefaultHTMLColourTable();
#ifdef _XBOX
	void loadHTMLColourTableFromXuiScene(HXUIOBJ hObj);
#endif

public:
	virtual BufferedImage *getImageResource(const wstring& File, bool filenameHasExtension = false, bool bTitleUpdateTexture=false, const wstring &drive =L"");
	virtual void loadColourTable();
	virtual void loadUI();
	virtual void unloadUI();
	virtual wstring getXuiRootPath();
	virtual PBYTE getPackIcon(DWORD &dwImageBytes);
	virtual PBYTE getPackComparison(DWORD &dwImageBytes);
	virtual unsigned int getDLCParentPackId();
	virtual unsigned char getDLCSubPackId();
	virtual ColourTable *getColourTable() { return m_colourTable; }
	virtual ArchiveFile *getArchiveFile() { return nullptr; }
};
