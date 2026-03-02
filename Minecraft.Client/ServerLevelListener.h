#pragma once
#include "..\Minecraft.World\LevelListener.h"
class Player;
class TileEntity;
class Entity;
class MinecraftServer;
class ServerLevel;

// 4J - renamed class to ServerLevelListener to avoid clash with LevelListener
class ServerLevelListener : public LevelListener
{
private:
	MinecraftServer *server;
    ServerLevel *level;

public:
	ServerLevelListener(MinecraftServer *server, ServerLevel *level);
	// 4J removed - virtual void addParticle(const wstring& name, double x, double y, double z, double xa, double ya, double za);
	virtual void addParticle(ePARTICLE_TYPE name, double x, double y, double z, double xa, double ya, double za); // 4J added
    virtual void allChanged();
    virtual void entityAdded(shared_ptr<Entity> entity);
    virtual void entityRemoved(shared_ptr<Entity> entity);
	virtual void playerRemoved(shared_ptr<Entity> entity);		// 4J added - for when a player is removed from the level's player array, not just the entity storage
    virtual void playSound(int iSound, double x, double y, double z, float volume, float pitch, float fClipSoundDist);
	virtual void playSound(shared_ptr<Entity> entity,int iSound, double x, double y, double z, float volume, float pitch, float fClipSoundDist);
    virtual void setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1, Level *level);	// 4J - added level param
    virtual void skyColorChanged();
    virtual void tileChanged(int x, int y, int z);
	virtual void tileLightChanged(int x, int y, int z);
    virtual void playStreamingMusic(const wstring& name, int x, int y, int z);
    virtual void levelEvent(shared_ptr<Player> source, int type, int x, int y, int z, int data);
	virtual void destroyTileProgress(int id, int x, int y, int z, int progress);
};
