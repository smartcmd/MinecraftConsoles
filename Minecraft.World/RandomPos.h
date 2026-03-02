#pragma once

class PathfinderMob;

class RandomPos
{
private:
	static Vec3 *tempDir;

public:
	static Vec3 *getPos(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, int quadrant = -1);		// 4J added quadrant
	static Vec3 *getPosTowards(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, Vec3 *towardsPos);
	static Vec3 *getPosAvoid(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, Vec3 *avoidPos);

private:
	static Vec3 *generateRandomPos(shared_ptr<PathfinderMob> mob, int xzDist, int yDist, Vec3 *dir, int quadrant = -1);		// 4J added quadrant
};