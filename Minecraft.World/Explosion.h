#pragma once
#include "TilePos.h"
#include "Player.h"

class Random;
class Level;

class Explosion
{
public:
	bool fire;
	bool destroyBlocks;

private:
	int size;

	Random *random;
	Level *level;

public:
	double x, y, z;
	shared_ptr<Entity> source;
	float r;

	unordered_set<TilePos, TilePosKeyHash, TilePosKeyEq> toBlow;
	
private:
	typedef unordered_map<shared_ptr<Player>, Vec3 * , PlayerKeyHash, PlayerKeyEq> playerVec3Map;
	playerVec3Map hitPlayers;

public:
	Explosion(Level *level, shared_ptr<Entity> source, double x, double y, double z, float r);
	~Explosion();

public:
	void explode();

public:
	void finalizeExplosion(bool generateParticles, vector<TilePos> *toBlowDirect = NULL);   // 4J - added toBlow parameter
	playerVec3Map *getHitPlayers();
	Vec3 *getHitPlayerKnockback( shared_ptr<Player> player );
	shared_ptr<LivingEntity> getSourceMob();
};