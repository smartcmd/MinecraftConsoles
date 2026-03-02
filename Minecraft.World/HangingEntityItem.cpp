#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "HangingEntityItem.h"
#include "HangingEntity.h"
#include "Painting.h"
#include "GenericStats.h"
#include "ItemFrame.h"


HangingEntityItem::HangingEntityItem(int id, eINSTANCEOF eClassType) : Item(id)
{
	//super(id);
	//this.clazz = clazz;
	this->eType=eClassType;
	// setItemCategory(CreativeModeTab.TAB_DECORATIONS);
}

bool HangingEntityItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int xt, int yt, int zt, int face, float clickX, float clickY, float clickZ, bool bTestOnly)
{
	if (face == Facing::DOWN) return false;
	if (face == Facing::UP) return false;

	if(bTestOnly)
	{
		if (!player->mayBuild(xt, yt, zt)) return false;

		return true;
	}

	int dir = Direction::FACING_DIRECTION[face];

	shared_ptr<HangingEntity> entity = createEntity(level, xt, yt, zt, dir);

	//if (!player->mayUseItemAt(xt, yt, zt, face, instance)) return false;
	if (!player->mayBuild(xt, yt, zt)) return false;

	if (entity != NULL && entity->survives()) 
	{
		if (!level->isClientSide) 
		{
			if(level->addEntity(entity)==TRUE)
			{
				// 4J-JEV: Hook for durango 'BlockPlaced' event.
				if (eType==eTYPE_PAINTING)			player->awardStat(GenericStats::blocksPlaced(Item::painting_Id), GenericStats::param_blocksPlaced(Item::painting_Id,instance->getAuxValue(),1));
				else if (eType==eTYPE_ITEM_FRAME)	player->awardStat(GenericStats::blocksPlaced(Item::itemFrame_Id), GenericStats::param_blocksPlaced(Item::itemFrame_Id,instance->getAuxValue(),1));

				instance->count--;
			}
			else
			{
				player->displayClientMessage(IDS_MAX_HANGINGENTITIES );
				return false;
			}
		}
		else
		{
			instance->count--;
		}
	}
	return true;
}


shared_ptr<HangingEntity> HangingEntityItem::createEntity(Level *level, int x, int y, int z, int dir) 
{
	if (eType == eTYPE_PAINTING) 
	{
		shared_ptr<Painting> painting = shared_ptr<Painting>(new Painting(level, x, y, z, dir));
		painting->PaintingPostConstructor(dir);
		
		return dynamic_pointer_cast<HangingEntity> (painting);
	} 
	else if (eType == eTYPE_ITEM_FRAME) 
	{
		shared_ptr<ItemFrame> itemFrame = shared_ptr<ItemFrame>(new ItemFrame(level, x, y, z, dir));

		return dynamic_pointer_cast<HangingEntity> (itemFrame);
	} 
	else 
	{
		return nullptr;
	}
}

