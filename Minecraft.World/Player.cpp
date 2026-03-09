// 4J TODO

// All the instanceof s from Java have been converted to dynamic_cast in this file
// Once all the classes are finished it may be that we do not need to use dynamic_cast
// for every test and a simple virtual function should suffice. We probably only need
// dynamic_cast to find one of the classes that an object derives from, and not to find
// the derived class itself (which should own the virtual GetType function)

#include "stdafx.h"
#include "JavaMath.h"
#include "net.minecraft.h"
#include "net.minecraft.world.h"
#include "net.minecraft.stats.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.chunk.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.boss.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
#include "net.minecraft.world.scores.h"
#include "net.minecraft.world.scores.criteria.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.inventory.h"
#include "net.minecraft.world.damagesource.h"
#include "net.minecraft.world.effect.h"
#include "net.minecraft.world.food.h"
#include "Inventory.h"
#include "Player.h"
#include "ParticleTypes.h"

#include "..\Minecraft.Client\Textures.h"

#include "..\Minecraft.Client\LocalPlayer.h"
#include "..\Minecraft.Client\HumanoidModel.h"
#include "SoundTypes.h"



void Player::_init()
{
	registerAttributes();
	setHealth(getMaxHealth());

	inventory = std::make_shared<Inventory>(this);

	userType = 0;
	oBob = bob = 0.0f;

	xCloakO = yCloakO = zCloakO = 0.0;
	xCloak = yCloak = zCloak = 0.0;

	m_isSleeping = false;

	customTextureUrl = L"";
	customTextureUrl2 = L"";
	m_uiPlayerCurrentSkin=0;

	bedPosition = nullptr;

	sleepCounter = 0;
	deathFadeCounter=0;

	bedOffsetX = bedOffsetY = bedOffsetZ = 0.0f;
	stats = nullptr;

	respawnPosition = nullptr;
	respawnForced = false;
	minecartAchievementPos = nullptr;

	fishing = nullptr;

	distanceWalk = distanceSwim = distanceFall = distanceClimb = distanceMinecart = distanceBoat = distancePig = 0;

	m_uiDebugOptions=0L;

	jumpTriggerTime = 0;
	takeXpDelay = 0;
	experienceLevel = totalExperience = 0;
	experienceProgress = 0.0f;

	useItem = nullptr;
	useItemDuration = 0;

	defaultWalkSpeed = 0.1f;
	defaultFlySpeed = 0.02f;

	lastLevelUpTime = 0;

	m_uiGamePrivileges = 0;

	m_ppAdditionalModelParts=nullptr;
	m_bCheckedForModelParts=false;
	m_bCheckedDLCForModelParts=false;

#if defined(__PS3__) || defined(__ORBIS__)
	m_ePlayerNameValidState=ePlayerNameValid_NotSet;
#endif

	enderChestInventory = std::make_shared<PlayerEnderChestContainer>();

	m_bAwardedOnARail=false;
}

Player::Player(Level *level, const wstring &name) : LivingEntity( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	this->name = name;

	_init();
	MemSect(11);
	inventoryMenu = new InventoryMenu(inventory, !level->isClientSide, this);
	MemSect(0);

	containerMenu = inventoryMenu;

	heightOffset = 1.62f;
	Pos *spawnPos = level->getSharedSpawnPos();
	moveTo(spawnPos->x + 0.5, spawnPos->y + 1, spawnPos->z + 0.5, 0, 0);
	delete spawnPos;

	rotOffs = 180;
	flameTime = 20;

	m_skinIndex = eDefaultSkins_Skin0;
	m_playerIndex = 0;
	m_dwSkinId = 0;
	m_dwCapeId = 0;

	// 4J Added
	m_xuid = INVALID_XUID;
	m_OnlineXuid = INVALID_XUID;
	//m_bShownOnMaps = true;
	setShowOnMaps(app.GetGameHostOption(eGameHostOption_Gamertags)!=0?true:false);
	m_bIsGuest = false;

#ifndef _XBOX_ONE
	// 4J: Set UUID to name on none-XB1 consoles, may change in future but for now
	// ownership of animals on these consoles is done by name
	setUUID(name);
#endif
}

Player::~Player()
{
	// TODO 4J
	//printf("A player has been destroyed.\n");
	delete inventoryMenu;

	// 4J Stu - Fix for #10938 - CRASH - Game hardlocks when client has an open chest and Xbox Guide while host exits without saving.
	// If the container menu is not the inventory menu, then the player has a menu open. These get deleted when the xui scene
	// is destroyed, so we can not delete it here
	//if( containerMenu != inventoryMenu ) delete containerMenu;
}

void Player::registerAttributes()
{
	LivingEntity::registerAttributes();

	getAttributes()->registerAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->setBaseValue(1);
}

void Player::defineSynchedData()
{
	LivingEntity::defineSynchedData();

	entityData->define(DATA_PLAYER_FLAGS_ID, static_cast<byte>(0));
	entityData->define(DATA_PLAYER_ABSORPTION_ID, static_cast<float>(0));
	entityData->define(DATA_SCORE_ID, (int) 0);
}

shared_ptr<ItemInstance> Player::getUseItem()
{
	return useItem;
}

int Player::getUseItemDuration()
{
	return useItemDuration;
}

bool Player::isUsingItem()
{
	return useItem != nullptr;
}

int Player::getTicksUsingItem()
{
	if (isUsingItem())
	{
		return useItem->getUseDuration() - useItemDuration;
	}
	return 0;
}

void Player::releaseUsingItem()
{
	if (useItem != nullptr)
	{
		useItem->releaseUsing(level, dynamic_pointer_cast<Player>( shared_from_this() ), useItemDuration);

		// 4J Stu - Fix for various bugs where an incorrect bow was displayed when it broke (#70859,#93972,#93974)
		if (useItem->count == 0)
		{
			removeSelectedItem();
		}
	}
	stopUsingItem();
}

void Player::stopUsingItem()
{
	useItem = nullptr;
	useItemDuration = 0;
	if (!level->isClientSide)
	{
		setUsingItemFlag(false);
	}
}

bool Player::isBlocking()
{
	return isUsingItem() && Item::items[useItem->id]->getUseAnimation(useItem) == UseAnim_block;
}

// 4J Stu - Added for things that should only be ticked once per simulation frame
void Player::updateFrameTick()
{
	if (useItem != nullptr)
	{
		shared_ptr<ItemInstance> item = inventory->getSelected();
		// 4J Stu - Fix for #45508 - TU5: Gameplay: Eating one piece of food will result in a second piece being eaten as well
		// Original code was item != useItem. Changed this now to use the equals function, and add the nullptr check as well for the other possible not equals (useItem is not nullptr if we are here)
		// This is because the useItem and item could be different objects due to an inventory update from the server, but still be the same item (with the same id,count and auxvalue)
		if (item == nullptr || !item->equals(useItem) )
		{
			stopUsingItem();
		}
		else
		{
			if (useItemDuration <= 25 && useItemDuration % 4 == 0)
			{
				spawnEatParticles(item, 5);
			}
			if (--useItemDuration == 0)
			{
				if (!level->isClientSide)
				{
					completeUsingItem();
				}
			}
		}
	}

	if (takeXpDelay > 0) takeXpDelay--;

	if (isSleeping())
	{
		sleepCounter++;
		if (sleepCounter > SLEEP_DURATION)
		{
			sleepCounter = SLEEP_DURATION;
		}

		if (!level->isClientSide)
		{
			if (!checkBed())
			{
				stopSleepInBed(true, true, false);
			}
			else if (level->isDay())
			{
				stopSleepInBed(false, true, true);
			}
		}
	}
	else if (sleepCounter > 0)
	{
		sleepCounter++;
		if (sleepCounter >= (SLEEP_DURATION + WAKE_UP_DURATION))
		{
			sleepCounter = 0;
		}
	}

	if(!isAlive())
	{
		deathFadeCounter++;
		if (deathFadeCounter > DEATHFADE_DURATION)
		{
			deathFadeCounter = DEATHFADE_DURATION;
		}
	}
}

void Player::tick()
{
	if(level->isClientSide)
	{
		// 4J Stu - Server player calls this differently so that it only happens once per simulation tick
		updateFrameTick();
	}

	LivingEntity::tick();

	if (!level->isClientSide)
	{
		if (containerMenu != nullptr && !containerMenu->stillValid( dynamic_pointer_cast<Player>( shared_from_this() ) ))
		{
			closeContainer();
			containerMenu = inventoryMenu;
		}
	}

	if (isOnFire() && (abilities.invulnerable || hasInvulnerablePrivilege() ) )
	{
		clearFire();
	}

	xCloakO = xCloak;
	yCloakO = yCloak;
	zCloakO = zCloak;

	double xca = x - xCloak;
	double yca = y - yCloak;
	double zca = z - zCloak;

	double m = 10;
	if (xca > m) xCloakO = xCloak = x;
	if (zca > m) zCloakO = zCloak = z;
	if (yca > m) yCloakO = yCloak = y;
	if (xca < -m) xCloakO = xCloak = x;
	if (zca < -m) zCloakO = zCloak = z;
	if (yca < -m) yCloakO = yCloak = y;

	xCloak += xca * 0.25;
	zCloak += zca * 0.25;
	yCloak += yca * 0.25;

	if (riding == nullptr)
	{
		if( minecartAchievementPos != nullptr )
		{
			delete minecartAchievementPos;
			minecartAchievementPos = nullptr;
		}
	}

	if (!level->isClientSide)
	{
		foodData.tick(dynamic_pointer_cast<Player>(shared_from_this()));
	}

	// 4J Stu Debugging
	if (!level->isClientSide)
	{
		static int count = 0;
		if( count++ == 100 )
		{
#if 0
#ifdef _WINDOWS64
			// Drop some items so we have them in inventory to play with
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Tile::recordPlayer) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::map) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::record_01) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::record_02) ) );
			this->drop( shared_ptr<ItemInstance>(new ItemInstance( Item::pickAxe_diamond, 1 )) );
#endif

#ifdef __PS3__
			// #ifdef _DEBUG
			// 		// Drop some items so we have them in inventory to play with
			// 		this->drop( shared_ptr<ItemInstance>( new ItemInstance(Tile::recordPlayer) ) );
			// 		this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::map) ) );
			// 		this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::record_01) ) );
			// 		this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::record_02) ) );
			// 		this->drop( shared_ptr<ItemInstance>(new ItemInstance( Item::pickAxe_diamond, 1 )) );
			// #endif
#endif

#ifdef _DURANGO
			// Drop some items so we have them in inventory to play with
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Tile::recordPlayer) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::map) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::record_01) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance(Item::record_02) ) );
			this->drop( shared_ptr<ItemInstance>(new ItemInstance( Item::pickAxe_diamond, 1 )) );
#endif
#endif
			// 4J-PB - Throw items out at the start of the level
			//this->drop( new ItemInstance( Item::pickAxe_diamond, 1 ) );
			//this->drop( new ItemInstance( Tile::workBench, 1 ) );
			//this->drop( new ItemInstance( Tile::treeTrunk, 8 ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Item::milk, 3 ) ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Item::sugar, 2 ) ) );
			//this->drop( new ItemInstance( Tile::stoneBrick, 8 ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Item::wheat, 3 ) ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Item::egg, 1 ) ) );
			//this->drop( new ItemInstance( Item::bow, 1 ) );
			//this->drop( new ItemInstance( Item::arrow, 10 ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Item::saddle, 10 ) ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Tile::fence, 64 ) ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Tile::fence, 64 ) ) );
			//this->drop( shared_ptr<ItemInstance>( new ItemInstance( Tile::fence, 64 ) ) );


			//shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(Pig::_class->newInstance( level ));
			//mob->moveTo(x+1, y, z+1, level->random->nextFloat() * 360, 0);
			//level->addEntity(mob);

			// 4J : WESTY : Spawn some wolves to befriend!
			/*
			shared_ptr<Mob> mob1 = dynamic_pointer_cast<Mob>(Wolf::_class->newInstance( level ));
			mob1->moveTo(x+1, y, z+1, level->random->nextFloat() * 360, 0);
			level->addEntity(mob1);

			shared_ptr<Mob> mob2 = dynamic_pointer_cast<Mob>(Wolf::_class->newInstance( level ));
			mob2->moveTo(x+2, y, z+1, level->random->nextFloat() * 360, 0);
			level->addEntity(mob2);

			shared_ptr<Mob> mob3 = dynamic_pointer_cast<Mob>(Wolf::_class->newInstance( level ));
			mob3->moveTo(x+1, y, z+2, level->random->nextFloat() * 360, 0);
			level->addEntity(mob3);

			shared_ptr<Mob> mob4 = dynamic_pointer_cast<Mob>(Wolf::_class->newInstance( level ));
			mob4->moveTo(x+3, y, z+1, level->random->nextFloat() * 360, 0);
			level->addEntity(mob4);

			shared_ptr<Mob> mob5 = dynamic_pointer_cast<Mob>(Wolf::_class->newInstance( level ));
			mob5->moveTo(x+1, y, z+3, level->random->nextFloat() * 360, 0);
			level->addEntity(mob5);
			*/

			//        inventory.add(new ItemInstance(Item.potion, 1, PotionBrewing.THROWABLE_MASK | 0xc));
			//        addEffect(new MobEffectInstance(MobEffect.blindness.id, 60));
			//        increaseXp(10);

			{
				//            ItemInstance itemInstance = new ItemInstance(Item.pickAxe_diamond);
				//            itemInstance.enchant(Enchantment.diggingBonus, 3);
				//            inventory.add(itemInstance);
			}
		}
#if 0
		// 4J Stu - This makes a tunnel with a powered track just over length to get the On A Rail achievement
		// It needs a few items at the start to get you going (a level and some powered rails) and of course a
		// minecart. For some reason some of the torches come off so it will also need some fixing along the way.
		static bool madeTrack = false;
		if( !madeTrack )
		{
			this->drop( shared_ptr<ItemInstance>( new ItemInstance( Item::minecart, 1 ) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance( Tile::goldenRail, 10 ) ) );
			this->drop( shared_ptr<ItemInstance>( new ItemInstance( Tile::lever, 10 ) ) );

			int poweredCount = 0;
			for(int i = 10; i < 2800; ++i)
			{
				level->setTileAndData(x+i,y-1,z-2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y,z-2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+1,z-2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+2,z-2,Tile::glowstone_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+3,z-2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);

				level->setTileAndData(x+i,y-1,z-1,Tile::stoneBrick_Id,0,Tile::UPDATE_CLIENTS);
				if(i%20 == 0)
				{
					level->setTileAndData(x+i,y,z-1,Tile::redstoneTorch_on_Id,0,Tile::UPDATE_CLIENTS);
					poweredCount = 4;
				}
				else
				{
					level->setTileAndData(x+i,y,z-1,0,0,Tile::UPDATE_CLIENTS);
				}
				level->setTileAndData(x+i,y+1,z-1,0,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+2,z-1,0,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+3,z-1,0,0,Tile::UPDATE_CLIENTS);

				level->setTileAndData(x+i,y-1,z,Tile::stoneBrick_Id,0,Tile::UPDATE_CLIENTS);
				if(poweredCount>0)
				{
					level->setTileAndData(x+i,y,z,Tile::goldenRail_Id,0,Tile::UPDATE_CLIENTS);
					--poweredCount;
				}
				else
				{
					level->setTileAndData(x+i,y,z,Tile::rail_Id,0,Tile::UPDATE_CLIENTS);
				}
				level->setTileAndData(x+i,y+1,z,0,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+2,z,0,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+3,z,0,0,Tile::UPDATE_CLIENTS);

				level->setTileAndData(x+i,y-1,z+1,Tile::stoneBrick_Id,0,Tile::UPDATE_CLIENTS);
				if((i+5)%20 == 0)
				{
					level->setTileAndData(x+i,y,z+1,Tile::torch_Id,0,Tile::UPDATE_CLIENTS);
				}
				else
				{
					level->setTileAndData(x+i,y,z+1,0,0,Tile::UPDATE_CLIENTS);
				}
				level->setTileAndData(x+i,y+1,z+1,0,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+2,z+1,0,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+3,z+1,0,0,Tile::UPDATE_CLIENTS);

				level->setTileAndData(x+i,y-1,z+2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y,z+2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+1,z+2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+2,z+2,Tile::glowstone_Id,0,Tile::UPDATE_CLIENTS);
				level->setTileAndData(x+i,y+3,z+2,Tile::quartzBlock_Id,0,Tile::UPDATE_CLIENTS);
			}
			madeTrack = true;
		}
#endif
	}
	//End 4J sTU
}

int Player::getPortalWaitTime()
{
	return abilities.invulnerable ? 0 : SharedConstants::TICKS_PER_SECOND * 4;
}

int Player::getDimensionChangingDelay()
{
	return SharedConstants::TICKS_PER_SECOND / 2;
}

void Player::playSound(int iSound, float volume, float pitch)
{
	// this sound method will play locally for the local player, and
	// broadcast to remote players
	level->playPlayerSound(dynamic_pointer_cast<Player>(shared_from_this()), iSound, volume, pitch);
}

void Player::spawnEatParticles(shared_ptr<ItemInstance> useItem, int count)
{
	if (useItem->getUseAnimation() == UseAnim_drink)
	{
		playSound(eSoundType_RANDOM_DRINK, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
	}
	if (useItem->getUseAnimation() == UseAnim_eat)
	{
		for (int i = 0; i < count; i++)
		{
			Vec3 *d = Vec3::newTemp((random->nextFloat() - 0.5) * 0.1, Math::random() * 0.1 + 0.1, 0);

			d->xRot(-xRot * PI / 180);
			d->yRot(-yRot * PI / 180);

			Vec3 *p = Vec3::newTemp((random->nextFloat() - 0.5) * 0.3, -random->nextFloat() * 0.6 - 0.3, 0.6);
			p->xRot(-xRot * PI / 180);
			p->yRot(-yRot * PI / 180);
			p = p->add(x, y + getHeadHeight(), z);

			level->addParticle(PARTICLE_ICONCRACK(useItem->getItem()->id,0), p->x, p->y, p->z, d->x, d->y + 0.05, d->z);
		}

		// 4J Stu - Was L"mob.eat" which doesnt exist
		playSound(eSoundType_RANDOM_EAT, 0.5f + 0.5f * random->nextInt(2), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
	}
}

void Player::completeUsingItem()
{
	if (useItem != nullptr)
	{
		spawnEatParticles(useItem, 16);

		int oldCount = useItem->count;
		shared_ptr<ItemInstance> itemInstance = useItem->useTimeDepleted(level, dynamic_pointer_cast<Player>(shared_from_this()));
		if (itemInstance != useItem || (itemInstance != nullptr && itemInstance->count != oldCount))
		{
			inventory->items[inventory->selected] = itemInstance;
			if (itemInstance->count == 0)
			{
				inventory->items[inventory->selected] = nullptr;
			}
		}
		stopUsingItem();
	}
}

void Player::handleEntityEvent(byte id)
{
	if (id == EntityEvent::USE_ITEM_COMPLETE)
	{
		completeUsingItem();
	}
	else
	{
		LivingEntity::handleEntityEvent(id);
	}
}

bool Player::isImmobile()
{
	return getHealth() <= 0 || isSleeping();
}

void Player::closeContainer()
{
	containerMenu = inventoryMenu;
}

void Player::ride(shared_ptr<Entity> e)
{
	if (riding != nullptr && e == nullptr)
	{
		if (!level->isClientSide) findStandUpPosition(riding);

		if (riding != nullptr)
		{
			riding->rider = weak_ptr<Entity>();
		}
		riding = nullptr;

		return;
	}
	this->abilities.flying = false;
	LivingEntity::ride(e);
}

void Player::setPlayerDefaultSkin(EDefaultSkins skin)
{
#ifndef _CONTENT_PACKAGE
	wprintf(L"Setting default skin to %d for player %ls\n", skin, name.c_str() );
#endif
	m_skinIndex = skin;
}

void Player::setCustomSkin(DWORD skinId)
{
#ifndef _CONTENT_PACKAGE
	wprintf(L"Attempting to set skin to %08X for player %ls\n", skinId, name.c_str() );
#endif
	EDefaultSkins playerSkin = eDefaultSkins_ServerSelected;

	// reset the idle
	setIsIdle(false);

	setAnimOverrideBitmask(getSkinAnimOverrideBitmask(skinId));
	if( !GET_IS_DLC_SKIN_FROM_BITMASK(skinId) )
	{
		// GET_UGC_SKIN_ID_FROM_BITMASK will always be zero - this was for a possible custom skin editor skin
		DWORD ugcSkinIndex = GET_UGC_SKIN_ID_FROM_BITMASK(skinId);
		DWORD defaultSkinIndex = GET_DEFAULT_SKIN_ID_FROM_BITMASK(skinId);
		if( ugcSkinIndex == 0 && defaultSkinIndex > 0 )
		{
			playerSkin = static_cast<EDefaultSkins>(defaultSkinIndex);
		}
	}

	if( playerSkin == eDefaultSkins_ServerSelected)
	{
		playerSkin = static_cast<EDefaultSkins>(m_playerIndex + 1);
	}

	// We always set a default skin, since we may be waiting for the player's custom skin to be transmitted
	setPlayerDefaultSkin( playerSkin );

	m_dwSkinId = skinId;
	this->customTextureUrl = app.getSkinPathFromId(skinId);

	// set the new player additional boxes
	/*vector<ModelPart *> *pvModelParts=app.GetAdditionalModelParts(m_dwSkinId);

	if(pvModelParts==nullptr)
	{
	// we don't have the data from the dlc skin yet
	app.DebugPrintf("Couldn't get model parts for skin %X\n",m_dwSkinId);

	// do we have it from the DLC pack?
	DLCSkinFile *pDLCSkinFile = app.m_dlcManager.getSkinFile(this->customTextureUrl);

	if(pDLCSkinFile!=nullptr)
	{
	DWORD dwBoxC=pDLCSkinFile->getAdditionalBoxesCount();
	if(dwBoxC!=0)
	{
	app.DebugPrintf("Got model parts from DLCskin for skin %X\n",m_dwSkinId);
	pvModelParts=app.SetAdditionalSkinBoxes(m_dwSkinId,pDLCSkinFile->getAdditionalBoxes());
	this->SetAdditionalModelParts(pvModelParts);
	}
	else
	{
	this->SetAdditionalModelParts(nullptr);
	}
	app.SetAnimOverrideBitmask(pDLCSkinFile->getSkinID(),pDLCSkinFile->getAnimOverrideBitmask());
	}
	else
	{
	this->SetAdditionalModelParts(nullptr);
	}
	}
	else
	{
	app.DebugPrintf("Got model parts from app.GetAdditionalModelParts for skin %X\n",m_dwSkinId);

	this->SetAdditionalModelParts(pvModelParts);
	}*/

	// reset the check for model parts
	m_bCheckedForModelParts=false;
	m_bCheckedDLCForModelParts=false;
	this->SetAdditionalModelParts(nullptr);


}

unsigned int Player::getSkinAnimOverrideBitmask(DWORD skinId)
{
	unsigned long bitmask = 0L;
	if( GET_IS_DLC_SKIN_FROM_BITMASK(skinId) )
	{
		// Temp check for anim override
		switch( GET_DLC_SKIN_ID_FROM_BITMASK(skinId) )
		{
		case 0x2://SP1_ZOMBIE:
		case 0x3://SP1_HEROBRINE:
		case 0xc8://SP3_ZOMBIE_PIGMAN:
		case 0xc9://SP3_ZOMBIE_HEROBRINE:
		case 0x1f8: // SPH_4JMUMMY
		case 0x220: // SPH_AOT_MUMMY
		case 0x23a: // SPH_CLIMAX_ZOMBIEBUSINESSMAN
		case 0x23d: // SPH_CLIMAX_EVILROBOT
		case 0x247: // SPH_CLIMAX_ZOMBIE
		case 0x194: // SOA_DEADLIGHT_SKINNY_ZOMBIE
		case 0x195: // SOA_DEADLIGHT_FEMALE_ZOMBIE
			bitmask = 1<<HumanoidModel::eAnim_ArmsOutFront;
			break;
		case 0x1fa://SPH_GHOST:
			bitmask = 1<<HumanoidModel::eAnim_ArmsOutFront | 1<<HumanoidModel::eAnim_NoLegAnim;
			break;
		case 0x1f4://SPH_GRIMREAPER:
			bitmask = 1<<HumanoidModel::eAnim_ArmsDown | 1<<HumanoidModel::eAnim_NoLegAnim;
			break;
		case 0x1f7: // SPH_4J_FRANKENSTEIN
			//bitmask = 1<<HumanoidModel::eAnim_HasIdle;
			break;
			break;
		default:
			// This is not one of the prefined skins
			// Does the app have an anim override for this skin?
			bitmask=app.GetAnimOverrideBitmask(skinId);
			break;
		}
	}
	return bitmask;
}

void Player::setXuid(PlayerUID xuid)
{
	m_xuid = xuid;
#ifdef _XBOX_ONE
	// 4J Stu - For XboxOne (and probably in the future all other platforms) we store a UUID for the player to use as the owner key for tamed animals
	// This should just be a string version of the xuid

	setUUID( xuid.toString() );
#endif
}

void Player::setCustomCape(DWORD capeId)
{
#ifndef _CONTENT_PACKAGE
	wprintf(L"Attempting to set cape to %08X for player %s\n", capeId, name.c_str() );
#endif

	m_dwCapeId = capeId;

	if(capeId > 0)
	{
		this->customTextureUrl2 = Player::getCapePathFromId(capeId);
	}
	else
	{
		MOJANG_DATA *pMojangData=app.GetMojangDataForXuid(getOnlineXuid());
		if(pMojangData)
		{
			// Cape
			if(pMojangData->wchCape)
			{
				this->customTextureUrl2= pMojangData->wchCape;
			}
			else
			{
				if(app.DefaultCapeExists())
				{
					this->customTextureUrl2= wstring(L"Special_Cape.png");
				}
				else
				{
					this->customTextureUrl2= wstring(L"");
				}
			}

		}
		else
		{
			// if there is a custom default cloak, then set it here
			if(app.DefaultCapeExists())
			{
				this->customTextureUrl2= wstring(L"Special_Cape.png");
			}
			else
			{
				this->customTextureUrl2 =wstring(L"");
			}
		}
	}
}

DWORD Player::getCapeIdFromPath(const wstring &cape)
{
	bool dlcCape = false;
	unsigned int capeId = 0;

	if(cape.size() >= 14)
	{
		dlcCape = cape.substr(0,3).compare(L"dlc") == 0;

		wstring capeValue = cape.substr(7,cape.size());
		capeValue = capeValue.substr(0,capeValue.find_first_of(L'.'));

		std::wstringstream ss;
		// 4J Stu - dlc skins are numbered using decimal to make it easier for artists/people to number manually
		// Everything else is numbered using hex
		if(dlcCape)
			ss << std::dec << capeValue.c_str();
		else
			ss << std::hex << capeValue.c_str();
		ss >> capeId;

		capeId = MAKE_SKIN_BITMASK(dlcCape, capeId);
	}
	return capeId;
}

wstring Player::getCapePathFromId(DWORD capeId)
{
	// 4J Stu - This function maps the encoded DWORD we store in the player profile
	// to a filename that is stored as a memory texture and shared between systems in game
	wchar_t chars[256];
	if( GET_IS_DLC_SKIN_FROM_BITMASK(capeId) )
	{
		// 4J Stu - DLC skins are numbered using decimal rather than hex to make it easier to number manually
		swprintf(chars,256,L"dlccape%08d.png",GET_DLC_SKIN_ID_FROM_BITMASK(capeId));

	}
	else
	{
		DWORD ugcCapeIndex = GET_UGC_SKIN_ID_FROM_BITMASK(capeId);
		DWORD defaultCapeIndex = GET_DEFAULT_SKIN_ID_FROM_BITMASK(capeId);
		if( ugcCapeIndex == 0 )
		{
			swprintf(chars,256,L"defcape%08X.png",defaultCapeIndex);
		}
		else
		{
			swprintf(chars,256,L"ugccape%08X.png",ugcCapeIndex);
		}
	}
	return chars;
}

void Player::ChangePlayerSkin()
{

	if(app.vSkinNames.size()>0)
	{

		m_uiPlayerCurrentSkin++;
		if(m_uiPlayerCurrentSkin>app.vSkinNames.size())
		{
			m_uiPlayerCurrentSkin=0;
			this->customTextureUrl=L"";
		}
		else
		{
			if(m_uiPlayerCurrentSkin>0)
			{
				// change this players custom texture url
				this->customTextureUrl=app.vSkinNames[m_uiPlayerCurrentSkin-1];
			}
		}
	}
}

void Player::prepareCustomTextures()
{
	MOJANG_DATA *pMojangData=app.GetMojangDataForXuid(getOnlineXuid());

	if(pMojangData)
	{
		// Skin
		if(pMojangData->wchSkin)
		{
			this->customTextureUrl= pMojangData->wchSkin;
		}

		// 4J Stu - Don't update the cape here, it gets set elsewhere
		// Cape
		//if(pMojangData->wchCape)
		//{
		//	this->customTextureUrl2= pMojangData->wchCape;
		//}
		//else
		//{
		//	if(app.DefaultCapeExists())
		//	{
		//		this->customTextureUrl2= wstring(L"Default_Cape.png");
		//	}
		//	else
		//	{
		//		this->customTextureUrl2= wstring(L"");
		//	}
		//}

	}
	else
	{
		// 4J Stu - Don't update the cape here, it gets set elsewhere
		// if there is a custom default cloak, then set it here
		//if(app.DefaultCapeExists())
		//{
		//	this->customTextureUrl2= wstring(L"Default_Cape.png");
		//}
		//else
		//{
		//	this->customTextureUrl2 =wstring(L"");
		//}
	}

	/*cloakTexture = wstring(L"http://s3.amazonaws.com/MinecraftCloaks/").append( name ).append( L".png" );*/
	//this->customTextureUrl2 = cloakTexture;
}

void Player::rideTick()
{
	if (!level->isClientSide && isSneaking())
	{
		ride(nullptr);
		setSneaking(false);
		return;
	}

	double preX = x, preY = y, preZ = z;
	float preYRot = yRot, preXRot = xRot;

	LivingEntity::rideTick();
	oBob = bob;
	bob = 0;

	checkRidingStatistiscs(x - preX, y - preY, z - preZ);

	// riding can be set to null inside 'Entity::rideTick()'.
	if ( riding != nullptr	&& (riding->GetType() & eTYPE_PIG) == eTYPE_PIG )
	{
		// 4J Stu - I don't know why we would want to do this, but it means that the players head is locked in position and can't move around
		//xRot = preXRot;
		//yRot = preYRot;

		shared_ptr<Pig> pig = dynamic_pointer_cast<Pig>(riding);
		yBodyRot = pig->yBodyRot;

		while (yBodyRot - yBodyRotO < -180)
			yBodyRotO -= 360;
		while (yBodyRot - yBodyRotO >= 180)
			yBodyRotO += 360;
	}
}


void Player::resetPos()
{
	heightOffset = 1.62f;
	setSize(0.6f, 1.8f);
	LivingEntity::resetPos();
	setHealth(getMaxHealth());
	deathTime = 0;
}

void Player::serverAiStep()
{
	LivingEntity::serverAiStep();
	updateSwingTime();
}


void Player::aiStep()
{
	if (jumpTriggerTime > 0) jumpTriggerTime--;

	if (level->difficulty == Difficulty::PEACEFUL && getHealth() < getMaxHealth() && level->getGameRules()->getBoolean(GameRules::RULE_NATURAL_REGENERATION))
	{
		if (tickCount % 20 * 12 == 0) heal(1);
	}
	inventory->tick();
	oBob = bob;

	LivingEntity::aiStep();

	AttributeInstance *speed = getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED);
	if (!level->isClientSide) speed->setBaseValue(abilities.getWalkingSpeed());
	flyingSpeed = defaultFlySpeed;
	if (isSprinting())
	{
		flyingSpeed += defaultFlySpeed * 0.3f;
	}

	setSpeed(static_cast<float>(speed->getValue()));

	float tBob = static_cast<float>(sqrt(xd * xd + zd * zd));

	// 4J added - we were getting a NaN with zero xd & zd
	if(( xd * xd + zd * zd ) < 0.00001f )
	{
		tBob = 0.0f;
	}

	float tTilt = static_cast<float>(atan(-yd * 0.2f)) * 15.0f;
	if (tBob > 0.1f) tBob = 0.1f;
	if (!onGround || getHealth() <= 0) tBob = 0;
	if (onGround || getHealth() <= 0) tTilt = 0;

	bob += (tBob - bob) * 0.4f;

	tilt += (tTilt - tilt) * 0.8f;

	if (getHealth() > 0)
	{
		AABB *pickupArea = nullptr;
		if (riding != nullptr && !riding->removed)
		{
			// if the player is riding, also touch entities under the
			// pig/horse
			pickupArea = bb->minmax(riding->bb)->grow(1, 0, 1);
		}
		else
		{
			pickupArea = bb->grow(1, .5, 1);
		}

		vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), pickupArea);
		if (entities != nullptr)
		{
			for (auto& e : *entities)
			{
				if ( e && !e->removed)
				{
					touch(e);
				}
			}
		}
	}
}


void Player::touch(shared_ptr<Entity> entity)
{
	entity->playerTouch( dynamic_pointer_cast<Player>( shared_from_this() ) );
}

int Player::getScore()
{
	return entityData->getInteger(DATA_SCORE_ID);
}

void Player::setScore(int value)
{
	entityData->set(DATA_SCORE_ID, value);
}

void Player::increaseScore(int amount)
{
	int score = getScore();
	entityData->set(DATA_SCORE_ID, score + amount);
}

void Player::die(DamageSource *source)
{
	LivingEntity::die(source);
	setSize(0.2f, 0.2f);
	setPos(x, y, z);
	yd = 0.1f;

	// 4J - TODO need to use a xuid
	if ( app.isXuidNotch( m_xuid ) )
	{
		drop(std::make_shared<ItemInstance>(Item::apple, 1), true);
	}
	if (!level->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY))
	{
		inventory->dropAll();
	}

	if (source != nullptr)
	{
		xd = -Mth::cos((hurtDir + yRot) * PI / 180) * 0.1f;
		zd = -Mth::sin((hurtDir + yRot) * PI / 180) * 0.1f;
	}
	else
	{
		xd = zd = 0;
	}
	heightOffset = 0.1f;
}

void Player::awardKillScore(shared_ptr<Entity> victim, int awardPoints)
{
	increaseScore(awardPoints);
	vector<Objective *> *objectives = getScoreboard()->findObjectiveFor(ObjectiveCriteria::KILL_COUNT_ALL);

	//if (victim instanceof Player)
	//{
	//	awardStat(Stats::playerKills, 1);
	//	objectives.addAll(getScoreboard().findObjectiveFor(ObjectiveCriteria::KILL_COUNT_PLAYERS));
	//}
	//else
	//{
	//	awardStat(Stats::mobKills, 1);
	//}

	if(objectives)
	{
		for (auto& objective : *objectives)
		{
			if ( objective )
			{
				Score *score = getScoreboard()->getPlayerScore(getAName(), objective);
				if ( score ) 
					score->increment();
			}
		}
	}
}

bool Player::isShootable()
{
	return true;
}

bool Player::isCreativeModeAllowed()
{
	return true;
}

shared_ptr<ItemEntity> Player::drop(bool all)
{
	return drop(inventory->removeItem(inventory->selected, all && inventory->getSelected() != nullptr ? inventory->getSelected()->count : 1), false);
}

shared_ptr<ItemEntity> Player::drop(shared_ptr<ItemInstance> item)
{
	return drop(item, false);
}

shared_ptr<ItemEntity> Player::drop(shared_ptr<ItemInstance> item, bool randomly)
{
	if (item == nullptr) return nullptr;
	if (item->count == 0) return nullptr;

	shared_ptr<ItemEntity> thrownItem = std::make_shared<ItemEntity>(level, x, y - 0.3f + getHeadHeight(), z, item);
	thrownItem->throwTime = 20 * 2;

	thrownItem->setThrower(getName());

	float pow = 0.1f;
	if (randomly)
	{
		float _pow = random->nextFloat() * 0.5f;
		float dir = random->nextFloat() * PI * 2;
		thrownItem->xd = -sin(dir) * _pow;
		thrownItem->zd = cos(dir) * _pow;
		thrownItem->yd = 0.2f;

	}
	else
	{
		pow = 0.3f;
		thrownItem->xd = -sin(yRot / 180 * PI) * cos(xRot / 180 * PI) * pow;
		thrownItem->zd = cos(yRot / 180 * PI) * cos(xRot / 180 * PI) * pow;
		thrownItem->yd = -sin(xRot / 180 * PI) * pow + 0.1f;
		pow = 0.02f;

		float dir = random->nextFloat() * PI * 2;
		pow *= random->nextFloat();
		thrownItem->xd += cos(dir) * pow;
		thrownItem->yd += (random->nextFloat() - random->nextFloat()) * 0.1f;
		thrownItem->zd += sin(dir) * pow;
	}

	reallyDrop(thrownItem);

	return thrownItem;
}


void Player::reallyDrop(shared_ptr<ItemEntity> thrownItem)
{
	level->addEntity(thrownItem);
}


float Player::getDestroySpeed(Tile *tile, bool hasProperTool)
{
	float speed = inventory->getDestroySpeed(tile);

	if (speed > 1)
	{
		int efficiency = EnchantmentHelper::getDiggingBonus(dynamic_pointer_cast<LivingEntity>(shared_from_this()));
		shared_ptr<ItemInstance> item = inventory->getSelected();

		if (efficiency > 0 && item != nullptr)
		{
			float boost = efficiency * efficiency + 1;

			if (item->canDestroySpecial(tile) || speed > 1)
			{
				speed += boost;
			}
			else
			{
				speed += boost * 0.08f;
			}
		}
	}

	if (hasEffect(MobEffect::digSpeed))
	{
		speed *= 1.0f + (getEffect(MobEffect::digSpeed)->getAmplifier() + 1) * .2f;
	}
	if (hasEffect(MobEffect::digSlowdown))
	{
		speed *= 1.0f - (getEffect(MobEffect::digSlowdown)->getAmplifier() + 1) * .2f;
	}

	if (isUnderLiquid(Material::water) && !EnchantmentHelper::hasWaterWorkerBonus(dynamic_pointer_cast<LivingEntity>(shared_from_this()))) speed /= 5;

	// 4J Stu - onGround is set to true on the client when we are flying, which means
	// the dig speed is out of sync with the server. Removing this speed change when
	// flying so that we always dig as the same speed
	//if (!onGround) speed /= 5;

	return speed;
}

bool Player::canDestroy(Tile *tile)
{
	return inventory->canDestroy(tile);
}

void Player::readAdditionalSaveData(CompoundTag *entityTag)
{
	LivingEntity::readAdditionalSaveData(entityTag);
	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) entityTag->getList(L"Inventory");
	inventory->load(inventoryList);
	inventory->selected = entityTag->getInt(L"SelectedItemSlot");
	m_isSleeping = entityTag->getBoolean(L"Sleeping");
	sleepCounter = entityTag->getShort(L"SleepTimer");

	experienceProgress = entityTag->getFloat(L"XpP");
	experienceLevel = entityTag->getInt(L"XpLevel");
	totalExperience = entityTag->getInt(L"XpTotal");
	setScore(entityTag->getInt(L"Score"));

	if (m_isSleeping)
	{
		bedPosition = new Pos( Mth::floor(x), Mth::floor(y), Mth::floor(z));
		stopSleepInBed(true, true, false);
	}

	if (entityTag->contains(L"SpawnX") && entityTag->contains(L"SpawnY") && entityTag->contains(L"SpawnZ"))
	{
		respawnPosition = new Pos(entityTag->getInt(L"SpawnX"), entityTag->getInt(L"SpawnY"), entityTag->getInt(L"SpawnZ"));
		respawnForced = entityTag->getBoolean(L"SpawnForced");
	}

	foodData.readAdditionalSaveData(entityTag);
	abilities.loadSaveData(entityTag);

	if (entityTag->contains(L"EnderItems"))
	{
		ListTag<CompoundTag> *enderItemsList = (ListTag<CompoundTag> *) entityTag->getList(L"EnderItems");
		enderChestInventory->setItemsByTag(enderItemsList);
	}

	// 4J Added
	m_uiGamePrivileges = entityTag->getInt(L"GamePrivileges");
}

void Player::addAdditonalSaveData(CompoundTag *entityTag)
{
	LivingEntity::addAdditonalSaveData(entityTag);
	entityTag->put(L"Inventory", inventory->save(new ListTag<CompoundTag>()));
	entityTag->putInt(L"SelectedItemSlot", inventory->selected);
	entityTag->putBoolean(L"Sleeping", m_isSleeping);
	entityTag->putShort(L"SleepTimer", static_cast<short>(sleepCounter));

	entityTag->putFloat(L"XpP", experienceProgress);
	entityTag->putInt(L"XpLevel", experienceLevel);
	entityTag->putInt(L"XpTotal", totalExperience);
	entityTag->putInt(L"Score", getScore());

	if (respawnPosition != nullptr)
	{
		entityTag->putInt(L"SpawnX", respawnPosition->x);
		entityTag->putInt(L"SpawnY", respawnPosition->y);
		entityTag->putInt(L"SpawnZ", respawnPosition->z);
		entityTag->putBoolean(L"SpawnForced", respawnForced);
	}

	foodData.addAdditonalSaveData(entityTag);
	abilities.addSaveData(entityTag);

	entityTag->put(L"EnderItems", enderChestInventory->createTag());

	// 4J Added
	entityTag->putInt(L"GamePrivileges",m_uiGamePrivileges);

}

bool Player::openContainer(shared_ptr<Container> container)
{
	return true;
}

bool Player::openHopper(shared_ptr<HopperTileEntity> container)
{
	return true;
}

bool Player::openHopper(shared_ptr<MinecartHopper> container)
{
	return true;
}

bool Player::openHorseInventory(shared_ptr<EntityHorse> horse, shared_ptr<Container> container)
{
	return true;
}

bool Player::startEnchanting(int x, int y, int z, const wstring &name)
{
	return true;
}

bool Player::startRepairing(int x, int y, int z)
{
	return true;
}

bool Player::startCrafting(int x, int y, int z)
{
	return true;
}

bool Player::openFireworks(int x, int y, int z)
{
	return true;
}

float Player::getHeadHeight()
{
	return 0.12f;
}


void Player::setDefaultHeadHeight()
{
	heightOffset = 1.62f;
}

bool Player::hurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return false;
	if ( hasInvulnerablePrivilege() || (abilities.invulnerable && !source->isBypassInvul()) )	return false;

	// 4J-JEV: Fix for PSVita: #3987 - [IN GAME] The user can take damage/die, when attempting to re-enter fly mode when falling from a height.
	if ( source == DamageSource::fall && isAllowedToFly() && abilities.flying )					return false;

	noActionTime = 0;
	if (getHealth() <= 0) return false;

	if (isSleeping() && !level->isClientSide)
	{
		stopSleepInBed(true, true, false);
	}

	if ( source->scalesWithDifficulty() )
	{
		if (level->difficulty == Difficulty::PEACEFUL) dmg = 0;
		if (level->difficulty == Difficulty::EASY) dmg = dmg / 2 + 1;
		if (level->difficulty == Difficulty::HARD) dmg = dmg * 3 / 2;
	}

	if (dmg == 0) return false;

	shared_ptr<Entity> attacker = source->getEntity();
	if ( attacker != nullptr && attacker->instanceof(eTYPE_ARROW) )
	{
		shared_ptr<Arrow> arrow = dynamic_pointer_cast<Arrow>(attacker);
		if ( arrow->owner != nullptr)
		{
			attacker = arrow->owner;
		}
	}

	return LivingEntity::hurt(source, dmg);
}

bool Player::canHarmPlayer(shared_ptr<Player> target)
{
	Team *team = getTeam();
	Team *otherTeam = target->getTeam();

	if (team == nullptr)
	{
		return true;
	}
	if (!team->isAlliedTo(otherTeam))
	{
		return true;
	}
	return team->isAllowFriendlyFire();
}

bool Player::canHarmPlayer(wstring targetName)
{
	return true;
}

void Player::hurtArmor(float damage)
{
	inventory->hurtArmor(damage);
}

int Player::getArmorValue()
{
	return inventory->getArmorValue();
}

float Player::getArmorCoverPercentage()
{
	int count = 0;
	for (int i = 0; i < inventory->armor.length; i++)
	{
		if (inventory->armor[i] != nullptr) {
			count++;
		}
	}
	return static_cast<float>(count) / static_cast<float>(inventory->armor.length);
}

void Player::actuallyHurt(DamageSource *source, float dmg)
{
	if (isInvulnerable()) return;
	if (!source->isBypassArmor() && isBlocking() && dmg > 0)
	{
		dmg = (1 + dmg) * .5f;
	}
	dmg = getDamageAfterArmorAbsorb(source, dmg);
	dmg = getDamageAfterMagicAbsorb(source, dmg);

	float originalDamage = dmg;
	dmg = max(dmg - getAbsorptionAmount(), 0.0f);
	setAbsorptionAmount(getAbsorptionAmount() - (originalDamage - dmg));
	if (dmg == 0) return;

	causeFoodExhaustion(source->getFoodExhaustion());
	float oldHealth = getHealth();
	setHealth(getHealth() - dmg);
	getCombatTracker()->recordDamage(source, oldHealth, dmg);
}

bool Player::openFurnace(shared_ptr<FurnaceTileEntity> container)
{
	return true;
}

bool Player::openTrap(shared_ptr<DispenserTileEntity> container)
{
	return true;
}

void Player::openTextEdit(shared_ptr<TileEntity> sign)
{
}

bool Player::openBrewingStand(shared_ptr<BrewingStandTileEntity> brewingStand)
{
	return true;
}

bool Player::openBeacon(shared_ptr<BeaconTileEntity> beacon)
{
	return true;
}

bool Player::openTrading(shared_ptr<Merchant> traderTarget, const wstring &name)
{
	return true;
}

/**
* Opens an iteminstance-dependent user interface.
*
* @param itemInstance
*/
void Player::openItemInstanceGui(shared_ptr<ItemInstance> itemInstance)
{
}

bool Player::interact(shared_ptr<Entity> entity)
{
	shared_ptr<Player> thisPlayer = dynamic_pointer_cast<Player>(shared_from_this());

	shared_ptr<ItemInstance> item = getSelectedItem();
	shared_ptr<ItemInstance> itemClone = (item != nullptr) ? item->copy() : nullptr;
	if ( entity->interact(thisPlayer) )
	{
		// [EB]: Added rude check to see if we're still talking about the
		// same item; this code caused bucket->milkbucket to be deleted because
		// the milkbuckets' stack got decremented to 0.
		if (item != nullptr && item == getSelectedItem())
		{
			if (item->count <= 0 && !abilities.instabuild)
			{
				removeSelectedItem();
			}
			else if (item->count < itemClone->count && abilities.instabuild)
			{
				item->count = itemClone->count;
			}
		}
		return true;
	}

	if ( (item != nullptr) && entity->instanceof(eTYPE_LIVINGENTITY) )
	{
		// 4J - PC Comments
		// Hack to prevent item stacks from decrementing if the player has
		// the ability to instabuild
		if(this->abilities.instabuild) item = itemClone;
		if(item->interactEnemy(thisPlayer, dynamic_pointer_cast<LivingEntity>(entity)))
		{
			// 4J - PC Comments
			// Don't remove the item in hand if the player has the ability
			// to instabuild
			if ( (item->count <= 0) && !abilities.instabuild)
			{
				removeSelectedItem();
			}
			return true;
		}
	}
	return false;
}

shared_ptr<ItemInstance> Player::getSelectedItem()
{
	return inventory->getSelected();
}

void Player::removeSelectedItem()
{
	inventory->setItem(inventory->selected, nullptr);
}

double Player::getRidingHeight()
{
	return heightOffset - 0.5f;
}

void Player::attack(shared_ptr<Entity> entity)
{
	if (!entity->isAttackable())
	{
		return;
	}

	if (entity->skipAttackInteraction(shared_from_this()))
	{
		return;
	}

	float dmg = static_cast<float>(getAttribute(SharedMonsterAttributes::ATTACK_DAMAGE)->getValue());

	int knockback = 0;
	float magicBoost = 0;

	if ( entity->instanceof(eTYPE_LIVINGENTITY) )
	{
		shared_ptr<Player> thisPlayer = dynamic_pointer_cast<Player>(shared_from_this());
		shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(entity);
		magicBoost = EnchantmentHelper::getDamageBonus(thisPlayer, mob);
		knockback += EnchantmentHelper::getKnockbackBonus(thisPlayer, mob);
	}
	if (isSprinting())
	{
		knockback += 1;
	}

	if (dmg > 0 || magicBoost > 0)
	{
		bool bCrit = fallDistance > 0 && !onGround && !onLadder() && !isInWater() && !hasEffect(MobEffect::blindness) && (riding == nullptr) && entity->instanceof(eTYPE_LIVINGENTITY);
		if (bCrit && dmg > 0)
		{
			dmg *= 1.5f;
		}
		dmg += magicBoost;

		// Ensure we put the entity on fire if we're hitting with a
		// fire-enchanted weapon
		bool setOnFireTemporatily = false;
		int fireAspect = EnchantmentHelper::getFireAspect(dynamic_pointer_cast<LivingEntity>(shared_from_this()));
		if ( entity->instanceof(eTYPE_MOB) && fireAspect > 0 && !entity->isOnFire())
		{
			setOnFireTemporatily = true;
			entity->setOnFire(1);
		}

		DamageSource *damageSource = DamageSource::playerAttack(dynamic_pointer_cast<Player>(shared_from_this()));
		bool wasHurt = entity->hurt(damageSource, dmg);
		delete damageSource;
		if (wasHurt)
		{
			if (knockback > 0)
			{
				entity->push(-Mth::sin(yRot * PI / 180) * knockback * .5f, 0.1, Mth::cos(yRot * PI / 180) * knockback * .5f);
				xd *= 0.6;
				zd *= 0.6;
				setSprinting(false);
			}

			if (bCrit)
			{
				crit(entity);
			}
			if (magicBoost > 0)
			{
				magicCrit(entity);
			}

			if (dmg >= 18)
			{
				awardStat(GenericStats::overkill(),GenericStats::param_overkill(dmg));
			}
			setLastHurtMob(entity);


			if ( entity->instanceof(eTYPE_LIVINGENTITY) )
			{
				shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(entity);
				ThornsEnchantment::doThornsAfterAttack(shared_from_this(), mob, random);
			}
		}

		shared_ptr<ItemInstance> item = getSelectedItem();
		shared_ptr<Entity> hurtTarget = entity;
		if ( entity->instanceof(eTYPE_MULTIENTITY_MOB_PART) )
		{
			shared_ptr<Entity> multiMob = dynamic_pointer_cast<Entity>((dynamic_pointer_cast<MultiEntityMobPart>(entity))->parentMob.lock());
			if ( (multiMob != nullptr) && multiMob->instanceof(eTYPE_LIVINGENTITY) )
			{
				hurtTarget = dynamic_pointer_cast<LivingEntity>( multiMob );
			}
		}
		if ( (item != nullptr) && hurtTarget->instanceof(eTYPE_LIVINGENTITY) )
		{
			item->hurtEnemy(dynamic_pointer_cast<LivingEntity>(hurtTarget), dynamic_pointer_cast<Player>( shared_from_this() ) );
			if (item->count <= 0)
			{
				removeSelectedItem();
			}
		}
		if ( entity->instanceof(eTYPE_LIVINGENTITY) )
		{
			//awardStat(Stats.damageDealt, (int) Math.round(dmg * 10));

			if (fireAspect > 0 && wasHurt)
			{
				entity->setOnFire(fireAspect * 4);
			}
			else if (setOnFireTemporatily)
			{
				entity->clearFire();
			}
		}

		causeFoodExhaustion(FoodConstants::EXHAUSTION_ATTACK);
	}

	// if (SharedConstants::INGAME_DEBUG_OUTPUT)
	// {
	// 		//sendMessage(ChatMessageComponent.forPlainText("DMG " + dmg + ", " + magicBoost + ", " + knockback));
	// }
}

void Player::crit(shared_ptr<Entity> entity)
{
}

void Player::magicCrit(shared_ptr<Entity> entity)
{
}

void Player::respawn()
{
	deathFadeCounter=0;
}


void Player::animateRespawn(shared_ptr<Player> player, Level *level)
{

	for (int i = 0; i < 45; i++)
	{
		float angle = i * PI * 4.0f / 25.0f;
		float xo = Mth::cos(angle) * 0.7f;
		float zo = Mth::sin(angle) * 0.7f;

		level->addParticle(eParticleType_netherportal, player->x + xo, player->y - player->heightOffset + 1.62f - i * .05f, player->z + zo, 0, 0, 0);
	}

}

Slot *Player::getInventorySlot(int slotId)
{
	return nullptr;
}

void Player::remove()
{
	LivingEntity::remove();
	inventoryMenu->removed( dynamic_pointer_cast<Player>( shared_from_this() ) );
	if (containerMenu != nullptr)
	{
		containerMenu->removed( dynamic_pointer_cast<Player>( shared_from_this() ) );
	}
}

bool Player::isInWall()
{
	return !m_isSleeping && LivingEntity::isInWall();
}

bool Player::isLocalPlayer()
{
	return false;
}

Player::BedSleepingResult Player::startSleepInBed(int x, int y, int z, bool bTestUse)
{
	if (!level->isClientSide || bTestUse)
	{
		if (isSleeping() || !isAlive())
		{
			return OTHER_PROBLEM;
		}

		if (!level->dimension->isNaturalDimension())
		{
			// may not sleep in this dimension
			return NOT_POSSIBLE_HERE;
		}

		// 4J-PB - I'm going to move the position of these tests below
		// The distance check should be before the day check, otherwise you can use the bed in daytime from far away
		// and you'll get the message about only sleeping at night

		if (abs(this->x - x) > 3 || abs(this->y - y) > 2 || abs(this->z - z) > 3)
		{
			// too far away
			return TOO_FAR_AWAY;
		}

		if (!bTestUse)
		{
			// 4J-PB - We still want the tooltip for Sleep

			double hRange = 8;
			double vRange = 5;
			vector<shared_ptr<Entity> > *monsters = level->getEntitiesOfClass(typeid(Monster), AABB::newTemp(x - hRange, y - vRange, z - hRange, x + hRange, y + vRange, z + hRange));
			if (!monsters->empty())
			{
				delete monsters;
				return NOT_SAFE;
			}
			delete monsters;
		}

		// This causes a message to be displayed, so we do want to show the tooltip in test mode
		if (!bTestUse && level->isDay())
		{
			// may not sleep during day
			return NOT_POSSIBLE_NOW;
		}
	}

	if(bTestUse)
	{
		// 4J-PB - we're just testing use, and we get here, then the bed can be used
		return OK;
	}

	if (isRiding())
	{
		ride(nullptr);
	}

	setSize(0.2f, 0.2f);
	heightOffset = .2f;
	if (level->hasChunkAt(x, y, z))
	{


		int data = level->getData(x, y, z);
		int direction = BedTile::getDirection(data);
		float xo = .5f, zo = .5f;

		switch (direction)
		{
		case Direction::SOUTH:
			zo = .9f;
			break;
		case Direction::NORTH:
			zo = .1f;
			break;
		case Direction::WEST:
			xo = .1f;
			break;
		case Direction::EAST:
			xo = .9f;
			break;
		}
		setBedOffset(direction);
		setPos(x + xo, y + 15.0f / 16.0f, z + zo);
	}
	else
	{
		setPos(x + .5f, y + 15.0f / 16.0f, z + .5f);
	}
	m_isSleeping = true;
	sleepCounter = 0;
	bedPosition = new Pos(x, y, z);
	xd = zd = yd = 0;

	if (!level->isClientSide)
	{
		level->updateSleepingPlayerList();
	}

	return OK;
}


void Player::setBedOffset(int bedDirection)
{
	// place position on pillow and feet at bottom
	bedOffsetX = 0;
	bedOffsetZ = 0;

	switch (bedDirection)
	{
	case Direction::SOUTH:
		bedOffsetZ = -1.8f;
		break;
	case Direction::NORTH:
		bedOffsetZ = 1.8f;
		break;
	case Direction::WEST:
		bedOffsetX = 1.8f;
		break;
	case Direction::EAST:
		bedOffsetX = -1.8f;
		break;
	}
}


/**
*
* @param forcefulWakeUp
*            If the player has been forced to wake up. When this happens,
*            the client will skip the wake-up animation. For example, when
*            the player is hurt or the bed is destroyed.
* @param updateLevelList
*            If the level's sleeping player list needs to be updated. This
*            is usually the case.
* @param saveRespawnPoint
*            TODO
*/
void Player::stopSleepInBed(bool forcefulWakeUp, bool updateLevelList, bool saveRespawnPoint)
{

	setSize(0.6f, 1.8f);
	setDefaultHeadHeight();

	Pos *pos = bedPosition;
	Pos *standUp = bedPosition;
	if (pos != nullptr && level->getTile(pos->x, pos->y, pos->z) == Tile::bed_Id)
	{
		BedTile::setOccupied(level, pos->x, pos->y, pos->z, false);

		standUp = BedTile::findStandUpPosition(level, pos->x, pos->y, pos->z, 0);
		if (standUp == nullptr)
		{
			standUp = new Pos(pos->x, pos->y + 1, pos->z);
		}
		setPos(standUp->x + .5f, standUp->y + heightOffset + .1f, standUp->z + .5f);
	}

	m_isSleeping = false;
	if (!level->isClientSide && updateLevelList)
	{
		level->updateSleepingPlayerList();
	}
	if (forcefulWakeUp)
	{
		sleepCounter = 0;
	}
	else
	{
		sleepCounter = SLEEP_DURATION;
	}
	if (saveRespawnPoint)
	{
		setRespawnPosition(bedPosition, false);
	}
}


bool Player::checkBed()
{
	return (level->getTile(bedPosition->x, bedPosition->y, bedPosition->z) == Tile::bed_Id);
}


Pos *Player::checkBedValidRespawnPosition(Level *level, Pos *pos, bool forced)
{
	// make sure the chunks around the bed exist
	ChunkSource *chunkSource = level->getChunkSource();
	chunkSource->create((pos->x - 3) >> 4, (pos->z - 3) >> 4);
	chunkSource->create((pos->x + 3) >> 4, (pos->z - 3) >> 4);
	chunkSource->create((pos->x - 3) >> 4, (pos->z + 3) >> 4);
	chunkSource->create((pos->x + 3) >> 4, (pos->z + 3) >> 4);

	// make sure the bed is still standing
	if (level->getTile(pos->x, pos->y, pos->z) != Tile::bed_Id)
	{
		Material *bottomMaterial = level->getMaterial(pos->x, pos->y, pos->z);
		Material *topMaterial = level->getMaterial(pos->x, pos->y + 1, pos->z);
		bool freeFeet = !bottomMaterial->isSolid() && !bottomMaterial->isLiquid();
		bool freeHead = !topMaterial->isSolid() && !topMaterial->isLiquid();

		if (forced && freeFeet && freeHead)
		{
			return pos;
		}
		return nullptr;
	}
	// make sure the bed still has a stand-up position
	Pos *standUp = BedTile::findStandUpPosition(level, pos->x, pos->y, pos->z, 0);
	return standUp;
}

float Player::getSleepRotation()
{
	if (bedPosition != nullptr)
	{
		int data = level->getData(bedPosition->x, bedPosition->y, bedPosition->z);
		int direction = BedTile::getDirection(data);

		switch (direction)
		{
		case Direction::SOUTH:
			return 90;
		case Direction::WEST:
			return 0;
		case Direction::NORTH:
			return 270;
		case Direction::EAST:
			return 180;
		}
	}
	return 0;
}

bool Player::isSleeping()
{
	return m_isSleeping;
}

bool Player::isSleepingLongEnough()
{
	return m_isSleeping && sleepCounter >= SLEEP_DURATION;
}

int Player::getSleepTimer()
{
	return sleepCounter;
}

// 4J-PB - added for death fade
int Player::getDeathFadeTimer()
{
	return deathFadeCounter;
}

bool Player::getPlayerFlag(int flag)
{
	return (entityData->getByte(DATA_PLAYER_FLAGS_ID) & (1 << flag)) != 0;
}

void Player::setPlayerFlag(int flag, bool value)
{
	byte currentValue = entityData->getByte(DATA_PLAYER_FLAGS_ID);
	if (value)
	{
		entityData->set(DATA_PLAYER_FLAGS_ID, static_cast<byte>(currentValue | (1 << flag)));
	}
	else
	{
		entityData->set(DATA_PLAYER_FLAGS_ID, static_cast<byte>(currentValue & ~(1 << flag)));
	}
}


/**
* This method is currently only relevant to client-side players. It will
* try to load the messageId from the language file and display it to the
* client.
*/
void Player::displayClientMessage(int messageId)
{

}

Pos *Player::getRespawnPosition()
{
	return respawnPosition;
}

bool Player::isRespawnForced()
{
	return respawnForced;
}

void Player::setRespawnPosition(Pos *respawnPosition, bool forced)
{
	if (respawnPosition != nullptr)
	{
		this->respawnPosition = new Pos(*respawnPosition);
		respawnForced = forced;
	}
	else
	{
		this->respawnPosition = nullptr;
		respawnForced = false;
	}
}

void Player::awardStat(Stat *stat, byteArray paramBlob)
{
	if (paramBlob.data != nullptr)
	{
		delete [] paramBlob.data;
	}
}


void Player::jumpFromGround()
{
	LivingEntity::jumpFromGround();

	// 4J Stu - This seems to have been missed from 1.7.3, but do we care?
	//awardStat(Stats::jump, 1);

	if (isSprinting())
	{
		causeFoodExhaustion(FoodConstants::EXHAUSTION_SPRINT_JUMP);
	}
	else
	{
		causeFoodExhaustion(FoodConstants::EXHAUSTION_JUMP);
	}
}


void Player::travel(float xa, float ya)
{
	double preX = x, preY = y, preZ = z;

	if (abilities.flying && riding == nullptr)
	{
		double ydo = yd;
		float ofs = flyingSpeed;
		flyingSpeed = abilities.getFlyingSpeed();
		LivingEntity::travel(xa, ya);
		yd = ydo * 0.6;
		flyingSpeed = ofs;
	}
	else
	{
		LivingEntity::travel(xa, ya);
	}

	checkMovementStatistiscs(x - preX, y - preY, z - preZ);
}

float Player::getSpeed()
{
	return static_cast<float>(getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->getValue());
}

void Player::checkMovementStatistiscs(double dx, double dy, double dz)
{

	if (riding != nullptr)
	{
		return;
	}
	if (isUnderLiquid(Material::water))
	{
		int distance = static_cast<int>(Math::round(sqrt(dx * dx + dy * dy + dz * dz) * 100.0f));
		if (distance > 0)
		{
			//awardStat(Stats::diveOneCm, distance);
			causeFoodExhaustion(FoodConstants::EXHAUSTION_SWIM * distance * .01f);
		}
	}
	else if (isInWater())
	{
		int horizontalDistance = static_cast<int>(Math::round(sqrt(dx * dx + dz * dz) * 100.0f));
		if (horizontalDistance > 0)
		{
			distanceSwim += horizontalDistance;
			if( distanceSwim >= 100 )
			{
				int newDistance = distanceSwim - (distanceSwim % 100);
				distanceSwim -= newDistance;
				awardStat( GenericStats::swimOneM(), GenericStats::param_swim(newDistance/100) );
			}
			causeFoodExhaustion(FoodConstants::EXHAUSTION_SWIM * horizontalDistance * .01f);
		}
	}
	else if (onLadder())
	{
		if (dy > 0)
		{
			distanceClimb += static_cast<int>(Math::round(dy * 100.0f));
			if( distanceClimb >= 100 )
			{
				int newDistance = distanceClimb - (distanceClimb % 100);
				distanceClimb -= newDistance;
				awardStat( GenericStats::climbOneM(), GenericStats::param_climb(newDistance/100) );
			}
		}
	}
	else if (onGround)
	{
		int horizontalDistance = static_cast<int>(Math::round(sqrt(dx * dx + dz * dz) * 100.0f));
		if (horizontalDistance > 0)
		{
			distanceWalk += horizontalDistance;
			if( distanceWalk >= 100 )
			{
				int newDistance = distanceWalk - (distanceWalk % 100);
				distanceWalk -= newDistance;
				awardStat( GenericStats::walkOneM(), GenericStats::param_walk(newDistance/100) );
			}
			if (isSprinting())
			{
				causeFoodExhaustion(FoodConstants::EXHAUSTION_SPRINT * horizontalDistance * .01f);
			}
			else
			{
				causeFoodExhaustion(FoodConstants::EXHAUSTION_WALK * horizontalDistance * .01f);
			}
		}
	}
}


void Player::checkRidingStatistiscs(double dx, double dy, double dz)
{
	if (riding != nullptr)
	{
		int distance = static_cast<int>(Math::round(sqrt(dx * dx + dy * dy + dz * dz) * 100.0f));
		if (distance > 0)
		{
			if ( riding->instanceof(eTYPE_MINECART) )
			{
				distanceMinecart += distance;
				if( distanceMinecart >= 100 )
				{
					int newDistance = distanceMinecart - (distanceMinecart % 100);
					distanceMinecart -= newDistance;
					awardStat( GenericStats::minecartOneM(), GenericStats::param_minecart(newDistance/100) );
				}

				int dist = 0;
				if (minecartAchievementPos == nullptr)
				{
					minecartAchievementPos = new Pos(Mth::floor(x), Mth::floor(y), Mth::floor(z));
				}
				// 4J-PB - changed this because our world isn't big enough to go 1000m
				else
				{
					// 4-JEV, changed slightly to add extra parameters for event on durango.
					int dist = minecartAchievementPos->dist(Mth::floor(x), Mth::floor(y), Mth::floor(z));
#ifdef _XBOX_ONE
					// 4J-PB - send the event to cause the progress bar to increase on XB1
					if (m_bAwardedOnARail==false)
					{
						if(dist < 500)
						{
							if((dist>0) && (dist%100==0))
							{
								awardStat(GenericStats::onARail(), GenericStats::param_onARail(dist));
							}
						}
						else
						{
							awardStat(GenericStats::onARail(), GenericStats::param_onARail(dist));
							m_bAwardedOnARail=true;
						}
					}
#else
					if ((m_bAwardedOnARail==false) && (dist >= 500))
					{
						awardStat(GenericStats::onARail(), GenericStats::param_onARail(dist));
						m_bAwardedOnARail=true;
					}
#endif
				}

			}
			else if ( riding->instanceof(eTYPE_BOAT) )
			{
				distanceBoat += distance;
				if( distanceBoat >= 100 )
				{
					int newDistance = distanceBoat - (distanceBoat % 100);
					distanceBoat -= newDistance;
					awardStat(GenericStats::boatOneM(), GenericStats::param_boat(newDistance/100) );
				}
			}
			else if ( riding->instanceof(eTYPE_PIG) )
			{
				distancePig += distance;
				if( distancePig >= 100 )
				{
					int newDistance = distancePig - (distancePig % 100);
					distancePig -= newDistance;
					awardStat(GenericStats::pigOneM(), GenericStats::param_pig(newDistance/100) );
				}
			}
		}
	}
}


void Player::causeFallDamage(float distance)
{
	if (abilities.mayfly) return;

	if (distance >= 2)
	{
		distanceFall += static_cast<int>(Math::round(distance * 100.0));
		if( distanceFall >= 100 )
		{
			int newDistance = distanceFall - (distanceFall % 100);
			distanceFall -= newDistance;
			awardStat(GenericStats::fallOneM(), GenericStats::param_fall(newDistance/100) );
		}
	}
	LivingEntity::causeFallDamage(distance);
}


void Player::killed(shared_ptr<LivingEntity> mob)
{
	// 4J-PB - added the lavaslime enemy - fix for #64007 - TU7: Code: Achievements: TCR#073: Killing Magma Cubes doesn't unlock "Monster Hunter" Achievement.
	if( mob->instanceof(eTYPE_ENEMY) || mob->GetType() == eTYPE_GHAST || mob->GetType() == eTYPE_SLIME || mob->GetType() == eTYPE_LAVASLIME || mob->GetType() == eTYPE_ENDERDRAGON)
	{
		awardStat(GenericStats::killEnemy(), GenericStats::param_noArgs());

		switch( mob->GetType() )
		{
		case eTYPE_CREEPER:
			awardStat(GenericStats::killsCreeper(), GenericStats::param_noArgs());
			break;
		case eTYPE_SKELETON:
			if( mob->isRiding() && mob->riding->GetType() == eTYPE_SPIDER )
				awardStat(GenericStats::killsSpiderJockey(), GenericStats::param_noArgs());
			else
				awardStat(GenericStats::killsSkeleton(), GenericStats::param_noArgs());
			break;
		case eTYPE_SPIDER:
			if( mob->rider.lock() != nullptr && mob->rider.lock()->GetType() == eTYPE_SKELETON )
				awardStat(GenericStats::killsSpiderJockey(), GenericStats::param_noArgs());
			else
				awardStat(GenericStats::killsSpider(), GenericStats::param_noArgs());
			break;
		case eTYPE_ZOMBIE:
			awardStat(GenericStats::killsZombie(), GenericStats::param_noArgs());
			break;
		case eTYPE_PIGZOMBIE:
			if( level->dimension->id == 0 )
				awardStat(GenericStats::killsZombiePigman(), GenericStats::param_noArgs());
			else
				awardStat(GenericStats::killsNetherZombiePigman(), GenericStats::param_noArgs());
			break;
		case eTYPE_GHAST:
			awardStat(GenericStats::killsGhast(), GenericStats::param_noArgs());
			break;
		case eTYPE_SLIME:
			awardStat(GenericStats::killsSlime(), GenericStats::param_noArgs());
			break;
		case eTYPE_ENDERDRAGON:
			awardStat(GenericStats::killsEnderdragon(), GenericStats::param_noArgs());
			break;
		}
	}
	else if( mob->GetType() == eTYPE_COW )
	{
		awardStat(GenericStats::killCow(), GenericStats::param_noArgs());
	}
}

void Player::makeStuckInWeb()
{
	if (!abilities.flying) LivingEntity::makeStuckInWeb();
}

Icon *Player::getItemInHandIcon(shared_ptr<ItemInstance> item, int layer)
{
	Icon *icon = LivingEntity::getItemInHandIcon(item, layer);
	if (item->id == Item::fishingRod->id && fishing != nullptr)
	{
		icon = Item::fishingRod->getEmptyIcon();
	}
	else if (item->getItem()->hasMultipleSpriteLayers())
	{
		return item->getItem()->getLayerIcon(item->getAuxValue(), layer);
	}
	else if (useItem != nullptr && item->id == Item::bow_Id)
	{
		int ticksHeld = (item->getUseDuration() - useItemDuration);
		if (ticksHeld >= BowItem::MAX_DRAW_DURATION - 2)
		{
			return Item::bow->getDrawnIcon(2);
		}
		if (ticksHeld > (2 * BowItem::MAX_DRAW_DURATION) / 3)
		{
			return Item::bow->getDrawnIcon(1);
		}
		if (ticksHeld > 0)
		{
			return Item::bow->getDrawnIcon(0);
		}
	}
	return icon;
}

shared_ptr<ItemInstance> Player::getArmor(int pos)
{
	return inventory->getArmor(pos);
}

void Player::increaseXp(int i)
{
	increaseScore(i);
	int max = INT_MAX - totalExperience;
	if (i > max)
	{
		i = max;
	}
	experienceProgress += static_cast<float>(i) / getXpNeededForNextLevel();
	totalExperience += i;
	while (experienceProgress >= 1)
	{
		experienceProgress = (experienceProgress - 1) * getXpNeededForNextLevel();
		giveExperienceLevels(1);
		experienceProgress /= getXpNeededForNextLevel();
	}
}

void Player::giveExperienceLevels(int amount)
{
	experienceLevel += amount;
	if (experienceLevel < 0)
	{
		experienceLevel = 0;
		experienceProgress = 0;
		totalExperience = 0;
	}

	if (amount > 0 && experienceLevel % 5 == 0 && lastLevelUpTime < tickCount - SharedConstants::TICKS_PER_SECOND * 5.0f)
	{
		float vol = experienceLevel > 30 ? 1 : experienceLevel / 30.0f;
		level->playEntitySound(shared_from_this(), eSoundType_RANDOM_LEVELUP, vol * 0.75f, 1);
		lastLevelUpTime = tickCount;
	}
}

int Player::getXpNeededForNextLevel()
{
	// Update xp calculations from 1.3
	if (experienceLevel >= 30)
	{
		return 17 + 15 * 3 + (experienceLevel - 30) * 7;
	}
	if (experienceLevel >= 15)
	{
		return 17 + (experienceLevel - 15) * 3;
	}
	return 17;
}

/**
* This method adds on to the player's exhaustion, which may decrease the
* player's food level.
*
* @param amount
*            Amount of exhaustion to add, between 0 and 20 (setting it to
*            20 will guarantee that at least 1, and at most 4, food points
*            are deducted). See FoodConstants for cost suggestions.
*/
void Player::causeFoodExhaustion(float amount)
{
	if( isAllowedToIgnoreExhaustion() || ( isAllowedToFly() && abilities.flying) ) return;
	if (abilities.invulnerable || hasInvulnerablePrivilege() ) return;

	// 4J Stu - Added 1.8.2 bug fix (TU6) - If players cannot eat, then their food bar should not decrease due to exhaustion
	if(app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0 && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild) != 0) return;

	if (!level->isClientSide)
	{
		foodData.addExhaustion(amount);
	}
}

FoodData *Player::getFoodData()
{
	return &foodData;
}

bool Player::canEat(bool magicalItem)
{
	return (magicalItem || foodData.needsFood()) && !abilities.invulnerable && !hasInvulnerablePrivilege();
}

bool Player::isHurt()
{
	return getHealth() > 0 && getHealth() < getMaxHealth();
}

void Player::startUsingItem(shared_ptr<ItemInstance> instance, int duration)
{
	if (instance == useItem) return;
	useItem = instance;
	useItemDuration = duration;
	if (!level->isClientSide)
	{
		setUsingItemFlag(true);
	}

	// 4J-JEV, hook for ItemUsed event, and ironbelly achievement.
	awardStat(GenericStats::itemsUsed(instance->getItem()->id),
		GenericStats::param_itemsUsed(dynamic_pointer_cast<Player>(shared_from_this()),instance));

#if (!defined _DURANGO) && (defined _EXTENDED_ACHIEVEMENTS)
	if ( (instance->getItem()->id == Item::rotten_flesh_Id) && (getFoodData()->getFoodLevel() == 0) )
		awardStat(GenericStats::ironBelly(), GenericStats::param_ironBelly());
#endif
}

bool Player::mayDestroyBlockAt(int x, int y, int z)
{
	if (abilities.mayBuild)
	{
		return true;
	}
	int t = level->getTile(x, y, z);
	if (t > 0) {
		Tile *tile = Tile::tiles[t];

		if (tile->material->isDestroyedByHand())
		{
			return true;
		}
		else if (getSelectedItem() != nullptr)
		{
			shared_ptr<ItemInstance> carried = getSelectedItem();

			if (carried->canDestroySpecial(tile) || carried->getDestroySpeed(tile) > 1)
			{
				return true;
			}
		}
	}
	return false;
}

bool Player::mayUseItemAt(int x, int y, int z, int face, shared_ptr<ItemInstance> item)
{
	if (abilities.mayBuild)
	{
		return true;
	}
	if (item != nullptr)
	{
		return item->mayBePlacedInAdventureMode();
	}
	return false;
}

int Player::getExperienceReward(shared_ptr<Player> killedBy)
{
	if (level->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY)) return 0;
	int reward = experienceLevel * 7;
	if (reward > 100)
	{
		return 100;
	}
	return reward;
}

bool Player::isAlwaysExperienceDropper()
{
	// players always drop experience
	return true;
}

wstring Player::getAName()
{
	return name;
}

bool Player::shouldShowName()
{
	return true;
}

void Player::restoreFrom(shared_ptr<Player> oldPlayer, bool restoreAll)
{
	if(restoreAll)
	{
		inventory->replaceWith(oldPlayer->inventory);

		setHealth(oldPlayer->getHealth());
		foodData = oldPlayer->foodData;

		experienceLevel = oldPlayer->experienceLevel;
		totalExperience = oldPlayer->totalExperience;
		experienceProgress = oldPlayer->experienceProgress;

		setScore(oldPlayer->getScore());
		portalEntranceDir = oldPlayer->portalEntranceDir;
	}
	else if (level->getGameRules()->getBoolean(GameRules::RULE_KEEPINVENTORY))
	{
		inventory->replaceWith(oldPlayer->inventory);

		experienceLevel = oldPlayer->experienceLevel;
		totalExperience = oldPlayer->totalExperience;
		experienceProgress = oldPlayer->experienceProgress;
		setScore(oldPlayer->getScore());
	}
	enderChestInventory = oldPlayer->enderChestInventory;
}

bool Player::makeStepSound()
{
	return !abilities.flying;
}

void Player::onUpdateAbilities()
{
}

void Player::setGameMode(GameType *mode)
{
}

wstring Player::getName()
{
	return name;
}

wstring Player::getDisplayName()
{
	//PlayerTeam.formatNameForTeam(getTeam(), name);

	// If player display name is not set, return name
	return m_displayName.size() > 0 ? m_displayName : name;
}

wstring Player::getNetworkName()
{
	// 4J: We can only transmit gamertag in network packets
	return name;
}

Level *Player::getCommandSenderWorld()
{
	return level;
}

shared_ptr<PlayerEnderChestContainer> Player::getEnderChestInventory()
{
	return enderChestInventory;
}

shared_ptr<ItemInstance> Player::getCarried(int slot)
{
	if (slot == 0) return inventory->getSelected();
	return inventory->armor[slot - 1];
}

shared_ptr<ItemInstance> Player::getCarriedItem()
{
	return inventory->getSelected();
}

void Player::setEquippedSlot(int slot, shared_ptr<ItemInstance> item)
{
	inventory->armor[slot] = item;
}

bool Player::isInvisibleTo(shared_ptr<Player> player)
{
	return isInvisible();
}

ItemInstanceArray Player::getEquipmentSlots()
{
	return inventory->armor;
}

bool Player::isCapeHidden()
{
	return getPlayerFlag(FLAG_HIDE_CAPE);
}

bool Player::isPushedByWater()
{
	return !abilities.flying;
}

Scoreboard *Player::getScoreboard()
{
	return level->getScoreboard();
}

Team *Player::getTeam()
{
	return getScoreboard()->getPlayersTeam(name);
}

void Player::setAbsorptionAmount(float absorptionAmount)
{
	if (absorptionAmount < 0) absorptionAmount = 0;
	getEntityData()->set(DATA_PLAYER_ABSORPTION_ID, absorptionAmount);
}

float Player::getAbsorptionAmount()
{
	return getEntityData()->getFloat(DATA_PLAYER_ABSORPTION_ID);
}

int Player::getTexture()
{
	switch(m_skinIndex)
	{
	case eDefaultSkins_Skin0:
		return TN_MOB_CHAR; // 4J - was L"/mob/char.png";
	case eDefaultSkins_Skin1:
		return TN_MOB_CHAR1; // 4J - was L"/mob/char1.png";
	case eDefaultSkins_Skin2:
		return TN_MOB_CHAR2; // 4J - was L"/mob/char2.png";
	case eDefaultSkins_Skin3:
		return TN_MOB_CHAR3; // 4J - was L"/mob/char3.png";
	case eDefaultSkins_Skin4:
		return TN_MOB_CHAR4; // 4J - was L"/mob/char4.png";
	case eDefaultSkins_Skin5:
		return TN_MOB_CHAR5; // 4J - was L"/mob/char5.png";
	case eDefaultSkins_Skin6:
		return TN_MOB_CHAR6; // 4J - was L"/mob/char6.png";
	case eDefaultSkins_Skin7:
		return TN_MOB_CHAR7; // 4J - was L"/mob/char7.png";

	default:
		return TN_MOB_CHAR; // 4J - was L"/mob/char.png";
	}
}

int Player::hash_fnct(const shared_ptr<Player> k)
{
	// TODO 4J Stu - Should we just be using the pointers and hashing them?
#ifdef __PS3__
	return (int)boost::hash_value( k->name ); // 4J Stu - Names are completely unique?
#else
	return static_cast<int>(std::hash<wstring>{}(k->name)); // 4J Stu - Names are completely unique?
#endif //__PS3__
}

bool Player::eq_test(const shared_ptr<Player> x, const shared_ptr<Player> y)
{
	// TODO 4J Stu - Should we just be using the pointers and comparing them for equality?
	return x->name.compare( y->name ) == 0; // 4J Stu - Names are completely unique?
}


unsigned int Player::getPlayerGamePrivilege(EPlayerGamePrivileges privilege)
{
	return Player::getPlayerGamePrivilege(m_uiGamePrivileges,privilege);
}

unsigned int Player::getPlayerGamePrivilege(unsigned int uiGamePrivileges, EPlayerGamePrivileges privilege)
{
	if( privilege == ePlayerGamePrivilege_All )
	{
		return uiGamePrivileges;
	}
	else if (privilege < ePlayerGamePrivilege_MAX )
	{
		return uiGamePrivileges&(1<<privilege);
	}
	return 0;
}

void Player::setPlayerGamePrivilege(EPlayerGamePrivileges privilege, unsigned int value)
{
	Player::setPlayerGamePrivilege(m_uiGamePrivileges,privilege,value);
}

void Player::setPlayerGamePrivilege(unsigned int &uiGamePrivileges, EPlayerGamePrivileges privilege, unsigned int value)
{
	if( privilege == ePlayerGamePrivilege_All )
	{
		uiGamePrivileges = value;
	}
	else if(privilege ==ePlayerGamePrivilege_HOST)
	{
		if(value == 0)
		{
			Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_Op,0);
		}
		else
		{
			Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_Op,1);
			Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanToggleInvisible,1);
			Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanToggleFly,1);
			Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanToggleClassicHunger,1);
			Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanTeleport,1);
		}
	}
	else if (privilege < ePlayerGamePrivilege_MAX )
	{
		if(value!=0)
		{
			uiGamePrivileges|=(1<<privilege);
		}
		else
		{
			// Some privileges will turn other things off as well
			switch(privilege)
			{
			case ePlayerGamePrivilege_CanToggleInvisible:
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_Invisible,0);
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_Invulnerable,0);
				break;
			case ePlayerGamePrivilege_CanToggleFly:
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanFly,0);
				break;
			case ePlayerGamePrivilege_CanToggleClassicHunger:
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_ClassicHunger,0);
				break;
			case ePlayerGamePrivilege_Op:
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanToggleInvisible,0);
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanToggleFly,0);
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanToggleClassicHunger,0);
				Player::setPlayerGamePrivilege(uiGamePrivileges,ePlayerGamePrivilege_CanTeleport,0);
				break;
			}
			// off
			uiGamePrivileges&=~(1<<privilege);
		}
	}
}

bool Player::isAllowedToUse(Tile *tile)
{
	bool allowed = true;
	if(tile != nullptr && app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
	{
		allowed = false;

		if(getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches) != 0)
		{
			switch(tile->id)
			{
			case Tile::door_wood_Id:
			case Tile::button_stone_Id:
			case Tile::button_wood_Id:
			case Tile::lever_Id:
			case Tile::fenceGate_Id:
			case Tile::trapdoor_Id:
				allowed = true;
				break;
			}
		}

		if(getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseContainers) != 0)
		{
			switch(tile->id)
			{
			case Tile::chest_Id:
			case Tile::furnace_Id:
			case Tile::furnace_lit_Id:
			case Tile::dispenser_Id:
			case Tile::brewingStand_Id:
			case Tile::enchantTable_Id:
			case Tile::workBench_Id:
			case Tile::anvil_Id:
			case Tile::enderChest_Id:
				allowed = true;
				break;
			}
		}

		if(!allowed && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild) == 0)
		{
			switch(tile->id)
			{
			case Tile::door_wood_Id:
			case Tile::button_stone_Id:
			case Tile::button_wood_Id:
			case Tile::lever_Id:
			case Tile::fenceGate_Id:
			case Tile::trapdoor_Id:
			case Tile::chest_Id:
			case Tile::furnace_Id:
			case Tile::furnace_lit_Id:
			case Tile::dispenser_Id:
			case Tile::brewingStand_Id:
			case Tile::enchantTable_Id:
			case Tile::workBench_Id:
			case Tile::anvil_Id:
			case Tile::enderChest_Id:
				allowed =  false;
				break;
			default:
				allowed = true;
				break;
			}
		}
	}

	return allowed;
}

bool Player::isAllowedToUse(shared_ptr<ItemInstance> item)
{
	bool allowed = true;
	if(item != nullptr && app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
	{
		if(getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild) != 0)
		{
			allowed = false;
		}

		// 4J Stu - TU8 Players should always be able to eat food items, even if the build option is turned of
		switch(item->id)
		{
			// food
		case Item::mushroomStew_Id:
		case Item::apple_Id:
		case Item::bread_Id:
		case Item::porkChop_raw_Id:
		case Item::porkChop_cooked_Id:
		case Item::apple_gold_Id:
		case Item::fish_raw_Id:
		case Item::fish_cooked_Id:
		case Item::cookie_Id:
		case Item::beef_cooked_Id:
		case Item::beef_raw_Id:
		case Item::chicken_cooked_Id:
		case Item::chicken_raw_Id:
		case Item::melon_Id:
		case Item::rotten_flesh_Id:
			// bow
		case Item::bow_Id:
		case Item::sword_diamond_Id:
		case Item::sword_gold_Id:
		case Item::sword_iron_Id:
		case Item::sword_stone_Id:
		case Item::sword_wood_Id:
			allowed = true;
			break;
		}
	}

	return allowed;
}

bool Player::isAllowedToInteract(shared_ptr<Entity> target)
{
	bool allowed = true;
	if(app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
	{
		if (target->instanceof(eTYPE_MINECART))
		{
			if (getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseContainers) == 0)
			{
				shared_ptr<Minecart> minecart = dynamic_pointer_cast<Minecart>( target );
				if (minecart->getType() == Minecart::TYPE_CHEST)
					allowed = false;
			}

		}
		else
		{
			if(getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild) != 0)
			{
				allowed = false;
			}

			if(getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotMine) != 0)
			{
				allowed = false;
			}
		}
	}

	return allowed;
}

bool Player::isAllowedToMine()
{
	bool allowed = true;
	if(app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
	{
		if(getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotMine) != 0)
		{
			allowed = false;
		}
	}
	return allowed;
}

bool Player::isAllowedToAttackPlayers()
{
	bool allowed = true;
	if( hasInvisiblePrivilege() || ((app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotAttackPlayers)) )
	{
		allowed = false;
	}
	return allowed;
}

bool Player::isAllowedToAttackAnimals()
{
	bool allowed = true;
	if( (app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0) && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotAttackAnimals) )
	{
		allowed = false;
	}
	return allowed;
}

bool Player::isAllowedToHurtEntity(shared_ptr<Entity> target)
{
	bool allowed = true;

	if(!isAllowedToMine())
	{
		switch(target->GetType())
		{
		case eTYPE_HANGING_ENTITY:
		case eTYPE_PAINTING:
		case eTYPE_ITEM_FRAME:

			// 4J-JEV: Fix for #88212,
			// Untrusted players shouldn't be able to damage minecarts or boats.
		case eTYPE_BOAT:
		case eTYPE_MINECART:

			allowed = false;
			break;
		};
	}
	return allowed;
}

bool Player::isAllowedToFly()
{
	bool allowed = false;
	if(app.GetGameHostOption(eGameHostOption_HostCanFly) != 0 && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanFly) != 0)
	{
		allowed = true;
	}
	return allowed;
}

bool Player::isAllowedToIgnoreExhaustion()
{
	bool allowed = false;
	if( (app.GetGameHostOption(eGameHostOption_HostCanChangeHunger) != 0 && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_ClassicHunger) != 0) ||
		(isAllowedToFly() && abilities.flying) )
	{
		allowed = true;
	}
	return allowed;
}

bool Player::isAllowedToTeleport()
{
	bool allowed = false;
	if( isModerator() && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanTeleport) != 0)
	{
		allowed = true;
	}
	return allowed;
}

bool Player::hasInvisiblePrivilege()
{
	bool enabled = false;
	if(app.GetGameHostOption(eGameHostOption_HostCanBeInvisible) != 0 && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invisible) != 0)
	{
		enabled = true;
	}
	return enabled;
}

bool Player::hasInvulnerablePrivilege()
{
	bool enabled = false;
	if(app.GetGameHostOption(eGameHostOption_HostCanBeInvisible) != 0 && getPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invulnerable) != 0)
	{
		enabled = true;
	}
	return enabled;
}

bool Player::isModerator()
{
	return getPlayerGamePrivilege(Player::ePlayerGamePrivilege_Op) != 0;
}

void Player::enableAllPlayerPrivileges(unsigned int &uigamePrivileges, bool enable)
{
	Player::setPlayerGamePrivilege(uigamePrivileges, Player::ePlayerGamePrivilege_CannotMine, enable?0:1);
	Player::setPlayerGamePrivilege(uigamePrivileges, Player::ePlayerGamePrivilege_CannotBuild, enable?0:1);
	Player::setPlayerGamePrivilege(uigamePrivileges, Player::ePlayerGamePrivilege_CannotAttackPlayers, enable?0:1);
	Player::setPlayerGamePrivilege(uigamePrivileges, Player::ePlayerGamePrivilege_CannotAttackAnimals, enable?0:1);
	Player::setPlayerGamePrivilege(uigamePrivileges, Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches, enable?1:0);
	Player::setPlayerGamePrivilege(uigamePrivileges, Player::ePlayerGamePrivilege_CanUseContainers, enable?1:0);
}

void Player::enableAllPlayerPrivileges(bool enable)
{
	Player::enableAllPlayerPrivileges(m_uiGamePrivileges,enable);
}

bool Player::canCreateParticles()
{
	return !hasInvisiblePrivilege();
}

vector<ModelPart *> *Player::GetAdditionalModelParts()
{
	if(m_ppAdditionalModelParts==nullptr && !m_bCheckedForModelParts)
	{
		bool hasCustomTexture = !customTextureUrl.empty();
		bool customTextureIsDefaultSkin = customTextureUrl.substr(0,3).compare(L"def") == 0;

		// see if we can find the parts
		m_ppAdditionalModelParts=app.GetAdditionalModelParts(m_dwSkinId);

		// If it's a default texture (which has no parts), we have the parts, or we already have the texture (in which case we should have parts if there are any) then we are done
		if(!hasCustomTexture || customTextureIsDefaultSkin || m_ppAdditionalModelParts != nullptr || app.IsFileInMemoryTextures(customTextureUrl))
		{
			m_bCheckedForModelParts=true;
		}
		if(m_ppAdditionalModelParts == nullptr && !m_bCheckedDLCForModelParts)
		{
			m_bCheckedDLCForModelParts = true;

			// we don't have the data from the dlc skin yet
			app.DebugPrintf("m_bCheckedForModelParts Couldn't get model parts for skin %X\n",m_dwSkinId);

			// do we have it from the DLC pack?
			DLCSkinFile *pDLCSkinFile = app.m_dlcManager.getSkinFile(this->customTextureUrl);

			if(pDLCSkinFile!=nullptr)
			{
				DWORD dwBoxC=pDLCSkinFile->getAdditionalBoxesCount();
				if(dwBoxC!=0)
				{
					app.DebugPrintf("m_bCheckedForModelParts Got model parts from DLCskin for skin %X\n",m_dwSkinId);
					m_ppAdditionalModelParts=app.SetAdditionalSkinBoxes(m_dwSkinId,pDLCSkinFile->getAdditionalBoxes());
				}

				app.SetAnimOverrideBitmask(pDLCSkinFile->getSkinID(),pDLCSkinFile->getAnimOverrideBitmask());

				m_bCheckedForModelParts=true;
			}
		}

		if(m_bCheckedForModelParts) setAnimOverrideBitmask(getSkinAnimOverrideBitmask(m_dwSkinId));
	}
	return m_ppAdditionalModelParts;
}

void Player::SetAdditionalModelParts(vector<ModelPart *> *ppAdditionalModelParts)
{
	m_ppAdditionalModelParts=ppAdditionalModelParts;
}

#if defined(__PS3__) || defined(__ORBIS__)

Player::ePlayerNameValidState Player::GetPlayerNameValidState(void)
{
	return m_ePlayerNameValidState;
}

void Player::SetPlayerNameValidState(bool bState)
{
	if(bState)
	{
		m_ePlayerNameValidState=ePlayerNameValid_True;
	}
	else
	{
		m_ePlayerNameValidState=ePlayerNameValid_False;

	}
}
#endif
