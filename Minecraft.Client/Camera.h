#pragma once
#include "..\Minecraft.World\FloatBuffer.h"
#include "..\Minecraft.World\IntBuffer.h"


class TilePos;
class Vec3;
class Player;
class Mob;

class Camera
{
public:
	static float xPlayerOffs;
    static float yPlayerOffs;
    static float zPlayerOffs;

private:
//	static IntBuffer *viewport;
	static FloatBuffer *modelview;
	static FloatBuffer *projection;
//	static FloatBuffer *position;

public:
	static float xa, ya, za, xa2, za2;

	static void prepare(shared_ptr<Player> player, bool mirror);

	static TilePos *getCameraTilePos(shared_ptr<LivingEntity> player, double alpha);
	static Vec3 *getCameraPos(shared_ptr<LivingEntity> player, double alpha);
	static int getBlockAt(Level *level, shared_ptr<LivingEntity> player, float alpha);
};