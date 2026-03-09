#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "SkullItem.h"

const unsigned int SkullItem::NAMES[SKULL_COUNT] = {IDS_ITEM_SKULL_SKELETON, IDS_ITEM_SKULL_WITHER, IDS_ITEM_SKULL_ZOMBIE, IDS_ITEM_SKULL_CHARACTER, IDS_ITEM_SKULL_CREEPER};

wstring SkullItem::ICON_NAMES[SKULL_COUNT] = {L"skeleton", L"wither", L"zombie", L"char", L"creeper"};

SkullItem::SkullItem(int id) : Item(id)
{
	//setItemCategory(CreativeModeTab.TAB_DECORATIONS);
	setMaxDamage(0);
	setStackedByData(true);
}

bool SkullItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly) //float clickX, float clickY, float clickZ)
{
	if (face == 0) return false;
	if (!level->getMaterial(x, y, z)->isSolid()) return false;

	if (face == 1) y++;

	if (face == 2) z--;
	if (face == 3) z++;
	if (face == 4) x--;
	if (face == 5) x++;

	//if (!player->mayUseItemAt(x, y, z, face, instance)) return false;
	if (!player->mayUseItemAt(x, y, z, face, instance)) return false;

	if (!Tile::skull->mayPlace(level, x, y, z)) return false;

	if(!bTestUseOnOnly)
	{
		level->setTileAndData(x, y, z, Tile::skull_Id, face, Tile::UPDATE_CLIENTS);

		int rot = 0;
		if (face == Facing::UP)
		{
			rot = Mth::floor(((player->yRot) * 16) / 360 + 0.5) & 15;
		}

		shared_ptr<TileEntity> skullTE = level->getTileEntity(x, y, z);
		shared_ptr<SkullTileEntity> skull = dynamic_pointer_cast<SkullTileEntity>(skullTE);

		if (skull != nullptr)
		{
			wstring extra = L"";
			if (instance->hasTag() && instance->getTag()->contains(L"SkullOwner"))
			{
				extra = instance->getTag()->getString(L"SkullOwner");
			}
			skull->setSkullType(instance->getAuxValue(), extra);
			skull->setRotation(rot);
			static_cast<SkullTile *>(Tile::skull)->checkMobSpawn(level, x, y, z, skull);
		}

		instance->count--;
	}
	return true;
}

bool SkullItem::mayPlace(Level *level, int x, int y, int z, int face, shared_ptr<Player> player, shared_ptr<ItemInstance> item)
{
	int currentTile = level->getTile(x, y, z);
	if (currentTile == Tile::topSnow_Id)
	{
		face = Facing::UP;
	}
	else if (currentTile != Tile::vine_Id && currentTile != Tile::tallgrass_Id && currentTile != Tile::deadBush_Id)
	{
		if (face == 0) y--;
		if (face == 1) y++;
		if (face == 2) z--;
		if (face == 3) z++;
		if (face == 4) x--;
		if (face == 5) x++;
	}

	return level->mayPlace(Tile::skull_Id, x, y, z, false, face, nullptr, item);
}

Icon *SkullItem::getIcon(int itemAuxValue)
{
	if (itemAuxValue < 0 || itemAuxValue >= SKULL_COUNT)
	{
		itemAuxValue = 0;
	}
	return icons[itemAuxValue];
}

int SkullItem::getLevelDataForAuxValue(int auxValue)
{
	return auxValue;
}

unsigned int SkullItem::getDescriptionId(int iData)
{
	if (iData < 0 || iData >= SKULL_COUNT)
	{
		iData = 0;
	}
	return NAMES[iData];
}

unsigned int SkullItem::getDescriptionId(shared_ptr<ItemInstance> instance)
{
	int auxValue = instance->getAuxValue();
	if (auxValue < 0 || auxValue >= SKULL_COUNT)
	{
		auxValue = 0;
	}
	return NAMES[auxValue];
}

wstring SkullItem::getHoverName(shared_ptr<ItemInstance> itemInstance)
{
#if 0
	if (itemInstance->getAuxValue() == SkullTileEntity::TYPE_CHAR && itemInstance->hasTag() && itemInstance->getTag()->contains(L"SkullOwner"))
	{
		return I18n.get("item.skull.player.name", itemInstance->getTag()->getString(L"SkullOwner"));
	}
	else
#endif
	{
		return Item::getHoverName(itemInstance);
	}
}

void SkullItem::registerIcons(IconRegister *iconRegister)
{
	for (int i = 0; i < SKULL_COUNT; i++)
	{
		icons[i] = iconRegister->registerIcon(getIconName() + L"_" + ICON_NAMES[i]);
	}
}