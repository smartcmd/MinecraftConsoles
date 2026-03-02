#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "ItemInstance.h"
#include "SignItem.h"
#include "GenericStats.h"

SignItem::SignItem(int id) : Item(id)
{
	// 4J-PB - Changed for TU9
	maxStackSize = 16;
}

bool SignItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if (face == 0) return false;
	if (!level->getMaterial(x, y, z)->isSolid()) return false;


	if (face == 1) y++;
	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	if (!Tile::sign->mayPlace(level, x, y, z)) return false;

	if (level->isClientSide)
	{
		return true;
	}

	if(!bTestUseOnOnly)
	{
		if (face == 1) 
		{
			int rot = Mth::floor(((player->yRot + 180) * 16) / 360 + 0.5) & 15;
			level->setTileAndData(x, y, z, Tile::sign_Id, rot, Tile::UPDATE_ALL);
		} 
		else
		{
			level->setTileAndData(x, y, z, Tile::wallSign_Id, face, Tile::UPDATE_ALL);
		}

		instance->count--;
		shared_ptr<SignTileEntity> ste = dynamic_pointer_cast<SignTileEntity>( level->getTileEntity(x, y, z) );
		if (ste != NULL) player->openTextEdit(ste);

		// 4J-JEV: Hook for durango 'BlockPlaced' event.
		player->awardStat(
			GenericStats::blocksPlaced((face==1) ? Tile::sign_Id : Tile::wallSign_Id),
			GenericStats::param_blocksPlaced( 
				(face==1) ? Tile::sign_Id : Tile::wallSign_Id,
				instance->getAuxValue(),
				1)
			);
	}
	return true;
}
