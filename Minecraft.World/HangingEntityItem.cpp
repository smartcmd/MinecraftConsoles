#include "stdafx.h"
#include "net.minecraft.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "HangingEntityItem.h"
#include "HangingEntity.h"
#include "Painting.h"
#include "GenericStats.h"
#include "ItemFrame.h"


HangingEntityItem::HangingEntityItem(int id, eINSTANCEOF eClassType) : Item(id)
{
	this->eType=eClassType;
}

bool HangingEntityItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int xt, int yt, int zt, int face, float clickX, float clickY, float clickZ, bool bTestOnly)
{
	if (face == Facing::DOWN) return false;
	if (face == Facing::UP) return false;

	if(bTestOnly)
	{
		if (!player->mayUseItemAt(xt, yt, zt, face, instance)) return false;

		return true;
	}

	int dir = Direction::FACING_DIRECTION[face];

	shared_ptr<HangingEntity> entity = createEntity(level, xt, yt, zt, dir, instance->getAuxValue() );

	if (!player->mayUseItemAt(xt, yt, zt, face, instance)) return false;

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


shared_ptr<HangingEntity> HangingEntityItem::createEntity(Level *level, int x, int y, int z, int dir, int auxValue)  // 4J added auxValue
{
	if (eType == eTYPE_PAINTING) 
	{
		shared_ptr<Painting> painting = shared_ptr<Painting>(new Painting(level, x, y, z, dir));

#ifndef _CONTENT_PACKAGE
		if (app.DebugArtToolsOn() && auxValue > 0) 
		{
			painting->PaintingPostConstructor(dir, auxValue - 1);
		}
		else
#endif
		{
			painting->PaintingPostConstructor(dir);
		}
		
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

// 4J Adding overrides for art tools
void HangingEntityItem::appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced)
{
#ifndef _CONTENT_PACKAGE
	if (eType == eTYPE_PAINTING && app.DebugArtToolsOn() && itemInstance->getAuxValue() > 0 ) 
	{
		int motive = itemInstance->getAuxValue() - 1;
		
		wchar_t formatted[256];
		ZeroMemory(formatted, 256 * sizeof(wchar_t));
		swprintf(formatted, 256, L"** %ls %dx%d",Painting::Motive::values[motive]->name.c_str(),Painting::Motive::values[motive]->w/16,Painting::Motive::values[motive]->h/16);

		wstring motiveName = formatted;

		lines->push_back(HtmlString(motiveName.c_str(), eHTMLColor_c));
	}
	else
#endif
	{
		return Item::appendHoverText(itemInstance, player, lines, advanced);
	}
}