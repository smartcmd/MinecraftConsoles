#pragma once
using namespace std;
class InputStream;

// This file aims to provide functionality to mimic the subset of java's Class class that we require. Classes that require this
// functionality derive from BaseObject, and each contain a unique nested class definition Class. There are #defines here to
// simplify declaring classes with this added functionality.


//		 0b FFFF CCCC CCCC CCCC CCCC CCCC CCEE EEEE
//			|||| |||| |||| |||| |||| |||| |||| ||||
//          |||| |||| |||| |||| |||| |||| |||| |||\- BIT00: ENUM:
//          |||| |||| |||| |||| |||| |||| |||| ||\-- BIT01: ENUM:
//          |||| |||| |||| |||| |||| |||| |||| |\--- BIT02: ENUM:
//          |||| |||| |||| |||| |||| |||| |||| \---- BIT03: ENUM:
//          |||| |||| |||| |||| |||| |||| ||||
//          |||| |||| |||| |||| |||| |||| |||\------ BIT04: ENUM:
//          |||| |||| |||| |||| |||| |||| ||\------- BIT05: ENUM:
//          |||| |||| |||| |||| |||| |||| |\-------- BIT06: CLASS: WATER_MOB
//          |||| |||| |||| |||| |||| |||| \--------- BIT07: CLASS: AMBIENT_MOB
//          |||| |||| |||| |||| |||| |||| 							
//          |||| |||| |||| |||| |||| |||\----------- BIT08: CLASS: !ENTITY (so we can hide TILE_ENTITY and DISPENSER_TILE_ENTITY bits which aren't relevant for entities)
//          |||| |||| |||| |||| |||| ||\------------ BIT09: CLASS: MINECART_CONTAINER
//          |||| |||| |||| |||| |||| |\------------- BIT10: CLASS: SLIME			
//          |||| |||| |||| |||| |||| \-------------- BIT11: CLASS: ZOMBIE			
//          |||| |||| |||| |||| ||||       		    	  
//          |||| |||| |||| |||| |||\---------------- BIT12: CLASS: SPIDER			
//          |||| |||| |||| |||| ||\----------------- BIT13: CLASS: COW				
//          |||| |||| |||| |||| |\------------------ BIT14: CLASS: TAMABLE			
//          |||| |||| |||| |||| \------------------- BIT15: CLASS: ANIMAL			
//			|||| |||| |||| ||||									   
//			|||| |||| |||| |||\--------------------- BIT16: CLASS: MONSTER			
//			|||| |||| |||| ||\---------------------- BIT17: CLASS: GOLEM			
//          |||| |||| |||| |\----------------------- BIT18: CLASS: AGABLE_MOB		
//			|||| |||| |||| \------------------------ BIT19: CLASS: PATHFINDER_MOB	
//			|||| |||| ||||  									   
//			|||| |||| |||\-------------------------- BIT20: CLASS: PLAYER			
//			|||| |||| ||\--------------------------- BIT21: CLASS: MOB				
//			|||| |||| |\---------------------------- BIT22: CLASS: HANGING_ENTITY	
//			|||| |||| \----------------------------- BIT23: CLASS: THROWABLE		
//			|||| ||||            								   
//          |||| |||\------------------------------- BIT24: CLASS: FIREBALL			
//          |||| ||\-------------------------------- BIT25: CLASS: MINECART			
//          |||| |\--------------------------------- BIT26: CLASS: LIVING_ENTITY	
//          |||| \---------------------------------- BIT27: CLASS: ENTITY			
//          ||||                		    
//          |||\------------------------------------ BIT28: FLAGS: valid in spawner flag
//          ||\------------------------------------- BIT29: FLAGS: Spawnlimitcheck
//          |\-------------------------------------- BIT30: FLAGS: Enemy
//          \--------------------------------------- BIT31: FLAGS: projectile


#define Bit(a) ((1)<<(a))

const unsigned int BIT_NOT_LIVING_ENTITY = Bit(25); 

// Classes

const unsigned int BIT_FLYING_MOB								= Bit( 9);
const unsigned int BIT_WATER_MOB								= Bit(10);
const unsigned int BIT_AMBIENT_MOB								= Bit(11);

const unsigned int BIT_NOT_ENTITY								= Bit(12);
const unsigned int BIT_SLIME 									= Bit(13);
const unsigned int BIT_ZOMBIE									= Bit(14);
const unsigned int BIT_SPIDER									= Bit(15);

const unsigned int BIT_COW										= Bit(16);
const unsigned int BIT_TAMABLE									= Bit(17);
const unsigned int BIT_ANIMAL									= Bit(18);
const unsigned int BIT_MONSTER									= Bit(19); const unsigned int BIT_MINECART_CONTAINER	= Bit(19) | BIT_NOT_LIVING_ENTITY;

const unsigned int BIT_GOLEM									= Bit(20); const unsigned int BIT_HANGING_ENTITY		= Bit(20) | BIT_NOT_LIVING_ENTITY;
const unsigned int BIT_AGABLE_MOB								= Bit(21); const unsigned int BIT_THROWABLE				= Bit(21) | BIT_NOT_LIVING_ENTITY;
const unsigned int BIT_PATHFINDER_MOB							= Bit(22); const unsigned int BIT_FIREBALL				= Bit(22) | BIT_NOT_LIVING_ENTITY;
const unsigned int BIT_PLAYER									= Bit(23); const unsigned int BIT_MINECART				= Bit(23) | BIT_NOT_LIVING_ENTITY;

const unsigned int BIT_MOB										= Bit(24); const unsigned int BIT_GLOBAL_ENTITY			= Bit(24) | BIT_NOT_LIVING_ENTITY;
//const unsigned int BIT_NOT_LIVING_ENTITY						= Bit(25); 
const unsigned int BIT_LIVING_ENTITY							= Bit(26);
const unsigned int BIT_ENTITY									= Bit(27);

// Flags
const unsigned int BIT_VALID_IN_SPAWNER							= Bit(28);
const unsigned int BIT_ANIMALS_SPAWN_LIMIT_CHECK				= Bit(29);
const unsigned int BIT_ENEMY									= Bit(30);
const unsigned int BIT_PROJECTILE								= Bit(31U);

// Tile Entities
const unsigned int BIT_TILE_ENTITY								= Bit(24) | BIT_NOT_ENTITY;
const unsigned int BIT_DISPENSERTILEENTITY						= Bit(25) | BIT_NOT_ENTITY;
const unsigned int BIT_OTHER_NOT_ENTITIES						= Bit(26) | BIT_NOT_ENTITY;


// 4J-JEV: These abstract classes only have one subclass, so ignore them.
//const unsigned int BIT_WATER_MOB					= Bit(15);
//const unsigned int BIT_FLYING_MOB					= Bit(17);
//const unsigned int BIT_AMBIENT_MOB				= Bit(18);
//const unsigned int BIT_GLOBAL_ENTITY				= Bit();

// #define ETYPE(a,b,c) ( (a) | (b) | (c) )

// 4J Stu - This Enum can be used as a more lightweight version of the above, without having do dynamic casts
// 4J-PB - for replacement of instanceof
enum eINSTANCEOF
{
	eTYPE_NOTSET=0,
	
	// Flags.
	eTYPE_VALID_IN_SPAWNER_FLAG		= BIT_VALID_IN_SPAWNER,
	eTYPE_ANIMALS_SPAWN_LIMIT_CHECK = BIT_ANIMALS_SPAWN_LIMIT_CHECK,
	eTYPE_ENEMY						= BIT_ENEMY,
	eTYPE_PROJECTILE				= BIT_PROJECTILE,

	eTYPE_ENTITY					= BIT_ENTITY,

	eTYPE_LIVINGENTITY	= eTYPE_ENTITY | BIT_LIVING_ENTITY,

	eTYPE_MOB		= eTYPE_LIVINGENTITY | BIT_MOB,

			eTYPE_PATHFINDER_MOB	= eTYPE_MOB | BIT_PATHFINDER_MOB,

					eTYPE_AGABLE_MOB	= eTYPE_PATHFINDER_MOB | BIT_AGABLE_MOB,

						eTYPE_VILLAGER	= eTYPE_AGABLE_MOB | 0x1, //0x12000,

						// 4J Stu - When adding new categories, please also update ConsoleSchematicFile::generateSchematicFile so these can be saved out to schematics
						eTYPE_ANIMAL	= eTYPE_AGABLE_MOB | BIT_ANIMAL,
		
							eTYPE_TAMABLE_ANIMAL	= eTYPE_ANIMAL | BIT_TAMABLE,				

								eTYPE_OCELOT		= eTYPE_TAMABLE_ANIMAL | eTYPE_ANIMALS_SPAWN_LIMIT_CHECK | 0x1,
								eTYPE_WOLF			= eTYPE_TAMABLE_ANIMAL                                   | 0x2,

							eTYPE_HORSE				= eTYPE_ANIMAL | eTYPE_ANIMALS_SPAWN_LIMIT_CHECK | 0x1,
							eTYPE_SHEEP				= eTYPE_ANIMAL | eTYPE_ANIMALS_SPAWN_LIMIT_CHECK | 0x2,
							eTYPE_PIG				= eTYPE_ANIMAL | eTYPE_ANIMALS_SPAWN_LIMIT_CHECK | 0x3,
							eTYPE_CHICKEN			= eTYPE_ANIMAL                                   | 0x4,

							eTYPE_COW				= eTYPE_ANIMAL | eTYPE_ANIMALS_SPAWN_LIMIT_CHECK | BIT_COW,
								eTYPE_MUSHROOMCOW	= eTYPE_COW | 0x1,

							
					eTYPE_WATERANIMAL	= eTYPE_PATHFINDER_MOB | BIT_WATER_MOB, //0x100,
						eTYPE_SQUID		= eTYPE_WATERANIMAL| 0x1,

					eTYPE_GOLEM			= eTYPE_PATHFINDER_MOB | BIT_GOLEM,

						eTYPE_SNOWMAN		= eTYPE_GOLEM | eTYPE_ANIMALS_SPAWN_LIMIT_CHECK | 0x1, //0x4,
						eTYPE_VILLAGERGOLEM	= eTYPE_GOLEM  									| 0x2, //0x1000,


					// 4J Stu - If you add new hostile mobs here you should also update the string lookup function at CConsoleMinecraftApp::getEntityName
					eTYPE_MONSTER     =	eTYPE_ENEMY | eTYPE_PATHFINDER_MOB | BIT_MONSTER,

						eTYPE_SPIDER			= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | BIT_SPIDER,
							eTYPE_CAVESPIDER	= eTYPE_SPIDER | 0x1,

						eTYPE_ZOMBIE			= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | BIT_ZOMBIE,
							eTYPE_PIGZOMBIE		= eTYPE_ZOMBIE | 0x1,

						eTYPE_CREEPER		= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x1,
						eTYPE_GIANT			= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x2,
						eTYPE_SKELETON		= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x3,
						eTYPE_ENDERMAN		= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x4,
						eTYPE_SILVERFISH	= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x5,
						eTYPE_BLAZE			= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x6,
						eTYPE_WITCH			= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x7,
						eTYPE_WITHERBOSS	= eTYPE_MONSTER | eTYPE_VALID_IN_SPAWNER_FLAG | 0x8,


				eTYPE_AMBIENT			= eTYPE_MOB | BIT_AMBIENT_MOB,
					eTYPE_BAT			= eTYPE_AMBIENT | eTYPE_VALID_IN_SPAWNER_FLAG | 0x1,

				eTYPE_FLYING_MOB		= eTYPE_MOB | BIT_FLYING_MOB,
					eTYPE_GHAST			= eTYPE_FLYING_MOB | eTYPE_VALID_IN_SPAWNER_FLAG | eTYPE_ENEMY | 0x1,

				eTYPE_SLIME				= eTYPE_MOB | eTYPE_VALID_IN_SPAWNER_FLAG | eTYPE_ENEMY | BIT_SLIME,
					eTYPE_LAVASLIME		= eTYPE_SLIME | 0x1,

				eTYPE_ENDERDRAGON	= eTYPE_MOB | 0x5,
	
			eTYPE_PLAYER			= eTYPE_LIVINGENTITY | BIT_PLAYER, //0x8000,
				eTYPE_SERVERPLAYER	= eTYPE_PLAYER | 0x1,
				eTYPE_REMOTEPLAYER	= eTYPE_PLAYER | 0x2,
				eTYPE_LOCALPLAYER	= eTYPE_PLAYER | 0x3,

		eTYPE_GLOBAL_ENTITY			= eTYPE_ENTITY | BIT_GLOBAL_ENTITY,
			eTYPE_LIGHTNINGBOLT		= eTYPE_GLOBAL_ENTITY | 0x1,

		eTYPE_MINECART					= eTYPE_ENTITY | BIT_MINECART, //0x200000,
			
			eTYPE_MINECART_RIDEABLE     = eTYPE_MINECART | 0x1,
			eTYPE_MINECART_SPAWNER      = eTYPE_MINECART | 0x6,
			eTYPE_MINECART_FURNACE      = eTYPE_MINECART | 0x3,
			eTYPE_MINECART_TNT			= eTYPE_MINECART | 0x4,

			eTYPE_MINECART_CONTAINER	= eTYPE_MINECART | BIT_MINECART_CONTAINER,
				
				eTYPE_MINECART_CHEST	= eTYPE_MINECART_CONTAINER | 0x2,
				eTYPE_MINECART_HOPPER	= eTYPE_MINECART_CONTAINER | 0x5,
		

		eTYPE_FIREBALL				= eTYPE_ENTITY | eTYPE_PROJECTILE | BIT_FIREBALL, //0x2,

			eTYPE_DRAGON_FIREBALL	= eTYPE_FIREBALL | 0x1,
			eTYPE_WITHER_SKULL      = eTYPE_FIREBALL | 0x2,
			eTYPE_LARGE_FIREBALL    = eTYPE_FIREBALL | 0x3,
			eTYPE_SMALL_FIREBALL	= eTYPE_FIREBALL | 0x4,
	

		eTYPE_THROWABLE				= eTYPE_ENTITY | eTYPE_PROJECTILE | BIT_THROWABLE,

			eTYPE_SNOWBALL			= eTYPE_THROWABLE | 0x1,
			eTYPE_THROWNEGG			= eTYPE_THROWABLE | 0x2,
			eTYPE_THROWNENDERPEARL	= eTYPE_THROWABLE | 0x3,
			eTYPE_THROWNPOTION		= eTYPE_THROWABLE | 0x4,
			eTYPE_THROWNEXPBOTTLE	= eTYPE_THROWABLE | 0x5,


		eTYPE_HANGING_ENTITY		= eTYPE_ENTITY | BIT_HANGING_ENTITY,

			eTYPE_PAINTING			= eTYPE_HANGING_ENTITY | 0x1,
			eTYPE_ITEM_FRAME		= eTYPE_HANGING_ENTITY | 0x2,
			eTYPE_LEASHFENCEKNOT	= eTYPE_HANGING_ENTITY | 0x3,
	
	
	// Other Entities.
	
	eTYPE_OTHER_ENTITIES = eTYPE_ENTITY + 1,

	eTYPE_EXPERIENCEORB			= (eTYPE_OTHER_ENTITIES +  2),		// 1.8.2
	eTYPE_EYEOFENDERSIGNAL		= (eTYPE_OTHER_ENTITIES +  3) | eTYPE_PROJECTILE,
	eTYPE_FIREWORKS_ROCKET		= (eTYPE_OTHER_ENTITIES +  4) | eTYPE_PROJECTILE,
	eTYPE_FISHINGHOOK			= (eTYPE_OTHER_ENTITIES +  5) | eTYPE_PROJECTILE,
	eTYPE_DELAYEDRELEASE		= (eTYPE_OTHER_ENTITIES +  6),		// 1.8.2
	eTYPE_BOAT					= (eTYPE_OTHER_ENTITIES +  7),
	eTYPE_FALLINGTILE			= (eTYPE_OTHER_ENTITIES +  8),
	eTYPE_ITEMENTITY			= (eTYPE_OTHER_ENTITIES +  9),
	eTYPE_PRIMEDTNT				= (eTYPE_OTHER_ENTITIES + 10),
	eTYPE_ARROW					= (eTYPE_OTHER_ENTITIES + 11) | eTYPE_PROJECTILE,
	eTYPE_MULTIENTITY_MOB_PART	= (eTYPE_OTHER_ENTITIES + 12),
	eTYPE_NETHER_SPHERE			= (eTYPE_OTHER_ENTITIES + 13),
	eTYPE_ENDER_CRYSTAL			= (eTYPE_OTHER_ENTITIES + 14),
	
	
	// === PARTICLES === //

	eType_BREAKINGITEMPARTICLE,
	eType_BUBBLEPARTICLE,
	eType_EXPLODEPARTICLE,
	eType_FLAMEPARTICLE,
	eType_FOOTSTEPPARTICLE,
	eType_HEARTPARTICLE,
	eType_LAVAPARTICLE,
	eType_NOTEPARTICLE,
	eType_NETHERPORTALPARTICLE,
	eType_REDDUSTPARTICLE,
	eType_SMOKEPARTICLE,
	eType_SNOWSHOVELPARTICLE,
	eType_SPLASHPARTICLE,
	eType_TAKEANIMATIONPARTICLE,
	eType_TERRAINPARTICLE,
	eType_WATERDROPPARTICLE,

	// 1.8.2
	eType_CRITPARTICLE,
	eType_CRITPARTICLE2,
	eType_HUGEEXPLOSIONPARTICLE,
	eType_HUGEEXPLOSIONSEEDPARTICLE,
	eType_PLAYERCLOUDPARTICLEPARTICLE,
	eType_SUSPENDEDPARTICLE,
	eType_SUSPENDEDTOWNPARTICLE,

	//1.0.1
	eTYPE_DRIPPARTICLE,
	eTYPE_ENCHANTMENTTABLEPARTICLE,
	eTYPE_SPELLPARTICLE,

	//TU9
	eTYPE_DRAGONBREATHPARTICLE,
	eType_ENDERPARTICLE,

	eType_FIREWORKSSTARTERPARTICLE,
	eType_FIREWORKSSPARKPARTICLE,
	eType_FIREWORKSOVERLAYPARTICLE,

	// === Tile Entities === //
	
	eTYPE_TILEENTITY						= BIT_TILE_ENTITY,

		eTYPE_CHESTTILEENTITY				= eTYPE_TILEENTITY | 0x01,
		eTYPE_MOBSPAWNERTILEENTITY			= eTYPE_TILEENTITY | 0x02,
		eTYPE_FURNACETILEENTITY				= eTYPE_TILEENTITY | 0x03,
		eTYPE_SIGNTILEENTITY				= eTYPE_TILEENTITY | 0x04,
		eTYPE_MUSICTILEENTITY				= eTYPE_TILEENTITY | 0x05,
		eTYPE_RECORDPLAYERTILE				= eTYPE_TILEENTITY | 0x06,
		eTYPE_PISTONPIECEENTITY				= eTYPE_TILEENTITY | 0x07,
		eTYPE_BREWINGSTANDTILEENTITY		= eTYPE_TILEENTITY | 0x08,
		eTYPE_ENCHANTMENTTABLEENTITY		= eTYPE_TILEENTITY | 0x09,
		eTYPE_THEENDPORTALTILEENTITY		= eTYPE_TILEENTITY | 0x0A,
		eTYPE_SKULLTILEENTITY				= eTYPE_TILEENTITY | 0x0B,
		eTYPE_ENDERCHESTTILEENTITY			= eTYPE_TILEENTITY | 0x0C,
		eTYPE_BEACONTILEENTITY				= eTYPE_TILEENTITY | 0x0D,
		eTYPE_COMMANDBLOCKTILEENTITY		= eTYPE_TILEENTITY | 0x0E,
		eTYPE_COMPARATORTILEENTITY			= eTYPE_TILEENTITY | 0x0F,
		eTYPE_DAYLIGHTDETECTORTILEENTITY	= eTYPE_TILEENTITY | 0x10,
		eTYPE_HOPPERTILEENTITY				= eTYPE_TILEENTITY | 0x11,
		
		eTYPE_DISPENSERTILEENTITY			= eTYPE_TILEENTITY | BIT_DISPENSERTILEENTITY,
			eTYPE_DROPPERTILEENTITY			= eTYPE_DISPENSERTILEENTITY | 0x1,


	// === Never used === //
	// exists to ensure all later entities don't match the bitmasks above

	eTYPE_OTHERS      = BIT_OTHER_NOT_ENTITIES,
	
	eType_NODE,
	eType_ITEM,
	eType_ITEMINSTANCE,
	eType_MAPITEM,
	eType_TILE,
	eType_FIRETILE,
};

inline bool eTYPE_DERIVED_FROM(eINSTANCEOF super, eINSTANCEOF sub)
{
	if ( (super & 0x3F) != 0x00 )	return super == sub;
	else							return (super & sub) == super;
}

inline bool eTYPE_FLAGSET(eINSTANCEOF flag, eINSTANCEOF claz)
{
	return (flag & claz) == flag;
}


/// FOR CHECKING ///

#if !(defined _WINDOWS64)

class SubClass
{
	static void checkDerivations() {}
};

#else

class SubClass
{
public:
	bool				m_isTerminal;
	const string		m_name;
	const eINSTANCEOF	m_id;
	vector<eINSTANCEOF> m_parents;

	static unordered_map<eINSTANCEOF, SubClass*> s_ids;

	SubClass(const string &name, eINSTANCEOF id)
		: m_name(name), m_id(id)
	{
		s_ids.insert( pair<eINSTANCEOF,SubClass*>(id,this) );
		m_isTerminal = true;
	}

	SubClass *addParent(eINSTANCEOF id)
	{
		SubClass *parent = s_ids.at(id);
		parent->m_isTerminal = false;

		m_parents.push_back(id);

		for ( auto& it : parent->m_parents )
		{
			m_parents.push_back(it);
		}

		return this;
	}

	bool justFlag()
	{
		return (m_id & 0xF00000) == m_id;
	}

#define SUBCLASS(x) (new SubClass( #x , x ))

	static void checkDerivations()
	{
		vector<SubClass *> *classes = new vector<SubClass*>();
				
		classes->push_back( SUBCLASS(eTYPE_VALID_IN_SPAWNER_FLAG) );
		classes->push_back( SUBCLASS(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK) );
		classes->push_back( SUBCLASS(eTYPE_ENEMY) );
		classes->push_back( SUBCLASS(eTYPE_PROJECTILE) );

		classes->push_back( SUBCLASS(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eTYPE_LIVINGENTITY)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eTYPE_MOB)->addParent(eTYPE_LIVINGENTITY) );
		classes->push_back( SUBCLASS(eTYPE_PATHFINDER_MOB)->addParent(eTYPE_MOB) );
		classes->push_back( SUBCLASS(eTYPE_AGABLE_MOB)->addParent(eTYPE_PATHFINDER_MOB) );
		classes->push_back( SUBCLASS(eTYPE_VILLAGER)->addParent(eTYPE_AGABLE_MOB) );
		classes->push_back( SUBCLASS(eTYPE_ANIMAL)->addParent(eTYPE_AGABLE_MOB) );
		classes->push_back( SUBCLASS(eTYPE_TAMABLE_ANIMAL	)->addParent( eTYPE_ANIMAL ) );
		classes->push_back( SUBCLASS(eTYPE_OCELOT		)->addParent( eTYPE_TAMABLE_ANIMAL)->addParent(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK ) );
		classes->push_back( SUBCLASS(eTYPE_WOLF			)->addParent( eTYPE_TAMABLE_ANIMAL ) );
		classes->push_back( SUBCLASS(eTYPE_HORSE				)->addParent( eTYPE_ANIMAL)->addParent(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK ) );
		classes->push_back( SUBCLASS(eTYPE_SHEEP				)->addParent( eTYPE_ANIMAL)->addParent(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK ) );
		classes->push_back( SUBCLASS(eTYPE_PIG				)->addParent( eTYPE_ANIMAL)->addParent(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK  ) );
		classes->push_back( SUBCLASS(eTYPE_CHICKEN			)->addParent( eTYPE_ANIMAL  ) );
		classes->push_back( SUBCLASS(eTYPE_COW				)->addParent( eTYPE_ANIMAL)->addParent(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK ) );
		classes->push_back( SUBCLASS(eTYPE_MUSHROOMCOW	)->addParent( eTYPE_COW ) );
		classes->push_back( SUBCLASS(eTYPE_WATERANIMAL	)->addParent(eTYPE_PATHFINDER_MOB) );
		classes->push_back( SUBCLASS(eTYPE_SQUID		)->addParent( eTYPE_WATERANIMAL ) );
		classes->push_back( SUBCLASS(eTYPE_GOLEM			)->addParent( eTYPE_PATHFINDER_MOB ) );
		classes->push_back( SUBCLASS(eTYPE_SNOWMAN		)->addParent( eTYPE_GOLEM)->addParent(eTYPE_ANIMALS_SPAWN_LIMIT_CHECK ) );
		classes->push_back( SUBCLASS(eTYPE_VILLAGERGOLEM	)->addParent( eTYPE_GOLEM  ) );
		classes->push_back( SUBCLASS(eTYPE_MONSTER     )->addParent(	eTYPE_ENEMY)->addParent(eTYPE_PATHFINDER_MOB ) );
		classes->push_back( SUBCLASS(eTYPE_SPIDER			)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG  ) );
		classes->push_back( SUBCLASS(eTYPE_CAVESPIDER	)->addParent( eTYPE_SPIDER  ) );
		classes->push_back( SUBCLASS(eTYPE_ZOMBIE			)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_PIGZOMBIE		)->addParent( eTYPE_ZOMBIE ) );
		classes->push_back( SUBCLASS(eTYPE_CREEPER		)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_GIANT			)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_SKELETON		)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_ENDERMAN		)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_SILVERFISH	)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_BLAZE			)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_WITCH			)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_WITHERBOSS	)->addParent( eTYPE_MONSTER)->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_AMBIENT		)->addParent( eTYPE_MOB ) );
		classes->push_back( SUBCLASS(eTYPE_BAT		)->addParent( eTYPE_AMBIENT )->addParent(eTYPE_VALID_IN_SPAWNER_FLAG ) );
		classes->push_back( SUBCLASS(eTYPE_FLYING_MOB	)->addParent( eTYPE_MOB ) );
		classes->push_back( SUBCLASS(eTYPE_GHAST		)->addParent( eTYPE_FLYING_MOB )->addParent(eTYPE_VALID_IN_SPAWNER_FLAG)->addParent(eTYPE_ENEMY ) );
		classes->push_back( SUBCLASS(eTYPE_SLIME				)->addParent( eTYPE_MOB )->addParent(eTYPE_VALID_IN_SPAWNER_FLAG)->addParent(eTYPE_ENEMY ) );
		classes->push_back( SUBCLASS(eTYPE_LAVASLIME		)->addParent( eTYPE_SLIME ) );
		classes->push_back( SUBCLASS(eTYPE_ENDERDRAGON	)->addParent( eTYPE_MOB ) );
		classes->push_back( SUBCLASS(eTYPE_PLAYER			)->addParent( eTYPE_LIVINGENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_SERVERPLAYER	)->addParent( eTYPE_PLAYER ) );
		classes->push_back( SUBCLASS(eTYPE_REMOTEPLAYER	)->addParent( eTYPE_PLAYER ) );
		classes->push_back( SUBCLASS(eTYPE_LOCALPLAYER	)->addParent( eTYPE_PLAYER ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART					)->addParent( eTYPE_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART_RIDEABLE     )->addParent( eTYPE_MINECART ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART_SPAWNER      )->addParent( eTYPE_MINECART ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART_FURNACE      )->addParent( eTYPE_MINECART ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART_TNT			)->addParent( eTYPE_MINECART ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART_CONTAINER	)->addParent( eTYPE_MINECART ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART_CHEST	)->addParent( eTYPE_MINECART_CONTAINER ) );
		classes->push_back( SUBCLASS(eTYPE_MINECART_HOPPER	)->addParent( eTYPE_MINECART_CONTAINER ) );
		classes->push_back( SUBCLASS(eTYPE_FIREBALL				)->addParent( eTYPE_ENTITY)->addParent(eTYPE_PROJECTILE ) );
		classes->push_back( SUBCLASS(eTYPE_DRAGON_FIREBALL	)->addParent( eTYPE_FIREBALL ) );
		classes->push_back( SUBCLASS(eTYPE_WITHER_SKULL      )->addParent( eTYPE_FIREBALL ) );
		classes->push_back( SUBCLASS(eTYPE_LARGE_FIREBALL    )->addParent( eTYPE_FIREBALL ) );
		classes->push_back( SUBCLASS(eTYPE_SMALL_FIREBALL	)->addParent( eTYPE_FIREBALL ) );
		classes->push_back( SUBCLASS(eTYPE_THROWABLE				)->addParent( eTYPE_ENTITY)->addParent(eTYPE_PROJECTILE ) );
		classes->push_back( SUBCLASS(eTYPE_SNOWBALL			)->addParent( eTYPE_THROWABLE ) );
		classes->push_back( SUBCLASS(eTYPE_THROWNEGG			)->addParent( eTYPE_THROWABLE ) );
		classes->push_back( SUBCLASS(eTYPE_THROWNENDERPEARL	)->addParent( eTYPE_THROWABLE ) );
		classes->push_back( SUBCLASS(eTYPE_THROWNPOTION		)->addParent( eTYPE_THROWABLE ) );
		classes->push_back( SUBCLASS(eTYPE_THROWNEXPBOTTLE	)->addParent( eTYPE_THROWABLE ) );
		classes->push_back( SUBCLASS(eTYPE_HANGING_ENTITY		)->addParent( eTYPE_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_PAINTING			)->addParent( eTYPE_HANGING_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_ITEM_FRAME		)->addParent( eTYPE_HANGING_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_LEASHFENCEKNOT	)->addParent( eTYPE_HANGING_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_GLOBAL_ENTITY			)->addParent( eTYPE_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_LIGHTNINGBOLT		)->addParent( eTYPE_GLOBAL_ENTITY ) );

		//classes->push_back( SUBCLASS(eTYPE_OTHER_ENTITIES )->addParent( eTYPE_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_EXPERIENCEORB			)->addParent( eTYPE_ENTITY ) );
		classes->push_back( SUBCLASS(eTYPE_EYEOFENDERSIGNAL		)->addParent( eTYPE_ENTITY)->addParent(eTYPE_PROJECTILE ) );
		classes->push_back( SUBCLASS(eTYPE_FIREWORKS_ROCKET		)->addParent( eTYPE_ENTITY)->addParent(eTYPE_PROJECTILE ) );
		classes->push_back( SUBCLASS(eTYPE_FISHINGHOOK			)->addParent( eTYPE_ENTITY)->addParent(eTYPE_PROJECTILE ) );
		classes->push_back( SUBCLASS(eTYPE_DELAYEDRELEASE		)->addParent( eTYPE_ENTITY 	 ) );
		classes->push_back( SUBCLASS(eTYPE_BOAT					)->addParent( eTYPE_ENTITY  ) );
		classes->push_back( SUBCLASS(eTYPE_FALLINGTILE			)->addParent( eTYPE_ENTITY  ) );
		classes->push_back( SUBCLASS(eTYPE_ITEMENTITY			)->addParent( eTYPE_ENTITY  ) );
		classes->push_back( SUBCLASS(eTYPE_PRIMEDTNT				)->addParent( eTYPE_ENTITY ) ); 
		classes->push_back( SUBCLASS(eTYPE_ARROW					)->addParent( eTYPE_ENTITY )->addParent(eTYPE_PROJECTILE ) );
		classes->push_back( SUBCLASS(eTYPE_MULTIENTITY_MOB_PART	)->addParent( eTYPE_ENTITY  ) );
		classes->push_back( SUBCLASS(eTYPE_NETHER_SPHERE			)->addParent( eTYPE_ENTITY  ) );
		classes->push_back( SUBCLASS(eTYPE_ENDER_CRYSTAL			)->addParent( eTYPE_ENTITY  ) );

		classes->push_back( SUBCLASS(eType_BREAKINGITEMPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_BUBBLEPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_EXPLODEPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_FLAMEPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_FOOTSTEPPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_HEARTPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_LAVAPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_NOTEPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_NETHERPORTALPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_REDDUSTPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_SMOKEPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_SNOWSHOVELPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_SPLASHPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_TAKEANIMATIONPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_TERRAINPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_WATERDROPPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_CRITPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_CRITPARTICLE2)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_HUGEEXPLOSIONPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_HUGEEXPLOSIONSEEDPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_PLAYERCLOUDPARTICLEPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_SUSPENDEDPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_SUSPENDEDTOWNPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eTYPE_DRIPPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eTYPE_ENCHANTMENTTABLEPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eTYPE_SPELLPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eTYPE_DRAGONBREATHPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_ENDERPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_FIREWORKSSTARTERPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_FIREWORKSSPARKPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eType_FIREWORKSOVERLAYPARTICLE)->addParent(eTYPE_ENTITY) );
		classes->push_back( SUBCLASS(eTYPE_TILEENTITY) );
		classes->push_back( SUBCLASS(eTYPE_CHESTTILEENTITY				)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_MOBSPAWNERTILEENTITY			)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_FURNACETILEENTITY			)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_SIGNTILEENTITY				)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_MUSICTILEENTITY				)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_RECORDPLAYERTILE				)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_PISTONPIECEENTITY			)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_BREWINGSTANDTILEENTITY		)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_ENCHANTMENTTABLEENTITY		)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_THEENDPORTALTILEENTITY		)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_SKULLTILEENTITY				)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_ENDERCHESTTILEENTITY			)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_BEACONTILEENTITY				)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_COMMANDBLOCKTILEENTITY		)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_COMPARATORTILEENTITY			)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_DAYLIGHTDETECTORTILEENTITY	)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_HOPPERTILEENTITY				)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_DISPENSERTILEENTITY			)->addParent( eTYPE_TILEENTITY			) );
		classes->push_back( SUBCLASS(eTYPE_DROPPERTILEENTITY			)->addParent( eTYPE_DISPENSERTILEENTITY ) );
		
		//classes->push_back( SUBCLASS(eTYPE_OTHERS) );
		classes->push_back( SUBCLASS(eType_NODE) );
		classes->push_back( SUBCLASS(eType_ITEM) );
		classes->push_back( SUBCLASS(eType_ITEMINSTANCE) );
		classes->push_back( SUBCLASS(eType_MAPITEM) );
		classes->push_back( SUBCLASS(eType_TILE) );
		classes->push_back( SUBCLASS(eType_FIRETILE) );

		vector< pair<SubClass*,SubClass*> > m_falsePositives;
		vector< pair<SubClass*,SubClass*> > m_falseNegatives;

		vector<SubClass*>::iterator it1;
		for (it1=classes->begin(); it1!=classes->end(); it1++)
		{
			SubClass *current = *it1;
			//if ( current->justFlag() ) continue;

			vector<SubClass *>::iterator it2;
			for (it2=classes->begin(); it2!=classes->end(); it2++)
			{
				SubClass *comparing = *it2;
				//if ( comparing->justFlag() ) continue;

				// We shouldn't be comparing to leaf classes anyway.
				//if ( comparing->m_isTerminal ) continue;

				eINSTANCEOF typeCurr, typeComp;
				typeCurr = current->m_id;
				typeComp = comparing->m_id;

				bool shouldDerive, doesDerive;
			
				{
					vector<eINSTANCEOF>::iterator it3;
					it3 = find(current->m_parents.begin(), current->m_parents.end(), typeComp);
					shouldDerive = (typeComp == typeCurr) || (it3 != current->m_parents.end());
				}

				doesDerive = eTYPE_DERIVED_FROM(typeComp, typeCurr);

				if (shouldDerive != doesDerive)
				{
					vector< pair<SubClass*,SubClass*> > *errorArray;
					if (shouldDerive)	errorArray = &m_falseNegatives;
					else				errorArray = &m_falsePositives;

					errorArray->push_back( pair<SubClass*,SubClass*>(comparing, current) );
				}
			}
		}

		vector< pair<SubClass*,SubClass*> >::iterator itrErr;
		for (itrErr = m_falsePositives.begin(); itrErr != m_falsePositives.end(); itrErr++)
		{
			SubClass *sub = itrErr->first, *super = itrErr->second;
			printf( "[Class.h] Error: '%s' incorrectly derives from '%s'.\n", sub->m_name.c_str(), super->m_name.c_str() );
		}
		for (itrErr = m_falseNegatives.begin(); itrErr != m_falseNegatives.end(); itrErr++)
		{
			SubClass *sub = itrErr->first, *super = itrErr->second;
			printf( "[Class.h] Error: '%s' doesn't derive '%s'.\n", sub->m_name.c_str(), super->m_name.c_str() );
		}

		if ( (m_falsePositives.size() > 0) || (m_falseNegatives.size() > 0) )
		{
			__debugbreak();
		}
	}
};

#endif