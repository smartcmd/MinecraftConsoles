#include "stdafx.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.level.h"
#include "FireworksItem.h"

const wstring FireworksItem::TAG_FIREWORKS = L"Fireworks";
const wstring FireworksItem::TAG_EXPLOSION = L"Explosion";
const wstring FireworksItem::TAG_EXPLOSIONS = L"Explosions";
const wstring FireworksItem::TAG_FLIGHT = L"Flight";
const wstring FireworksItem::TAG_E_TYPE = L"Type";
const wstring FireworksItem::TAG_E_TRAIL = L"Trail";
const wstring FireworksItem::TAG_E_FLICKER = L"Flicker";
const wstring FireworksItem::TAG_E_COLORS = L"Colors";
const wstring FireworksItem::TAG_E_FADECOLORS = L"FadeColors";

FireworksItem::FireworksItem(int id) : Item(id)
{
}

bool FireworksItem::useOn(shared_ptr<ItemInstance> instance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	// 4J-JEV: Fix for xb1 #173493 - CU7: Content: UI: Missing tooltip for Firework Rocket.
	if (bTestUseOnOnly) return true;

	if (!level->isClientSide)
	{
		shared_ptr<FireworksRocketEntity> f(new FireworksRocketEntity(level, x + clickX, y + clickY, z + clickZ, instance));
		level->addEntity(f);

		if (!player->abilities.instabuild)
		{
			instance->count--;
		}
		return true;
	}

	return false;
}

void FireworksItem::appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced)
{
	if (!itemInstance->hasTag())
	{
		return;
	}
	CompoundTag *fireTag = itemInstance->getTag()->getCompound(TAG_FIREWORKS);
	if (fireTag == nullptr)
	{
		return;
	}
	if (fireTag->contains(TAG_FLIGHT))
	{
		lines->push_back(wstring(app.GetString(IDS_ITEM_FIREWORKS_FLIGHT)) + L" " + std::to_wstring((fireTag->getByte(TAG_FLIGHT))));
	}

	ListTag<CompoundTag> *explosions = (ListTag<CompoundTag> *) fireTag->getList(TAG_EXPLOSIONS);
	if (explosions != nullptr && explosions->size() > 0)
	{

		for (int i = 0; i < explosions->size(); i++)
		{
			CompoundTag *expTag = explosions->get(i);

			vector<HtmlString> eLines;
			FireworksChargeItem::appendHoverText(expTag, &eLines);

			if (eLines.size() > 0)
			{
				// Indent lines after first line
				for (size_t i = 1; i < eLines.size(); i++)
				{
					eLines[i].indent = true;
				}

				lines->insert(lines->end(), eLines.begin(), eLines.end());
			}
		}
	}
}