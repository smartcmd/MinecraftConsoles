#pragma once
using namespace std;

#pragma message("LevelGenerationOptions.h ")

#include "GameRuleDefinition.h"
#include "../../../Minecraft.World/StructureFeature.h"

class ApplySchematicRuleDefinition;
class LevelChunk;
class ConsoleGenerateStructure;
class ConsoleSchematicFile;
class LevelRuleset;
class BiomeOverride;
class StartFeature;

class GrSource
{
public:
	// 4J-JEV:
	// Moved all this here; I didn't like that all this header information
	// was being mixed in with all the game information as they have
	// completely different lifespans.

	virtual bool requiresTexturePack()=0;
	virtual UINT getRequiredTexturePackId()=0;
	virtual wstring getDefaultSaveName()=0;
	virtual LPCWSTR getWorldName()=0;
	virtual LPCWSTR getDisplayName()=0;
	virtual wstring getGrfPath()=0;
	virtual bool requiresBaseSave() = 0;
	virtual wstring getBaseSavePath() = 0;

	virtual void setRequiresTexturePack(bool)=0;
	virtual void setRequiredTexturePackId(UINT)=0;
	virtual void setDefaultSaveName(const wstring &)=0;
	virtual void setWorldName(const wstring &)=0;
	virtual void setDisplayName(const wstring &)=0;
	virtual void setGrfPath(const wstring &)=0;
	virtual void setBaseSavePath(const wstring &)=0;

	virtual bool ready()=0;

	//virtual void getGrfData(PBYTE &pData, DWORD &pSize)=0;
};

class JustGrSource : public GrSource
{
protected:
	wstring m_worldName;
	wstring m_displayName;
	wstring m_defaultSaveName;
	bool m_bRequiresTexturePack;
	int m_requiredTexturePackId;
	wstring m_grfPath;
	wstring m_baseSavePath;
	bool m_bRequiresBaseSave;

public:
	virtual bool requiresTexturePack();
	virtual UINT getRequiredTexturePackId();
	virtual wstring getDefaultSaveName();
	virtual LPCWSTR getWorldName();
	virtual LPCWSTR getDisplayName();
	virtual wstring getGrfPath();
	virtual bool requiresBaseSave();
	virtual wstring getBaseSavePath();

	virtual void setRequiresTexturePack(bool x);
	virtual void setRequiredTexturePackId(UINT x);
	virtual void setDefaultSaveName(const wstring &x);
	virtual void setWorldName(const wstring &x);
	virtual void setDisplayName(const wstring &x);
	virtual void setGrfPath(const wstring &x);
	virtual void setBaseSavePath(const wstring &x);

	virtual bool ready();

	JustGrSource();
};

class LevelGenerationOptions : public GameRuleDefinition
{
public:
	enum eSrc
	{
		eSrc_none,

		eSrc_fromSave,	// Neither content or header is persistent.

		eSrc_fromDLC,	// Header is persistent, content should be deleted to conserve space.

		eSrc_tutorial,	// Both header and content is persistent, content cannot be reloaded.

		eSrc_MAX
	};

private:
	eSrc m_src;

	GrSource *m_pSrc;
	GrSource *info();

	bool m_hasLoadedData;

	PBYTE m_pbBaseSaveData;
	DWORD m_dwBaseSaveSize;

public:

	void setSrc(eSrc src);
	eSrc getSrc();

	bool isTutorial();
	bool isFromSave();
	bool isFromDLC();

	bool requiresTexturePack();
	UINT getRequiredTexturePackId();
	wstring getDefaultSaveName();
	LPCWSTR getWorldName();
	LPCWSTR getDisplayName();
	wstring getGrfPath();
	bool requiresBaseSave();
	wstring getBaseSavePath();

	void setGrSource(GrSource *grs);

	void setRequiresTexturePack(bool x);
	void setRequiredTexturePackId(UINT x);
	void setDefaultSaveName(const wstring &x);
	void setWorldName(const wstring &x);
	void setDisplayName(const wstring &x);
	void setGrfPath(const wstring &x);
	void setBaseSavePath(const wstring &x);

	bool ready();

	void setBaseSaveData(PBYTE pbData, DWORD dwSize);
	PBYTE getBaseSaveData(DWORD &size);
	bool hasBaseSaveData();
	void deleteBaseSaveData();

	bool hasLoadedData();
	void setLoadedData();

private:
	// This should match the "MapOptionsRule" definition in the XML schema
	int64_t m_seed;
	bool m_useFlatWorld;
	Pos *m_spawnPos;
	int m_bHasBeenInCreative;
	vector<ApplySchematicRuleDefinition *> m_schematicRules;
	vector<ConsoleGenerateStructure *> m_structureRules;
	bool m_bHaveMinY;
	int m_minY;
	unordered_map<wstring, ConsoleSchematicFile *> m_schematics;
	vector<BiomeOverride *> m_biomeOverrides;
	vector<StartFeature *> m_features;

	bool m_bRequiresGameRules;
	LevelRuleset *m_requiredGameRules;

	StringTable *m_stringTable;

	DLCPack *m_parentDLCPack;
	bool m_bLoadingData;

public:
	LevelGenerationOptions(DLCPack *parentPack = nullptr);
	~LevelGenerationOptions();

	virtual ConsoleGameRules::EGameRuleType getActionType();

	virtual void writeAttributes(DataOutputStream *dos, UINT numAttributes);
	virtual void getChildren(vector<GameRuleDefinition *> *children);
	virtual GameRuleDefinition *addChild(ConsoleGameRules::EGameRuleType ruleType);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	int64_t getLevelSeed();
	int getLevelHasBeenInCreative();
	Pos *getSpawnPos();
	bool getuseFlatWorld();

	void processSchematics(LevelChunk *chunk);
	void processSchematicsLighting(LevelChunk *chunk);

	bool checkIntersects(int x0, int y0, int z0, int x1, int y1, int z1);

private:
	void clearSchematics();

public:
	ConsoleSchematicFile *loadSchematicFile(const wstring &filename, PBYTE pbData, DWORD dwLen);

public:
	ConsoleSchematicFile *getSchematicFile(const wstring &filename);
	void releaseSchematicFile(const wstring &filename);

	bool requiresGameRules();
	void setRequiredGameRules(LevelRuleset *rules);
	LevelRuleset *getRequiredGameRules();

	void getBiomeOverride(int biomeId, BYTE &tile, BYTE &topTile);
	bool isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes feature, int *orientation = nullptr);

	void loadStringTable(StringTable *table);
	LPCWSTR getString(const wstring &key);

	unordered_map<wstring, ConsoleSchematicFile *> *getUnfinishedSchematicFiles();

	void loadBaseSaveData();
	static int packMounted(LPVOID pParam,int iPad,DWORD dwErr,DWORD dwLicenceMask);

	// 4J-JEV:
	// ApplySchematicRules contain limited state
	// which needs to be reset BEFORE a new game starts.
	void reset_start();

	// 4J-JEV:
	// This file contains state that needs to be deleted
	// or reset once a game has finished.
	void reset_finish();
};