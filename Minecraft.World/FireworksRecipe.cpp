#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "FireworksRecipe.h"

DWORD FireworksRecipe::tlsIdx = 0;
FireworksRecipe::ThreadStorage *FireworksRecipe::tlsDefault = NULL;

FireworksRecipe::ThreadStorage::ThreadStorage()
{
	resultItem = nullptr;
}

void FireworksRecipe::CreateNewThreadStorage()
{
	ThreadStorage *tls = new ThreadStorage();
	if(tlsDefault == NULL )
	{
		tlsIdx = TlsAlloc();
		tlsDefault = tls;
	}
	TlsSetValue(tlsIdx, tls);
}

void FireworksRecipe::UseDefaultThreadStorage()
{
	TlsSetValue(tlsIdx, tlsDefault);
}

void FireworksRecipe::ReleaseThreadStorage()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(tlsIdx);
	if( tls == tlsDefault ) return;

	delete tls;
}

void FireworksRecipe::setResultItem(shared_ptr<ItemInstance> item)
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(tlsIdx);
	tls->resultItem = item;
}

FireworksRecipe::FireworksRecipe()
{
	//resultItem = nullptr;
}

bool FireworksRecipe::matches(shared_ptr<CraftingContainer> craftSlots, Level *level)
{
	shared_ptr<ItemInstance> resultItem = nullptr;

	int paperCount = 0;
	int sulphurCount = 0;
	int colorCount = 0;
	int chargeCount = 0;
	int chargeComponents = 0;
	int typeComponents = 0;

	for (int slot = 0; slot < craftSlots->getContainerSize(); slot++)
	{
		shared_ptr<ItemInstance> item = craftSlots->getItem(slot);
		if (item == NULL) continue;

		if (item->id == Item::gunpowder_Id)
		{
			sulphurCount++;
		}
		else if (item->id == Item::fireworksCharge_Id)
		{
			chargeCount++;
		}
		else if (item->id == Item::dye_powder_Id)
		{
			colorCount++;
		}
		else if (item->id == Item::paper_Id)
		{
			paperCount++;
		}
		else if (item->id == Item::yellowDust_Id)
		{
			// glowstone dust gives flickering
			chargeComponents++;
		}
		else if (item->id == Item::diamond_Id)
		{
			// diamonds give trails
			chargeComponents++;
		}
		else if (item->id == Item::fireball_Id)
		{
			// fireball gives larger explosion
			typeComponents++;
		}
		else if (item->id == Item::feather_Id)
		{
			// burst
			typeComponents++;
		}
		else if (item->id == Item::goldNugget_Id)
		{
			// star
			typeComponents++;
		}
		else if (item->id == Item::skull_Id)
		{
			// creeper
			typeComponents++;
		}
		else
		{
			setResultItem(resultItem);
			return false;
		}
	}
	chargeComponents += colorCount + typeComponents;

	if (sulphurCount > 3 || paperCount > 1)
	{
		setResultItem(resultItem);
		return false;
	}

	// create fireworks
	if (sulphurCount >= 1 && paperCount == 1 && chargeComponents == 0)
	{
		resultItem = shared_ptr<ItemInstance>( new ItemInstance(Item::fireworks) );
		if (chargeCount > 0)
		{
			CompoundTag *itemTag = new CompoundTag();
			CompoundTag *fireTag = new CompoundTag(FireworksItem::TAG_FIREWORKS);
			ListTag<CompoundTag> *expTags = new ListTag<CompoundTag>(FireworksItem::TAG_EXPLOSIONS);

			for (int slot = 0; slot < craftSlots->getContainerSize(); slot++)
			{
				shared_ptr<ItemInstance> item = craftSlots->getItem(slot);
				if (item == NULL || item->id != Item::fireworksCharge_Id) continue;

				if (item->hasTag() && item->getTag()->contains(FireworksItem::TAG_EXPLOSION))
				{
					expTags->add((CompoundTag *)item->getTag()->getCompound(FireworksItem::TAG_EXPLOSION)->copy());
				}
			}

			fireTag->put(FireworksItem::TAG_EXPLOSIONS, expTags);
			fireTag->putByte(FireworksItem::TAG_FLIGHT, (byte) sulphurCount);
			itemTag->put(FireworksItem::TAG_FIREWORKS, fireTag);

			resultItem->setTag(itemTag);
		}
		setResultItem(resultItem);
		return true;
	}
	// create firecharge
	if (sulphurCount == 1 && paperCount == 0 && chargeCount == 0 && colorCount > 0 && typeComponents <= 1)
	{

		resultItem = shared_ptr<ItemInstance>( new ItemInstance(Item::fireworksCharge) );
		CompoundTag *itemTag = new CompoundTag();
		CompoundTag *expTag = new CompoundTag(FireworksItem::TAG_EXPLOSION);

		byte type = 0;

		vector<int> colors;
		for (int slot = 0; slot < craftSlots->getContainerSize(); slot++)
		{
			shared_ptr<ItemInstance> item = craftSlots->getItem(slot);
			if (item == NULL) continue;

			if (item->id == Item::dye_powder_Id)
			{
				colors.push_back(DyePowderItem::COLOR_RGB[item->getAuxValue()]);
			}
			else if (item->id == Item::yellowDust_Id)
			{
				// glowstone dust gives flickering
				expTag->putBoolean(FireworksItem::TAG_E_FLICKER, true);
			}
			else if (item->id == Item::diamond_Id)
			{
				// diamonds give trails
				expTag->putBoolean(FireworksItem::TAG_E_TRAIL, true);
			}
			else if (item->id == Item::fireball_Id)
			{
				type = FireworksItem::TYPE_BIG;
			}
			else if (item->id == Item::feather_Id)
			{
				type = FireworksItem::TYPE_BURST;
			}
			else if (item->id == Item::goldNugget_Id)
			{
				type = FireworksItem::TYPE_STAR;
			}
			else if (item->id == Item::skull_Id)
			{
				type = FireworksItem::TYPE_CREEPER;
			}
		}
		intArray colorArray(colors.size());
		for (int i = 0; i < colorArray.length; i++)
		{
			colorArray[i] = colors.at(i);
		}
		expTag->putIntArray(FireworksItem::TAG_E_COLORS, colorArray);

		expTag->putByte(FireworksItem::TAG_E_TYPE, type);

		itemTag->put(FireworksItem::TAG_EXPLOSION, expTag);
		resultItem->setTag(itemTag);
		
		setResultItem(resultItem);
		return true;
	}
	// apply fade colors to firecharge
	if (sulphurCount == 0 && paperCount == 0 && chargeCount == 1 && colorCount > 0 && colorCount == chargeComponents)
	{

		vector<int> colors;
		for (int slot = 0; slot < craftSlots->getContainerSize(); slot++)
		{
			shared_ptr<ItemInstance> item = craftSlots->getItem(slot);
			if (item == NULL) continue;

			if (item->id == Item::dye_powder_Id)
			{
				colors.push_back(DyePowderItem::COLOR_RGB[item->getAuxValue()]);
			}
			else if (item->id == Item::fireworksCharge_Id)
			{
				resultItem = item->copy();
				resultItem->count = 1;
			}
		}
		intArray colorArray(colors.size());
		for (int i = 0; i < colorArray.length; i++)
		{
			colorArray[i] = colors.at(i);
		}
		if (resultItem != NULL && resultItem->hasTag())
		{
			CompoundTag *compound = resultItem->getTag()->getCompound(FireworksItem::TAG_EXPLOSION);
			if (compound == NULL)
			{
				delete colorArray.data;
				
				setResultItem(resultItem);
				return false;
			}
			compound->putIntArray(FireworksItem::TAG_E_FADECOLORS, colorArray);
		}
		else
		{
			delete colorArray.data;
			
			setResultItem(resultItem);
			return false;
		}
		
		setResultItem(resultItem);
		return true;
	}
	
	setResultItem(resultItem);
	return false;
}

shared_ptr<ItemInstance> FireworksRecipe::assemble(shared_ptr<CraftingContainer> craftSlots)
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(tlsIdx);
	return tls->resultItem->copy();
	//return resultItem->copy();
}

int FireworksRecipe::size()
{
	return 10;
}

const ItemInstance *FireworksRecipe::getResultItem()
{
	ThreadStorage *tls = (ThreadStorage *)TlsGetValue(tlsIdx);
	return tls->resultItem.get();
	//return resultItem.get();
}

void FireworksRecipe::updatePossibleRecipes(shared_ptr<CraftingContainer> craftSlots, bool *firework, bool *charge, bool *fade)
{
	*firework = false;
	*charge = false;
	*fade = false;

	int paperCount = 0;
	int sulphurCount = 0;
	int colorCount = 0;
	int chargeCount = 0;
	int chargeComponents = 0;
	int typeComponents = 0;

	for (int slot = 0; slot < craftSlots->getContainerSize(); slot++)
	{
		shared_ptr<ItemInstance> item = craftSlots->getItem(slot);
		if (item == NULL) continue;

		if (item->id == Item::gunpowder_Id)
		{
			sulphurCount++;
		}
		else if (item->id == Item::fireworksCharge_Id)
		{
			chargeCount++;
		}
		else if (item->id == Item::dye_powder_Id)
		{
			colorCount++;
		}
		else if (item->id == Item::paper_Id)
		{
			paperCount++;
		}
		else if (item->id == Item::yellowDust_Id)
		{
			// glowstone dust gives flickering
			chargeComponents++;
		}
		else if (item->id == Item::diamond_Id)
		{
			// diamonds give trails
			chargeComponents++;
		}
		else if (item->id == Item::fireball_Id)
		{
			// fireball gives larger explosion
			typeComponents++;
		}
		else if (item->id == Item::feather_Id)
		{
			// burst
			typeComponents++;
		}
		else if (item->id == Item::goldNugget_Id)
		{
			// star
			typeComponents++;
		}
		else if (item->id == Item::skull_Id)
		{
			// creeper
			typeComponents++;
		}
		else
		{
			return;
		}
	}
	chargeComponents += colorCount + typeComponents;

	if (sulphurCount > 3 || paperCount > 1)
	{
		return;
	}

	// create fireworks
	if ( paperCount <= 1 && chargeComponents == 0 )
	{
		*firework = true;
	}
	// create firecharge
	if ( sulphurCount <= 1 && colorCount >= 0 && paperCount == 0 && chargeCount == 0 && typeComponents <= 1 )
	{
		*charge = true;
	}
	// apply fade colors to firecharge
	if ( sulphurCount == 0 && paperCount == 0 && chargeCount <= 1 && colorCount >= 0 )
	{
		*fade = true;
	}
}

bool FireworksRecipe::isValidIngredient(shared_ptr<ItemInstance> item, bool firework, bool charge, bool fade)
{
	bool valid = false;
	switch(item->id)
	{
	case Item::gunpowder_Id:
		valid = firework || charge;
		break;
	case Item::fireworksCharge_Id:
		valid = firework || fade;
		break;
	case Item::dye_powder_Id:
		valid = charge || fade;
		break;
	case Item::paper_Id:
		valid = firework;
		break;
	case Item::yellowDust_Id:
		valid = charge;
		break;
	case Item::diamond_Id:
		valid = charge;
		break;
	case Item::fireball_Id:
		valid = charge;
		break;
	case Item::feather_Id:
		valid = charge;
		break;
	case Item::goldNugget_Id:
		valid = charge;
		break;
	case Item::skull_Id:
		valid = charge;
		break;
	}
	return valid;
}