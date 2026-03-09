#pragma once
#include "AbstractTexturePack.h"

class DefaultTexturePack : public AbstractTexturePack
{
public:
	DefaultTexturePack();
	DLCPack * getDLCPack() {return nullptr;}

protected:
	//@Override
	void loadIcon();
	void loadName();
	void loadDescription();

public:
	//@Override
	bool hasFile(const wstring &name);
	bool isTerrainUpdateCompatible();

	wstring getDesc1() {return app.GetString(IDS_DEFAULT_TEXTUREPACK);}

protected:
	//@Override
	InputStream *getResourceImplementation(const wstring &name); // throws FileNotFoundException

public:
	virtual bool hasData() { return true; }
	virtual bool hasAudio() { return false; }
	virtual bool isLoadingData() { return false; }
	virtual void loadUI();
	virtual void unloadUI();
};