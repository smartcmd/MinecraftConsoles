#include "stdafx.h"
#include "ColourTable.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

unordered_map<wstring,eMinecraftColour> ColourTable::s_colourNamesMap;

const wchar_t *ColourTable::ColourTableElements[eMinecraftColour_COUNT] =
{
	L"NOTSET",

	L"Foliage_Evergreen",
	L"Foliage_Birch",
	L"Foliage_Default",
	L"Foliage_Common",
	L"Foliage_Ocean",
	L"Foliage_Plains",
	L"Foliage_Desert",
	L"Foliage_ExtremeHills",
	L"Foliage_Forest",
	L"Foliage_Taiga",
	L"Foliage_Swampland",
	L"Foliage_River",
	L"Foliage_Hell",
	L"Foliage_Sky",
	L"Foliage_FrozenOcean",
	L"Foliage_FrozenRiver",
	L"Foliage_IcePlains",
	L"Foliage_IceMountains",
	L"Foliage_MushroomIsland",
	L"Foliage_MushroomIslandShore",
	L"Foliage_Beach",
	L"Foliage_DesertHills",
	L"Foliage_ForestHills",
	L"Foliage_TaigaHills",
	L"Foliage_ExtremeHillsEdge",
	L"Foliage_Jungle",
	L"Foliage_JungleHills",
	
	L"Grass_Common",
	L"Grass_Ocean",
	L"Grass_Plains",
	L"Grass_Desert",
	L"Grass_ExtremeHills",
	L"Grass_Forest",
	L"Grass_Taiga",
	L"Grass_Swampland",
	L"Grass_River",
	L"Grass_Hell",
	L"Grass_Sky",
	L"Grass_FrozenOcean",
	L"Grass_FrozenRiver",
	L"Grass_IcePlains",
	L"Grass_IceMountains",
	L"Grass_MushroomIsland",
	L"Grass_MushroomIslandShore",
	L"Grass_Beach",
	L"Grass_DesertHills",
	L"Grass_ForestHills",
	L"Grass_TaigaHills",
	L"Grass_ExtremeHillsEdge",
	L"Grass_Jungle",
	L"Grass_JungleHills",

	L"Water_Ocean",
	L"Water_Plains",
	L"Water_Desert",
	L"Water_ExtremeHills",
	L"Water_Forest",
	L"Water_Taiga",
	L"Water_Swampland",
	L"Water_River",
	L"Water_Hell",
	L"Water_Sky",
	L"Water_FrozenOcean",
	L"Water_FrozenRiver",
	L"Water_IcePlains",
	L"Water_IceMountains",
	L"Water_MushroomIsland",
	L"Water_MushroomIslandShore",
	L"Water_Beach",
	L"Water_DesertHills",
	L"Water_ForestHills",
	L"Water_TaigaHills",
	L"Water_ExtremeHillsEdge",
	L"Water_Jungle",
	L"Water_JungleHills",

	L"Sky_Ocean",
	L"Sky_Plains",
	L"Sky_Desert",
	L"Sky_ExtremeHills",
	L"Sky_Forest",
	L"Sky_Taiga",
	L"Sky_Swampland",
	L"Sky_River",
	L"Sky_Hell",
	L"Sky_Sky",
	L"Sky_FrozenOcean",
	L"Sky_FrozenRiver",
	L"Sky_IcePlains",
	L"Sky_IceMountains",
	L"Sky_MushroomIsland",
	L"Sky_MushroomIslandShore",
	L"Sky_Beach",
	L"Sky_DesertHills",
	L"Sky_ForestHills",
	L"Sky_TaigaHills",
	L"Sky_ExtremeHillsEdge",
	L"Sky_Jungle",
	L"Sky_JungleHills",

	L"Tile_RedstoneDust",
	L"Tile_RedstoneDustUnlit",
	L"Tile_RedstoneDustLitMin",
	L"Tile_RedstoneDustLitMax",
	L"Tile_StemMin",
	L"Tile_StemMax",
	L"Tile_WaterLily",

	L"Sky_Dawn_Dark",
	L"Sky_Dawn_Bright",

	L"Material_None",
	L"Material_Grass",
	L"Material_Sand",
	L"Material_Cloth",
	L"Material_Fire",
	L"Material_Ice",
	L"Material_Metal",
	L"Material_Plant",
	L"Material_Snow",
	L"Material_Clay",
	L"Material_Dirt",
	L"Material_Stone",
	L"Material_Water",
	L"Material_Wood",
	L"Material_Emerald",

	L"Particle_Note_00",
	L"Particle_Note_01",
	L"Particle_Note_02",
	L"Particle_Note_03",
	L"Particle_Note_04",
	L"Particle_Note_05",
	L"Particle_Note_06",
	L"Particle_Note_07",
	L"Particle_Note_08",
	L"Particle_Note_09",
	L"Particle_Note_10",
	L"Particle_Note_11",
	L"Particle_Note_12",
	L"Particle_Note_13",
	L"Particle_Note_14",
	L"Particle_Note_15",
	L"Particle_Note_16",
	L"Particle_Note_17",
	L"Particle_Note_18",
	L"Particle_Note_19",
	L"Particle_Note_20",
	L"Particle_Note_21",
	L"Particle_Note_22",
	L"Particle_Note_23",
	L"Particle_Note_24",

	L"Particle_NetherPortal",
	L"Particle_EnderPortal",
	L"Particle_Smoke",
	L"Particle_Ender",

	L"Particle_Explode",
	L"Particle_HugeExplosion",

	L"Particle_DripWater",
	L"Particle_DripLavaStart",
	L"Particle_DripLavaEnd",

	L"Particle_EnchantmentTable",
	L"Particle_DragonBreathMin",
	L"Particle_DragonBreathMax",
	L"Particle_Suspend",

	L"Particle_CritStart", // arrow in air
	L"Particle_CritEnd", // arrow in air
	
	L"Effect_MovementSpeed",
	L"Effect_MovementSlowDown",
	L"Effect_DigSpeed",
	L"Effect_DigSlowdown",
	L"Effect_DamageBoost",
	L"Effect_Heal",
	L"Effect_Harm",
	L"Effect_Jump",
	L"Effect_Confusion",
	L"Effect_Regeneration",
	L"Effect_DamageResistance",
	L"Effect_FireResistance",
	L"Effect_WaterBreathing",
	L"Effect_Invisiblity",
	L"Effect_Blindness",
	L"Effect_NightVision",
	L"Effect_Hunger",
	L"Effect_Weakness",
	L"Effect_Poison",
	L"Effect_Wither",
	L"Effect_HealthBoost",
	L"Effect_Absorption",
	L"Effect_Saturation",

	L"Potion_BaseColour",

	L"Mob_Creeper_Colour1",
	L"Mob_Creeper_Colour2",
	L"Mob_Skeleton_Colour1",
	L"Mob_Skeleton_Colour2",
	L"Mob_Spider_Colour1",
	L"Mob_Spider_Colour2",
	L"Mob_Zombie_Colour1",
	L"Mob_Zombie_Colour2",
	L"Mob_Slime_Colour1",
	L"Mob_Slime_Colour2",
	L"Mob_Ghast_Colour1",
	L"Mob_Ghast_Colour2",
	L"Mob_PigZombie_Colour1",
	L"Mob_PigZombie_Colour2",
	L"Mob_Enderman_Colour1",
	L"Mob_Enderman_Colour2",
	L"Mob_CaveSpider_Colour1",
	L"Mob_CaveSpider_Colour2",
	L"Mob_Silverfish_Colour1",
	L"Mob_Silverfish_Colour2",
	L"Mob_Blaze_Colour1",
	L"Mob_Blaze_Colour2",
	L"Mob_LavaSlime_Colour1",
	L"Mob_LavaSlime_Colour2",
	L"Mob_Pig_Colour1",
	L"Mob_Pig_Colour2",
	L"Mob_Sheep_Colour1",
	L"Mob_Sheep_Colour2",
	L"Mob_Cow_Colour1",
	L"Mob_Cow_Colour2",
	L"Mob_Chicken_Colour1",
	L"Mob_Chicken_Colour2",
	L"Mob_Squid_Colour1",
	L"Mob_Squid_Colour2",
	L"Mob_Wolf_Colour1",
	L"Mob_Wolf_Colour2",
	L"Mob_MushroomCow_Colour1",
	L"Mob_MushroomCow_Colour2",
	L"Mob_Ocelot_Colour1",
	L"Mob_Ocelot_Colour2",
	L"Mob_Villager_Colour1",
	L"Mob_Villager_Colour2",
	L"Mob_Bat_Colour1",
	L"Mob_Bat_Colour2",
	L"Mob_Witch_Colour1",
	L"Mob_Witch_Colour2",
	L"Mob_Horse_Colour1",
	L"Mob_Horse_Colour2",

	L"Armour_Default_Leather_Colour",
	L"Under_Water_Clear_Colour",
	L"Under_Lava_Clear_Colour",
	L"In_Cloud_Base_Colour",

	L"Under_Water_Fog_Colour",
	L"Under_Lava_Fog_Colour",
	L"In_Cloud_Fog_Colour",

	L"Default_Fog_Colour",
	L"Nether_Fog_Colour",
	L"End_Fog_Colour",

	L"Sign_Text",
	L"Map_Text",

	L"Leash_Light_Colour",
	L"Leash_Dark_Colour",

	L"Fire_Overlay",
	
	L"HTMLColor_0",
	L"HTMLColor_1",
	L"HTMLColor_2",
	L"HTMLColor_3",
	L"HTMLColor_4",
	L"HTMLColor_5",
	L"HTMLColor_6",
	L"HTMLColor_7",
	L"HTMLColor_8",
	L"HTMLColor_9",
	L"HTMLColor_a",
	L"HTMLColor_b",
	L"HTMLColor_c",
	L"HTMLColor_d",
	L"HTMLColor_e",
	L"HTMLColor_f",
	L"HTMLColor_dark_0",
	L"HTMLColor_dark_1",
	L"HTMLColor_dark_2",
	L"HTMLColor_dark_3",
	L"HTMLColor_dark_4",
	L"HTMLColor_dark_5",
	L"HTMLColor_dark_6",
	L"HTMLColor_dark_7",
	L"HTMLColor_dark_8",
	L"HTMLColor_dark_9",
	L"HTMLColor_dark_a",
	L"HTMLColor_dark_b",
	L"HTMLColor_dark_c",
	L"HTMLColor_dark_d",
	L"HTMLColor_dark_e",
	L"HTMLColor_dark_f",
	L"HTMLColor_T1",
	L"HTMLColor_T2",
	L"HTMLColor_T3",
	L"HTMLColor_Black",
	L"HTMLColor_White",
	L"Color_EnchantText",
	L"Color_EnchantTextFocus",
	L"Color_EnchantTextDisabled",
	L"Color_RenamedItemTitle",
};

void ColourTable::staticCtor()
{
	for(unsigned int i = eMinecraftColour_NOT_SET; i < eMinecraftColour_COUNT; ++i)
	{
		s_colourNamesMap.insert( unordered_map<wstring,eMinecraftColour>::value_type( ColourTableElements[i], static_cast<eMinecraftColour>(i)) );
	}
}

ColourTable::ColourTable(PBYTE pbData, DWORD dwLength)
{
	loadColoursFromData(pbData, dwLength);
}

ColourTable::ColourTable(ColourTable *defaultColours, PBYTE pbData, DWORD dwLength)
{
	// 4J Stu - Default the colours that of the table passed in
	XMemCpy( (void *)m_colourValues, (void *)defaultColours->m_colourValues, sizeof(int) * eMinecraftColour_COUNT);
	loadColoursFromData(pbData, dwLength);
}
void ColourTable::loadColoursFromData(PBYTE pbData, DWORD dwLength)
{
	byteArray src(pbData, dwLength);

	ByteArrayInputStream bais(src);
	DataInputStream dis(&bais);

	int versionNumber = dis.readInt();
	int coloursCount = dis.readInt();

	for(int i = 0; i < coloursCount; ++i)
	{
		wstring colourId = dis.readUTF();
		int colourValue = dis.readInt();
		setColour(colourId, colourValue);
		auto it = s_colourNamesMap.find(colourId); // ?
	}

	bais.reset();
}

void ColourTable::setColour(const wstring &colourName, int value)
{
	auto it = s_colourNamesMap.find(colourName);
	if(it != s_colourNamesMap.end())
	{
		m_colourValues[static_cast<int>(it->second)] = value;
	}
}

void ColourTable::setColour(const wstring &colourName, const wstring &value)
{
	setColour(colourName, _fromHEXString<int>(value));
}

unsigned int ColourTable::getColour(eMinecraftColour id)
{
	return m_colourValues[static_cast<int>(id)];
}
