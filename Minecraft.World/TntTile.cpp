#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.h"
#include "net.minecraft.h"

#include "TntTile.h"
#include "SoundTypes.h"


TntTile::TntTile(int id) : Tile(id, Material::explosive)
{
	iconTop = NULL;
	iconBottom = NULL;
}

Icon *TntTile::getTexture(int face, int data)
{
	if (face == Facing::DOWN) return iconBottom;
	if (face == Facing::UP) return iconTop;
	return icon;
}

void TntTile::onPlace(Level *level, int x, int y, int z)
{
	Tile::onPlace(level, x, y, z);
    if (level->hasNeighborSignal(x, y, z) && app.GetGameHostOption(eGameHostOption_TNT))
	{
        destroy(level, x, y, z, EXPLODE_BIT);
        level->setTile(x, y, z, 0);
    }
}

void TntTile::neighborChanged(Level *level, int x, int y, int z, int type)
{
	if (type > 0 && Tile::tiles[type]->isSignalSource())
	{
		if (level->hasNeighborSignal(x, y, z) && app.GetGameHostOption(eGameHostOption_TNT))
		{
			destroy(level, x, y, z, EXPLODE_BIT);
			level->setTile(x, y, z, 0);
		}
	}
}

int TntTile::getResourceCount(Random *random)
{
	return 1;
}

void TntTile::wasExploded(Level *level, int x, int y, int z)
{
	// 4J - added - don't every create on the client, I think this must be the cause of a bug reported in the java
	// version where white tnts are created in the network game
	if (level->isClientSide) return;	

	// 4J - added condition to have finite limit of these
	// 4J-JEV: Fix for #90934 - Customer Encountered: TU11: Content: Gameplay: TNT blocks are triggered by explosions even though "TNT explodes" option is unchecked.
	if( level->newPrimedTntAllowed() && app.GetGameHostOption(eGameHostOption_TNT) )
	{
		shared_ptr<PrimedTnt> primed = shared_ptr<PrimedTnt>( new PrimedTnt(level, x + 0.5f, y + 0.5f, z + 0.5f) );
		primed->life = level->random->nextInt(primed->life / 4) + primed->life / 8;
		level->addEntity(primed);
	}
}

void TntTile::destroy(Level *level, int x, int y, int z, int data)
{
	if (level->isClientSide) return;

	if ((data & EXPLODE_BIT) == 1 )
	{
		// 4J - added condition to have finite limit of these
		if( level->newPrimedTntAllowed() && app.GetGameHostOption(eGameHostOption_TNT) )
		{
			shared_ptr<PrimedTnt> tnt = shared_ptr<PrimedTnt>(  new PrimedTnt(level, x + 0.5f, y + 0.5f, z + 0.5f) );
			level->addEntity(tnt);
			level->playSound(tnt, eSoundType_RANDOM_FUSE, 1, 1.0f);
		}
	}
}

bool TntTile::use(Level *level, int x, int y, int z, shared_ptr<Player> player, int clickedFace, float clickX, float clickY, float clickZ, bool soundOnly/*=false*/) // 4J added soundOnly param
{
	if (soundOnly) return false;
	if (player->getSelectedItem() != NULL && player->getSelectedItem()->id == Item::flintAndSteel_Id)
	{
		destroy(level, x, y, z, EXPLODE_BIT);
		level->setTile(x, y, z, 0);
		return true;
	}
	return Tile::use(level, x, y, z, player, clickedFace, clickX, clickY, clickZ);
}

void TntTile::entityInside(Level *level, int x, int y, int z, shared_ptr<Entity> entity)
{
	if (entity->GetType() == eTYPE_ARROW && !level->isClientSide)
	{
		// 4J Stu - Don't need to cast this
		//shared_ptr<Arrow> arrow = dynamic_pointer_cast<Arrow>(entity);
		if (entity->isOnFire())
		{
			destroy(level, x, y, z, EXPLODE_BIT);
			level->setTile(x, y, z, 0);
		}
	}
}

shared_ptr<ItemInstance> TntTile::getSilkTouchItemInstance(int data)
{
	return nullptr;
}

void TntTile::registerIcons(IconRegister *iconRegister) 
{
	icon = iconRegister->registerIcon(L"tnt_side");
	iconTop = iconRegister->registerIcon(L"tnt_top");
	iconBottom = iconRegister->registerIcon(L"tnt_bottom");
}