#pragma once

// 4J-PB added to avoid string compares on adding particles
enum ePARTICLE_TYPE
{
	eParticleType_bubble,
	eParticleType_smoke,
	eParticleType_note,
	eParticleType_netherportal, // 4J - This particle should only be used by the Nether portal. Everything else should use eParticleType_end
	eParticleType_endportal, // 4J - Seperated this from torches and fires
	eParticleType_explode,
	eParticleType_flame,
	eParticleType_lava,
	eParticleType_footstep,
	eParticleType_splash,
	eParticleType_largesmoke,
	eParticleType_reddust,
	eParticleType_snowballpoof,
	eParticleType_snowshovel,
	eParticleType_slime,
	eParticleType_heart,
	eParticleType_suspended,
	eParticleType_depthsuspend,
	eParticleType_crit,
	eParticleType_hugeexplosion,
	eParticleType_largeexplode,
	eParticleType_townaura,
	eParticleType_spell,
	eParticleType_witchMagic,
	eParticleType_mobSpell,
	eParticleType_mobSpellAmbient,
	eParticleType_instantSpell,
	eParticleType_magicCrit,
	eParticleType_dripWater,
	eParticleType_dripLava,
	eParticleType_enchantmenttable,
	eParticleType_dragonbreath,
	eParticleType_ender, // 4J Added - These are things that used the "portal" particle but are actually end related entities
	eParticleType_angryVillager,
	eParticleType_happyVillager,
	eParticleType_fireworksspark,

	// 4J-JEV: In the java, the particle name was used to sneak parameters in for the Terrain and IconCrack particle constructors.
	
	eParticleType_iconcrack_base = 0x100000,	// There's  range of iconcrack particle types based on item id and data.
	eParticleType_iconcrack_last = 0x1FFFFF,
	eParticleType_tilecrack_base = 0x200000,	// There's a range of tilecrack particle types based on tile id and data.
	eParticleType_tilecrack_last = 0x2FFFFF,
								// 0x0000FF, <- these bits are for storing the data value.
								// 0x0FFF00, <- these bits are for encoding tile/item id.
								// 0x300000, <- these bits show if its an icon/tile or not.

};

#define PARTICLE_TILECRACK(id,data) ( (ePARTICLE_TYPE) ( ((int) eParticleType_tilecrack_base) | ((0x0FFF & id) << 8) | (0x0FF & data)) )
#define PARTICLE_ICONCRACK(id,data) ( (ePARTICLE_TYPE) ( ((int) eParticleType_iconcrack_base) | ((0x0FFF & id) << 8) | (0x0FF & data)) )

#define PARTICLE_CRACK_ID(ePType)   ((0x0FFF00 & (int)ePType) >> 8)
#define PARTICLE_CRACK_DATA(ePType) (0x0FF & (int)ePType)