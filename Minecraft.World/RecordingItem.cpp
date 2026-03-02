#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.h"
#include "ItemInstance.h"
#include "RecordingItem.h"
#include "GenericStats.h"

RecordingItem::RecordingItem(int id, const wstring& recording) : Item(id), recording( recording )
{
	this->maxStackSize = 1;
}

Icon *RecordingItem::getIcon(int auxValue)
{
	return icon;
}

bool RecordingItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-PB - Adding a test only version to allow tooltips to be displayed
	if (level->getTile(x, y, z) == Tile::recordPlayer_Id && level->getData(x, y, z) == 0)
	{
		if(!bTestUseOnOnly)
		{
			if (level->isClientSide) return true;

			((RecordPlayerTile *) Tile::recordPlayer)->setRecord(level, x, y, z, id);
			level->levelEvent(nullptr, LevelEvent::SOUND_PLAY_RECORDING, x, y, z, id);
			itemInstance->count--;

			player->awardStat(
				GenericStats::musicToMyEars(),
				GenericStats::param_musicToMyEars(id)
				);
		}
		return true;
	}
	return false;
}

void RecordingItem::appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<wstring> *lines, bool advanced, vector<wstring> &unformattedStrings)
{
	eMinecraftColour rarityColour = getRarity(shared_ptr<ItemInstance>())->color;
	int colour = app.GetHTMLColour(rarityColour);
	wchar_t formatted[256];

	swprintf(formatted, 256, L"<font color=\"#%08x\">%ls</font>",colour,L"C418 - ", recording.c_str());

	lines->push_back(formatted);

	unformattedStrings.push_back(recording);
}

const Rarity *RecordingItem::getRarity(shared_ptr<ItemInstance> itemInstance)
{
	return (Rarity *)Rarity::rare;
}

void RecordingItem::registerIcons(IconRegister *iconRegister)
{
	icon = iconRegister->registerIcon(L"record_" + recording);
}
