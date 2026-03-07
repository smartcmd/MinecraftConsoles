#include "stdafx.h"

#include "Consoles_SoundEngine.h"



const WCHAR *ConsoleSoundEngine::wchSoundNames[eSoundType_MAX]=
{
	L"mob.chicken.say",						//	eSoundType_MOB_CHICKEN_AMBIENT	
	L"mob.chicken.hurt",					//	eSoundType_MOB_CHICKEN_HURT
	L"mob.chicken.plop",					//	eSoundType_MOB_CHICKENPLOP
	L"mob.cow.say",							//	eSoundType_MOB_COW_AMBIENT
	L"mob.cow.hurt",						//	eSoundType_MOB_COW_HURT
	L"mob.pig.say",							//	eSoundType_MOB_PIG_AMBIENT
	L"mob.pig.death",					//	eSoundType_MOB_PIG_DEATH
	L"mob.sheep.say",						//	eSoundType_MOB_SHEEP_AMBIENT
	L"mob.wolf.growl",					//	eSoundType_MOB_WOLF_GROWL
	L"mob.wolf.whine",					//	eSoundType_MOB_WOLF_WHINE
	L"mob.wolf.panting",				//	eSoundType_MOB_WOLF_PANTING
	L"mob.wolf.bark",					//	eSoundType_MOB_WOLF_BARK
	L"mob.wolf.hurt",					//	eSoundType_MOB_WOLF_HURT
	L"mob.wolf.death",					//	eSoundType_MOB_WOLF_DEATH
	L"mob.wolf.shake",					//	eSoundType_MOB_WOLF_SHAKE
	L"mob.blaze.breathe",				//	eSoundType_MOB_BLAZE_BREATHE
	L"mob.blaze.hit",					//	eSoundType_MOB_BLAZE_HURT
	L"mob.blaze.death",					//	eSoundType_MOB_BLAZE_DEATH
	L"mob.ghast.moan",					//	eSoundType_MOB_GHAST_MOAN
	L"mob.ghast.scream",				//	eSoundType_MOB_GHAST_SCREAM
	L"mob.ghast.death",					//	eSoundType_MOB_GHAST_DEATH
	L"mob.ghast.fireball",				//	eSoundType_MOB_GHAST_FIREBALL
	L"mob.ghast.charge",				//	eSoundType_MOB_GHAST_CHARGE
	L"mob.endermen.idle",				//	eSoundType_MOB_ENDERMEN_IDLE
	L"mob.endermen.hit",				//	eSoundType_MOB_ENDERMEN_HIT
	L"mob.endermen.death",				//	eSoundType_MOB_ENDERMEN_DEATH
	L"mob.endermen.portal",				//	eSoundType_MOB_ENDERMEN_PORTAL
	L"mob.zombiepig.zpig",				//	eSoundType_MOB_ZOMBIEPIG_AMBIENT
	L"mob.zombiepig.zpighurt",			//	eSoundType_MOB_ZOMBIEPIG_HURT
	L"mob.zombiepig.zpigdeath",			//	eSoundType_MOB_ZOMBIEPIG_DEATH
	L"mob.zombiepig.zpigangry",			//	eSoundType_MOB_ZOMBIEPIG_ZPIGANGRY
	L"mob.silverfish.say",				//	eSoundType_MOB_SILVERFISH_AMBIENT,
	L"mob.silverfish.hit",				//	eSoundType_MOB_SILVERFISH_HURT
	L"mob.silverfish.kill",				//	eSoundType_MOB_SILVERFISH_DEATH,
	L"mob.silverfish.step",				//	eSoundType_MOB_SILVERFISH_STEP,
	L"mob.skeleton",					//	eSoundType_MOB_SKELETON_AMBIENT,
	L"mob.skeletonhurt",				//	eSoundType_MOB_SKELETON_HURT,
	L"mob.spider.say",						//	eSoundType_MOB_SPIDER_AMBIENT,
	L"mob.spider.death",					//	eSoundType_MOB_SPIDER_DEATH,
	L"mob.slime",						//	eSoundType_MOB_SLIME,
	L"mob.slime.attack",					//	eSoundType_MOB_SLIME_ATTACK,
	L"mob.creeper.say",						//	eSoundType_MOB_CREEPER_HURT,
	L"mob.creeper.death",				//	eSoundType_MOB_CREEPER_DEATH,
	L"mob.zombie.say",						//	eSoundType_MOB_ZOMBIE_AMBIENT,
	L"mob.zombie.hurt",					//	eSoundType_MOB_ZOMBIE_HURT,
	L"mob.zombie.death",					//	eSoundType_MOB_ZOMBIE_DEATH,
	L"mob.zombie.wood",					//	eSoundType_MOB_ZOMBIE_WOOD,
	L"mob.zombie.woodbreak",			//	eSoundType_MOB_ZOMBIE_WOOD_BREAK,
	L"mob.zombie.metal",				//	eSoundType_MOB_ZOMBIE_METAL,
	L"mob.magmacube.big",				//	eSoundType_MOB_MAGMACUBE_BIG,
	L"mob.magmacube.small",				//	eSoundType_MOB_MAGMACUBE_SMALL,
	L"mob.cat.purr",					//  eSoundType_MOB_CAT_PURR
	L"mob.cat.purreow",					//  eSoundType_MOB_CAT_PURREOW
	L"mob.cat.meow",					//  eSoundType_MOB_CAT_MEOW
	// 4J-PB - correct the name of the event for hitting ocelots
	L"mob.cat.hit",					//  eSoundType_MOB_CAT_HITT
//	L"mob.irongolem.throw",				//  eSoundType_MOB_IRONGOLEM_THROW
//	L"mob.irongolem.hit",				//  eSoundType_MOB_IRONGOLEM_HIT
//	L"mob.irongolem.death",				//  eSoundType_MOB_IRONGOLEM_DEATH
//	L"mob.irongolem.walk",				//  eSoundType_MOB_IRONGOLEM_WALK
	L"random.bow",						//	eSoundType_RANDOM_BOW,
	L"random.bowhit",					//	eSoundType_RANDOM_BOW_HIT,
	L"random.explode",					//	eSoundType_RANDOM_EXPLODE,
	L"random.fizz",						//	eSoundType_RANDOM_FIZZ,	
	L"random.pop",						//	eSoundType_RANDOM_POP,
	L"random.fuse",						//	eSoundType_RANDOM_FUSE,
	L"random.drink",					//	eSoundType_RANDOM_DRINK,
	L"random.eat",						//	eSoundType_RANDOM_EAT,
	L"random.burp",						//	eSoundType_RANDOM_BURP,
	L"random.splash",					//	eSoundType_RANDOM_SPLASH,
	L"random.click",					//	eSoundType_RANDOM_CLICK,
	L"random.glass",					//	eSoundType_RANDOM_GLASS,
	L"random.orb",						//	eSoundType_RANDOM_ORB,
	L"random.break",					//	eSoundType_RANDOM_BREAK,
	L"random.chestopen",				//	eSoundType_RANDOM_CHEST_OPEN,
	L"random.chestclosed",				//	eSoundType_RANDOM_CHEST_CLOSE,
	L"random.door_open",				//	eSoundType_RANDOM_DOOR_OPEN,
	L"random.door_close",				//	eSoundType_RANDOM_DOOR_CLOSE,
	L"ambient.weather.rain",			//	eSoundType_AMBIENT_WEATHER_RAIN,
	L"ambient.weather.thunder",			//	eSoundType_AMBIENT_WEATHER_THUNDER,
	L"ambient.cave",				//	eSoundType_CAVE_CAVE, DON'T USE FOR XBOX 360!!!
#ifdef _XBOX
	L"ambient.cave.cave2",				//	eSoundType_CAVE_CAVE2 - removed the two sounds that were at 192k in the first ambient cave event
#endif
	L"portal.portal",					//	eSoundType_PORTAL_PORTAL,
	// 4J-PB - added a couple that were still using wstring
	L"portal.trigger",					//	eSoundType_PORTAL_TRIGGER
	L"portal.travel",					//	eSoundType_PORTAL_TRAVEL
	
	L"fire.ignite",						//	eSoundType_FIRE_IGNITE,
	L"fire.fire",						//	eSoundType_FIRE_FIRE,
	L"damage.hurtflesh",				//	eSoundType_DAMAGE_HURT,
	L"damage.fallsmall",				//	eSoundType_DAMAGE_FALL_SMALL,
	L"damage.fallbig",					//	eSoundType_DAMAGE_FALL_BIG,
	L"note.harp",						//	eSoundType_NOTE_HARP,
	L"note.bd",							//	eSoundType_NOTE_BD,
	L"note.snare",						//	eSoundType_NOTE_SNARE,
	L"note.hat",						//	eSoundType_NOTE_HAT,
	L"note.bassattack",					//	eSoundType_NOTE_BASSATTACK,
	L"tile.piston.in",					//	eSoundType_TILE_PISTON_IN,
	L"tile.piston.out",					//	eSoundType_TILE_PISTON_OUT,
	L"liquid.water",					//	eSoundType_LIQUID_WATER,
	L"liquid.lavapop",					//	eSoundType_LIQUID_LAVA_POP,
	L"liquid.lava",						//	eSoundType_LIQUID_LAVA,
	L"step.stone",						//	eSoundType_STEP_STONE,
	L"step.wood",						//	eSoundType_STEP_WOOD,
	L"step.gravel",						//	eSoundType_STEP_GRAVEL,
	L"step.grass",						//	eSoundType_STEP_GRASS,
	L"step.metal",						//	eSoundType_STEP_METAL,
	L"step.cloth",						//	eSoundType_STEP_CLOTH,
	L"step.sand",						//	eSoundType_STEP_SAND,
	
	// below this are the additional sounds from the second soundbank
	L"mob.enderdragon.end",				//	eSoundType_MOB_ENDERDRAGON_END
	L"mob.enderdragon.growl",			//	eSoundType_MOB_ENDERDRAGON_GROWL
	L"mob.enderdragon.hit",				//	eSoundType_MOB_ENDERDRAGON_HIT
	L"mob.enderdragon.wings",			//	eSoundType_MOB_ENDERDRAGON_MOVE
	L"mob.irongolem.throw",				//  eSoundType_MOB_IRONGOLEM_THROW
	L"mob.irongolem.hit",				//  eSoundType_MOB_IRONGOLEM_HIT
	L"mob.irongolem.death",				//  eSoundType_MOB_IRONGOLEM_DEATH
	L"mob.irongolem.walk",				//  eSoundType_MOB_IRONGOLEM_WALK

	// TU14
	L"damage.thorns",					//  eSoundType_DAMAGE_THORNS
	L"random.anvil_break",				//  eSoundType_RANDOM_ANVIL_BREAK
	L"random.anvil_land",				//  eSoundType_RANDOM_ANVIL_LAND
	L"random.anvil_use",				//  eSoundType_RANDOM_ANVIL_USE
	L"mob.villager.haggle",				//  eSoundType_MOB_VILLAGER_HAGGLE
	L"mob.villager.idle",				//  eSoundType_MOB_VILLAGER_IDLE
	L"mob.villager.hit",				//  eSoundType_MOB_VILLAGER_HIT
	L"mob.villager.death",				//  eSoundType_MOB_VILLAGER_DEATH
	L"mob.villager.yes",				//  eSoundType_MOB_VILLAGER_YES
	L"mob.villager.no",					//  eSoundType_MOB_VILLAGER_NO
	L"mob.zombie.infect",				//  eSoundType_MOB_ZOMBIE_INFECT
	L"mob.zombie.unfect",				//  eSoundType_MOB_ZOMBIE_UNFECT
	L"mob.zombie.remedy",				//  eSoundType_MOB_ZOMBIE_REMEDY
	L"step.snow",						//  eSoundType_STEP_SNOW
	L"step.ladder",						//  eSoundType_STEP_LADDER
	L"dig.cloth",						//  eSoundType_DIG_CLOTH	
	L"dig.grass",						//  eSoundType_DIG_GRASS	
	L"dig.gravel",						//  eSoundType_DIG_GRAVEL	
	L"dig.sand",						//  eSoundType_DIG_SAND	
	L"dig.snow",						//  eSoundType_DIG_SNOW	
	L"dig.stone",						//  eSoundType_DIG_STONE	
	L"dig.wood",						//  eSoundType_DIG_WOOD	

	// 1.6.4
	L"fireworks.launch",				//eSoundType_FIREWORKS_LAUNCH,
	L"fireworks.blast",					//eSoundType_FIREWORKS_BLAST,
	L"fireworks.blast_far",				//eSoundType_FIREWORKS_BLAST_FAR,
	L"fireworks.large_blast",			//eSoundType_FIREWORKS_LARGE_BLAST,
	L"fireworks.large_blast_far",		//eSoundType_FIREWORKS_LARGE_BLAST_FAR,
	L"fireworks.twinkle",				//eSoundType_FIREWORKS_TWINKLE,
	L"fireworks.twinkle_far",			//eSoundType_FIREWORKS_TWINKLE_FAR,

	L"mob.bat.idle",					//eSoundType_MOB_BAT_IDLE,
	L"mob.bat.hurt",					//eSoundType_MOB_BAT_HURT,
	L"mob.bat.death",					//eSoundType_MOB_BAT_DEATH,
	L"mob.bat.takeoff",					//eSoundType_MOB_BAT_TAKEOFF,

	L"mob.wither.spawn",				//eSoundType_MOB_WITHER_SPAWN,
	L"mob.wither.idle",					//eSoundType_MOB_WITHER_IDLE,	
	L"mob.wither.hurt",					//eSoundType_MOB_WITHER_HURT, 
	L"mob.wither.death",				//eSoundType_MOB_WITHER_DEATH,
	L"mob.wither.shoot",				//eSoundType_MOB_WITHER_SHOOT,

	L"mob.cow.step",					//eSoundType_MOB_COW_STEP,
	L"mob.chicken.step",				//eSoundType_MOB_CHICKEN_STEP,
	L"mob.pig.step",					//eSoundType_MOB_PIG_STEP,
	L"mob.enderman.stare",				//eSoundType_MOB_ENDERMAN_STARE,
	L"mob.enderman.scream",				//eSoundType_MOB_ENDERMAN_SCREAM,
	L"mob.sheep.shear",					//eSoundType_MOB_SHEEP_SHEAR,
	L"mob.sheep.step",					//eSoundType_MOB_SHEEP_STEP,
	L"mob.skeleton.death",				//eSoundType_MOB_SKELETON_DEATH,
	L"mob.skeleton.step",				//eSoundType_MOB_SKELETON_STEP,
	L"mob.spider.step",					//eSoundType_MOB_SPIDER_STEP,
	L"mob.wolf.step",					//eSoundType_MOB_WOLF_STEP,
	L"mob.zombie.step",					//eSoundType_MOB_ZOMBIE_STEP,

	L"liquid.swim",						//eSoundType_LIQUID_SWIM,

	L"mob.horse.land",					//eSoundType_MOB_HORSE_LAND,
	L"mob.horse.armor",					//eSoundType_MOB_HORSE_ARMOR,
	L"mob.horse.leather",				//eSoundType_MOB_HORSE_LEATHER,
	L"mob.horse.zombie.death",			//eSoundType_MOB_HORSE_ZOMBIE_DEATH,
	L"mob.horse.skeleton.death",		//eSoundType_MOB_HORSE_SKELETON_DEATH,
	L"mob.horse.donkey.death",			//eSoundType_MOB_HORSE_DONKEY_DEATH,
	L"mob.horse.death",					//eSoundType_MOB_HORSE_DEATH,
	L"mob.horse.zombie.hit",			//eSoundType_MOB_HORSE_ZOMBIE_HIT,
	L"mob.horse.skeleton.hit",			//eSoundType_MOB_HORSE_SKELETON_HIT,
	L"mob.horse.donkey.hit",			//eSoundType_MOB_HORSE_DONKEY_HIT,
	L"mob.horse.hit",					//eSoundType_MOB_HORSE_HIT,
	L"mob.horse.zombie.idle",			//eSoundType_MOB_HORSE_ZOMBIE_IDLE,
	L"mob.horse.skeleton.idle",			//eSoundType_MOB_HORSE_SKELETON_IDLE,
	L"mob.horse.donkey.idle",			//eSoundType_MOB_HORSE_DONKEY_IDLE,
	L"mob.horse.idle",					//eSoundType_MOB_HORSE_IDLE,
	L"mob.horse.donkey.angry",			//eSoundType_MOB_HORSE_DONKEY_ANGRY,
	L"mob.horse.angry",					//eSoundType_MOB_HORSE_ANGRY,
	L"mob.horse.gallop",				//eSoundType_MOB_HORSE_GALLOP,
	L"mob.horse.breathe",				//eSoundType_MOB_HORSE_BREATHE,
	L"mob.horse.wood",					//eSoundType_MOB_HORSE_WOOD,
	L"mob.horse.soft",					//eSoundType_MOB_HORSE_SOFT,
	L"mob.horse.jump",					//eSoundType_MOB_HORSE_JUMP,

	L"mob.witch.ambient",					//eSoundType_MOB_WITCH_IDLE,
	L"mob.witch.hurt",					//eSoundType_MOB_WITCH_HURT,
	L"mob.witch.death",					//eSoundType_MOB_WITCH_DEATH,

	L"mob.slime.big",					//eSoundType_MOB_SLIME_BIG,
	L"mob.slime.small",					//eSoundType_MOB_SLIME_SMALL,

	L"eating",							//eSoundType_EATING						<--- missing
	L"random.levelup",					//eSoundType_RANDOM_LEVELUP

	// 4J-PB  - Some sounds were updated, but we can't do that for the 360 or we have to do a new sound bank
	// instead, we'll add the sounds as new ones and change the code to reference them
	L"fire.new_ignite",
};


const WCHAR *ConsoleSoundEngine::wchUISoundNames[eSFX_MAX]=
{
	L"back",
	L"craft",
	L"craftfail",
	L"focus",
	L"press",
	L"scroll",
};
