#pragma once
using namespace std;

class TileEntity;
#include "Player.h"
#include "ParticleTypes.h"

class LevelListener
{
public:
	virtual void tileChanged(int x, int y, int z) = 0;

	virtual void tileLightChanged(int x, int y, int z) = 0;

	virtual void setTilesDirty(int x0, int y0, int z0, int x1, int y1, int z1, Level *level) = 0;		// 4J - added level param

	virtual void allChanged() = 0;

	//virtual void playSound(const wstring& name, double x, double y, double z, float volume, float pitch) = 0;
	virtual void playSound(int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist=16.0f) = 0;
	virtual void playSound(shared_ptr<Entity> entity,int iSound, double x, double y, double z, float volume, float pitch, float fSoundClipDist=16.0f) = 0;

	// 4J removed - virtual void addParticle(const wstring& name, double x, double y, double z, double xa, double ya, double za) = 0;

	virtual void addParticle(ePARTICLE_TYPE name, double x, double y, double z, double xa, double ya, double za) = 0; // 4J added

	virtual void entityAdded(shared_ptr<Entity> entity) = 0;

	virtual void entityRemoved(shared_ptr<Entity> entity) = 0;

	virtual void playerRemoved(shared_ptr<Entity> entity) = 0;		// 4J added - for when a player is removed from the level's player array, not just the entity storage

	virtual void skyColorChanged() = 0;

	virtual void playStreamingMusic(const wstring& name, int x, int y, int z) = 0;

	virtual void levelEvent(shared_ptr<Player> source, int type, int x, int y, int z, int data) = 0;

	virtual void destroyTileProgress(int id, int x, int y, int z, int progress) = 0;
};