#pragma once

//#include "

class ConsoleGameRules
{
public:
	enum EGameRuleType
	{
		eGameRuleType_Invalid = -1,
		eGameRuleType_Root = 0, // This is the top level rule that defines a game mode, this is used to generate data for new players

		eGameRuleType_LevelGenerationOptions,
		eGameRuleType_ApplySchematic,
		eGameRuleType_GenerateStructure,
		eGameRuleType_GenerateBox,
		eGameRuleType_PlaceBlock,
		eGameRuleType_PlaceContainer,
		eGameRuleType_PlaceSpawner,
		eGameRuleType_BiomeOverride,
		eGameRuleType_StartFeature,

		eGameRuleType_AddItem,
		eGameRuleType_AddEnchantment,

		eGameRuleType_LevelRules,
		eGameRuleType_NamedArea,

		eGameRuleType_UseTileRule,
		eGameRuleType_CollectItemRule,
		eGameRuleType_CompleteAllRule,
		eGameRuleType_UpdatePlayerRule,

		eGameRuleType_Count
	};

	enum EGameRuleAttr
	{
		eGameRuleAttr_Invalid = -1,

		eGameRuleAttr_descriptionName = 0,
		eGameRuleAttr_promptName,
		eGameRuleAttr_dataTag,

		eGameRuleAttr_enchantmentId,
		eGameRuleAttr_enchantmentLevel,

		eGameRuleAttr_itemId,
		eGameRuleAttr_quantity,
		eGameRuleAttr_auxValue,
		eGameRuleAttr_slot,

		eGameRuleAttr_name,

		eGameRuleAttr_food,
		eGameRuleAttr_health,

		eGameRuleAttr_tileId,
		eGameRuleAttr_useCoords,

		eGameRuleAttr_seed,
		eGameRuleAttr_flatworld,

		eGameRuleAttr_filename,
		eGameRuleAttr_rot,

		eGameRuleAttr_data,
		eGameRuleAttr_block,
		eGameRuleAttr_entity,

		eGameRuleAttr_facing,

		eGameRuleAttr_edgeTile,
		eGameRuleAttr_fillTile,
		eGameRuleAttr_skipAir,

		eGameRuleAttr_x,
		eGameRuleAttr_x0,
		eGameRuleAttr_x1,
		
		eGameRuleAttr_y,
		eGameRuleAttr_y0,
		eGameRuleAttr_y1,

		eGameRuleAttr_z,
		eGameRuleAttr_z0,
		eGameRuleAttr_z1,

		eGameRuleAttr_chunkX,
		eGameRuleAttr_chunkZ,

		eGameRuleAttr_yRot,

		eGameRuleAttr_spawnX,
		eGameRuleAttr_spawnY,
		eGameRuleAttr_spawnZ,

		eGameRuleAttr_orientation,
		eGameRuleAttr_dimension,

		eGameRuleAttr_topTileId,
		eGameRuleAttr_biomeId,

		eGameRuleAttr_feature,

		eGameRuleAttr_Count
	};

	static void write(DataOutputStream *dos, ConsoleGameRules::EGameRuleType eType)
	{
		dos->writeInt(eType);
	}

	static void write(DataOutputStream *dos, ConsoleGameRules::EGameRuleAttr eAttr)
	{
		dos->writeInt( eGameRuleType_Count + eAttr );
	}

};