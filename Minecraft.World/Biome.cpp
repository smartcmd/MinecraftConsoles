#include "stdafx.h"
#include "Color.h"
#include "../Minecraft.Client/Minecraft.h"
#include "net.minecraft.world.level.levelgen.feature.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.entity.animal.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.h"
#include "Biome.h"
#include "net.minecraft.world.level.biome.h"

//public static final Biome[] biomes = new Biome[256];
Biome *Biome::biomes[256];

Biome *Biome::ocean = NULL;
Biome *Biome::plains = NULL;
Biome *Biome::desert = NULL;

Biome *Biome::extremeHills = NULL;
Biome *Biome::forest = NULL;
Biome *Biome::taiga = NULL;

Biome *Biome::swampland = NULL;
Biome *Biome::river = NULL;

Biome *Biome::hell = NULL;
Biome *Biome::sky = NULL;

Biome *Biome::frozenOcean = NULL;
Biome *Biome::frozenRiver = NULL;
Biome *Biome::iceFlats = NULL;
Biome *Biome::iceMountains = NULL;
Biome *Biome::mushroomIsland = NULL;
Biome *Biome::mushroomIslandShore = NULL;
Biome *Biome::beaches = NULL;
Biome *Biome::desertHills = NULL;
Biome *Biome::forestHills = NULL;
Biome *Biome::taigaHills = NULL;
Biome *Biome::smallerExtremeHills = NULL;

Biome *Biome::jungle = NULL;
Biome *Biome::jungleHills = NULL;


void Biome::staticCtor()
{
	//public static final Biome[] biomes = new Biome[256];

	Biome::ocean = (new OceanBiome(0))->setColor(0x000070)->setName(L"Ocean")->setDepthAndScale(-1, 0.4f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Ocean, eMinecraftColour_Foliage_Ocean, eMinecraftColour_Water_Ocean,eMinecraftColour_Sky_Ocean);
	Biome::plains = (new PlainsBiome(1))->setColor(0x8db360)->setName(L"Plains")->setTemperatureAndDownfall(0.8f, 0.4f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Plains, eMinecraftColour_Foliage_Plains, eMinecraftColour_Water_Plains,eMinecraftColour_Sky_Plains);
	Biome::desert = (new DesertBiome(2))->setColor(0xFA9418)->setName(L"Desert")->setNoRain()->setTemperatureAndDownfall(2, 0)->setDepthAndScale(0.1f, 0.2f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Desert, eMinecraftColour_Foliage_Desert, eMinecraftColour_Water_Desert,eMinecraftColour_Sky_Desert);

	Biome::extremeHills = (new ExtremeHillsBiome(3))->setColor(0x606060)->setName(L"Extreme Hills")->setDepthAndScale(0.3f, 1.5f)->setTemperatureAndDownfall(0.2f, 0.3f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_ExtremeHills, eMinecraftColour_Foliage_ExtremeHills, eMinecraftColour_Water_ExtremeHills,eMinecraftColour_Sky_ExtremeHills);
	Biome::forest = (new ForestBiome(4))->setColor(0x056621)->setName(L"Forest")->setLeafColor(0x4EBA31)->setTemperatureAndDownfall(0.7f, 0.8f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Forest, eMinecraftColour_Foliage_Forest, eMinecraftColour_Water_Forest,eMinecraftColour_Sky_Forest);
	// 4J - brought forward Taiga temperature change from 0.3f to 0.05f, from 1.2.3
	Biome::taiga = (new TaigaBiome(5))->setColor(0x0b6659)->setName(L"Taiga")->setLeafColor(0x4EBA31)->setSnowCovered()->setTemperatureAndDownfall(0.05f, 0.8f)->setDepthAndScale(0.1f, 0.4f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Taiga, eMinecraftColour_Foliage_Taiga, eMinecraftColour_Water_Taiga,eMinecraftColour_Sky_Taiga);

	Biome::swampland = (new SwampBiome(6))->setColor(0x07F9B2)->setName(L"Swampland")->setLeafColor(0x8BAF48)->setDepthAndScale(-0.2f, 0.1f)->setTemperatureAndDownfall(0.8f, 0.9f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Swampland, eMinecraftColour_Foliage_Swampland, eMinecraftColour_Water_Swampland,eMinecraftColour_Sky_Swampland);
	Biome::river = (new RiverBiome(7))->setColor(0x0000ff)->setName(L"River")->setDepthAndScale(-0.5f, 0)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_River, eMinecraftColour_Foliage_River, eMinecraftColour_Water_River,eMinecraftColour_Sky_River);

	Biome::hell = (new HellBiome(8))->setColor(0xff0000)->setName(L"Hell")->setNoRain()->setTemperatureAndDownfall(2, 0)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Hell, eMinecraftColour_Foliage_Hell, eMinecraftColour_Water_Hell,eMinecraftColour_Sky_Hell);
	Biome::sky = (new TheEndBiome(9))->setColor(0x8080ff)->setName(L"Sky")->setNoRain()->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Sky, eMinecraftColour_Foliage_Sky, eMinecraftColour_Water_Sky,eMinecraftColour_Sky_Sky);

	Biome::frozenOcean = (new OceanBiome(10))->setColor(0x9090a0)->setName(L"FrozenOcean")->setSnowCovered()->setDepthAndScale(-1, 0.5f)->setTemperatureAndDownfall(0, 0.5f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_FrozenOcean, eMinecraftColour_Foliage_FrozenOcean, eMinecraftColour_Water_FrozenOcean,eMinecraftColour_Sky_FrozenOcean);
	Biome::frozenRiver = (new RiverBiome(11))->setColor(0xa0a0ff)->setName(L"FrozenRiver")->setSnowCovered()->setDepthAndScale(-0.5f, 0)->setTemperatureAndDownfall(0, 0.5f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_FrozenRiver, eMinecraftColour_Foliage_FrozenRiver, eMinecraftColour_Water_FrozenRiver,eMinecraftColour_Sky_FrozenRiver);
	Biome::iceFlats = (new IceBiome(12))->setColor(0xffffff)->setName(L"Ice Plains")->setSnowCovered()->setTemperatureAndDownfall(0, 0.5f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_IcePlains, eMinecraftColour_Foliage_IcePlains, eMinecraftColour_Water_IcePlains,eMinecraftColour_Sky_IcePlains);
	Biome::iceMountains = (new IceBiome(13))->setColor(0xa0a0a0)->setName(L"Ice Mountains")->setSnowCovered()->setDepthAndScale(0.3f, 1.3f)->setTemperatureAndDownfall(0, 0.5f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_IceMountains, eMinecraftColour_Foliage_IceMountains, eMinecraftColour_Water_IceMountains,eMinecraftColour_Sky_IceMountains);

	Biome::mushroomIsland = (new MushroomIslandBiome(14))->setColor(0xff00ff)->setName(L"MushroomIsland")->setTemperatureAndDownfall(0.9f, 1.0f)->setDepthAndScale(0.2f, 1.0f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_MushroomIsland, eMinecraftColour_Foliage_MushroomIsland, eMinecraftColour_Water_MushroomIsland,eMinecraftColour_Sky_MushroomIsland);
	Biome::mushroomIslandShore = (new MushroomIslandBiome(15))->setColor(0xa000ff)->setName(L"MushroomIslandShore")->setTemperatureAndDownfall(0.9f, 1.0f)->setDepthAndScale(-1, 0.1f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_MushroomIslandShore, eMinecraftColour_Foliage_MushroomIslandShore, eMinecraftColour_Water_MushroomIslandShore,eMinecraftColour_Sky_MushroomIslandShore);

	Biome::beaches = (new BeachBiome(16))->setColor(0xfade55)->setName(L"Beach")->setTemperatureAndDownfall(0.8f, 0.4f)->setDepthAndScale(0.0f, 0.1f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Beach, eMinecraftColour_Foliage_Beach, eMinecraftColour_Water_Beach,eMinecraftColour_Sky_Beach);
	Biome::desertHills = (new DesertBiome(17))->setColor(0xd25f12)->setName(L"DesertHills")->setNoRain()->setTemperatureAndDownfall(2, 0)->setDepthAndScale(0.3f, 0.8f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_DesertHills, eMinecraftColour_Foliage_DesertHills, eMinecraftColour_Water_DesertHills,eMinecraftColour_Sky_DesertHills);
	Biome::forestHills = (new ForestBiome(18))->setColor(0x22551c)->setName(L"ForestHills")->setLeafColor(0x4EBA31)->setTemperatureAndDownfall(0.7f, 0.8f)->setDepthAndScale(0.3f, 0.7f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_ForestHills, eMinecraftColour_Foliage_ForestHills, eMinecraftColour_Water_ForestHills,eMinecraftColour_Sky_ForestHills);
	Biome::taigaHills = (new TaigaBiome(19))->setColor(0x163933)->setName(L"TaigaHills")->setSnowCovered()->setLeafColor(0x4EBA31)->setTemperatureAndDownfall(0.05f, 0.8f)->setDepthAndScale(0.3f, 0.8f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_TaigaHills, eMinecraftColour_Foliage_TaigaHills, eMinecraftColour_Water_TaigaHills,eMinecraftColour_Sky_TaigaHills);
	Biome::smallerExtremeHills = (new ExtremeHillsBiome(20))->setColor(0x72789a)->setName(L"Extreme Hills Edge")->setDepthAndScale(0.2f, 0.8f)->setTemperatureAndDownfall(0.2f, 0.3f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_ExtremeHillsEdge, eMinecraftColour_Foliage_ExtremeHillsEdge, eMinecraftColour_Water_ExtremeHillsEdge,eMinecraftColour_Sky_ExtremeHillsEdge);

	Biome::jungle = (new JungleBiome(21))->setColor(0x537b09)->setName(L"Jungle")->setLeafColor(0x537b09)->setTemperatureAndDownfall(1.2f, 0.9f)->setDepthAndScale(0.2f, 0.4f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_Jungle, eMinecraftColour_Foliage_Jungle, eMinecraftColour_Water_Jungle,eMinecraftColour_Sky_Jungle);
	Biome::jungleHills = (new JungleBiome(22))->setColor(0x2c4205)->setName(L"JungleHills")->setLeafColor(0x537b09)->setTemperatureAndDownfall(1.2f, 0.9f)->setDepthAndScale(1.8f, 0.5f)->setLeafFoliageWaterSkyColor(eMinecraftColour_Grass_JungleHills, eMinecraftColour_Foliage_JungleHills, eMinecraftColour_Water_JungleHills,eMinecraftColour_Sky_JungleHills);
}

Biome::Biome(int id) : id(id)
{
	// 4J Stu Default inits
	color = 0;
	//  snowCovered = false;	// 4J - this isn't set by the java game any more so removing to save confusion

	topMaterial = (byte) Tile::grass_Id;
	material = (byte) Tile::dirt_Id;
	leafColor = 0x4EE031;
	_hasRain = true;
	depth = 0.1f;
	scale = 0.3f;
	temperature = 0.5f;
	downfall = 0.5f;
	//waterColor = 0xffffff; // 4J Stu - Not used
	decorator = NULL;

	m_grassColor = eMinecraftColour_NOT_SET;
	m_foliageColor = eMinecraftColour_NOT_SET;
	m_waterColor = eMinecraftColour_NOT_SET;

	/*	4J - removing these so that we can consistently return newly created trees via getTreeFeature, and let the calling function be resposible for deleting the returned tree
	normalTree = new TreeFeature();
	fancyTree = new BasicTree();
	birchTree = new BirchFeature();
	swampTree = new SwampTreeFeature();
	*/

	biomes[id] = this;
	decorator = createDecorator();

	friendlies.push_back(new MobSpawnerData(eTYPE_SHEEP, 12, 4, 4));
	friendlies.push_back(new MobSpawnerData(eTYPE_PIG, 10, 4, 4));
	friendlies_chicken.push_back(new MobSpawnerData(eTYPE_CHICKEN, 10, 4, 4));		// 4J - moved chickens to their own category
	friendlies.push_back(new MobSpawnerData(eTYPE_COW, 8, 4, 4));

	enemies.push_back(new MobSpawnerData(eTYPE_SPIDER, 10, 4, 4));
	enemies.push_back(new MobSpawnerData(eTYPE_ZOMBIE, 10, 4, 4));
	enemies.push_back(new MobSpawnerData(eTYPE_SKELETON, 10, 4, 4));
	enemies.push_back(new MobSpawnerData(eTYPE_CREEPER, 10, 4, 4));
	enemies.push_back(new MobSpawnerData(eTYPE_SLIME, 10, 4, 4));
	enemies.push_back(new MobSpawnerData(eTYPE_ENDERMAN, 1, 1, 4));

	// wolves are added to forests and taigas

	waterFriendlies.push_back(new MobSpawnerData(eTYPE_SQUID, 10, 4, 4));

	ambientFriendlies.push_back(new MobSpawnerData(eTYPE_BAT, 10, 8, 8));
}

Biome::~Biome()
{
	if(decorator != NULL) delete decorator;
}

BiomeDecorator *Biome::createDecorator()
{
	return new BiomeDecorator(this);
}

// 4J Added
Biome *Biome::setLeafFoliageWaterSkyColor(eMinecraftColour grassColor, eMinecraftColour foliageColor, eMinecraftColour waterColour, eMinecraftColour skyColour)
{
	m_grassColor = grassColor;
	m_foliageColor = foliageColor;
	m_waterColor = waterColour;
	m_skyColor = skyColour;
	return this;
}

Biome *Biome::setTemperatureAndDownfall(float temp, float downfall)
{
	temperature = temp;
	this->downfall = downfall;
	return this;
}

Biome *Biome::setDepthAndScale(float depth, float scale)
{
	this->depth = depth;
	this->scale = scale;
	return this;
}

Biome *Biome::setNoRain()
{
	_hasRain = false;
	return this;
}

Feature *Biome::getTreeFeature(Random *random)
{
	if (random->nextInt(10) == 0)
	{
		return new BasicTree(false); // 4J used to return member fancyTree, now returning newly created object so that caller can be consistently resposible for cleanup
	}
	return new TreeFeature(false); // 4J used to return member normalTree, now returning newly created object so that caller can be consistently resposible for cleanup
}

Feature *Biome::getGrassFeature(Random *random)
{
	return new TallGrassFeature(Tile::tallgrass_Id, TallGrass::TALL_GRASS);
}

Biome *Biome::setSnowCovered()
{
	snowCovered = true;
	return this;
}

Biome *Biome::setName(const wstring &name)
{
	this->m_name = name;
	return this;
}

Biome *Biome::setLeafColor(int leafColor)
{
	this->leafColor = leafColor;
	return this;
}

Biome *Biome::setColor(int color)
{
	this->color = color;
	return this;
}

int Biome::getSkyColor(float temp)
{
	//temp /= 3.0f;
	//if (temp < -1) temp = -1;
	//if (temp > 1) temp = 1;
	//return Color::getHSBColor(224 / 360.0f - temp * 0.05f, 0.50f + temp * 0.1f, 1.0f).getRGB();

	// 4J Stu - Load colour from texture pack
	return Minecraft::GetInstance()->getColourTable()->getColor( m_skyColor );
}

vector<Biome::MobSpawnerData *> *Biome::getMobs(MobCategory *category)
{
	if (category == MobCategory::monster) return &enemies;
	if (category == MobCategory::creature) return &friendlies;
	if (category == MobCategory::waterCreature) return &waterFriendlies;
	if (category == MobCategory::creature_chicken) return &friendlies_chicken;
	if (category == MobCategory::creature_wolf) return &friendlies_wolf;
	if (category == MobCategory::creature_mushroomcow) return &friendlies_mushroomcow;
	if (category == MobCategory::ambient) return &ambientFriendlies;
	return NULL;
}

bool Biome::hasSnow()
{
	// 4J - snowCovered flag removed as it wasn't being set by the game anymore - snow is now temperature dependent to match code in rain rendering, shouldFreeze functions etc.
	if( !_hasRain ) return false;

	if( getTemperature() >= 0.15f ) return false;

	return true;
}

bool Biome::hasRain()
{
	// 4J - snowCovered flag removed as it wasn't being set by the game anymore, replaced by call to hasSnow()
	if( hasSnow() ) return false;
	//    if (snowCovered) return false;
	return _hasRain;
}

bool Biome::isHumid()
{
	return downfall > .85f;
}

float Biome::getCreatureProbability()
{
	return 0.1f;
}

int Biome::getDownfallInt()
{
	return (int) (downfall * 65536);
}

int Biome::getTemperatureInt()
{
	return (int) (temperature * 65536);
}

// 4J - brought forward from 1.2.3
float Biome::getDownfall()
{
	return downfall;
}

// 4J - brought forward from 1.2.3
float Biome::getTemperature()
{
	return temperature;
}

void Biome::decorate(Level *level, Random *random, int xo, int zo)
{
	decorator->decorate(level, random, xo, zo);
}

int Biome::getGrassColor()
{
	//double temp = Mth::clamp(getTemperature(), 0.0f, 1.0f);
	//double rain = Mth::clamp(getDownfall(), 0.0f, 1.0f);

	//return GrassColor::get(temp, rain);
	return Minecraft::GetInstance()->getColourTable()->getColor( m_grassColor );
}

int Biome::getFolageColor()
{
	//double temp = Mth::clamp(getTemperature(), 0.0f, 1.0f);
	//double rain = Mth::clamp(getDownfall(), 0.0f, 1.0f);

	//return FoliageColor::get(temp, rain);
	return Minecraft::GetInstance()->getColourTable()->getColor( m_foliageColor );
}

// 4J Added
int Biome::getWaterColor()
{
	return Minecraft::GetInstance()->getColourTable()->getColor( m_waterColor );
}