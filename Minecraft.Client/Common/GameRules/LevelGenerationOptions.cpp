#include "stdafx.h"

#include <unordered_set>

#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\Pos.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "Common\DLC\DLCGameRulesHeader.h"
#include "..\..\StringTable.h"
#include "LevelGenerationOptions.h"
#include "ConsoleGameRules.h"

JustGrSource::JustGrSource()
{
	m_displayName = L"Default_DisplayName";
	m_worldName= L"Default_WorldName";
	m_defaultSaveName = L"Default_DefaultSaveName";
	m_bRequiresTexturePack = false;
	m_requiredTexturePackId = 0;
	m_grfPath = L"__NO_GRF_PATH__";
	m_bRequiresBaseSave = false;
}

bool JustGrSource::requiresTexturePack() {return m_bRequiresTexturePack;}
UINT JustGrSource::getRequiredTexturePackId() {return m_requiredTexturePackId;}
wstring JustGrSource::getDefaultSaveName() {return m_defaultSaveName;}
LPCWSTR JustGrSource::getWorldName() {return m_worldName.c_str();}
LPCWSTR JustGrSource::getDisplayName() {return m_displayName.c_str();}
wstring JustGrSource::getGrfPath() {return m_grfPath;}
bool JustGrSource::requiresBaseSave() { return m_bRequiresBaseSave; };
wstring JustGrSource::getBaseSavePath() { return m_baseSavePath; };

void JustGrSource::setRequiresTexturePack(bool x) {m_bRequiresTexturePack = x;}
void JustGrSource::setRequiredTexturePackId(UINT x) {m_requiredTexturePackId = x;}
void JustGrSource::setDefaultSaveName(const wstring &x) {m_defaultSaveName = x;}
void JustGrSource::setWorldName(const wstring &x) {m_worldName = x;}
void JustGrSource::setDisplayName(const wstring &x) {m_displayName = x;}
void JustGrSource::setGrfPath(const wstring &x) {m_grfPath = x;}
void JustGrSource::setBaseSavePath(const wstring &x) { m_baseSavePath = x; m_bRequiresBaseSave = true; }

bool JustGrSource::ready() { return true; }

LevelGenerationOptions::LevelGenerationOptions(DLCPack *parentPack)
{
	m_spawnPos = NULL;
	m_stringTable = NULL;

	m_hasLoadedData = false;

	m_seed = 0;
	m_bHasBeenInCreative = true;
	m_useFlatWorld = false;
	m_bHaveMinY = false;
	m_minY = INT_MAX;
	m_bRequiresGameRules = false;

	m_pbBaseSaveData = NULL;
	m_dwBaseSaveSize = 0;

	m_parentDLCPack = parentPack;
	m_bLoadingData = false;
}

LevelGenerationOptions::~LevelGenerationOptions()
{
	clearSchematics();
	if(m_spawnPos != NULL) delete m_spawnPos;
	for (auto& it : m_schematicRules )
	{
		delete it;
	}

	for (auto& it : m_structureRules )
	{
		delete it;
	}

	for (auto& it : m_biomeOverrides )
	{
		delete it;
	}

	for (auto& it : m_features )
	{
		delete it;
	}

	if (m_stringTable)
		if (!isTutorial())
			delete m_stringTable;

	if (isFromSave()) delete m_pSrc;
}

ConsoleGameRules::EGameRuleType LevelGenerationOptions::getActionType() { return ConsoleGameRules::eGameRuleType_LevelGenerationOptions; }

void LevelGenerationOptions::writeAttributes(DataOutputStream *dos, UINT numAttrs)
{
	GameRuleDefinition::writeAttributes(dos, numAttrs + 5);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_spawnX);
	dos->writeUTF(std::to_wstring(m_spawnPos->x));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_spawnY);
	dos->writeUTF(std::to_wstring(m_spawnPos->y));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_spawnZ);
	dos->writeUTF(std::to_wstring(m_spawnPos->z));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_seed);
	dos->writeUTF(std::to_wstring(m_seed));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_flatworld);
	dos->writeUTF(std::to_wstring(m_useFlatWorld));
}

void LevelGenerationOptions::getChildren(vector<GameRuleDefinition *> *children)
{
	GameRuleDefinition::getChildren(children);

	vector<ApplySchematicRuleDefinition *> used_schematics;
	for (auto& it : m_schematicRules )
		if ( it && !it->isComplete() )
			used_schematics.push_back( it );

	for (auto& it : m_structureRules)
		if ( it )
			children->push_back( it );

	for (auto& it : used_schematics)
		if ( it )
			children->push_back( it );

	for (auto& it : m_biomeOverrides)
		if ( it )
			children->push_back( it );

	for (auto& it : m_features)
		if ( it )
			children->push_back( it );
}

GameRuleDefinition *LevelGenerationOptions::addChild(ConsoleGameRules::EGameRuleType ruleType)
{
	GameRuleDefinition *rule = NULL;
	if(ruleType == ConsoleGameRules::eGameRuleType_ApplySchematic)
	{
		rule = new ApplySchematicRuleDefinition(this);
		m_schematicRules.push_back((ApplySchematicRuleDefinition *)rule);
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_GenerateStructure)
	{
		rule = new ConsoleGenerateStructure();
		m_structureRules.push_back((ConsoleGenerateStructure *)rule);
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_BiomeOverride)
	{
		rule = new BiomeOverride();
		m_biomeOverrides.push_back((BiomeOverride *)rule);
	}
	else if(ruleType == ConsoleGameRules::eGameRuleType_StartFeature)
	{
		rule = new StartFeature();
		m_features.push_back((StartFeature *)rule);
	}
	else
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"LevelGenerationOptions: Attempted to add invalid child rule - %d\n", ruleType );
#endif
	}
	return rule;
}

void LevelGenerationOptions::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"seed") == 0)
	{
		m_seed = _fromString<int64_t>(attributeValue);
		app.DebugPrintf("LevelGenerationOptions: Adding parameter m_seed=%I64d\n",m_seed);
	}
	else if(attributeName.compare(L"spawnX") == 0)
	{
		if(m_spawnPos == NULL) m_spawnPos = new Pos();
		int value = _fromString<int>(attributeValue);
		m_spawnPos->x = value;
		app.DebugPrintf("LevelGenerationOptions: Adding parameter spawnX=%d\n",value);
	}
	else if(attributeName.compare(L"spawnY") == 0)
	{
		if(m_spawnPos == NULL) m_spawnPos = new Pos();
		int value = _fromString<int>(attributeValue);
		m_spawnPos->y = value;
		app.DebugPrintf("LevelGenerationOptions: Adding parameter spawnY=%d\n",value);
	}
	else if(attributeName.compare(L"spawnZ") == 0)
	{
		if(m_spawnPos == NULL) m_spawnPos = new Pos();
		int value = _fromString<int>(attributeValue);
		m_spawnPos->z = value;
		app.DebugPrintf("LevelGenerationOptions: Adding parameter spawnZ=%d\n",value);
	}
	else if(attributeName.compare(L"flatworld") == 0)
	{
		if(attributeValue.compare(L"true") == 0) m_useFlatWorld = true;
		app.DebugPrintf("LevelGenerationOptions: Adding parameter flatworld=%s\n",m_useFlatWorld?"TRUE":"FALSE");
	}
	else if(attributeName.compare(L"saveName") == 0)
	{
		wstring string(attributeValue);
		if(!string.empty()) setDefaultSaveName( string );
		else setDefaultSaveName( attributeValue );
		app.DebugPrintf("LevelGenerationOptions: Adding parameter saveName=%ls\n", getDefaultSaveName().c_str());
	}
	else if(attributeName.compare(L"worldName") == 0)
	{
		wstring string(attributeValue);
		if(!string.empty()) setWorldName( string );
		else setWorldName( attributeValue );
		app.DebugPrintf("LevelGenerationOptions: Adding parameter worldName=%ls\n", getWorldName());
	}
	else if(attributeName.compare(L"displayName") == 0)
	{
		wstring string(attributeValue);
		if(!string.empty()) setDisplayName( string );
		else setDisplayName( attributeValue );
		app.DebugPrintf("LevelGenerationOptions: Adding parameter displayName=%ls\n", getDisplayName());
	}
	else if(attributeName.compare(L"texturePackId") == 0)
	{
		setRequiredTexturePackId( _fromString<unsigned int>(attributeValue) );
		setRequiresTexturePack( true );
		app.DebugPrintf("LevelGenerationOptions: Adding parameter texturePackId=%0x\n", getRequiredTexturePackId());
	}
	else if(attributeName.compare(L"isTutorial") == 0)
	{
		if(attributeValue.compare(L"true") == 0) setSrc(eSrc_tutorial);
		app.DebugPrintf("LevelGenerationOptions: Adding parameter isTutorial=%s\n",isTutorial()?"TRUE":"FALSE");
	}
	else if(attributeName.compare(L"baseSaveName") == 0)
	{
		setBaseSavePath( attributeValue );
		app.DebugPrintf("LevelGenerationOptions: Adding parameter baseSaveName=%ls\n", getBaseSavePath().c_str());
	}
	else if(attributeName.compare(L"hasBeenInCreative") == 0)
	{
		bool value = _fromString<bool>(attributeValue);
		m_bHasBeenInCreative = value;
		app.DebugPrintf("LevelGenerationOptions: Adding parameter gameMode=%d\n", m_bHasBeenInCreative);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

void LevelGenerationOptions::processSchematics(LevelChunk *chunk)
{
	PIXBeginNamedEvent(0,"Processing schematics for chunk (%d,%d)", chunk->x, chunk->z);
	AABB *chunkBox = AABB::newTemp(chunk->x*16,0,chunk->z*16,chunk->x*16 + 16,Level::maxBuildHeight,chunk->z*16 + 16);
	for( ApplySchematicRuleDefinition *rule : m_schematicRules )
		rule->processSchematic(chunkBox, chunk);

	int cx = (chunk->x << 4);
	int cz = (chunk->z << 4);

	for ( ConsoleGenerateStructure *structureStart : m_structureRules )
	{
		if (structureStart->getBoundingBox()->intersects(cx, cz, cx + 15, cz + 15))
		{
			BoundingBox *bb = new BoundingBox(cx, cz, cx + 15, cz + 15);
			structureStart->postProcess(chunk->level, NULL, bb);
			delete bb;
		}
	}
	PIXEndNamedEvent();
}

void LevelGenerationOptions::processSchematicsLighting(LevelChunk *chunk)
{
	PIXBeginNamedEvent(0,"Processing schematics (lighting) for chunk (%d,%d)", chunk->x, chunk->z);
	AABB *chunkBox = AABB::newTemp(chunk->x*16,0,chunk->z*16,chunk->x*16 + 16,Level::maxBuildHeight,chunk->z*16 + 16);
	for ( ApplySchematicRuleDefinition *rule : m_schematicRules )
	{
		rule->processSchematicLighting(chunkBox, chunk);
	}
	PIXEndNamedEvent();
}

bool LevelGenerationOptions::checkIntersects(int x0, int y0, int z0, int x1, int y1, int z1)
{
	PIXBeginNamedEvent(0,"Check Intersects");

	// As an optimisation, we can quickly discard things below a certain y which makes most ore checks faster due to
	// a) ores generally being below ground/sea level and b) tutorial world additions generally being above ground/sea level
	if(!m_bHaveMinY)
	{
		for ( ApplySchematicRuleDefinition *rule : m_schematicRules )
		{
			int minY = rule->getMinY();
			if(minY < m_minY) m_minY = minY;
		}

		for ( ConsoleGenerateStructure *structureStart : m_structureRules )
		{
			int minY = structureStart->getMinY();
			if(minY < m_minY) m_minY = minY;
		}

		m_bHaveMinY = true;
	}

	// 4J Stu - We DO NOT intersect if our upper bound is below the lower bound for all schematics
	if( y1 < m_minY ) return false;

	bool intersects = false;
	for( ApplySchematicRuleDefinition *rule : m_schematicRules )
	{
		intersects = rule->checkIntersects(x0,y0,z0,x1,y1,z1);
		if(intersects) break;
	}

	if(!intersects)
	{
		for( ConsoleGenerateStructure *structureStart : m_structureRules )
		{
			intersects = structureStart->checkIntersects(x0,y0,z0,x1,y1,z1);
			if(intersects) break;
		}
	}
	PIXEndNamedEvent();
	return intersects;
}

void LevelGenerationOptions::clearSchematics()
{
	for ( auto& it : m_schematics )
	{
		delete it.second;
	}
	m_schematics.clear();
}

ConsoleSchematicFile *LevelGenerationOptions::loadSchematicFile(const wstring &filename, PBYTE pbData, DWORD dwLen)
{
	// If we have already loaded this, just return
	auto it = m_schematics.find(filename);
	if(it != m_schematics.end())
	{
#ifndef _CONTENT_PACKAGE
		wprintf(L"We have already loaded schematic file %ls\n", filename.c_str() );
#endif
		it->second->incrementRefCount();
		return it->second;
	}

	ConsoleSchematicFile *schematic = NULL;
	byteArray data(pbData,dwLen);
	ByteArrayInputStream bais(data);
	DataInputStream dis(&bais);
	schematic = new ConsoleSchematicFile();
	schematic->load(&dis);
	m_schematics[filename] = schematic;
	bais.reset();
	return schematic;
}

ConsoleSchematicFile *LevelGenerationOptions::getSchematicFile(const wstring &filename)
{
	ConsoleSchematicFile *schematic = NULL;
	// If we have already loaded this, just return
	auto it = m_schematics.find(filename);
	if(it != m_schematics.end())
	{
		schematic = it->second;
	}
	return schematic;
}

void LevelGenerationOptions::releaseSchematicFile(const wstring &filename)
{
	// 4J Stu - We don't want to delete them when done, but probably want to keep a set of active schematics for the current world
	// auto it = m_schematics.find(filename);
	//if(it != m_schematics.end())
	//{
	//	ConsoleSchematicFile *schematic = it->second;
	//	schematic->decrementRefCount();
	//	if(schematic->shouldDelete())
	//	{
	//		delete schematic;
	//		m_schematics.erase(it);
	//	}
	//}
}

void LevelGenerationOptions::loadStringTable(StringTable *table)
{
	m_stringTable = table;
}

LPCWSTR LevelGenerationOptions::getString(const wstring &key)
{
	if(m_stringTable == NULL)
	{
		return L"";
	}
	else
	{
		return m_stringTable->getString(key);
	}
}

void LevelGenerationOptions::getBiomeOverride(int biomeId, BYTE &tile, BYTE &topTile)
{
	for ( BiomeOverride *bo : m_biomeOverrides )
	{
		if ( bo && bo->isBiome(biomeId) )
		{
			bo->getTileValues(tile,topTile);
			break;
		}
	}
}

bool LevelGenerationOptions::isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes feature, int *orientation)
{
	bool isFeature = false;

	for( StartFeature *sf : m_features )
	{
		if(sf->isFeatureChunk(chunkX, chunkZ, feature, orientation))
		{
			isFeature = true;
			break;
		}
	}
	return isFeature;
}

unordered_map<wstring, ConsoleSchematicFile *> *LevelGenerationOptions::getUnfinishedSchematicFiles()
{
	// Clean schematic rules.
	unordered_set<wstring> usedFiles = unordered_set<wstring>();
	for ( auto& it : m_schematicRules )
		if ( !it->isComplete() )
			usedFiles.insert( it->getSchematicName() );

	// Clean schematic files.
	unordered_map<wstring, ConsoleSchematicFile *> *out
		= new unordered_map<wstring, ConsoleSchematicFile *>();
	for ( auto& it : usedFiles )
		out->insert( pair<wstring, ConsoleSchematicFile *>(it, getSchematicFile(it)) );

	return out;
}

void LevelGenerationOptions::loadBaseSaveData()
{
	int mountIndex = -1;
	if(m_parentDLCPack != NULL) mountIndex = m_parentDLCPack->GetDLCMountIndex();

	if(mountIndex > -1)
	{
#ifdef _DURANGO
		if(StorageManager.MountInstalledDLC(ProfileManager.GetPrimaryPad(),mountIndex,&LevelGenerationOptions::packMounted,this,L"WPACK")!=ERROR_IO_PENDING)
#else
		if(StorageManager.MountInstalledDLC(ProfileManager.GetPrimaryPad(),mountIndex,&LevelGenerationOptions::packMounted,this,"WPACK")!=ERROR_IO_PENDING)
#endif
		{
			// corrupt DLC
			setLoadedData();
			app.DebugPrintf("Failed to mount LGO DLC %d for pad %d\n",mountIndex,ProfileManager.GetPrimaryPad());
		}
		else
		{
			m_bLoadingData = true;
			app.DebugPrintf("Attempted to mount DLC data for LGO %d\n", mountIndex);
		}
	}
	else
	{
		setLoadedData();
		app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadTexturePack);
	}
}

int LevelGenerationOptions::packMounted(LPVOID pParam,int iPad,DWORD dwErr,DWORD dwLicenceMask)
{
	LevelGenerationOptions *lgo = (LevelGenerationOptions *)pParam;
	lgo->m_bLoadingData = false;
	if(dwErr!=ERROR_SUCCESS)
	{
		// corrupt DLC
		app.DebugPrintf("Failed to mount LGO DLC for pad %d: %d\n",iPad,dwErr);
	}
	else
	{
		app.DebugPrintf("Mounted DLC for LGO, attempting to load data\n");
		DWORD dwFilesProcessed = 0;
		int gameRulesCount = lgo->m_parentDLCPack->getDLCItemsCount(DLCManager::e_DLCType_GameRulesHeader);
		for(int i = 0; i < gameRulesCount; ++i)
		{
			DLCGameRulesHeader *dlcFile = (DLCGameRulesHeader *) lgo->m_parentDLCPack->getFile(DLCManager::e_DLCType_GameRulesHeader, i);

			if (!dlcFile->getGrfPath().empty())
			{
				File grf( app.getFilePath(lgo->m_parentDLCPack->GetPackID(), dlcFile->getGrfPath(),true, L"WPACK:" ) );
				if (grf.exists())
				{
#ifdef _UNICODE
					wstring path = grf.getPath();
					const WCHAR *pchFilename=path.c_str();
					HANDLE fileHandle = CreateFile(
						pchFilename, // file name
						GENERIC_READ, // access mode
						0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
						NULL, // Unused
						OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
						FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
						NULL // Unsupported
						);
#else
					const char *pchFilename=wstringtofilename(grf.getPath());
					HANDLE fileHandle = CreateFile(
						pchFilename, // file name
						GENERIC_READ, // access mode
						0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
						NULL, // Unused
						OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
						FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
						NULL // Unsupported
						);
#endif

					if( fileHandle != INVALID_HANDLE_VALUE )
					{
						DWORD dwFileSize = grf.length();
						DWORD bytesRead;
						PBYTE pbData =  (PBYTE) new BYTE[dwFileSize];
						BOOL bSuccess = ReadFile(fileHandle,pbData,dwFileSize,&bytesRead,NULL);
						if(bSuccess==FALSE)
						{
							app.FatalLoadError();
						}
						CloseHandle(fileHandle);

						// 4J-PB - is it possible that we can get here after a read fail and it's not an error?
						dlcFile->setGrfData(pbData, dwFileSize, lgo->m_stringTable);

						delete [] pbData;

						app.m_gameRules.setLevelGenerationOptions( dlcFile->lgo );
					}
				}
			}
		}
		if(lgo->requiresBaseSave() && !lgo->getBaseSavePath().empty() )
		{
			File save(app.getFilePath(lgo->m_parentDLCPack->GetPackID(), lgo->getBaseSavePath(),true, L"WPACK:" ));
			if (save.exists())
			{
#ifdef _UNICODE
				wstring path = save.getPath();
				const WCHAR *pchFilename=path.c_str();
				HANDLE fileHandle = CreateFile(
					pchFilename, // file name
					GENERIC_READ, // access mode
					0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
					NULL, // Unused
					OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
					FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
					NULL // Unsupported
					);
#else
				const char *pchFilename=wstringtofilename(save.getPath());
				HANDLE fileHandle = CreateFile(
					pchFilename, // file name
					GENERIC_READ, // access mode
					0, // share mode // TODO 4J Stu - Will we need to share file? Probably not but...
					NULL, // Unused
					OPEN_EXISTING , // how to create // TODO 4J Stu - Assuming that the file already exists if we are opening to read from it
					FILE_FLAG_SEQUENTIAL_SCAN, // file attributes
					NULL // Unsupported
					);
#endif

				if( fileHandle != INVALID_HANDLE_VALUE )
				{
					DWORD bytesRead,dwFileSize = GetFileSize(fileHandle,NULL);
					PBYTE pbData =  (PBYTE) new BYTE[dwFileSize];
					BOOL bSuccess = ReadFile(fileHandle,pbData,dwFileSize,&bytesRead,NULL);
					if(bSuccess==FALSE)
					{
						app.FatalLoadError();
					}
					CloseHandle(fileHandle);

					// 4J-PB - is it possible that we can get here after a read fail and it's not an error?
					lgo->setBaseSaveData(pbData, dwFileSize);
				}
			}

		}
#ifdef _DURANGO
		DWORD result = StorageManager.UnmountInstalledDLC(L"WPACK");
#else
		DWORD result = StorageManager.UnmountInstalledDLC("WPACK");
#endif

	}

	lgo->setLoadedData();

	return 0;
}

void LevelGenerationOptions::reset_start()
{
	for ( auto& it : m_schematicRules )
	{
		if ( it )
			it->reset();
	}
}

void LevelGenerationOptions::reset_finish()
{
	//if (m_spawnPos)				{ delete m_spawnPos; m_spawnPos = NULL; }
	//if (m_stringTable)			{ delete m_stringTable; m_stringTable = NULL; }

	if (isFromDLC())
	{
		m_hasLoadedData = false;
	}
}


GrSource *LevelGenerationOptions::info() { return m_pSrc; }
void LevelGenerationOptions::setSrc(eSrc src) { m_src = src; }
LevelGenerationOptions::eSrc LevelGenerationOptions::getSrc() { return m_src; }

bool LevelGenerationOptions::isTutorial() { return getSrc() == eSrc_tutorial; }
bool LevelGenerationOptions::isFromSave() { return getSrc() == eSrc_fromSave; }
bool LevelGenerationOptions::isFromDLC() { return getSrc() == eSrc_fromDLC; }

bool LevelGenerationOptions::requiresTexturePack() { return info()->requiresTexturePack(); }
UINT LevelGenerationOptions::getRequiredTexturePackId() { return info()->getRequiredTexturePackId(); }

wstring LevelGenerationOptions::getDefaultSaveName()
{
	switch (getSrc())
	{
	case eSrc_fromSave:		return getString( info()->getDefaultSaveName() );
	case eSrc_fromDLC:		return getString( info()->getDefaultSaveName() );
	case eSrc_tutorial:		return app.GetString(IDS_TUTORIALSAVENAME);
	}
	return L"";
}
LPCWSTR LevelGenerationOptions::getWorldName()
{
	switch (getSrc())
	{
	case eSrc_fromSave:		return getString( info()->getWorldName() );
	case eSrc_fromDLC:		return getString( info()->getWorldName() );
	case eSrc_tutorial:		return app.GetString(IDS_PLAY_TUTORIAL);
	}
	return L"";
}
LPCWSTR LevelGenerationOptions::getDisplayName()
{
	switch (getSrc())
	{
	case eSrc_fromSave:		return getString( info()->getDisplayName() );
	case eSrc_fromDLC:		return getString( info()->getDisplayName() );
	case eSrc_tutorial:		return L"";
	}
	return L"";
}

wstring LevelGenerationOptions::getGrfPath() {	return info()->getGrfPath(); }
bool LevelGenerationOptions::requiresBaseSave() { return info()->requiresBaseSave(); }
wstring LevelGenerationOptions::getBaseSavePath() { return info()->getBaseSavePath(); }

void LevelGenerationOptions::setGrSource(GrSource *grs) { m_pSrc = grs; }

void LevelGenerationOptions::setRequiresTexturePack(bool x) { info()->setRequiresTexturePack(x); }
void LevelGenerationOptions::setRequiredTexturePackId(UINT x) { info()->setRequiredTexturePackId(x); }
void LevelGenerationOptions::setDefaultSaveName(const wstring &x) { info()->setDefaultSaveName(x); }
void LevelGenerationOptions::setWorldName(const wstring &x) { info()->setWorldName(x); }
void LevelGenerationOptions::setDisplayName(const wstring &x) { info()->setDisplayName(x); }
void LevelGenerationOptions::setGrfPath(const wstring &x) { info()->setGrfPath(x); }
void LevelGenerationOptions::setBaseSavePath(const wstring &x) { info()->setBaseSavePath(x); }

bool LevelGenerationOptions::ready() { return info()->ready(); }

void LevelGenerationOptions::setBaseSaveData(PBYTE pbData, DWORD dwSize) { m_pbBaseSaveData = pbData; m_dwBaseSaveSize = dwSize; }
PBYTE LevelGenerationOptions::getBaseSaveData(DWORD &size) { size = m_dwBaseSaveSize; return m_pbBaseSaveData; }
bool LevelGenerationOptions::hasBaseSaveData() { return m_dwBaseSaveSize > 0 && m_pbBaseSaveData != NULL; }
void LevelGenerationOptions::deleteBaseSaveData() { if(m_pbBaseSaveData) delete m_pbBaseSaveData; m_pbBaseSaveData = NULL; m_dwBaseSaveSize = 0; }

bool LevelGenerationOptions::hasLoadedData() { return m_hasLoadedData; }
void LevelGenerationOptions::setLoadedData() { m_hasLoadedData = true; }

int64_t LevelGenerationOptions::getLevelSeed() { return m_seed; }
int LevelGenerationOptions::getLevelHasBeenInCreative() { return m_bHasBeenInCreative; }
Pos *LevelGenerationOptions::getSpawnPos() { return m_spawnPos; }
bool LevelGenerationOptions::getuseFlatWorld() { return m_useFlatWorld; }

bool LevelGenerationOptions::requiresGameRules() { return m_bRequiresGameRules; }
void LevelGenerationOptions::setRequiredGameRules(LevelRuleset *rules) { m_requiredGameRules = rules; m_bRequiresGameRules = true; }
LevelRuleset *LevelGenerationOptions::getRequiredGameRules() { return m_requiredGameRules; }