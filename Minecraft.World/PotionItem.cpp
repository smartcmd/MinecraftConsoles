#include "stdafx.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.alchemy.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.h"
#include "MobEffectInstance.h"
#include "StringHelpers.h"
#include "SharedConstants.h"
#include "PotionItem.h"
#include "SoundTypes.h"

const wstring PotionItem::DEFAULT_ICON = L"potion";
const wstring PotionItem::THROWABLE_ICON = L"potion_splash";
const wstring PotionItem::CONTENTS_ICON = L"potion_contents";

// 4J Added
vector<pair<int, int> >  PotionItem::s_uniquePotionValues;

PotionItem::PotionItem(int id) : Item(id)
{
	setMaxStackSize(1);
	setStackedByData(true);
	setMaxDamage(0);

	iconThrowable = nullptr;
	iconDrinkable = nullptr;
	iconOverlay = nullptr;
}

vector<MobEffectInstance *> *PotionItem::getMobEffects(shared_ptr<ItemInstance> potion)
{
	if (!potion->hasTag() || !potion->getTag()->contains(L"CustomPotionEffects"))
	{
		vector<MobEffectInstance *> *effects = nullptr;
        auto it = cachedMobEffects.find(potion->getAuxValue());
        if(it != cachedMobEffects.end()) effects = it->second;
		if (effects == nullptr)
		{
			effects = PotionBrewing::getEffects(potion->getAuxValue(), false);
			cachedMobEffects[potion->getAuxValue()] = effects;
		}

		// Result should be a new (unmanaged) vector, so create a new one
		return effects == nullptr ? nullptr : new vector<MobEffectInstance *>(*effects);
	}
	else
	{
		vector<MobEffectInstance *> *effects = new vector<MobEffectInstance *>();
		ListTag<CompoundTag> *customList = (ListTag<CompoundTag> *) potion->getTag()->getList(L"CustomPotionEffects");

		for (int i = 0; i < customList->size(); i++)
		{
			CompoundTag *tag = customList->get(i);
			effects->push_back(MobEffectInstance::load(tag));
		}

		return effects;
	}
}

vector<MobEffectInstance *> *PotionItem::getMobEffects(int auxValue)
{
	vector<MobEffectInstance *> *effects = nullptr;
    auto it = cachedMobEffects.find(auxValue);
    if(it != cachedMobEffects.end()) effects = it->second;
	if (effects == nullptr)
	{
		effects = PotionBrewing::getEffects(auxValue, false);
		if(effects != nullptr) cachedMobEffects.insert( std::pair<int, vector<MobEffectInstance *> *>(auxValue, effects) );
	}
	return effects;
}

shared_ptr<ItemInstance> PotionItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (!player->abilities.instabuild) instance->count--;

	if (!level->isClientSide)
	{
		vector<MobEffectInstance *> *effects = getMobEffects(instance);
		if (effects != nullptr)
		{
			for(auto& effect : *effects)
			{
				player->addEffect(new MobEffectInstance(effect));
			}
		}
	}
	if (!player->abilities.instabuild)
	{
		if (instance->count <= 0)
		{
			return std::make_shared<ItemInstance>(Item::glassBottle);
		}
		else
		{
			player->inventory->add(std::make_shared<ItemInstance>(Item::glassBottle));
		}
	}

	return instance;
}

int PotionItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return DRINK_DURATION;
}

UseAnim PotionItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_drink;
}

bool PotionItem::TestUse(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player)
{
	return true;
}

shared_ptr<ItemInstance> PotionItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (isThrowable(instance->getAuxValue()))
	{
		if (!player->abilities.instabuild) instance->count--;
		level->playEntitySound(player, eSoundType_RANDOM_BOW, 0.5f, 0.4f / (random->nextFloat() * 0.4f + 0.8f));
		if (!level->isClientSide) level->addEntity(std::make_shared<ThrownPotion>(level, player, instance->getAuxValue()));
		return instance;
	}
	player->startUsingItem(instance, getUseDuration(instance));
	return instance;
}

bool PotionItem::useOn(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, Level *level, int x, int y, int z, int face, float clickX, float clickY, float clickZ, bool bTestUseOnOnly)
{
	return false;
}

Icon *PotionItem::getIcon(int auxValue)
{
	if (isThrowable(auxValue))
	{
		return iconThrowable;
	}
	return iconDrinkable;
}

Icon *PotionItem::getLayerIcon(int auxValue, int spriteLayer)
{
	if (spriteLayer == 0)
	{
		return iconOverlay;
	}
	return Item::getLayerIcon(auxValue, spriteLayer);
}

bool PotionItem::isThrowable(int auxValue)
{
	return ((auxValue & PotionBrewing::THROWABLE_MASK) != 0);
}

int PotionItem::getColor(int data)
{
	return PotionBrewing::getColorValue(data, false);
}

int PotionItem::getColor(shared_ptr<ItemInstance> item, int spriteLayer)
{
	if (spriteLayer > 0)
	{
		return 0xffffff;
	}
	return PotionBrewing::getColorValue(item->getAuxValue(), false);
}

bool PotionItem::hasMultipleSpriteLayers()
{
	return true;
}

bool PotionItem::hasInstantenousEffects(int itemAuxValue)
{
	vector<MobEffectInstance *> *mobEffects = getMobEffects(itemAuxValue);
	if (mobEffects == nullptr || mobEffects->empty())
	{
		return false;
	}
	for(auto& effect : *mobEffects)
	{
		if (MobEffect::effects[effect->getId()]->isInstantenous())
		{
			return true;
		}
	}
	return false;
}

wstring PotionItem::getHoverName(shared_ptr<ItemInstance> itemInstance)
{
	if (itemInstance->getAuxValue() == 0)
	{
		return app.GetString(IDS_ITEM_WATER_BOTTLE); // I18n.get("item.emptyPotion.name").trim();
	}

	wstring elementName = Item::getHoverName(itemInstance);
	if (isThrowable(itemInstance->getAuxValue()))
	{
		//elementName = I18n.get("potion.prefix.grenade").trim() + " " + elementName;
		elementName = replaceAll(elementName,L"{*splash*}",app.GetString(IDS_POTION_PREFIX_GRENADE));
	}
	else
	{
		elementName = replaceAll(elementName,L"{*splash*}",L"");
	}

	vector<MobEffectInstance *> *effects = ((PotionItem *) Item::potion)->getMobEffects(itemInstance);
	if (effects != nullptr && !effects->empty())
	{
		//String postfixString = effects.get(0).getDescriptionId();
		//postfixString += ".postfix";
		//return elementName + " " + I18n.get(postfixString).trim();

		elementName = replaceAll(elementName,L"{*prefix*}",L"");
		elementName = replaceAll(elementName,L"{*postfix*}",app.GetString(effects->at(0)->getPostfixDescriptionId()));
	}
	else
	{
		//String appearanceName = PotionBrewing.getAppearanceName(itemInstance.getAuxValue());
		//return I18n.get(appearanceName).trim() + " " + elementName;

		elementName = replaceAll(elementName,L"{*prefix*}",app.GetString( PotionBrewing::getAppearanceName(itemInstance->getAuxValue())));
		elementName = replaceAll(elementName,L"{*postfix*}",L"");
	}
	return elementName;
}

void PotionItem::appendHoverText(shared_ptr<ItemInstance> itemInstance, shared_ptr<Player> player, vector<HtmlString> *lines, bool advanced)
{
	if (itemInstance->getAuxValue() == 0)
	{
		return;
	}
	vector<MobEffectInstance *> *effects = ((PotionItem *) Item::potion)->getMobEffects(itemInstance);
	attrAttrModMap modifiers;
	if (effects != nullptr && !effects->empty())
	{
		//for (MobEffectInstance effect : effects)
		for(auto& effect : *effects)
		{
			wstring effectString = app.GetString( effect->getDescriptionId() );

			MobEffect *mobEffect = MobEffect::effects[effect->getId()];
			unordered_map<Attribute*, AttributeModifier*> *effectModifiers = mobEffect->getAttributeModifiers();

			if (effectModifiers != nullptr && effectModifiers->size() > 0)
			{
				for(auto& it : *effectModifiers)
				{
					// 4J - anonymous modifiers added here are destroyed shortly?
					AttributeModifier *original = it.second;
					AttributeModifier *modifier = new AttributeModifier(mobEffect->getAttributeModifierValue(effect->getAmplifier(), original), original->getOperation());
					modifiers.insert( std::pair<eATTRIBUTE_ID, AttributeModifier*>( it.first->getId(), modifier) );
				}
			}

			// Don't want to delete this (that's a pointer to mobEffects internal vector of modifiers)
			// delete effectModifiers;

			if (effect->getAmplifier() > 0)
			{
				wstring potencyString = L"";
				switch(effect->getAmplifier())
				{
				case 1:
					potencyString = L" ";
					potencyString += app.GetString( IDS_POTION_POTENCY_1 );
					break;
				case 2:
					potencyString = L" ";
					potencyString += app.GetString( IDS_POTION_POTENCY_2 );
					break;
				case 3:
					potencyString = L" ";
					potencyString += app.GetString( IDS_POTION_POTENCY_3 );
					break;
				default:
					potencyString = app.GetString( IDS_POTION_POTENCY_0 );
					break;
				}
				effectString += potencyString; // + I18n.get("potion.potency." + effect.getAmplifier()).trim();
			}
			if (effect->getDuration() > SharedConstants::TICKS_PER_SECOND)
			{
				effectString += L" (" + MobEffect::formatDuration(effect) + L")";
			}

			eMinecraftColour color = eMinecraftColour_NOT_SET;

			if (mobEffect->isHarmful())
			{
				color = eHTMLColor_c;
			}
			else
			{
				color = eHTMLColor_7;
			}

			lines->push_back(HtmlString(effectString, color));
		}
	}
	else
	{
		wstring effectString = app.GetString(IDS_POTION_EMPTY); //I18n.get("potion.empty").trim();

		lines->push_back(HtmlString(effectString, eHTMLColor_7)); //"�7"
	}

	if (!modifiers.empty())
	{
		// Add new line
		lines->emplace_back(L"");
		lines->emplace_back(app.GetString(IDS_POTION_EFFECTS_WHENDRANK), eHTMLColor_5);

		// Add modifier descriptions
		for (auto& modifier : modifiers)
		{
			// 4J: Moved modifier string building to AttributeModifier
			lines->push_back(modifier.second->getHoverText(modifier.first));
		}
	}
}

bool PotionItem::isFoil(shared_ptr<ItemInstance> itemInstance)
{
	vector<MobEffectInstance *> *mobEffects = getMobEffects(itemInstance);
	return mobEffects != nullptr && !mobEffects->empty();
}

unsigned int PotionItem::getUseDescriptionId(shared_ptr<ItemInstance> instance)
{
	int brew = instance->getAuxValue();
	if(brew == 0) return IDS_POTION_DESC_WATER_BOTTLE;
	else if( MACRO_POTION_IS_REGENERATION(brew)) return IDS_POTION_DESC_REGENERATION;
	else if( MACRO_POTION_IS_SPEED(brew)	) return IDS_POTION_DESC_MOVESPEED;
	else if( MACRO_POTION_IS_FIRE_RESISTANCE(brew)) return IDS_POTION_DESC_FIRERESISTANCE;
	else if( MACRO_POTION_IS_INSTANTHEALTH(brew)) return IDS_POTION_DESC_HEAL;
	else if( MACRO_POTION_IS_NIGHTVISION(brew)) return IDS_POTION_DESC_NIGHTVISION;
	else if( MACRO_POTION_IS_INVISIBILITY(brew)) return IDS_POTION_DESC_INVISIBILITY;
	else if( MACRO_POTION_IS_WEAKNESS(brew)) return IDS_POTION_DESC_WEAKNESS;
	else if( MACRO_POTION_IS_STRENGTH(brew)) return IDS_POTION_DESC_DAMAGEBOOST;
	else if( MACRO_POTION_IS_SLOWNESS(brew)) return IDS_POTION_DESC_MOVESLOWDOWN;
	else if( MACRO_POTION_IS_POISON(brew)) return IDS_POTION_DESC_POISON;
	else if( MACRO_POTION_IS_INSTANTDAMAGE(brew)) return IDS_POTION_DESC_HARM;
	return IDS_POTION_DESC_EMPTY;
}

void PotionItem::registerIcons(IconRegister *iconRegister)
{
	iconDrinkable = iconRegister->registerIcon(DEFAULT_ICON);
	iconThrowable = iconRegister->registerIcon(THROWABLE_ICON);
	iconOverlay = iconRegister->registerIcon(CONTENTS_ICON);
}

Icon *PotionItem::getTexture(const wstring &name)
{
	if (name.compare(DEFAULT_ICON) == 0) return Item::potion->iconDrinkable;
	if (name.compare(THROWABLE_ICON) == 0) return Item::potion->iconThrowable;
	if (name.compare(CONTENTS_ICON) == 0) return Item::potion->iconOverlay;
	return nullptr;
}


// 4J Stu - Based loosely on a function that gets added in java much later on (1.3)
vector<pair<int, int> > *PotionItem::getUniquePotionValues()
{
	if (s_uniquePotionValues.empty())
	{
		for (int brew = 0; brew <= PotionBrewing::BREW_MASK; ++brew)
		{
			vector<MobEffectInstance *> *effects = PotionBrewing::getEffects(brew, false);

			if (effects != nullptr)
			{
				if(!effects->empty())
				{
					// 4J Stu - Based on implementation of Java List.hashCode() at http://docs.oracle.com/javase/6/docs/api/java/util/List.html#hashCode()
					// and adding deleting to clear up as we go
					int effectsHashCode = 1;
					for(auto& effect : *effects)
					{
						effectsHashCode = 31*effectsHashCode + (effect ? 0 : effect->hashCode());
						delete effect;
					}

					bool toAdd = true;
					for(auto& it : s_uniquePotionValues)
					{
						// Some potions hash the same (identical effects) but are throwable so account for that
						if(it.first == effectsHashCode && !(!isThrowable(it.second) && isThrowable(brew)) )
						{
							toAdd = false;
							break;
						}
					}
					if( toAdd )
					{
						s_uniquePotionValues.push_back(pair<int,int>(effectsHashCode, brew) );
					}
				}
				delete effects;
			}
		}
	}
	return &s_uniquePotionValues;
}