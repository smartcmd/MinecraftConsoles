#include "stdafx.h"
#include "Class.h"
#include "Painting.h"
#include "System.h"
#include "Entity.h"
#include "WitherBoss.h"
#include "net.minecraft.world.entity.ambient.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.item.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "net.minecraft.world.entity.npc.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.level.h"
#include "com.mojang.nbt.h"
#include "EntityIO.h"

unordered_map<wstring, entityCreateFn> *EntityIO::idCreateMap = new unordered_map<wstring, entityCreateFn>;
unordered_map<eINSTANCEOF, wstring, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq> *EntityIO::classIdMap = new unordered_map<eINSTANCEOF, wstring, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>;
unordered_map<int, entityCreateFn> *EntityIO::numCreateMap = new unordered_map<int, entityCreateFn>;
unordered_map<int, eINSTANCEOF> *EntityIO::numClassMap = new unordered_map<int, eINSTANCEOF>;
unordered_map<eINSTANCEOF, int, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq> *EntityIO::classNumMap = new unordered_map<eINSTANCEOF, int, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>;
unordered_map<wstring, int> *EntityIO::idNumMap = new unordered_map<wstring, int>;
unordered_map<int, EntityIO::SpawnableMobInfo *> EntityIO::idsSpawnableInCreative;

void EntityIO::setId(entityCreateFn createFn, eINSTANCEOF clas, const wstring &id, int idNum)
{
	idCreateMap->insert( unordered_map<wstring, entityCreateFn>::value_type(id, createFn) );
	classIdMap->insert( unordered_map<eINSTANCEOF, wstring, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>::value_type(clas,id ) );
	numCreateMap->insert( unordered_map<int, entityCreateFn>::value_type(idNum, createFn) );
	numClassMap->insert( unordered_map<int, eINSTANCEOF>::value_type(idNum, clas) );
	classNumMap->insert( unordered_map<eINSTANCEOF, int, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>::value_type(clas, idNum) );
	idNumMap->insert( unordered_map<wstring, int>::value_type(id, idNum) );
}

void EntityIO::setId(entityCreateFn createFn, eINSTANCEOF clas, const wstring &id, int idNum, eMinecraftColour color1, eMinecraftColour color2, int nameId)
{
	setId(createFn, clas, id, idNum);

	idsSpawnableInCreative.insert( unordered_map<int, SpawnableMobInfo *>::value_type( idNum, new SpawnableMobInfo(idNum, color1, color2, nameId) ) );
}

void EntityIO::staticCtor()
{
	setId(ItemEntity::create, eTYPE_ITEMENTITY,  L"Item", 1);
	setId(ExperienceOrb::create, eTYPE_EXPERIENCEORB, L"XPOrb", 2);

	setId(LeashFenceKnotEntity::create, eTYPE_LEASHFENCEKNOT, L"LeashKnot", 8);
	setId(Painting::create, eTYPE_PAINTING, L"Painting", 9);
	setId(Arrow::create, eTYPE_ARROW, L"Arrow", 10);
	setId(Snowball::create, eTYPE_SNOWBALL, L"Snowball", 11);
	setId(LargeFireball::create, eTYPE_FIREBALL, L"Fireball", 12);
	setId(SmallFireball::create, eTYPE_SMALL_FIREBALL, L"SmallFireball", 13);
	setId(ThrownEnderpearl::create, eTYPE_THROWNENDERPEARL, L"ThrownEnderpearl", 14);
	setId(EyeOfEnderSignal::create, eTYPE_EYEOFENDERSIGNAL, L"EyeOfEnderSignal", 15);
	setId(ThrownPotion::create, eTYPE_THROWNPOTION, L"ThrownPotion", 16);
	setId(ThrownExpBottle::create, eTYPE_THROWNEXPBOTTLE, L"ThrownExpBottle", 17);
	setId(ItemFrame::create, eTYPE_ITEM_FRAME, L"ItemFrame", 18);
	setId(WitherSkull::create, eTYPE_WITHER_SKULL, L"WitherSkull", 19);

	setId(PrimedTnt::create, eTYPE_PRIMEDTNT, L"PrimedTnt", 20);
	setId(FallingTile::create, eTYPE_FALLINGTILE, L"FallingSand", 21);

	setId(FireworksRocketEntity::create, eTYPE_FIREWORKS_ROCKET, L"FireworksRocketEntity", 22);

	setId(Boat::create, eTYPE_BOAT, L"Boat", 41);
	setId(MinecartRideable::create, eTYPE_MINECART_RIDEABLE, L"MinecartRideable", 42);
	setId(MinecartChest::create, eTYPE_MINECART_CHEST, L"MinecartChest", 43);
	setId(MinecartFurnace::create, eTYPE_MINECART_FURNACE, L"MinecartFurnace", 44);
	setId(MinecartTNT::create, eTYPE_MINECART_TNT, L"MinecartTNT", 45);
	setId(MinecartHopper::create, eTYPE_MINECART_HOPPER, L"MinecartHopper", 46);
	setId(MinecartSpawner::create, eTYPE_MINECART_SPAWNER, L"MinecartSpawner", 47);

	setId(Mob::create, eTYPE_MOB, L"Mob", 48);
	setId(Monster::create, eTYPE_MONSTER, L"Monster", 49);

	setId(Creeper::create, eTYPE_CREEPER, L"Creeper", 50, eMinecraftColour_Mob_Creeper_Colour1, eMinecraftColour_Mob_Creeper_Colour2, IDS_CREEPER);
	setId(Skeleton::create, eTYPE_SKELETON, L"Skeleton", 51, eMinecraftColour_Mob_Skeleton_Colour1, eMinecraftColour_Mob_Skeleton_Colour2, IDS_SKELETON);
	setId(Spider::create, eTYPE_SPIDER, L"Spider", 52, eMinecraftColour_Mob_Spider_Colour1, eMinecraftColour_Mob_Spider_Colour2, IDS_SPIDER);
	setId(Giant::create, eTYPE_GIANT, L"Giant", 53);
	setId(Zombie::create, eTYPE_ZOMBIE, L"Zombie", 54, eMinecraftColour_Mob_Zombie_Colour1, eMinecraftColour_Mob_Zombie_Colour2, IDS_ZOMBIE);
	setId(Slime::create, eTYPE_SLIME, L"Slime", 55, eMinecraftColour_Mob_Slime_Colour1, eMinecraftColour_Mob_Slime_Colour2, IDS_SLIME);
	setId(Ghast::create, eTYPE_GHAST, L"Ghast", 56, eMinecraftColour_Mob_Ghast_Colour1, eMinecraftColour_Mob_Ghast_Colour2, IDS_GHAST);
	setId(PigZombie::create, eTYPE_PIGZOMBIE, L"PigZombie", 57, eMinecraftColour_Mob_PigZombie_Colour1, eMinecraftColour_Mob_PigZombie_Colour2, IDS_PIGZOMBIE);
	setId(EnderMan::create, eTYPE_ENDERMAN, L"Enderman", 58, eMinecraftColour_Mob_Enderman_Colour1, eMinecraftColour_Mob_Enderman_Colour2, IDS_ENDERMAN);
	setId(CaveSpider::create, eTYPE_CAVESPIDER, L"CaveSpider", 59, eMinecraftColour_Mob_CaveSpider_Colour1, eMinecraftColour_Mob_CaveSpider_Colour2, IDS_CAVE_SPIDER);
	setId(Silverfish::create, eTYPE_SILVERFISH, L"Silverfish", 60, eMinecraftColour_Mob_Silverfish_Colour1, eMinecraftColour_Mob_Silverfish_Colour2, IDS_SILVERFISH);
	setId(Blaze::create, eTYPE_BLAZE, L"Blaze", 61, eMinecraftColour_Mob_Blaze_Colour1, eMinecraftColour_Mob_Blaze_Colour2, IDS_BLAZE);
	setId(LavaSlime::create, eTYPE_LAVASLIME, L"LavaSlime", 62, eMinecraftColour_Mob_LavaSlime_Colour1, eMinecraftColour_Mob_LavaSlime_Colour2, IDS_LAVA_SLIME);
	setId(EnderDragon::create, eTYPE_ENDERDRAGON, L"EnderDragon", 63, eMinecraftColour_Mob_Enderman_Colour1, eMinecraftColour_Mob_Enderman_Colour1, IDS_ENDERDRAGON);
	setId(WitherBoss::create, eTYPE_WITHERBOSS, L"WitherBoss", 64);
	setId(Bat::create, eTYPE_BAT, L"Bat", 65, eMinecraftColour_Mob_Bat_Colour1, eMinecraftColour_Mob_Bat_Colour2, IDS_BAT);
	setId(Witch::create, eTYPE_WITCH, L"Witch", 66, eMinecraftColour_Mob_Witch_Colour1, eMinecraftColour_Mob_Witch_Colour2, IDS_WITCH);

	setId(Pig::create, eTYPE_PIG, L"Pig", 90, eMinecraftColour_Mob_Pig_Colour1, eMinecraftColour_Mob_Pig_Colour2, IDS_PIG);
	setId(Sheep::create, eTYPE_SHEEP, L"Sheep", 91, eMinecraftColour_Mob_Sheep_Colour1, eMinecraftColour_Mob_Sheep_Colour2, IDS_SHEEP);
	setId(Cow::create, eTYPE_COW, L"Cow", 92, eMinecraftColour_Mob_Cow_Colour1, eMinecraftColour_Mob_Cow_Colour2, IDS_COW);
	setId(Chicken::create, eTYPE_CHICKEN, L"Chicken", 93, eMinecraftColour_Mob_Chicken_Colour1, eMinecraftColour_Mob_Chicken_Colour2, IDS_CHICKEN);
	setId(Squid::create, eTYPE_SQUID, L"Squid", 94, eMinecraftColour_Mob_Squid_Colour1, eMinecraftColour_Mob_Squid_Colour2, IDS_SQUID);
	setId(Wolf::create, eTYPE_WOLF, L"Wolf", 95, eMinecraftColour_Mob_Wolf_Colour1, eMinecraftColour_Mob_Wolf_Colour2, IDS_WOLF);
	setId(MushroomCow::create, eTYPE_MUSHROOMCOW, L"MushroomCow", 96, eMinecraftColour_Mob_MushroomCow_Colour1, eMinecraftColour_Mob_MushroomCow_Colour2, IDS_MUSHROOM_COW);
	setId(SnowMan::create, eTYPE_SNOWMAN, L"SnowMan", 97);
	setId(Ocelot::create, eTYPE_OCELOT, L"Ozelot", 98, eMinecraftColour_Mob_Ocelot_Colour1, eMinecraftColour_Mob_Ocelot_Colour2, IDS_OZELOT);
	setId(VillagerGolem::create, eTYPE_VILLAGERGOLEM, L"VillagerGolem", 99);
	setId(EntityHorse::create, eTYPE_HORSE, L"EntityHorse", 100, eMinecraftColour_Mob_Horse_Colour1, eMinecraftColour_Mob_Horse_Colour2, IDS_HORSE);

	setId(Villager::create, eTYPE_VILLAGER, L"Villager", 120, eMinecraftColour_Mob_Villager_Colour1, eMinecraftColour_Mob_Villager_Colour2, IDS_VILLAGER);

	setId(EnderCrystal::create, eTYPE_ENDER_CRYSTAL, L"EnderCrystal", 200);

	// 4J Added
	setId(DragonFireball::create, eTYPE_DRAGON_FIREBALL, L"DragonFireball", 1000);

	// 4J-PB - moved to allow the eggs to be named and coloured in the Creative Mode menu
	// 4J Added for custom spawn eggs
	setId(EntityHorse::create,	eTYPE_HORSE,	L"EntityHorse", 100 | ((EntityHorse::TYPE_DONKEY + 1) << 12),	eMinecraftColour_Mob_Horse_Colour1,		eMinecraftColour_Mob_Horse_Colour2,		IDS_DONKEY);
	setId(EntityHorse::create,	eTYPE_HORSE,	L"EntityHorse", 100 | ((EntityHorse::TYPE_MULE + 1) << 12),		eMinecraftColour_Mob_Horse_Colour1,		eMinecraftColour_Mob_Horse_Colour2,		IDS_MULE);

#ifndef _CONTENT_PACKAGE
	setId(EntityHorse::create,	eTYPE_HORSE,	L"EntityHorse", 100 | ((EntityHorse::TYPE_SKELETON + 1) << 12),	eMinecraftColour_Mob_Horse_Colour1,		eMinecraftColour_Mob_Horse_Colour2,		IDS_SKELETON_HORSE);
	setId(EntityHorse::create,	eTYPE_HORSE,	L"EntityHorse", 100 | ((EntityHorse::TYPE_UNDEAD + 1) << 12),	eMinecraftColour_Mob_Horse_Colour1,		eMinecraftColour_Mob_Horse_Colour2,		IDS_ZOMBIE_HORSE);
	setId(Ocelot::create,		eTYPE_OCELOT,	L"Ozelot",		98 | ((Ocelot::TYPE_BLACK + 1) << 12),			eMinecraftColour_Mob_Ocelot_Colour1,	eMinecraftColour_Mob_Ocelot_Colour2,	IDS_OZELOT );
	setId(Ocelot::create,		eTYPE_OCELOT,	L"Ozelot",		98 | ((Ocelot::TYPE_RED + 1) << 12),			eMinecraftColour_Mob_Ocelot_Colour1,	eMinecraftColour_Mob_Ocelot_Colour2,	IDS_OZELOT );
	setId(Ocelot::create,		eTYPE_OCELOT,	L"Ozelot",		98 | ((Ocelot::TYPE_SIAMESE + 1) << 12),		eMinecraftColour_Mob_Ocelot_Colour1,	eMinecraftColour_Mob_Ocelot_Colour2,	IDS_OZELOT );
	setId(Spider::create,		eTYPE_SPIDER,	L"Spider",		52 | (2 << 12),									eMinecraftColour_Mob_Spider_Colour1,	eMinecraftColour_Mob_Spider_Colour2,	IDS_SKELETON );
#endif
}

shared_ptr<Entity> EntityIO::newEntity(const wstring& id, Level *level)
{
	shared_ptr<Entity> entity;

	auto it = idCreateMap->find(id);
	if(it != idCreateMap->end() )
	{
		entityCreateFn create = it->second;
		if (create != NULL) entity = shared_ptr<Entity>(create(level));
		if( ( entity != NULL ) && entity->GetType() == eTYPE_ENDERDRAGON )
		{
			dynamic_pointer_cast<EnderDragon>(entity)->AddParts();		// 4J added to finalise creation
		}
	}

	return entity;
}

shared_ptr<Entity> EntityIO::loadStatic(CompoundTag *tag, Level *level)
{
	shared_ptr<Entity> entity;

	if (tag->getString(L"id").compare(L"Minecart") == 0)
	{
		// I don't like this any more than you do. Sadly, compatibility...

		switch (tag->getInt(L"Type"))
		{
		case Minecart::TYPE_CHEST:
			tag->putString(L"id", L"MinecartChest");
			break;
		case Minecart::TYPE_FURNACE:
			tag->putString(L"id", L"MinecartFurnace");
			break;
		case Minecart::TYPE_RIDEABLE:
			tag->putString(L"id", L"MinecartRideable");
			break;
		}

		tag->remove(L"Type");
	}

	auto it = idCreateMap->find(tag->getString(L"id"));
	if(it != idCreateMap->end() )
	{
		entityCreateFn create = it->second;
		if (create != NULL) entity = shared_ptr<Entity>(create(level));
		if( ( entity != NULL ) && entity->GetType() == eTYPE_ENDERDRAGON )
		{
			dynamic_pointer_cast<EnderDragon>(entity)->AddParts();		// 4J added to finalise creation
		}
	}

	if (entity != NULL)
	{
		entity->load(tag);
	}
	else
	{
#ifdef _DEBUG
		app.DebugPrintf("Skipping Entity with id %ls\n", tag->getString(L"id").c_str() );
#endif
	}
	return entity;
}

shared_ptr<Entity> EntityIO::newById(int id, Level *level)
{
	shared_ptr<Entity> entity;

	auto it = numCreateMap->find(id);
	if(it != numCreateMap->end() )
	{
		entityCreateFn create = it->second;
		if (create != NULL) entity = shared_ptr<Entity>(create(level));
		if( ( entity != NULL ) && entity->GetType() == eTYPE_ENDERDRAGON )
		{
			dynamic_pointer_cast<EnderDragon>(entity)->AddParts();		// 4J added to finalise creation
		}
	}

	if (entity != NULL)
	{
	}
	else
	{
		//printf("Skipping Entity with id %d\n", id ) ;
	}
	return entity;
}

shared_ptr<Entity> EntityIO::newByEnumType(eINSTANCEOF eType, Level *level)
{
	shared_ptr<Entity> entity;

	auto it = classNumMap->find( eType );
	if( it != classNumMap->end() )
	{
		auto it2 = numCreateMap->find(it->second);
		if(it2 != numCreateMap->end() )
		{
			entityCreateFn create = it2->second;
			if (create != NULL) entity = shared_ptr<Entity>(create(level));
			if( ( entity != NULL ) && entity->GetType() == eTYPE_ENDERDRAGON )
			{
				dynamic_pointer_cast<EnderDragon>(entity)->AddParts();		// 4J added to finalise creation
			}
		}
	}

	return entity;
}

int EntityIO::getId(shared_ptr<Entity> entity)
{
	unordered_map<eINSTANCEOF, int, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>::iterator it = classNumMap->find( entity->GetType() );
	return (*it).second;
}

wstring EntityIO::getEncodeId(shared_ptr<Entity> entity)
{
	unordered_map<eINSTANCEOF, wstring, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>::iterator it = classIdMap->find( entity->GetType() );
	if( it != classIdMap->end() )
		return (*it).second;
	else
		return L"";
}

int EntityIO::getId(const wstring &encodeId)
{
	auto it = idNumMap->find(encodeId);
	if (it == idNumMap->end())
	{
		// defaults to pig...
		return 90;
	}
	return it->second;
}

wstring EntityIO::getEncodeId(int entityIoValue)
{
	//Class<? extends Entity> class1 = numClassMap.get(entityIoValue);
	//if (class1 != null)
	//{
	//return classIdMap.get(class1);
	//}

	auto it = numClassMap->find(entityIoValue);
	if(it != numClassMap->end() )
	{
		unordered_map<eINSTANCEOF, wstring, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>::iterator classIdIt = classIdMap->find( it->second );
		if( classIdIt != classIdMap->end() )
			return (*classIdIt).second;
		else
			return L"";
	}

	return L"";
}

int EntityIO::getNameId(int entityIoValue)
{
	int id = -1;

	auto it = idsSpawnableInCreative.find(entityIoValue);
	if(it != idsSpawnableInCreative.end())
	{
		id = it->second->nameId;
	}

	return id;
}

eINSTANCEOF EntityIO::getType(const wstring &idString)
{
	auto it = numClassMap->find(getId(idString));
	if(it != numClassMap->end() )
	{
		return it->second;
	}
	return eTYPE_NOTSET;
}

eINSTANCEOF EntityIO::getClass(int id)
{
	auto it = numClassMap->find(id);
	if(it != numClassMap->end() )
	{
		return it->second;
	}
	return eTYPE_NOTSET;
}

int EntityIO::eTypeToIoid(eINSTANCEOF eType)
{
	unordered_map<eINSTANCEOF, int, eINSTANCEOFKeyHash, eINSTANCEOFKeyEq>::iterator it = classNumMap->find( eType );
	if( it != classNumMap->end() )
		return it->second;
	return -1;
}
