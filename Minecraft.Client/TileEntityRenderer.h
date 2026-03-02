#pragma once
#include "Textures.h"
class TileEntityRenderDispatcher;
class TileEntity;
class Level;
class Font;

class TileEntityRenderer
{
protected:
	TileEntityRenderDispatcher *tileEntityRenderDispatcher;
public:
	virtual void render(shared_ptr<TileEntity> entity, double x, double y, double z, float a, bool setColor, float alpha, bool useCompiled) = 0;		// 4J added setColor param and alpha and useCompiled
	virtual void onNewLevel(Level *level) {}
protected:
	void bindTexture(int resourceName);		// 4J - changed from wstring to int
    void bindTexture(const wstring& urlTexture, int backupTexture);	// 4J - changed from wstring to int
private:
	Level *getLevel();
public:
	virtual void init(TileEntityRenderDispatcher *tileEntityRenderDispatcher);
    Font *getFont();
};
