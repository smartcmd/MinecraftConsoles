#include "stdafx.h"
#include "DirtyChunkSorter.h"
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "Chunk.h"

DirtyChunkSorter::DirtyChunkSorter(shared_ptr<LivingEntity> cameraEntity, int playerIndex)	// 4J - added player index
{
	this->cameraEntity = cameraEntity;
	this->playerIndex = playerIndex;
}

bool DirtyChunkSorter::operator()(const Chunk *c0, const Chunk *c1) const
{
    bool i0 = c0->clipChunk->visible;
    bool i1 = c1->clipChunk->visible;
    if (i0 && !i1) return false;
    if (i1 && !i0) return true;

    double d0 = c0->distanceToSqr(cameraEntity);
    double d1 = c1->distanceToSqr(cameraEntity);

    if (d0 < d1) return false;
    if (d0 > d1) return true;

    return c0->id >= c1->id;	// 4J - was c0.id < c1.id ? 1 : -1
}