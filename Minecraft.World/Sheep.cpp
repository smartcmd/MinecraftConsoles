using namespace std;

#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.crafting.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.entity.ai.goal.h"
#include "net.minecraft.world.entity.ai.navigation.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.global.h"
#include "net.minecraft.world.entity.player.h"
#include "Sheep.h"
#include "..\Minecraft.Client\Textures.h"
#include "MobCategory.h"
#include "GenericStats.h"

const float Sheep::COLOR[][3] =
{
	{ 1.0f, 1.0f, 1.0f }, // white
	{ 0.85f, 0.5f, 0.2f }, // orange
	{ 0.7f, 0.3f, 0.85f }, // magenta
	{ 0.4f, 0.6f, 0.85f }, // light blue
	{ 0.9f, 0.9f, 0.2f }, // yellow
	{ 0.5f, 0.8f, 0.1f }, // light green
	{ 0.95f, 0.5f, 0.65f }, // pink
	{ 0.3f, 0.3f, 0.3f }, // gray
	{ 0.6f, 0.6f, 0.6f }, // silver
	{ 0.3f, 0.5f, 0.65f }, // cyan
	{ 0.5f, 0.25f, 0.7f }, // purple
	{ 0.2f, 0.3f, 0.7f }, // blue
	{ 0.4f, 0.3f, 0.2f }, // brown
	{ 0.4f, 0.5f, 0.2f }, // green
	{ 0.6f, 0.2f, 0.2f }, // red
	{ 0.1f, 0.1f, 0.1f }, // black
};

Sheep::Sheep(Level *level) : Animal( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	this->textureIdx = TN_MOB_SHEEP; // 4J - was L"/mob/sheep.png";
	this->setSize(0.9f, 1.3f);

	eatAnimationTick = 0;

	eatTileGoal = new EatTileGoal(this);

	float walkSpeed = 0.23f;
	getNavigation()->setAvoidWater(true);
	goalSelector.addGoal(0, new FloatGoal(this));
	goalSelector.addGoal(1, new PanicGoal(this, 0.38f));
	goalSelector.addGoal(2, new BreedGoal(this, walkSpeed));
	goalSelector.addGoal(3, new TemptGoal(this, 0.25f, Item::wheat_Id, false));
	goalSelector.addGoal(4, new FollowParentGoal(this, 0.25f));
	goalSelector.addGoal(5, eatTileGoal, false);
	goalSelector.addGoal(6, new RandomStrollGoal(this, walkSpeed));
	goalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6));
	goalSelector.addGoal(8, new RandomLookAroundGoal(this));

	container = shared_ptr<CraftingContainer>(new CraftingContainer(new SheepContainer(), 2, 1));
	container->setItem(0, shared_ptr<ItemInstance>( new ItemInstance(Item::dye_powder, 1, 0)));
	container->setItem(1, shared_ptr<ItemInstance>( new ItemInstance(Item::dye_powder, 1, 0)));
}

bool Sheep::useNewAi()
{
	return true;
}

void Sheep::newServerAiStep()
{
	eatAnimationTick = eatTileGoal->getEatAnimationTick();
	Animal::newServerAiStep();
}

void Sheep::aiStep()
{
	if (level->isClientSide) eatAnimationTick = max(0, eatAnimationTick - 1);
	Animal::aiStep();
}

int Sheep::getMaxHealth()
{
	return 8;
}

void Sheep::defineSynchedData() 
{
	Animal::defineSynchedData();

	// sheared and color share a byte
	entityData->define(DATA_WOOL_ID, ((byte) 0)); //was new Byte((byte), 0)
}

void Sheep::dropDeathLoot(bool wasKilledByPlayer, int playerBonusLevel)
{
	if(!isSheared())
	{
		// killing a non-sheared sheep will drop a single block of cloth
		spawnAtLocation(shared_ptr<ItemInstance>( new ItemInstance(Tile::cloth_Id, 1, getColor()) ), 0.0f);
	}
}

int Sheep::getDeathLoot()
{
	return Tile::cloth_Id;
}

void Sheep::handleEntityEvent(byte id)
{
	if (id == EntityEvent::EAT_GRASS)
	{
		eatAnimationTick = EAT_ANIMATION_TICKS;
	}
	else
	{
		Animal::handleEntityEvent(id);
	}
}

float Sheep::getHeadEatPositionScale(float a)
{
	if (eatAnimationTick <= 0)
	{
		return 0;
	}
	if (eatAnimationTick >= 4 && eatAnimationTick <= (EAT_ANIMATION_TICKS - 4))
	{
		return 1;
	}
	if (eatAnimationTick < 4)
	{
		return ((float) eatAnimationTick - a) / 4.0f;
	}
	return -((float) (eatAnimationTick - EAT_ANIMATION_TICKS) - a) / 4.0f;
}

float Sheep::getHeadEatAngleScale(float a)
{
	if (eatAnimationTick > 4 && eatAnimationTick <= (EAT_ANIMATION_TICKS - 4))
	{
		float scale = ((float) (eatAnimationTick - 4) - a) / (float) (EAT_ANIMATION_TICKS - 8);
		return PI * .20f + PI * .07f * Mth::sin(scale * 28.7f);
	}
	if (eatAnimationTick > 0)
	{
		return PI * .20f;
	}
	return ((xRot / (180.0f / PI)));
}

bool Sheep::interact(shared_ptr<Player> player)
{
    shared_ptr<ItemInstance> item = player->inventory->getSelected();

	// 4J-JEV: Fix for #88212,
	// Untrusted players shouldn't be able to sheer sheep.
	if (!player->isAllowedToInteract( shared_from_this() ))
		return false; //Animal::interact(player);

    if (item != NULL && item->id == Item::shears->id && !isSheared() && !isBaby())
	{
        if (!level->isClientSide)
		{
            setSheared(true);
            int count = 1 + random->nextInt(3);
            for (int i = 0; i < count; i++)
			{
                shared_ptr<ItemEntity> ie = spawnAtLocation(shared_ptr<ItemInstance>( new ItemInstance(Tile::cloth_Id, 1, getColor()) ), 1.0f);
                ie->yd += random->nextFloat() * 0.05f;
                ie->xd += (random->nextFloat() - random->nextFloat()) * 0.1f;
                ie->zd += (random->nextFloat() - random->nextFloat()) * 0.1f;
            }

			player->awardStat( GenericStats::shearedEntity(eTYPE_SHEEP), GenericStats::param_shearedEntity(eTYPE_SHEEP) );
        }
        item->hurt(1, player);
    }

    return Animal::interact(player);
}

void Sheep::addAdditonalSaveData(CompoundTag *tag) 
{
	Animal::addAdditonalSaveData(tag);
	tag->putBoolean(L"Sheared", isSheared());
	tag->putByte(L"Color", (byte) getColor());
}

void Sheep::readAdditionalSaveData(CompoundTag *tag) 
{
	Animal::readAdditionalSaveData(tag);
	setSheared(tag->getBoolean(L"Sheared"));
	setColor((int) tag->getByte(L"Color"));
}

int Sheep::getAmbientSound() 
{
	return eSoundType_MOB_SHEEP_AMBIENT;
}

int Sheep::getHurtSound() 
{
	return eSoundType_MOB_SHEEP_AMBIENT;
}

int Sheep::getDeathSound() 
{
	return eSoundType_MOB_SHEEP_AMBIENT;
}

int Sheep::getColor() 
{
	return (entityData->getByte(DATA_WOOL_ID) & 0x0f);
}

void Sheep::setColor(int color) 
{
	byte current = entityData->getByte(DATA_WOOL_ID);
	entityData->set(DATA_WOOL_ID, (byte) ((current & 0xf0) | (color & 0x0f)));
}

bool Sheep::isSheared() 
{
	return (entityData->getByte(DATA_WOOL_ID) & 0x10) != 0;
}

void Sheep::setSheared(bool value) 
{
	byte current = entityData->getByte(DATA_WOOL_ID);
	if (value) 
	{
		entityData->set(DATA_WOOL_ID, (byte) (current | 0x10));
	} 
	else 
	{
		entityData->set(DATA_WOOL_ID, (byte) (current & ~0x10));
	}
}

int Sheep::getSheepColor(Random *random) 
{
	int nextInt = random->nextInt(100);
	if (nextInt < 5) 
	{
		return 15 - DyePowderItem::BLACK;
	}
	if (nextInt < 10) 
	{
		return 15 - DyePowderItem::GRAY;
	}
	if (nextInt < 15) 
	{
		return 15 - DyePowderItem::SILVER;
	}
	if (nextInt < 18) 
	{
		return 15 - DyePowderItem::BROWN;
	}
	if (random->nextInt(500) == 0) return 15 - DyePowderItem::PINK;
	return 0; // white
}

shared_ptr<AgableMob> Sheep::getBreedOffspring(shared_ptr<AgableMob> target)
{
	// 4J - added limit to number of animals that can be bred
	if( level->canCreateMore( GetType(), Level::eSpawnType_Breed) )
	{
		shared_ptr<Sheep> otherSheep = dynamic_pointer_cast<Sheep>( target );
		shared_ptr<Sheep> sheep = shared_ptr<Sheep>( new Sheep(level) );
		int color = getOffspringColor(dynamic_pointer_cast<Animal>(shared_from_this()), otherSheep);
		sheep->setColor(15 - color);
		return sheep;
	}
	else
	{
		return nullptr;
	}
}

void Sheep::ate()
{
	setSheared(false);
	if (isBaby())
	{
		// remove a minute from aging
		int age = getAge() + SharedConstants::TICKS_PER_SECOND * 60;
		if (age > 0)
		{
			age = 0;
		}
		setAge(age);
	}
}

void Sheep::finalizeMobSpawn()
{
	setColor(Sheep::getSheepColor(level->random));
}

int Sheep::getOffspringColor(shared_ptr<Animal> animal, shared_ptr<Animal> partner)
{
	int parent1DyeColor = getDyeColor(animal);
	int parent2DyeColor = getDyeColor(partner);

	container->getItem(0)->setAuxValue(parent1DyeColor);
	container->getItem(1)->setAuxValue(parent2DyeColor);

	shared_ptr<ItemInstance> instance = Recipes::getInstance()->getItemFor(container, animal->level);

	int color = 0;
	if (instance != NULL && instance->getItem()->id == Item::dye_powder_Id)
	{
		color = instance->getAuxValue();
	}
	else
	{
		color = level->random->nextBoolean() ? parent1DyeColor : parent2DyeColor;
	}
	return color;
}

int Sheep::getDyeColor(shared_ptr<Animal> animal)
{
	return 15 - dynamic_pointer_cast<Sheep>(animal)->getColor();
}
