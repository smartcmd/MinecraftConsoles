#pragma once
using namespace std;

class Material;

class MobCategory
{
public:
	// 4J - putting constants for xbox spawning in one place to tidy things up a bit - all numbers are per level
	static int max_natural_monsters;						// Max number of enemies (skeleton, zombie, creeper etc) that the mob spawner will produce
	static int max_natural_animals;							// Max number of animals (cows, sheep, pigs) that the mob spawner will produce	
	static int max_natural_ambient;							// Ambient mobs
	static int max_natural_squid;							// Max number of squid that the mob spawner will produce
	static int max_natural_chickens;						// Max number of chickens that the mob spawner will produce
	static int max_natural_wolves;							// Max number of wolves that the mob spawner will produce
	static int max_natural_mushroomcows;					// Max number of mushroom cows that the mob spawner will produce

	static int max_snow_golems;								// Max number of snow golems that can be created by placing blocks - 4J-PB increased limit due to player requests
	static int max_iron_golems;								// Max number of iron golems that can be created by placing blocks - 4J-PB increased limit due to player requests
	static int max_bosses;									// Max number of bosses (enderdragon/wither)

	static int max_animals_with_breeding;					// Max number of animals that we can produce (in total), when breeding
	static int max_chickens_with_breeding;					// Max number of chickens that we can produce (in total), when breeding/hatching
	static int max_mushroomcows_with_breeding;				// Max number of mushroom cows that we can produce (in total), when breeding
	static int max_wolves_with_breeding;					// Max number of wolves that we can produce (in total), when breeding
	static int max_villagers_with_breeding;					// Max number of villagers that we can produce (in total), when breeding

	static int max_animals_with_spawn_egg;					// Max number of animals that we can produce (in total), when using spawn eggs
	static int max_chickens_with_spawn_egg;					// Max number of chickens that we can produce (in total), when using spawn eggs
	static int max_wolves_with_spawn_egg;					// Max number of wolves that we can produce (in total), when using spawn eggs
	static int max_monsters_with_spawn_egg;					// Max number of monsters that we can produce (in total), when using spawn eggs
	static int max_villagers_with_spawn_egg;				// Max number of villagers that we can produce (in total), when using spawn eggs - 4J-PB increased limit due to player requests
	static int max_mushroomcows_with_spawn_egg;				// Max number of mushroom cows that we can produce (in total), when using spawn eggs
	static int max_squids_with_spawn_egg;					// Max number of squids that we can produce (in total), when using spawn eggs
	static int max_ambient_with_spawn_egg;					// Max number of ambient mobs that we can produce (in total), when using spawn eggs

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
	// Use pointers to allow for this to be changed indirectly whilst staying a const
	const int *m_max;
	const Material *spawnPositionMaterial;
	const bool m_isFriendly;
	const bool m_isPersistent;
	const bool m_isSingleType; // 4J Added
	const eINSTANCEOF m_eBase; // 4J added

	MobCategory(int *maxVar, Material *spawnPositionMaterial, bool isFriendly, bool isPersistent, eINSTANCEOF eBase, bool isSingleType);

public:
	const type_info getBaseClass();
	const eINSTANCEOF getEnumBaseClass();	// 4J added
	int getMaxInstancesPerChunk();
	// int getMaxInstancesPerLevel(); 4J added ~ Removed as it has been merged with getMaxInstancesPerChunk()
	Material *getSpawnPositionMaterial();
	bool isFriendly();
	bool isSingleType();
	bool isPersistent();

public:
	static void staticCtor();
};
