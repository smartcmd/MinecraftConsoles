#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "BeaconTile.h"

BeaconTile::BeaconTile(int id) : BaseEntityTile(id, Material::glass, isSolidRender())
{
	setDestroyTime(3.0f);
}

shared_ptr<TileEntity> BeaconTile::newTileEntity(Level *level)
{
	return shared_ptr<BeaconTileEntity>( new BeaconTileEntity() );
}

bool BeaconTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	if (level->isClientSide) return true;

	shared_ptr<BeaconTileEntity> beacon = dynamic_pointer_cast<BeaconTileEntity>( level->getTileEntity(x, y, z) );
	if (beacon != NULL) player->openBeacon(beacon);

	return true;
}

bool BeaconTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool BeaconTile::isCubeShaped()
{
	return false;
}

bool BeaconTile::blocksLight()
{
	return false;
}

int BeaconTile::getRenderShape()
{
	return SHAPE_BEACON;
}

void BeaconTile::registerIcons(IconRegister *iconRegister)
{
	BaseEntityTile::registerIcons(iconRegister);
}

void BeaconTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	BaseEntityTile::setPlacedBy(level, x, y, z, by, itemInstance);
	if (itemInstance->hasCustomHoverName())
	{
		dynamic_pointer_cast<BeaconTileEntity>( level->getTileEntity(x, y, z))->setCustomName(itemInstance->getHoverName());
	}
}

bool BeaconTile::TestUse()
{
	return true;
}