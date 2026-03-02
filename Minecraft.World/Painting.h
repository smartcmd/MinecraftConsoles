#pragma once
using namespace std;

#include "Entity.h"
#include "HangingEntity.h"

class Level;
class CompoundTag;
class DamageSource;

class Painting : public HangingEntity
{

public:
	eINSTANCEOF GetType() { return eTYPE_PAINTING; }
	static Entity *create(Level *level) { return new Painting(level); }

private:
	//int checkInterval;

public:
	enum MotiveEnum {
		Kebab = 0, //
		Aztec, //
		Alban, //
		Aztec2, //
		Bomb, //
		Plant, //
		Wasteland, //

		Pool, //
		Courbet, //
		Sea, //
		Sunset, //
		Creebet, //

		Wanderer, //
		Graham, //

		Match, //
		Bust, //
		Stage, //
		Void, //
		SkullAndRoses, //
		Wither,
		Fighters, //

		Pointer, //
		Pigscene, //
		BurningSkull, //

		Skeleton, //
		DonkeyKong, //

		LAST_VALUE
	};

	// TODO 4J Replace the ENUM with static consts
	class Motive
	{
	public:
		static const Motive *values[];

		static const int MAX_MOTIVE_NAME_LENGTH;

		const wstring name;
		const int w, h;
		const int uo, vo;

	//private:
		Motive(wstring name, int w, int h, int uo, int vo) : name( name ), w( w ), h( h ), uo( uo ), vo( vo ) {};
	};

public:
// 	int dir;
// 
// 	int xTile, yTile, zTile;

	Motive *motive;

private:
	// 4J - added for common ctor code
	void _init( Level *level );

public:
	Painting(Level *level);
	Painting(Level *level, int xTile, int yTile, int zTile, int dir);
	Painting(Level *level, int x, int y, int z, int dir, wstring motiveName);

	// 4J Stu - Added this so that we can use some shared_ptr functions that were needed in the ctor
	void PaintingPostConstructor(int dir);

protected:
	//void defineSynchedData();

public:
	//void setDir(int dir);

private:
	//float offs(int w);

public:
	//virtual void tick();
	//bool survives();
	//virtual bool isPickable();
	//virtual bool hurt(DamageSource *source, int damage);
	virtual void addAdditonalSaveData(CompoundTag *tag);
	virtual void readAdditionalSaveData(CompoundTag *tag);
	//static Motive *randomMotive();

	//virtual void move(double xa, double ya, double za, bool noEntityCubes=false);	// 4J - added noEntityCubes parameter
	//virtual void push(double xa, double ya, double za);

	virtual int getWidth();
	virtual int getHeight();
	virtual void dropItem();
};
