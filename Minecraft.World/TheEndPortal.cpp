#include "stdafx.h"
#include "TheEndPortal.h"
#include "TheEndPortalTileEntity.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.storage.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.h"

DWORD TheEndPortal::tlsIdx = TlsAlloc();

// 4J - allowAnywhere is a static in java, implementing as TLS here to make thread safe
bool TheEndPortal::allowAnywhere()
{
	return (TlsGetValue(tlsIdx) != nullptr);
}

void TheEndPortal::allowAnywhere(bool set)
{
	TlsSetValue(tlsIdx,(LPVOID)(set?1:0));
}

TheEndPortal::TheEndPortal(int id, Material *material) : BaseEntityTile(id, material, isSolidRender())
{
	this->setLightEmission(1.0f);
}

shared_ptr<TileEntity> TheEndPortal::newTileEntity(Level *level)
{
	return std::make_shared<TheEndPortalTileEntity>();
}

void TheEndPortal::updateShape(LevelSource *level, int x, int y, int z, int forceData, shared_ptr<TileEntity> forceEntity) // 4J added forceData, forceEntity param
{
	float r = 1 / 16.0f;
	setShape(0, 0, 0, 1, r, 1);
}

bool TheEndPortal::shouldRenderFace(LevelSource *level, int x, int y, int z, int face)
{
	if (face != 0) return false;
	return BaseEntityTile::shouldRenderFace(level, x, y, z, face);
}

void TheEndPortal::addAABBs(Level *level, int x, int y, int z, AABB *box, AABBList *boxes, shared_ptr<Entity> source)
{
}

bool TheEndPortal::isSolidRender(bool isServerLevel)
{
	return false;
}

bool TheEndPortal::isCubeShaped()
{
	return false;
}

int TheEndPortal::getResourceCount(Random *random)
{
	return 0;
}

void TheEndPortal::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	if (entity->GetType() == eTYPE_EXPERIENCEORB ) return;		// 4J added

	if (entity->riding == nullptr && entity->rider.lock() == nullptr)
	{
		if (!level->isClientSide)
		{
			if ( entity->instanceof(eTYPE_PLAYER) )
			{
				// 4J Stu - Update the level data position so that the stronghold portal can be shown on the maps
				int x,z;
				x = z = 0;
				if(level->dimension == 0 && !level->getLevelData()->getHasStrongholdEndPortal() && app.GetTerrainFeaturePosition( eTerrainFeature_StrongholdEndPortal, &x, &z) )
				{
					level->getLevelData()->setXStrongholdEndPortal(x);
					level->getLevelData()->setZStrongholdEndPortal(z);
					level->getLevelData()->setHasStrongholdEndPortal();
				}
			}
			entity->changeDimension(1);
		}
	}
}

void TheEndPortal::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	double x = xt + random->nextFloat();
	double y = yt + 0.8f;
	double z = zt + random->nextFloat();
	double xa = 0;
	double ya = 0;
	double za = 0;

	level->addParticle(eParticleType_endportal, x, y, z, xa, ya, za);
}

int TheEndPortal::getRenderShape()
{
	return SHAPE_INVISIBLE;
}

void TheEndPortal::onPlace(Level *level, int x, int y, int z)
{
	if (allowAnywhere()) return;

	if (level->dimension->id != 0)
	{
		level->removeTile(x, y, z);
		return;
	}
}

int TheEndPortal::cloneTileId(Level *level, int x, int y, int z)
{
	return 0;
}

void TheEndPortal::registerIcons(IconRegister *iconRegister)
{
	// don't register null, because of particles
	icon = iconRegister->registerIcon(L"portal");
}
