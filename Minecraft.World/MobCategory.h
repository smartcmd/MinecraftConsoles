#pragma once
using namespace std;

class Material;

class MobCategory
{
public:
	// 4J - putting constants for xbox spawning in one place to tidy things up a bit - all numbers are per level
	static const int CONSOLE_MONSTERS_HARD_LIMIT = 70; //50									// Max number of enemies (skeleton, zombie, creeper etc) that the mob spawner will produce
	static const int CONSOLE_ANIMALS_HARD_LIMIT = 70;  //50									// Max number of animals (cows, sheep, pigs) that the mob spawner will produce	
	static const int CONSOLE_AMBIENT_HARD_LIMIT = 20;  //20								// Ambient mobs

	static const int MAX_XBOX_CHICKENS = 16; //8										// Max number of chickens that the mob spawner will produce
	static const int MAX_XBOX_WOLVES = 16; //8										// Max number of wolves that the mob spawner will produce
	static const int MAX_XBOX_MUSHROOMCOWS = 8;	//2								// Max number of mushroom cows that the mob spawner will produce
	static const int MAX_XBOX_SNOWMEN = 16;										// Max number of snow golems that can be created by placing blocks - 4J-PB increased limit due to player requests
	static const int MAX_XBOX_IRONGOLEM = 16;									// Max number of iron golems that can be created by placing blocks - 4J-PB increased limit due to player requests
	static const int CONSOLE_SQUID_HARD_LIMIT = 10; //5
	static const int MAX_CONSOLE_BOSS = 1;										// Max number of bosses (enderdragon/wither)

	static const int MAX_XBOX_ANIMALS_WITH_BREEDING = CONSOLE_ANIMALS_HARD_LIMIT + 20;	// Max number of animals that we can produce (in total), when breeding
	static const int MAX_XBOX_CHICKENS_WITH_BREEDING = MAX_XBOX_CHICKENS + 8;	// Max number of chickens that we can produce (in total), when breeding/hatching
	static const int MAX_XBOX_MUSHROOMCOWS_WITH_BREEDING = MAX_XBOX_MUSHROOMCOWS + 20;	// Max number of mushroom cows that we can produce (in total), when breeding
	static const int MAX_XBOX_WOLVES_WITH_BREEDING = MAX_XBOX_WOLVES + 8;		// Max number of wolves that we can produce (in total), when breeding
	static const int MAX_VILLAGERS_WITH_BREEDING = 50; //35

	static const int MAX_XBOX_ANIMALS_WITH_SPAWN_EGG = MAX_XBOX_ANIMALS_WITH_BREEDING + 20;
	static const int MAX_XBOX_CHICKENS_WITH_SPAWN_EGG = MAX_XBOX_CHICKENS_WITH_BREEDING + 10;
	static const int MAX_XBOX_WOLVES_WITH_SPAWN_EGG = MAX_XBOX_WOLVES_WITH_BREEDING + 10;
	static const int MAX_XBOX_MONSTERS_WITH_SPAWN_EGG = CONSOLE_MONSTERS_HARD_LIMIT + 20;
	static const int MAX_XBOX_VILLAGERS_WITH_SPAWN_EGG = MAX_VILLAGERS_WITH_BREEDING + 15; // 4J-PB - increased this limit due to player requests
	static const int MAX_XBOX_MUSHROOMCOWS_WITH_SPAWN_EGG = MAX_XBOX_MUSHROOMCOWS_WITH_BREEDING + 8;
	static const int MAX_XBOX_SQUIDS_WITH_SPAWN_EGG = CONSOLE_SQUID_HARD_LIMIT + 8;
	static const int MAX_AMBIENT_WITH_SPAWN_EGG = CONSOLE_AMBIENT_HARD_LIMIT + 8;

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
	const int m_maxPerLevel;
	const Material *spawnPositionMaterial;
	const bool m_isFriendly;
	const bool m_isPersistent;
	const bool m_isSingleType; // 4J Added
	const eINSTANCEOF m_eBase; // 4J added

	MobCategory(int maxVar, Material *spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, bool isSingleType, int maxPerLevel);

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
