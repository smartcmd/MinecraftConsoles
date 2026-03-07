#include "stdafx.h"
#include "../../../Minecraft.World/compression.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/File.h"
#include "../../../Minecraft.World/compression.h"
#include "../DLC/DLCPack.h"
#include "../DLC/DLCLocalisationFile.h"
#include "../DLC/DLCGameRulesFile.h"
#include "../DLC/DLCGameRules.h"
#include "../DLC/DLCGameRulesHeader.h"
#include "../../StringTable.h"
#include "ConsoleGameRules.h"
#include "GameRuleManager.h"

WCHAR *GameRuleManager::wchTagNameA[] =
{
	L"", // eGameRuleType_Root
	L"MapOptions", // eGameRuleType_LevelGenerationOptions
	L"ApplySchematic", // eGameRuleType_ApplySchematic
	L"GenerateStructure", // eGameRuleType_GenerateStructure
	L"GenerateBox", // eGameRuleType_GenerateBox
	L"PlaceBlock", // eGameRuleType_PlaceBlock
	L"PlaceContainer", // eGameRuleType_PlaceContainer
	L"PlaceSpawner", // eGameRuleType_PlaceSpawner
	L"BiomeOverride", // eGameRuleType_BiomeOverride
	L"StartFeature", // eGameRuleType_StartFeature
	L"AddItem", // eGameRuleType_AddItem
	L"AddEnchantment", // eGameRuleType_AddEnchantment
	L"LevelRules", // eGameRuleType_LevelRules
	L"NamedArea", // eGameRuleType_NamedArea
	L"UseTile", // eGameRuleType_UseTileRule
	L"CollectItem", // eGameRuleType_CollectItemRule
	L"CompleteAll", // eGameRuleType_CompleteAllRule
	L"UpdatePlayer", // eGameRuleType_UpdatePlayerRule
};

WCHAR *GameRuleManager::wchAttrNameA[] =
{
	L"descriptionName", // eGameRuleAttr_descriptionName
	L"promptName", // eGameRuleAttr_promptName
	L"dataTag", // eGameRuleAttr_dataTag
	L"enchantmentId", // eGameRuleAttr_enchantmentId
	L"enchantmentLevel", // eGameRuleAttr_enchantmentLevel
	L"itemId", // eGameRuleAttr_itemId
	L"quantity", // eGameRuleAttr_quantity
	L"auxValue", // eGameRuleAttr_auxValue
	L"slot", // eGameRuleAttr_slot
	L"name", // eGameRuleAttr_name
	L"food", // eGameRuleAttr_food
	L"health", // eGameRuleAttr_health
	L"tileId", // eGameRuleAttr_tileId
	L"useCoords", // eGameRuleAttr_useCoords
	L"seed", // eGameRuleAttr_seed
	L"flatworld", // eGameRuleAttr_flatworld
	L"filename", // eGameRuleAttr_filename
	L"rot", // eGameRuleAttr_rot
	L"data", // eGameRuleAttr_data
	L"block", // eGameRuleAttr_block
	L"entity", // eGameRuleAttr_entity
	L"facing", // eGameRuleAttr_facing
	L"edgeTile", // eGameRuleAttr_edgeTile
	L"fillTile", // eGameRuleAttr_fillTile
	L"skipAir", // eGameRuleAttr_skipAir
	L"x", // eGameRuleAttr_x
	L"x0", // eGameRuleAttr_x0
	L"x1", // eGameRuleAttr_x1
	L"y", // eGameRuleAttr_y
	L"y0", // eGameRuleAttr_y0
	L"y1", // eGameRuleAttr_y1
	L"z", // eGameRuleAttr_z
	L"z0", // eGameRuleAttr_z0
	L"z1", // eGameRuleAttr_z1
	L"chunkX", // eGameRuleAttr_chunkX
	L"chunkZ", // eGameRuleAttr_chunkZ
	L"yRot", // eGameRuleAttr_yRot
	L"spawnX", // eGameRuleAttr_spawnX
	L"spawnY", // eGameRuleAttr_spawnY
	L"spawnZ", // eGameRuleAttr_spawnZ
	L"orientation",
	L"dimension",
	L"topTileId", // eGameRuleAttr_topTileId
	L"biomeId", // eGameRuleAttr_biomeId
	L"feature", // eGameRuleAttr_feature
};

GameRuleManager::GameRuleManager()
{
	m_currentGameRuleDefinitions = NULL;
	m_currentLevelGenerationOptions = NULL;
}

void GameRuleManager::loadGameRules(DLCPack *pack)
{
	StringTable *strings = NULL;

	if(pack->doesPackContainFile(DLCManager::e_DLCType_LocalisationData,L"languages.loc"))
	{
		DLCLocalisationFile *localisationFile = (DLCLocalisationFile *)pack->getFile(DLCManager::e_DLCType_LocalisationData, L"languages.loc");
		strings = localisationFile->getStringTable();
	}

	int gameRulesCount = pack->getDLCItemsCount(DLCManager::e_DLCType_GameRulesHeader);
	for(int i = 0; i < gameRulesCount; ++i)
	{
		DLCGameRulesHeader *dlcHeader = (DLCGameRulesHeader *)pack->getFile(DLCManager::e_DLCType_GameRulesHeader, i);
		DWORD dSize;
		byte *dData = dlcHeader->getData(dSize);

		LevelGenerationOptions *createdLevelGenerationOptions = new LevelGenerationOptions(pack);
		//	= loadGameRules(dData, dSize); //, strings);

		createdLevelGenerationOptions->setGrSource( dlcHeader );
		createdLevelGenerationOptions->setSrc( LevelGenerationOptions::eSrc_fromDLC );

		readRuleFile(createdLevelGenerationOptions, dData, dSize, strings);

		dlcHeader->lgo = createdLevelGenerationOptions;
	}

	gameRulesCount = pack->getDLCItemsCount(DLCManager::e_DLCType_GameRules);
	for (int i = 0; i < gameRulesCount; ++i)
	{
		DLCGameRulesFile *dlcFile = (DLCGameRulesFile *)pack->getFile(DLCManager::e_DLCType_GameRules, i);

		DWORD dSize;
		byte *dData = dlcFile->getData(dSize);

		LevelGenerationOptions *createdLevelGenerationOptions = new LevelGenerationOptions(pack);
		//	= loadGameRules(dData, dSize); //, strings);

		createdLevelGenerationOptions->setGrSource( new JustGrSource() );
		createdLevelGenerationOptions->setSrc( LevelGenerationOptions::eSrc_tutorial );

		readRuleFile(createdLevelGenerationOptions, dData, dSize, strings);

		createdLevelGenerationOptions->setLoadedData();
	}
}

LevelGenerationOptions *GameRuleManager::loadGameRules(byte *dIn, UINT dSize)
{
	LevelGenerationOptions *lgo = new LevelGenerationOptions();
	lgo->setGrSource( new JustGrSource() );
	lgo->setSrc( LevelGenerationOptions::eSrc_fromSave );
	loadGameRules(lgo, dIn, dSize);
	lgo->setLoadedData();
	return lgo;
}

// 4J-JEV: Reverse of saveGameRules.
void GameRuleManager::loadGameRules(LevelGenerationOptions *lgo, byte *dIn, UINT dSize)
{
	app.DebugPrintf("GameRuleManager::LoadingGameRules:\n");

	ByteArrayInputStream bais( byteArray(dIn,dSize) );
	DataInputStream dis(&bais);

	// Read file header.

	//dis.readInt(); // File Size

	short version = dis.readShort();
	assert( 0x1 == version  );
	app.DebugPrintf("\tversion=%d.\n", version);

	for (int i = 0; i < 8; i++) dis.readByte();

	BYTE compression_type = dis.readByte();

	app.DebugPrintf("\tcompressionType=%d.\n", compression_type);

	UINT compr_len, decomp_len;
	compr_len = dis.readInt();
	decomp_len = dis.readInt();

	app.DebugPrintf("\tcompr_len=%d.\n\tdecomp_len=%d.\n", compr_len, decomp_len);


	// Decompress File Body

	byteArray content(new BYTE[decomp_len], decomp_len),
				compr_content(new BYTE[compr_len], compr_len);
	dis.read(compr_content);

	Compression::getCompression()->SetDecompressionType( (Compression::ECompressionTypes)compression_type );
	Compression::getCompression()->DecompressLZXRLE(	content.data, &content.length,
													compr_content.data, compr_content.length);
	Compression::getCompression()->SetDecompressionType( SAVE_FILE_PLATFORM_LOCAL );

	dis.close();
	bais.close();

	delete [] compr_content.data;

	ByteArrayInputStream bais2( content );
	DataInputStream dis2( &bais2 );

	// Read StringTable.
	byteArray bStringTable;
	bStringTable.length = dis2.readInt();
	bStringTable.data = new BYTE[ bStringTable.length ];
	dis2.read(bStringTable);
	StringTable *strings = new StringTable(bStringTable.data, bStringTable.length);

	// Read RuleFile.
	byteArray bRuleFile;
	bRuleFile.length = content.length - bStringTable.length;
	bRuleFile.data = new BYTE[ bRuleFile.length ];
	dis2.read(bRuleFile);

	// 4J-JEV: I don't believe that the path-name is ever used.
	//DLCGameRulesFile *dlcgr = new DLCGameRulesFile(L"__PLACEHOLDER__");
	//dlcgr->addData(bRuleFile.data,bRuleFile.length);

	if (readRuleFile(lgo, bRuleFile.data, bRuleFile.length, strings))
	{
		// Set current gen options and ruleset.
		//createdLevelGenerationOptions->setFromSaveGame(true);
		lgo->setSrc(LevelGenerationOptions::eSrc_fromSave);
		setLevelGenerationOptions( lgo );
		//m_currentGameRuleDefinitions = lgo->getRequiredGameRules();
	}
	else
	{
		delete lgo;
	}

	//delete [] content.data;

	// Close and return.
	dis2.close();
	bais2.close();

	return ;
}

// 4J-JEV: Reverse of loadGameRules.
void GameRuleManager::saveGameRules(byte **dOut, UINT *dSize)
{
	if (m_currentGameRuleDefinitions == NULL &&
		m_currentLevelGenerationOptions == NULL)
	{
		app.DebugPrintf("GameRuleManager:: Nothing here to save.");
		*dOut = NULL;
		*dSize = 0;
		return;
	}

	app.DebugPrintf("GameRuleManager::saveGameRules:\n");

	// Initialise output stream.
	ByteArrayOutputStream baos;
	DataOutputStream dos(&baos);

	// Write header.

	// VERSION NUMBER
	dos.writeShort( 0x1 ); // version_number

	// Write 8 bytes of empty space in case we need them later.
	// Mainly useful for the ones we save embedded in game saves.
	for (UINT i = 0; i < 8; i++)
		dos.writeByte(0x0);

	dos.writeByte(APPROPRIATE_COMPRESSION_TYPE); // m_compressionType

	// -- START COMPRESSED -- //
	ByteArrayOutputStream compr_baos;
	DataOutputStream compr_dos(&compr_baos);

	if (m_currentGameRuleDefinitions == NULL)
	{
		compr_dos.writeInt( 0 ); // numStrings for StringTable
		compr_dos.writeInt( version_number );
		compr_dos.writeByte(Compression::eCompressionType_None); // compression type
		for (int i=0; i<2; i++) compr_dos.writeByte(0x0); // Padding.
		compr_dos.writeInt( 0 ); // StringLookup.length
		compr_dos.writeInt( 0 ); // SchematicFiles.length
		compr_dos.writeInt( 0 ); // XmlObjects.length
	}
	else
	{
		StringTable *st = m_currentGameRuleDefinitions->getStringTable();

		if (st == NULL)
		{
			app.DebugPrintf("GameRuleManager::saveGameRules: StringTable == NULL!");
		}
		else
		{
			// Write string table.
			byteArray stba;
			m_currentGameRuleDefinitions->getStringTable()->getData(&stba.data, &stba.length);
			compr_dos.writeInt( stba.length );
			compr_dos.write( stba );

			// Write game rule file to second
			// buffer and generate string lookup.
			writeRuleFile(&compr_dos);
		}
	}

	// Compress compr_dos and write to dos.
	byteArray compr_ba(new BYTE[ compr_baos.buf.length ], compr_baos.buf.length);
	Compression::getCompression()->CompressLZXRLE(	compr_ba.data, &compr_ba.length,
												compr_baos.buf.data, compr_baos.buf.length );

	app.DebugPrintf("\tcompr_ba.length=%d.\n\tcompr_baos.buf.length=%d.\n",
		compr_ba.length, compr_baos.buf.length );

	dos.writeInt( compr_ba.length ); // Write length
	dos.writeInt( compr_baos.buf.length );
	dos.write(compr_ba);

	delete [] compr_ba.data;

	compr_dos.close();
	compr_baos.close();
	// -- END COMPRESSED -- //

	// return
	*dSize = baos.buf.length;
	*dOut = baos.buf.data;

	baos.buf.data = NULL;

	dos.close(); baos.close();
}

// 4J-JEV: Reverse of readRuleFile.
void GameRuleManager::writeRuleFile(DataOutputStream *dos)
{
	// Write Header
	dos->writeShort(version_number); // Version number.
	dos->writeByte(Compression::eCompressionType_None); // compression type
	for (int i=0; i<8; i++) dos->writeBoolean(false); // Padding.

	// Write string lookup.
	int numStrings = ConsoleGameRules::eGameRuleType_Count + ConsoleGameRules::eGameRuleAttr_Count;
	dos->writeInt(numStrings);
	for (int i = 0; i < ConsoleGameRules::eGameRuleType_Count; i++)	dos->writeUTF( wchTagNameA[i] );
	for (int i = 0; i < ConsoleGameRules::eGameRuleAttr_Count; i++)	dos->writeUTF( wchAttrNameA[i] );

	// Write schematic files.
	unordered_map<wstring, ConsoleSchematicFile *> *files;
	files = getLevelGenerationOptions()->getUnfinishedSchematicFiles();
	for ( auto& it : *files )
	{
		const wstring& filename = it.first;
		ConsoleSchematicFile *file = it.second;

		ByteArrayOutputStream fileBaos;
		DataOutputStream fileDos(&fileBaos);
		file->save(&fileDos);

		dos->writeUTF(filename);
		//dos->writeInt(file->m_data.length);
		dos->writeInt(fileBaos.buf.length);
		dos->write((byteArray)fileBaos.buf);

		fileDos.close(); fileBaos.close();
	}

	// Write xml objects.
	dos->writeInt( 2 ); // numChildren
	m_currentLevelGenerationOptions->write(dos);
	m_currentGameRuleDefinitions->write(dos);
}

bool GameRuleManager::readRuleFile(LevelGenerationOptions *lgo, byte *dIn, UINT dSize, StringTable *strings) //(DLCGameRulesFile *dlcFile, StringTable *strings)
{
	bool levelGenAdded = false;
	bool gameRulesAdded = false;
	LevelGenerationOptions *levelGenerator = lgo;//new LevelGenerationOptions();
	LevelRuleset *gameRules = new LevelRuleset();

	//DWORD dwLen = 0;
	//PBYTE pbData = dlcFile->getData(dwLen);
	//byteArray data(pbData,dwLen);

	byteArray data(dIn, dSize);
	ByteArrayInputStream bais(data);
	DataInputStream dis(&bais);

	// Read File.

	// version_number
	int64_t version = dis.readShort();
	unsigned char compressionType = 0;
	if(version == 0)
	{
		for (int i = 0; i < 14; i++) dis.readByte(); // Read padding.
	}
	else
	{
		compressionType = dis.readByte();

		// Read the spare bytes we inserted for future use
		for(int i = 0; i < 8; ++i) dis.readBoolean();
	}

	ByteArrayInputStream *contentBais = NULL;
	DataInputStream *contentDis = NULL;

	if(compressionType == Compression::eCompressionType_None)
	{
		// No compression
		// No need to read buffer size, as we can read the stream as it is;
		app.DebugPrintf("De-compressing game rules with: None\n");
		contentDis = &dis;
	}
	else
	{
		unsigned int uncompressedSize = dis.readInt();
		unsigned int compressedSize = dis.readInt();
		byteArray compressedBuffer(compressedSize);
		dis.read(compressedBuffer);

		byteArray decompressedBuffer = byteArray(uncompressedSize);

		switch(compressionType)
		{
		case Compression::eCompressionType_None:
			memcpy(decompressedBuffer.data, compressedBuffer.data, uncompressedSize);
			break;

		case Compression::eCompressionType_RLE:
			app.DebugPrintf("De-compressing game rules with: RLE\n");
			Compression::getCompression()->Decompress( decompressedBuffer.data, &decompressedBuffer.length, compressedBuffer.data, compressedSize);
			break;

		default:
			app.DebugPrintf("De-compressing game rules.");
#ifndef _CONTENT_PACKAGE
			assert( compressionType == APPROPRIATE_COMPRESSION_TYPE );
#endif
			// 4J-JEV: DecompressLZXRLE uses the correct platform specific compression type. (need to assert that the data is compressed with it though).
			Compression::getCompression()->DecompressLZXRLE(decompressedBuffer.data, &decompressedBuffer.length, compressedBuffer.data, compressedSize);
			break;
/* 4J-JEV:
	Each platform has only 1 method of compression, 'compression.h' file deals with it.

		case Compression::eCompressionType_LZXRLE:
			app.DebugPrintf("De-compressing game rules with: LZX+RLE\n");
			Compression::getCompression()->DecompressLZXRLE( decompressedBuffer.data, &uncompressedSize, compressedBuffer.data, compressedSize);
			break;
		default:
			app.DebugPrintf("Invalid compression type %d found\n", compressionType);
			__debugbreak();

			delete [] compressedBuffer.data; delete [] decompressedBuffer.data;
			dis.close(); bais.reset();

			if(!gameRulesAdded) delete gameRules;
			return false;
			*/
		};

		delete [] compressedBuffer.data;

		contentBais = new ByteArrayInputStream(decompressedBuffer);
		contentDis = new DataInputStream(contentBais);
	}

	// string lookup.
	UINT numStrings = contentDis->readInt();
	vector<wstring> tagsAndAtts;
	for (UINT i = 0; i < numStrings; i++)
		tagsAndAtts.push_back( contentDis->readUTF() );

	unordered_map<int, ConsoleGameRules::EGameRuleType> tagIdMap;
	for(int type = (int)ConsoleGameRules::eGameRuleType_Root; type < (int)ConsoleGameRules::eGameRuleType_Count; ++type)
	{
		for(UINT i = 0; i < numStrings; ++i)
		{
			if(tagsAndAtts[i].compare(wchTagNameA[type]) == 0)
			{
				tagIdMap.insert( unordered_map<int, ConsoleGameRules::EGameRuleType>::value_type(i, (ConsoleGameRules::EGameRuleType)type) );
				break;
			}
		}
	}

	// 4J-JEV: TODO: As yet unused.
	/*
	unordered_map<int, ConsoleGameRules::EGameRuleAttr> attrIdMap;
	for(int attr = (int)ConsoleGameRules::eGameRuleAttr_descriptionName; attr < (int)ConsoleGameRules::eGameRuleAttr_Count; ++attr)
	{
		for (UINT i = 0; i < numStrings; i++)
		{
			if (tagsAndAtts[i].compare(wchAttrNameA[attr]) == 0)
			{
				tagIdMap.insert( unordered_map<int, ConsoleGameRules::EGameRuleAttr>::value_type(i , (ConsoleGameRules::EGameRuleAttr)attr) );
				break;
			}
		}
	}*/

	// subfile
	UINT numFiles = contentDis->readInt();
	for (UINT i = 0; i < numFiles; i++)
	{
		wstring sFilename = contentDis->readUTF();
		int length = contentDis->readInt();
		byteArray ba( length );

		contentDis->read(ba);

		levelGenerator->loadSchematicFile(sFilename, ba.data, ba.length);

	}

	LEVEL_GEN_ID lgoID = LEVEL_GEN_ID_NULL;

	// xml objects
	UINT numObjects = contentDis->readInt();
	for(UINT i = 0; i < numObjects; ++i)
	{
		int tagId = contentDis->readInt();
		ConsoleGameRules::EGameRuleType tagVal = ConsoleGameRules::eGameRuleType_Invalid;
		auto it = tagIdMap.find(tagId);
		if(it != tagIdMap.end()) tagVal = it->second;

		GameRuleDefinition *rule = NULL;

		if(tagVal == ConsoleGameRules::eGameRuleType_LevelGenerationOptions)
		{
			rule = levelGenerator;
			levelGenAdded = true;
			//m_levelGenerators.addLevelGenerator(L"",levelGenerator);
			lgoID = addLevelGenerationOptions(levelGenerator);
			levelGenerator->loadStringTable(strings);
		}
		else if(tagVal == ConsoleGameRules::eGameRuleType_LevelRules)
		{
			rule = gameRules;
			gameRulesAdded = true;
			m_levelRules.addLevelRule(L"",gameRules);
			levelGenerator->setRequiredGameRules(gameRules);
			gameRules->loadStringTable(strings);
		}

		readAttributes(contentDis, &tagsAndAtts, rule);
		readChildren(contentDis, &tagsAndAtts, &tagIdMap, rule);
	}

	if(compressionType != 0)
	{
		// Not default
		contentDis->close();
		if(contentBais != NULL) delete contentBais;
		delete contentDis;
	}

	dis.close();
	bais.reset();

	//if(!levelGenAdded) { delete levelGenerator; levelGenerator = NULL; }
	if(!gameRulesAdded) delete gameRules;

	return true;
	//return levelGenerator;
}

LevelGenerationOptions *GameRuleManager::readHeader(DLCGameRulesHeader *grh)
{
	LevelGenerationOptions *out =
		new LevelGenerationOptions();


	out->setSrc(LevelGenerationOptions::eSrc_fromDLC);
	out->setGrSource(grh);
	addLevelGenerationOptions(out);

	return out;
}

void GameRuleManager::readAttributes(DataInputStream *dis, vector<wstring> *tagsAndAtts, GameRuleDefinition *rule)
{
	int numAttrs = dis->readInt();
	for (UINT att = 0; att < numAttrs; ++att)
	{
		int attID = dis->readInt();
		wstring value = dis->readUTF();

		if(rule != NULL) rule->addAttribute(tagsAndAtts->at(attID),value);
	}
}

void GameRuleManager::readChildren(DataInputStream *dis, vector<wstring> *tagsAndAtts, unordered_map<int, ConsoleGameRules::EGameRuleType> *tagIdMap, GameRuleDefinition *rule)
{
	int numChildren = dis->readInt();
	for(UINT child = 0; child < numChildren; ++child)
	{
		int tagId = dis->readInt();
		ConsoleGameRules::EGameRuleType tagVal = ConsoleGameRules::eGameRuleType_Invalid;
		auto it = tagIdMap->find(tagId);
		if(it != tagIdMap->end()) tagVal = it->second;

		GameRuleDefinition *childRule = NULL;
		if(rule != NULL) childRule = rule->addChild(tagVal);

		readAttributes(dis,tagsAndAtts,childRule);
		readChildren(dis,tagsAndAtts,tagIdMap,childRule);
	}
}

void GameRuleManager::processSchematics(LevelChunk *levelChunk)
{
	if(getLevelGenerationOptions() != NULL)
	{
		LevelGenerationOptions *levelGenOptions = getLevelGenerationOptions();
		levelGenOptions->processSchematics(levelChunk);
	}
}

void GameRuleManager::processSchematicsLighting(LevelChunk *levelChunk)
{
	if(getLevelGenerationOptions() != NULL)
	{
		LevelGenerationOptions *levelGenOptions = getLevelGenerationOptions();
		levelGenOptions->processSchematicsLighting(levelChunk);
	}
}

void GameRuleManager::loadDefaultGameRules()
{
#ifdef _XBOX
#ifdef _TU_BUILD
	wstring fileRoot = L"UPDATE:\\res\\GameRules\\Tutorial.pck";
#else
	wstring fileRoot = L"GAME:\\res\\TitleUpdate\\GameRules\\Tutorial.pck";
#endif
	File packedTutorialFile(fileRoot);
	if(loadGameRulesPack(&packedTutorialFile))
	{
		m_levelGenerators.getLevelGenerators()->at(0)->setWorldName(app.GetString(IDS_PLAY_TUTORIAL));
		//m_levelGenerators.getLevelGenerators()->at(0)->setDefaultSaveName(L"Tutorial");
		m_levelGenerators.getLevelGenerators()->at(0)->setDefaultSaveName(app.GetString(IDS_TUTORIALSAVENAME));
	}

#else // _XBOX

#ifdef _WINDOWS64
	File packedTutorialFile(L"Windows64Media\\Tutorial\\Tutorial.pck");
	if(!packedTutorialFile.exists()) packedTutorialFile = File(L"Windows64\\Tutorial\\Tutorial.pck");
#elif defined(__ORBIS__)
	File packedTutorialFile(L"/app0/orbis/Tutorial/Tutorial.pck");
#elif defined(__PSVITA__)
	File packedTutorialFile(L"PSVita/Tutorial/Tutorial.pck");
#elif defined(__PS3__)
	File packedTutorialFile(L"PS3/Tutorial/Tutorial.pck");
#else
	File packedTutorialFile(L"Tutorial\\Tutorial.pck");
#endif
	if(loadGameRulesPack(&packedTutorialFile))
	{
		m_levelGenerators.getLevelGenerators()->at(0)->setWorldName(app.GetString(IDS_PLAY_TUTORIAL));
		//m_levelGenerators.getLevelGenerators()->at(0)->setDefaultSaveName(L"Tutorial");
		m_levelGenerators.getLevelGenerators()->at(0)->setDefaultSaveName(app.GetString(IDS_TUTORIALSAVENAME));
	}
#if 0
	wstring fpTutorial = L"Tutorial.pck";
	if(app.getArchiveFileSize(fpTutorial) >= 0)
	{
		DLCPack *pack = new DLCPack(L"",0xffffffff);
		DWORD dwFilesProcessed = 0;
		if ( app.m_dlcManager.readDLCDataFile(dwFilesProcessed,fpTutorial,pack,true) )
		{
			app.m_dlcManager.addPack(pack);
			//m_levelGenerators.getLevelGenerators()->at(0)->setWorldName(app.GetString(IDS_PLAY_TUTORIAL));
			//m_levelGenerators.getLevelGenerators()->at(0)->setDefaultSaveName(app.GetString(IDS_TUTORIALSAVENAME));
		}
		else delete pack;
	}
#endif
#endif
}

bool GameRuleManager::loadGameRulesPack(File *path)
{
	bool success = false;
	if(path->exists())
	{
		DLCPack *pack = new DLCPack(L"",0xffffffff);
		DWORD dwFilesProcessed = 0;
		if( app.m_dlcManager.readDLCDataFile(dwFilesProcessed, path->getPath(),pack))
		{
			app.m_dlcManager.addPack(pack);
			success = true;
		}
		else
		{
			delete pack;
		}
	}
	return success;
}

void GameRuleManager::setLevelGenerationOptions(LevelGenerationOptions *levelGen)
{
	unloadCurrentGameRules();

	m_currentGameRuleDefinitions = NULL;
	m_currentLevelGenerationOptions = levelGen;

	if(m_currentLevelGenerationOptions != NULL && m_currentLevelGenerationOptions->requiresGameRules() )
	{
		m_currentGameRuleDefinitions = m_currentLevelGenerationOptions->getRequiredGameRules();
	}

	if(m_currentLevelGenerationOptions != NULL)
		m_currentLevelGenerationOptions->reset_start();
}

LPCWSTR	GameRuleManager::GetGameRulesString(const wstring &key)
{
	if(m_currentGameRuleDefinitions != NULL && !key.empty() )
	{
		return m_currentGameRuleDefinitions->getString(key);
	}
	else
	{
		return L"";
	}
}

LEVEL_GEN_ID GameRuleManager::addLevelGenerationOptions(LevelGenerationOptions *lgo)
{
	vector<LevelGenerationOptions *> *lgs = m_levelGenerators.getLevelGenerators();

	for (int i = 0; i<lgs->size(); i++)
		if (lgs->at(i) == lgo)
			return i;

	lgs->push_back(lgo);
	return lgs->size() - 1;
}

void GameRuleManager::unloadCurrentGameRules()
{
	if (m_currentLevelGenerationOptions != NULL)
	{
		if (m_currentGameRuleDefinitions != NULL
			&& m_currentLevelGenerationOptions->isFromSave())
				m_levelRules.removeLevelRule( m_currentGameRuleDefinitions );

		if (m_currentLevelGenerationOptions->isFromSave())
		{
			m_levelGenerators.removeLevelGenerator( m_currentLevelGenerationOptions );

			delete m_currentLevelGenerationOptions;
		}
		else if (m_currentLevelGenerationOptions->isFromDLC())
		{
			m_currentLevelGenerationOptions->reset_finish();
		}
	}

	m_currentGameRuleDefinitions = NULL;
	m_currentLevelGenerationOptions = NULL;
}
