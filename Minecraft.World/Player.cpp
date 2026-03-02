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
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.dimension.h"
#include "net.minecraft.world.level.material.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.tile.entity.h"
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

	inventory = shared_ptr<Inventory>( new Inventory( this ) );

	userType = 0;
	score = 0;
	oBob = bob = 0.0f;
	swinging = false;
	swingTime = 0;

	//string name;
	dimension = 0;
	//string cloakTexture;

	xCloakO = yCloakO = zCloakO = 0.0;
	xCloak = yCloak = zCloak = 0.0;

	m_isSleeping = false;

	m_uiPlayerCurrentSkin=0;

	bedPosition = NULL;


	sleepCounter = 0;
	deathFadeCounter=0;


	bedOffsetX = bedOffsetY = bedOffsetZ = 0.0f;
	stats = NULL;


	respawnPosition = NULL;
	minecartAchievementPos = NULL;


	changingDimensionDelay = 20;


	isInsidePortal = false;


	portalTime = oPortalTime = 0.0f;

	dmgSpill = 0;


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

	m_uiGamePrivileges = 0;

	m_ppAdditionalModelParts=NULL;
	m_bCheckedForModelParts=false;
	m_bCheckedDLCForModelParts=false;

#if defined(__PS3__) || defined(__ORBIS__)
	m_ePlayerNameValidState=ePlayerNameValid_NotSet;
#endif

	enderChestInventory = shared_ptr<PlayerEnderChestContainer>(new PlayerEnderChestContainer());

	m_bAwardedOnARail=false;
}

Player::Player(Level *level) : Mob( level )
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();

	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that the derived version of the function is called
	health = getMaxHealth();

	_init();
	MemSect(11);
	inventoryMenu = new InventoryMenu(inventory, !level->isClientSide, this);
	MemSect(0);

	containerMenu = inventoryMenu;

	heightOffset = 1.62f;
	Pos *spawnPos = level->getSharedSpawnPos();
	this->moveTo(spawnPos->x + 0.5, spawnPos->y + 1, spawnPos->z + 0.5, 0, 0);
	delete spawnPos;

	modelName = L"humanoid";
	rotOffs = 180;
	flameTime = 20;

	textureIdx = TN_MOB_CHAR; // 4J - was L"/mob/char.png";
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
	m_UUID = L"";
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

int Player::getMaxHealth()
{
	return MAX_HEALTH;
}

void Player::defineSynchedData()
{
	this->Mob::defineSynchedData();

	entityData->define(DATA_PLAYER_FLAGS_ID, (byte) 0);
	entityData->define(DATA_PLAYER_RUNNING_ID, (byte) 0);
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
	return useItem != NULL;
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
	if (useItem != NULL)
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


void Player::tick()
{
	if (useItem != NULL)
	{
		shared_ptr<ItemInstance> item = inventory->getSelected();
		// 4J Stu - Fix for #45508 - TU5: Gameplay: Eating one piece of food will result in a second piece being eaten as well
		// Original code was item != useItem. Changed this now to use the equals function, and add the NULL check as well for the other possible not equals (useItem is not NULL if we are here)
		// This is because the useItem and item could be different objects due to an inventory update from the server, but still be the same item (with the same id,count and auxvalue)
		if (item == NULL || !item->equals(useItem) )
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
	this->Mob::tick();

	if (!level->isClientSide)
	{
		if (containerMenu != NULL && !containerMenu->stillValid( dynamic_pointer_cast<Player>( shared_from_this() ) ))
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

	if (riding == NULL) 
	{
		if( minecartAchievementPos != NULL )
		{
			delete minecartAchievementPos;
			minecartAchievementPos = NULL;
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

			level->setTime( 0 );
			int poweredCount = 0;
			for(int i = 10; i < 2800; ++i)
			{
				level->setTile(x+i,y-1,z-2,Tile::quartzBlock_Id);
				level->setTile(x+i,y,z-2,Tile::quartzBlock_Id);
				level->setTile(x+i,y+1,z-2,Tile::quartzBlock_Id);
				level->setTile(x+i,y+2,z-2,Tile::lightGem_Id);
				level->setTile(x+i,y+3,z-2,Tile::quartzBlock_Id);

				level->setTile(x+i,y-1,z-1,Tile::stoneBrick_Id);
				if(i%20 == 0)
				{
					level->setTile(x+i,y,z-1,Tile::notGate_on_Id);
					poweredCount = 4;
				}
				else
				{
					level->setTile(x+i,y,z-1,0);
				}
				level->setTile(x+i,y+1,z-1,0);
				level->setTile(x+i,y+2,z-1,0);
				level->setTile(x+i,y+3,z-1,0);

				level->setTile(x+i,y-1,z,Tile::stoneBrick_Id);
				if(poweredCount>0)
				{
					level->setTile(x+i,y,z,Tile::goldenRail_Id);
					--poweredCount;
				}
				else
				{
					level->setTile(x+i,y,z,Tile::rail_Id);
				}
				level->setTile(x+i,y+1,z,0);
				level->setTile(x+i,y+2,z,0);
				level->setTile(x+i,y+3,z,0);

				level->setTile(x+i,y-1,z+1,Tile::stoneBrick_Id);
				if((i+5)%20 == 0)
				{
					level->setTile(x+i,y,z+1,Tile::torch_Id);
				}
				else
				{
					level->setTile(x+i,y,z+1,0);
				}
				level->setTile(x+i,y+1,z+1,0);
				level->setTile(x+i,y+2,z+1,0);
				level->setTile(x+i,y+3,z+1,0);

				level->setTile(x+i,y-1,z+2,Tile::quartzBlock_Id);
				level->setTile(x+i,y,z+2,Tile::quartzBlock_Id);
				level->setTile(x+i,y+1,z+2,Tile::quartzBlock_Id);
				level->setTile(x+i,y+2,z+2,Tile::lightGem_Id);
				level->setTile(x+i,y+3,z+2,Tile::quartzBlock_Id);
			}
			madeTrack = true;
		}
#endif
	}
	//End 4J sTU
}

void Player::spawnEatParticles(shared_ptr<ItemInstance> useItem, int count)
{
	if (useItem->getUseAnimation() == UseAnim_drink)
	{
		level->playSound(shared_from_this(), eSoundType_RANDOM_DRINK, 0.5f, level->random->nextFloat() * 0.1f + 0.9f);
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
		level->playSound(shared_from_this(), eSoundType_RANDOM_EAT, 0.5f + 0.5f * random->nextInt(2), (random->nextFloat() - random->nextFloat()) * 0.2f + 1.0f);
	}
}

void Player::completeUsingItem()
{
	if (useItem != NULL)
	{
		spawnEatParticles(useItem, 16);

		int oldCount = useItem->count;
		shared_ptr<ItemInstance> itemInstance = useItem->useTimeDepleted(level, dynamic_pointer_cast<Player>(shared_from_this()));
		if (itemInstance != useItem || (itemInstance != NULL && itemInstance->count != oldCount))
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
		Mob::handleEntityEvent(id);
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
	if (riding != NULL && e == NULL)
	{
		if (!level->isClientSide) findStandUpPosition(riding);

		if (riding != NULL)
		{
			riding->rider = weak_ptr<Entity>();
		}
		riding = nullptr;

		return;
	}
	Mob::ride(e);
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
			playerSkin = (EDefaultSkins) defaultSkinIndex;
		}
	}

	if( playerSkin == eDefaultSkins_ServerSelected)
	{
		playerSkin = (EDefaultSkins)(m_playerIndex + 1);
	}

	// We always set a default skin, since we may be waiting for the player's custom skin to be transmitted
	setPlayerDefaultSkin( playerSkin );

	m_dwSkinId = skinId;
	this->customTextureUrl = app.getSkinPathFromId(skinId);

	// set the new player additional boxes
	/*vector<ModelPart *> *pvModelParts=app.GetAdditionalModelParts(m_dwSkinId);

	if(pvModelParts==NULL)
	{
	// we don't have the data from the dlc skin yet
	app.DebugPrintf("Couldn't get model parts for skin %X\n",m_dwSkinId);

	// do we have it from the DLC pack?
	DLCSkinFile *pDLCSkinFile = app.m_dlcManager.getSkinFile(this->customTextureUrl);

	if(pDLCSkinFile!=NULL)
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
	this->SetAdditionalModelParts(NULL);
	}
	app.SetAnimOverrideBitmask(pDLCSkinFile->getSkinID(),pDLCSkinFile->getAnimOverrideBitmask());
	}
	else
	{
	this->SetAdditionalModelParts(NULL);
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
	this->SetAdditionalModelParts(NULL);


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
	double preX = x, preY = y, preZ = z;
	float preYRot = yRot, preXRot = xRot;

	this->Mob::rideTick();
	oBob = bob;
	bob = 0;

	checkRidingStatistiscs(x - preX, y - preY, z - preZ);

	// riding can be set to null inside 'Entity::rideTick()'.
	if ( riding != NULL	&& (riding->GetType() & eTYPE_PIG) == eTYPE_PIG )
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
	this->Mob::resetPos();
	setHealth(getMaxHealth());
	deathTime = 0;
}

int Player::getCurrentSwingDuration()
{
	if (hasEffect(MobEffect::digSpeed))
	{
		return SWING_DURATION - (1 + getEffect(MobEffect::digSpeed)->getAmplifier()) * 1;
	}
	if (hasEffect(MobEffect::digSlowdown))
	{
		return SWING_DURATION + (1 + getEffect(MobEffect::digSlowdown)->getAmplifier()) * 2;
	}
	return SWING_DURATION;
}

void Player::serverAiStep()
{
	int currentSwingDuration = getCurrentSwingDuration();
	if (swinging)
	{
		swingTime++;
		if (swingTime >= currentSwingDuration)
		{
			swingTime = 0;
			swinging = false;
		}
	}
	else
	{
		swingTime = 0;
	}

	attackAnim = swingTime / (float) currentSwingDuration;
}


void Player::aiStep()
{
	if (jumpTriggerTime > 0) jumpTriggerTime--;

	if (level->difficulty == Difficulty::PEACEFUL && getHealth() < getMaxHealth())
	{
		if (tickCount % 20 * 12 == 0) heal(1);
	}
	inventory->tick();
	oBob = bob;

	this->Mob::aiStep();

	this->walkingSpeed = abilities.getWalkingSpeed();
	this->flyingSpeed = defaultFlySpeed;
	if (isSprinting())
	{
		walkingSpeed += abilities.getWalkingSpeed() * 0.3f;
		flyingSpeed += defaultFlySpeed * 0.3f;
	}

	float tBob = (float) sqrt(xd * xd + zd * zd);

	// 4J added - we were getting a NaN with zero xd & zd
	if(( xd * xd + zd * zd ) < 0.00001f )
	{
		tBob = 0.0f;
	}

	float tTilt = (float) atan(-yd * 0.2f) * 15.0f;
	if (tBob > 0.1f) tBob = 0.1f;
	if (!onGround || getHealth() <= 0) tBob = 0;
	if (onGround || getHealth() <= 0) tTilt = 0;

	bob += (tBob - bob) * 0.4f;

	tilt += (tTilt - tilt) * 0.8f;

	if (getHealth() > 0)
	{
		vector<shared_ptr<Entity> > *entities = level->getEntities(shared_from_this(), bb->grow(1, 0, 1));
		if (entities != NULL)
		{
			AUTO_VAR(itEnd, entities->end());
			for (AUTO_VAR(it, entities->begin()); it != itEnd; it++)
			{
				shared_ptr<Entity> e = *it; //entities->at(i);
				if (!e->removed)
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

// 4J - Removed 1.0.1
//bool Player::addResource(int resource)
//{
//	return inventory->add(shared_ptr<ItemInstance>( new ItemInstance(resource, 1, 0) ) );
//}

int Player::getScore()
{
	return score;
}

void Player::die(DamageSource *source)
{
	this->Mob::die(source);
	this->setSize(0.2f, 0.2f);
	setPos(x, y, z);
	yd = 0.1f;

	// 4J - TODO need to use a xuid
	if ( app.isXuidNotch( m_xuid ) )
	{
		drop(shared_ptr<ItemInstance>( new ItemInstance(Item::apple, 1) ), true);
	}
	inventory->dropAll();

	if (source != NULL)
	{
		xd = -Mth::cos((hurtDir + yRot) * PI / 180) * 0.1f;
		zd = -Mth::sin((hurtDir + yRot) * PI / 180) * 0.1f;
	}
	else
	{
		xd = zd = 0;
	}
	this->heightOffset = 0.1f;
}

void Player::awardKillScore(shared_ptr<Entity> victim, int score)
{
	this->score += score;
}

int Player::decreaseAirSupply(int currentSupply)
{
	int oxygenBonus = EnchantmentHelper::getOxygenBonus(inventory);
	if (oxygenBonus > 0)
	{
		if (random->nextInt(oxygenBonus + 1) > 0)
		{
			// the oxygen bonus prevents us from drowning
			return currentSupply;
		}
	}
	return Mob::decreaseAirSupply(currentSupply);
}

bool Player::isShootable()
{
	return true;
}

bool Player::isCreativeModeAllowed()
{
	return true;
}

shared_ptr<ItemEntity> Player::drop()
{
	return drop(inventory->removeItem(inventory->selected, 1), false);
}

shared_ptr<ItemEntity> Player::drop(shared_ptr<ItemInstance> item)
{
	return drop(item, false);
}

shared_ptr<ItemEntity> Player::drop(shared_ptr<ItemInstance> item, bool randomly)
{
	if (item == NULL) return nullptr;

	shared_ptr<ItemEntity> thrownItem = shared_ptr<ItemEntity>( new ItemEntity(level, x, y - 0.3f + getHeadHeight(), z, item) );
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


float Player::getDestroySpeed(Tile *tile)
{
	float speed = inventory->getDestroySpeed(tile);

	int efficiency = EnchantmentHelper::getDiggingBonus(inventory);
	if (efficiency > 0 && inventory->canDestroy(tile))
	{
		speed += (efficiency * efficiency + 1);
	}

	if (hasEffect(MobEffect::digSpeed))
	{
		speed *= 1.0f + (getEffect(MobEffect::digSpeed)->getAmplifier() + 1) * .2f;
	}
	if (hasEffect(MobEffect::digSlowdown))
	{
		speed *= 1.0f - (getEffect(MobEffect::digSlowdown)->getAmplifier() + 1) * .2f;
	}

	if (isUnderLiquid(Material::water) && !EnchantmentHelper::hasWaterWorkerBonus(inventory)) speed /= 5;

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
	Mob::readAdditionalSaveData(entityTag);
	ListTag<CompoundTag> *inventoryList = (ListTag<CompoundTag> *) entityTag->getList(L"Inventory");
	inventory->load(inventoryList);
	dimension = entityTag->getInt(L"Dimension");
	m_isSleeping = entityTag->getBoolean(L"Sleeping");
	sleepCounter = entityTag->getShort(L"SleepTimer");

	experienceProgress = entityTag->getFloat(L"XpP");
	experienceLevel = entityTag->getInt(L"XpLevel");
	totalExperience = entityTag->getInt(L"XpTotal");

	if (m_isSleeping)
	{
		bedPosition = new Pos( Mth::floor(x), Mth::floor(y), Mth::floor(z));
		stopSleepInBed(true, true, false);
	}

	if (entityTag->contains(L"SpawnX") && entityTag->contains(L"SpawnY") && entityTag->contains(L"SpawnZ"))
	{
		respawnPosition = new Pos(entityTag->getInt(L"SpawnX"), entityTag->getInt(L"SpawnY"), entityTag->getInt(L"SpawnZ"));
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
	Mob::addAdditonalSaveData(entityTag);
	entityTag->put(L"Inventory", inventory->save(new ListTag<CompoundTag>()));
	entityTag->putInt(L"Dimension", dimension);
	entityTag->putBoolean(L"Sleeping", m_isSleeping);
	entityTag->putShort(L"SleepTimer", (short) sleepCounter);

	entityTag->putFloat(L"XpP", experienceProgress);
	entityTag->putInt(L"XpLevel", experienceLevel);
	entityTag->putInt(L"XpTotal", totalExperience);

	if (respawnPosition != NULL)
	{
		entityTag->putInt(L"SpawnX", respawnPosition->x);
		entityTag->putInt(L"SpawnY", respawnPosition->y);
		entityTag->putInt(L"SpawnZ", respawnPosition->z);
	}

	foodData.addAdditonalSaveData(entityTag);
	abilities.addSaveData(entityTag);

	entityTag->put(L"EnderItems", enderChestInventory->createTag());

	// 4J Added
	entityTag->putInt(L"GamePrivileges",m_uiGamePrivileges);

}

Pos *Player::getRespawnPosition(Level *level, CompoundTag *entityTag)
{
	if (entityTag->contains(L"SpawnX") && entityTag->contains(L"SpawnY") && entityTag->contains(L"SpawnZ"))
	{
		return new Pos(entityTag->getInt(L"SpawnX"), entityTag->getInt(L"SpawnY"), entityTag->getInt(L"SpawnZ"));
	}
	return level->getSharedSpawnPos();
}

bool Player::openContainer(shared_ptr<Container> container)
{
	return true;
}

bool Player::startEnchanting(int x, int y, int z)
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

void Player::take(shared_ptr<Entity> e, int orgCount)
{
}

float Player::getHeadHeight()
{
	return 0.12f;
}


void Player::setDefaultHeadHeight()
{
	heightOffset = 1.62f;
}

bool Player::hurt(DamageSource *source, int dmg)
{
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
	if ( dynamic_pointer_cast<Arrow>( attacker ) != NULL )
	{
		if ((dynamic_pointer_cast<Arrow>(attacker))->owner != NULL)
		{
			attacker = (dynamic_pointer_cast<Arrow>(attacker))->owner;
		}
	}
	if ( dynamic_pointer_cast<Mob>( attacker ) != NULL )
	{
		// aggreviate all pet wolves nearby
		directAllTameWolvesOnTarget(dynamic_pointer_cast<Mob>(attacker), false);
	}

	return this->Mob::hurt(source, dmg);
}

int Player::getDamageAfterMagicAbsorb(DamageSource *damageSource, int damage)
{
	int remainingDamage = Mob::getDamageAfterMagicAbsorb(damageSource, damage);
	if (remainingDamage <= 0)
	{
		return 0;
	}

	int enchantmentArmor = EnchantmentHelper::getDamageProtection(inventory, damageSource);
	if (enchantmentArmor > 20)
	{
		enchantmentArmor = 20;
	}
	if (enchantmentArmor > 0 && enchantmentArmor <= 20)
	{
		int absorb = 25 - enchantmentArmor;
		int v = remainingDamage * absorb + dmgSpill;
		remainingDamage = v / 25;
		dmgSpill = v % 25;
	}

	return remainingDamage;
}

bool Player::isPlayerVersusPlayer()
{
	return false;
}

void Player::directAllTameWolvesOnTarget(shared_ptr<Mob> target, bool skipSitting)
{

	// filter un-attackable mobs
	if ((dynamic_pointer_cast<Creeper>( target ) != NULL) || (dynamic_pointer_cast<Ghast>( target) != NULL))
	{
		return;
	}
	// never target wolves that has this player as owner
	if (dynamic_pointer_cast<Wolf>(target) != NULL)
	{
		shared_ptr<Wolf> wolfTarget = dynamic_pointer_cast<Wolf>(target);
		if (wolfTarget->isTame() && m_UUID.compare( wolfTarget->getOwnerUUID() ) == 0 )
		{
			return;
		}
	}
	if ((dynamic_pointer_cast<Player>( target ) != NULL) && !isPlayerVersusPlayer())
	{
		// pvp is off
		return;
	}


	// TODO: Optimize this? Most of the time players wont have pets:
	vector<shared_ptr<Entity> > *nearbyWolves = level->getEntitiesOfClass(typeid(Wolf), AABB::newTemp(x, y, z, x + 1, y + 1, z + 1)->grow(16, 4, 16));
	AUTO_VAR(itEnd, nearbyWolves->end());
	for (AUTO_VAR(it, nearbyWolves->begin()); it != itEnd; it++)
	{
		shared_ptr<Wolf> wolf = dynamic_pointer_cast<Wolf>(*it);;
		if (wolf->isTame() && wolf->getAttackTarget() == NULL && m_UUID.compare( wolf->getOwnerUUID() ) == 0)
		{
			if (!skipSitting || !wolf->isSitting())
			{
				wolf->setSitting(false);
				wolf->setAttackTarget(target);
			}
		}
	}
	delete nearbyWolves;

}

void Player::hurtArmor(int damage)
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
        if (inventory->armor[i] != NULL) {
            count++;
        }
    }
    return (float) count / (float) inventory->armor.length;
}

void Player::actuallyHurt(DamageSource *source, int dmg)
{
	if (!source->isBypassArmor() && isBlocking())
	{
		dmg = (1 + dmg) >> 1;
	}
	dmg = getDamageAfterArmorAbsorb(source, dmg);
	dmg = getDamageAfterMagicAbsorb(source, dmg);
	causeFoodExhaustion(source->getFoodExhaustion());
	//this->Mob::actuallyHurt(source, dmg);
	health -= dmg;
}


bool Player::openFurnace(shared_ptr<FurnaceTileEntity> container)
{
	return true;
}

bool Player::openTrap(shared_ptr<DispenserTileEntity> container)
{
	return true;
}

void Player::openTextEdit(shared_ptr<SignTileEntity> sign)
{
}

bool Player::openBrewingStand(shared_ptr<BrewingStandTileEntity> brewingStand)
{
	return true;
}

bool Player::openTrading(shared_ptr<Merchant> traderTarget)
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
	if (entity->interact( dynamic_pointer_cast<Player>( shared_from_this() ) )) return true;
	shared_ptr<ItemInstance> item = getSelectedItem();
	if (item != NULL && dynamic_pointer_cast<Mob>( entity ) != NULL)
	{		
		// 4J - PC Comments
		// Hack to prevent item stacks from decrementing if the player has
		// the ability to instabuild
		if(this->abilities.instabuild) item = item->copy();
		if(item->interactEnemy(dynamic_pointer_cast<Mob>(entity)))
		{
			// 4J - PC Comments
			// Don't remove the item in hand if the player has the ability
			// to
			// instabuild
			if (item->count <= 0 && !this->abilities.instabuild)
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

void Player::swing()
{
	if (!swinging || swingTime >= getCurrentSwingDuration() / 2 || swingTime < 0)
	{
		swingTime = -1;
		swinging = true;
	}
}

void Player::attack(shared_ptr<Entity> entity)
{
	if (!entity->isAttackable())
	{
		return;
	}

	int dmg = inventory->getAttackDamage(entity);

	if (hasEffect(MobEffect::damageBoost))
	{
		dmg += (3 << getEffect(MobEffect::damageBoost)->getAmplifier());
	}
	if (hasEffect(MobEffect::weakness))
	{
		dmg -= (2 << getEffect(MobEffect::weakness)->getAmplifier());
	}

	int knockback = 0;
	int magicBoost = 0;
	shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(entity);
	if (mob != NULL)
	{
		magicBoost = EnchantmentHelper::getDamageBonus(inventory, mob);
		knockback += EnchantmentHelper::getKnockbackBonus(inventory, mob);
	}
	if (isSprinting())
	{
		knockback += 1;
	}

	if (dmg > 0 || magicBoost > 0)
	{
		bool bCrit = fallDistance > 0 && !onGround && !onLadder() && !isInWater() && !hasEffect(MobEffect::blindness) && riding == NULL && mob != NULL;
		if (bCrit)
		{
			dmg += random->nextInt(dmg / 2 + 2);
		}
		dmg += magicBoost;
		
		// Ensure we put the entity on fire if we're hitting with a
		// fire-enchanted weapon
		bool setOnFireTemporatily = false;
		int fireAspect = EnchantmentHelper::getFireAspect(dynamic_pointer_cast<Mob>(shared_from_this()));
		if (dynamic_pointer_cast<Mob>(entity) && fireAspect > 0 && !entity->isOnFire())
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

			shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(entity);
			if (mob)
			{
				ThornsEnchantment::doThornsAfterAttack(shared_from_this(), mob, random);
			}
		}

		shared_ptr<ItemInstance> item = getSelectedItem();
		if (item != NULL && dynamic_pointer_cast<Mob>( entity ) != NULL)
		{
			item->hurtEnemy(dynamic_pointer_cast<Mob>(entity), dynamic_pointer_cast<Player>( shared_from_this() ) );
			if (item->count <= 0)
			{
				removeSelectedItem();
			}
		}
		if (dynamic_pointer_cast<Mob>( entity ) != NULL)
		{
			if (entity->isAlive())
			{
				directAllTameWolvesOnTarget(dynamic_pointer_cast<Mob>(entity), true);
			}
			// 4J Stu - Brought forward wasHurt check to Fix 66140 - Bug: Fire Aspect bypasses "Player v Player" being Disabled
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
	return NULL;
}

void Player::remove()
{
	this->Mob::remove();
	inventoryMenu->removed( dynamic_pointer_cast<Player>( shared_from_this() ) );
	if (containerMenu != NULL)
	{
		containerMenu->removed( dynamic_pointer_cast<Player>( shared_from_this() ) );
	}
}

bool Player::isInWall()
{
	return !m_isSleeping && this->Mob::isInWall();
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

	// 4J Stu - You can use a bed from within a minecart, and this causes all sorts of problems.
	ride(nullptr);

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
	if (pos != NULL && level->getTile(pos->x, pos->y, pos->z) == Tile::bed_Id)
	{
		BedTile::setOccupied(level, pos->x, pos->y, pos->z, false);

		standUp = BedTile::findStandUpPosition(level, pos->x, pos->y, pos->z, 0);
		if (standUp == NULL)
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
		setRespawnPosition(bedPosition);
	}
}


bool Player::checkBed()
{
	return (level->getTile(bedPosition->x, bedPosition->y, bedPosition->z) == Tile::bed_Id);
}


Pos *Player::checkBedValidRespawnPosition(Level *level, Pos *pos)
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
		return NULL;
	}
	// make sure the bed still has a stand-up position
	Pos *standUp = BedTile::findStandUpPosition(level, pos->x, pos->y, pos->z, 0);
	return standUp;
}

float Player::getSleepRotation()
{
	if (bedPosition != NULL)
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
		entityData->set(DATA_PLAYER_FLAGS_ID, (byte) (currentValue | (1 << flag)));
	}
	else
	{
		entityData->set(DATA_PLAYER_FLAGS_ID, (byte) (currentValue & ~(1 << flag)));
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

void Player::setRespawnPosition(Pos *respawnPosition)
{
	if (respawnPosition != NULL)
	{
		this->respawnPosition = new Pos(*respawnPosition);
	}
	else
	{
		this->respawnPosition = NULL;
	}
}

void Player::awardStat(Stat *stat, byteArray paramBlob)
{
	if (paramBlob.data != NULL)
	{
		delete [] paramBlob.data;
	}
}


void Player::jumpFromGround()
{
	this->Mob::jumpFromGround();

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

	if (abilities.flying && riding == NULL)
	{
		double ydo = this->yd;
		float ofs = flyingSpeed;
		flyingSpeed = abilities.getFlyingSpeed();
		this->Mob::travel(xa, ya);
		this->yd = ydo * 0.6;
		flyingSpeed = ofs;
	}
	else
	{
		this->Mob::travel(xa, ya);
	}

	checkMovementStatistiscs(x - preX, y - preY, z - preZ);
}


void Player::checkMovementStatistiscs(double dx, double dy, double dz)
{

	if (riding != NULL)
	{
		return;
	}
	if (isUnderLiquid(Material::water))
	{
		int distance = (int) Math::round(sqrt(dx * dx + dy * dy + dz * dz) * 100.0f);
		if (distance > 0)
		{
			//awardStat(Stats::diveOneCm, distance);
			causeFoodExhaustion(FoodConstants::EXHAUSTION_SWIM * distance * .01f);
		}
	}
	else if (isInWater())
	{
		int horizontalDistance = (int) Math::round(sqrt(dx * dx + dz * dz) * 100.0f);
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
			distanceClimb += (int) Math::round(dy * 100.0f);
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
		int horizontalDistance = (int) Math::round(sqrt(dx * dx + dz * dz) * 100.0f);
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
	if (riding != NULL)
	{
		int distance = (int) Math::round(sqrt(dx * dx + dy * dy + dz * dz) * 100.0f);
		if (distance > 0)
		{
			if ( dynamic_pointer_cast<Minecart>( riding ) )
			{
				distanceMinecart += distance;
				if( distanceMinecart >= 100 )
				{
					int newDistance = distanceMinecart - (distanceMinecart % 100);
					distanceMinecart -= newDistance;
					awardStat( GenericStats::minecartOneM(), GenericStats::param_minecart(newDistance/100) );
				}

				int dist = 0;
				if (minecartAchievementPos == NULL)
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
			else if (dynamic_pointer_cast<Boat>( riding ) != NULL)
			{
				distanceBoat += distance;
				if( distanceBoat >= 100 )
				{
					int newDistance = distanceBoat - (distanceBoat % 100);
					distanceBoat -= newDistance;
					awardStat(GenericStats::boatOneM(), GenericStats::param_boat(newDistance/100) );
				}
			}
			else if (dynamic_pointer_cast<Pig>( riding ) != NULL)
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
		distanceFall += (int) Math::round(distance * 100.0);
		if( distanceFall >= 100 )
		{
			int newDistance = distanceFall - (distanceFall % 100);
			distanceFall -= newDistance;
			awardStat(GenericStats::fallOneM(), GenericStats::param_fall(newDistance/100) );
		}
	}
	this->Mob::causeFallDamage(distance);
}


void Player::killed(shared_ptr<Mob> mob)
{
	// 4J-PB - added the lavaslime enemy - fix for #64007 - TU7: Code: Achievements: TCR#073: Killing Magma Cubes doesn't unlock "Monster Hunter" Achievement.
	if( dynamic_pointer_cast<Monster>( mob ) != NULL || mob->GetType() == eTYPE_GHAST || mob->GetType() == eTYPE_SLIME || mob->GetType() == eTYPE_LAVASLIME || mob->GetType() == eTYPE_ENDERDRAGON)
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
			if( mob->rider.lock() != NULL && mob->rider.lock()->GetType() == eTYPE_SKELETON )
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

Icon *Player::getItemInHandIcon(shared_ptr<ItemInstance> item, int layer)
{
	Icon *icon = Mob::getItemInHandIcon(item, layer);
	if (item->id == Item::fishingRod->id && fishing != NULL)
	{
		icon = Item::fishingRod->getEmptyIcon();
	}
	else if (item->getItem()->hasMultipleSpriteLayers())
	{
		return item->getItem()->getLayerIcon(item->getAuxValue(), layer);
	}
	else if (useItem != NULL && item->id == Item::bow_Id)
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

void Player::handleInsidePortal()
{
	if (changingDimensionDelay > 0)
	{
		changingDimensionDelay = 10;
		return;
	}

	isInsidePortal = true;
}

void Player::increaseXp(int i)
{
	// Update xp calculations from 1.3
	score += i;
	int max = INT_MAX - totalExperience;
	if (i > max)
	{
		i = max;
	}
	experienceProgress += (float) i / getXpNeededForNextLevel();
	totalExperience += i;
	while (experienceProgress >= 1)
	{
		experienceProgress = (experienceProgress - 1) * getXpNeededForNextLevel();
		levelUp();
		experienceProgress /= getXpNeededForNextLevel();
	}
}

void Player::withdrawExperienceLevels(int amount)
{
	experienceLevel -= amount;
	if (experienceLevel < 0)
	{
		experienceLevel = 0;
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

void Player::levelUp()
{
	experienceLevel++;
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

bool Player::mayBuild(int x, int y, int z)
{
	return abilities.mayBuild;
}

int Player::getExperienceReward(shared_ptr<Player> killedBy)
{
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

void Player::changeDimension(int i)
{
}

void Player::restoreFrom(shared_ptr<Player> oldPlayer, bool restoreAll)
{
	if(restoreAll)
	{
		inventory->replaceWith(oldPlayer->inventory);

		health = oldPlayer->health;
		foodData = oldPlayer->foodData;

		experienceLevel = oldPlayer->experienceLevel;
		totalExperience = oldPlayer->totalExperience;
		experienceProgress = oldPlayer->experienceProgress;

		score = oldPlayer->score;

		for(AUTO_VAR(it, oldPlayer->activeEffects.begin()); it != oldPlayer->activeEffects.end(); ++it)
		{
			MobEffectInstance *instance = it->second;
			addEffectNoUpdate( instance );
		}
		oldPlayer->activeEffects.clear();
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
	return displayName;
}

//Language getLanguage() { return Language.getInstance(); }
//String localize(String key, Object... args) { return getLanguage().getElement(key, args); }

shared_ptr<PlayerEnderChestContainer> Player::getEnderChestInventory()
{
	return enderChestInventory;
}

shared_ptr<ItemInstance> Player::getCarriedItem()
{
	return inventory->getSelected();
}

bool Player::isInvisibleTo(shared_ptr<Player> player)
{
    return isInvisible();
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
	return (int)std::hash<wstring>{}( k->name ); // 4J Stu - Names are completely unique?
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
	if(tile != NULL && app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
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
	if(item != NULL && app.GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
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
		if (target->GetType() == eTYPE_MINECART)
		{
			if (getPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseContainers) == 0)
			{
				shared_ptr<Minecart> minecart = dynamic_pointer_cast<Minecart>( target );
				if (minecart->type == Minecart::CHEST)
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
	if(m_ppAdditionalModelParts==NULL && !m_bCheckedForModelParts)
	{
		bool hasCustomTexture = !customTextureUrl.empty();
		bool customTextureIsDefaultSkin = customTextureUrl.substr(0,3).compare(L"def") == 0;

		// see if we can find the parts
		m_ppAdditionalModelParts=app.GetAdditionalModelParts(m_dwSkinId);

		// If it's a default texture (which has no parts), we have the parts, or we already have the texture (in which case we should have parts if there are any) then we are done
		if(!hasCustomTexture || customTextureIsDefaultSkin || m_ppAdditionalModelParts != NULL || app.IsFileInMemoryTextures(customTextureUrl))
		{
			m_bCheckedForModelParts=true;
		}
		if(m_ppAdditionalModelParts == NULL && !m_bCheckedDLCForModelParts)
		{
			m_bCheckedDLCForModelParts = true;

			// we don't have the data from the dlc skin yet
			app.DebugPrintf("m_bCheckedForModelParts Couldn't get model parts for skin %X\n",m_dwSkinId);

			// do we have it from the DLC pack?
			DLCSkinFile *pDLCSkinFile = app.m_dlcManager.getSkinFile(this->customTextureUrl);

			if(pDLCSkinFile!=NULL)
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
