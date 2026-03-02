#include "stdafx.h"
#include "DistanceChunkSorter.h"
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "Chunk.h"

DistanceChunkSorter::DistanceChunkSorter(shared_ptr<Entity> player)
{
    ix = -player->x;
    iy = -player->y;
    iz = -player->z;
}

bool DistanceChunkSorter::operator()(const Chunk *c0, const Chunk *c1) const
{
    double xd0 = c0->xm + ix;
    double yd0 = c0->ym + iy;
    double zd0 = c0->zm + iz;
				   
    double xd1 = c1->xm + ix;
    double yd1 = c1->ym + iy;
    double zd1 = c1->zm + iz;

	return (((xd0 * xd0 + yd0 * yd0 + zd0 * zd0) - (xd1 * xd1 + yd1 * yd1 + zd1 * zd1)) * 1024) < 0.0;
}