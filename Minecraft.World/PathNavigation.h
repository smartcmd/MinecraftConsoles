#pragma once

class Mob;
class Level;
class Path;

class PathNavigation
{
private:
	Mob *mob;
	Level *level;
	Path *path;
	double speedModifier;
	AttributeInstance *dist;
	bool avoidSun;
	int _tick;
	int lastStuckCheck;
	Vec3 *lastStuckCheckPos;

	bool _canPassDoors;
	bool _canOpenDoors;
	bool avoidWater;
	bool canFloat;

public:
	PathNavigation(Mob *mob, Level *level);
	~PathNavigation();

	void setAvoidWater(bool avoidWater);
	bool getAvoidWater();
	void setCanOpenDoors(bool canOpenDoors);
	bool canPassDoors();
	void setCanPassDoors(bool canPass);
	bool canOpenDoors();
	void setAvoidSun(bool avoidSun);
	void setSpeedModifier(double speedModifier);
	void setCanFloat(bool canFloat);
	float getMaxDist();
	Path *createPath(double x, double y, double z);
	bool moveTo(double x, double y, double z, double speedModifier);
	Path *createPath(shared_ptr<Entity> target);
	bool moveTo(shared_ptr<Entity> target, double speedModifier);
	bool moveTo(Path *newPath, double speedModifier);
	Path *getPath();
	void tick();

private:
	void updatePath();

public:
	bool isDone();

	void stop();

private:
	Vec3 *getTempMobPos();
	int getSurfaceY();
	bool canUpdatePath();
	bool isInLiquid();
	void trimPathFromSun();
	bool canMoveDirectly(Vec3 *startPos, Vec3 *stopPos, int sx, int sy, int sz);
	bool canWalkOn(int x, int y, int z, int sx, int sy, int sz, Vec3 *startPos, double goalDirX, double goalDirZ);
	bool canWalkAbove(int startX, int startY, int startZ, int sx, int sy, int sz, Vec3 *startPos, double goalDirX, double goalDirZ);

public:
	// 4J Added override to update ai elements when loading entity from schematics
	void setLevel(Level *level);
};