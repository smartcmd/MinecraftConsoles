#pragma once
using namespace std;

class Material;

class MobCategory
{
public:
	// 4J - putting constants for xbox spawning in one place to tidy things up a bit - all numbers are per level
	static int maxNaturalMonsters;						// Max number of enemies (skeleton, zombie, creeper etc) that the mob spawner will produce
	static int maxNaturalAnimals;						// Max number of animals (cows, sheep, pigs) that the mob spawner will produce	
	static int maxNaturalAmbient;						// Ambient mobs
	static int maxNaturalSquid;							// Max number of squid that the mob spawner will produce
	static int maxNaturalChickens;						// Max number of chickens that the mob spawner will produce
	static int maxNaturalWolves;						// Max number of wolves that the mob spawner will produce
	static int maxNaturalMushroomCows;					// Max number of mushroom cows that the mob spawner will produce

	static int maxSnowGolems;							// Max number of snow golems that can be created by placing blocks - 4J-PB increased limit due to player requests
	static int maxIronGolems;							// Max number of iron golems that can be created by placing blocks - 4J-PB increased limit due to player requests
	static int maxBosses;								// Max number of bosses (enderdragon/wither)

	static int maxAnimalsWithBreeding;					// Max number of animals that we can produce (in total), when breeding
	static int maxChickensWithBreeding;					// Max number of chickens that we can produce (in total), when breeding/hatching
	static int maxMushroomCowsWithBreeding;				// Max number of mushroom cows that we can produce (in total), when breeding
	static int maxWolvesWithBreeding;					// Max number of wolves that we can produce (in total), when breeding
	static int maxVillagersWithBreeding;				// Max number of villagers that we can produce (in total), when breeding

	static int maxAnimalsWithSpawnEgg;					// Max number of animals that we can produce (in total), when using spawn eggs
	static int maxChickensWithSpawnEgg;					// Max number of chickens that we can produce (in total), when using spawn eggs
	static int maxWolvesWithSpawnEgg;					// Max number of wolves that we can produce (in total), when using spawn eggs
	static int maxMonstersWithSpawnEgg;					// Max number of monsters that we can produce (in total), when using spawn eggs
	static int maxVillagersWithSpawnEgg;				// Max number of villagers that we can produce (in total), when using spawn eggs - 4J-PB increased limit due to player requests
	static int maxMushroomCowsWithSpawnEgg;				// Max number of mushroom cows that we can produce (in total), when using spawn eggs
	static int maxSquidsWithSpawnEgg;					// Max number of squids that we can produce (in total), when using spawn eggs
	static int maxAmbientWithSpawnEgg;					// Max number of ambient mobs that we can produce (in total), when using spawn eggs

	/*
		Maximum animals = 50 + 20 + 20 = 90
		Maximum monsters = 50 + 20 = 70
		Maximum chickens = 8 + 8 + 10 = 26
		Maximum wolves = 8 + 8 + 10 = 26
		Maximum mooshrooms = 2 + 20 + 8 = 30
		Maximum snowmen = 16
		Maximum iron golem = 16
		Maximum squid = 5 + 8 = 13
		Maximum villagers = 35 + 15 = 50

		Maximum natural = 50 + 50 + 8 + 8 + 2 + 5 + 35 = 158
		Total maxium = 90 + 70 + 26 + 26 + 30 + 16 + 16 + 13 + 50 = 337
	*/

	static MobCategory *monster;
	static MobCategory *creature;
	static MobCategory *ambient;
	static MobCategory *waterCreature;
	// 4J added extra categories, to break these out of general creatures & give us more control of levels
	static MobCategory *creature_wolf;
	static MobCategory *creature_chicken;
	static MobCategory *creature_mushroomcow;

	// 4J Stu Sometimes we want to access the values by name, other times iterate over all values
	// Added these arrays so we can static initialise a collection which we can iterate over
	static MobCategoryArray values;


private:
	const int m_max;
	int m_maxPerLevel = 0;
	const Material *spawnPositionMaterial;
	const bool m_isFriendly;
	const bool m_isPersistent;
	const bool m_isSingleType; // 4J Added
	const eINSTANCEOF m_eBase; // 4J added
	// Added for animals so that wolves, mushroomcows, and chickens can be included in the category count.
	// Only used when mob cap is unlimited, done to ensure the wolves and chickens can spawn properly.
	// When using the shorter constructor, this will be set to m_eBase.
	const eINSTANCEOF m_eJavaBase;

	MobCategory(int maxVar, Material *spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, bool isSingleType);
	// Additional constructor, allows specifying the enum to use if mob cap is unlimited
	MobCategory(int maxVar, Material * spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, eINSTANCEOF eJavaBase, bool isSingleType);

public:
	const type_info getBaseClass();
	const eINSTANCEOF getEnumBaseClass();	// 4J added
	int getMaxInstancesPerChunk();
	int getMaxInstancesPerLevel();		// 4J added
	Material *getSpawnPositionMaterial();
	bool isFriendly();
	bool isSingleType();
	bool isPersistent();

public:
	static void staticCtor();
};
