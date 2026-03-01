#include "stdafx.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"
#include "EnderChestTile.h"

EnderChestTile::EnderChestTile(int id) : BaseEntityTile(id, Material::stone, isSolidRender())
{
	updateDefaultShape();
}

// 4J Added override
void EnderChestTile::updateDefaultShape()
{
	setShape(1 / 16.0f, 0, 1 / 16.0f, 15 / 16.0f, 14 / 16.0f, 15 / 16.0f);
}

bool EnderChestTile::isSolidRender(bool isServerLevel)
{
	return false;
}

bool EnderChestTile::isCubeShaped()
{
	return false;
}

int EnderChestTile::getRenderShape()
{
	return Tile::SHAPE_ENTITYTILE_ANIMATED;
}

int EnderChestTile::getResource(int data, Random *random, int playerBonusLevel)
{
	return Tile::obsidian_Id;
}

int EnderChestTile::getResourceCount(Random *random)
{
	return 8;
}

bool EnderChestTile::isSilkTouchable()
{
	return true;
}

void EnderChestTile::setPlacedBy(Level *level, int x, int y, int z, shared_ptr<LivingEntity> by, shared_ptr<ItemInstance> itemInstance)
{
	int facing = 0;
	int dir = (Mth::floor(by->yRot * 4 / (360) + 0.5f)) & 3;

	if (dir == 0) facing = Facing::NORTH;
	if (dir == 1) facing = Facing::EAST;
	if (dir == 2) facing = Facing::SOUTH;
	if (dir == 3) facing = Facing::WEST;

	level->setData(x, y, z, facing, Tile::UPDATE_CLIENTS);
}

bool EnderChestTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly)
{
	shared_ptr<PlayerEnderChestContainer> container = player->getEnderChestInventory();
	shared_ptr<EnderChestTileEntity> enderChest = dynamic_pointer_cast<EnderChestTileEntity>(level->getTileEntity(x, y, z));
	if (container == NULL || enderChest == NULL) return true;

	if (level->isSolidBlockingTile(x, y + 1, z)) return true;

	if (level->isClientSide)
	{
		return true;
	}

	container->setActiveChest(enderChest);
	player->openContainer(container);

	return true;
}

shared_ptr<TileEntity> EnderChestTile::newTileEntity(Level *level)
{
	return shared_ptr<EnderChestTileEntity>(new EnderChestTileEntity());
}

void EnderChestTile::animateTick(Level *level, int xt, int yt, int zt, Random *random)
{
	for (int i = 0; i < 3; i++)
	{
		double x = xt + random->nextFloat();
		double y = yt + random->nextFloat();
		double z = zt + random->nextFloat();
		double xa = 0;
		double ya = 0;
		double za = 0;
		int flipX = random->nextInt(2) * 2 - 1;
		int flipZ = random->nextInt(2) * 2 - 1;
		xa = (random->nextFloat() - 0.5) * 0.125;
		ya = (random->nextFloat() - 0.5) * 0.125;
		za = (random->nextFloat() - 0.5) * 0.125;
		z = zt + 0.5 + (0.25) * flipZ;
		za = (random->nextFloat() * 1) * flipZ;
		x = xt + 0.5 + (0.25) * flipX;
		xa = (random->nextFloat() * 1) * flipX;

		level->addParticle(eParticleType_ender, x, y, z, xa, ya, za);
	}
}

void EnderChestTile::registerIcons(IconRegister *iconRegister)
{
	// Register obsidian as the chest's icon, because it's used by the
	// particles when destroying the chest
	icon = iconRegister->registerIcon(L"obsidian");
}
