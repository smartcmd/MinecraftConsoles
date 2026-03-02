#pragma once
#include "GameRuleDefinition.h"
#include "ConsoleSchematicFile.h"

class AABB;
class Vec3;
class LevelChunk;
class LevelGenerationOptions;
class GRFObject;

class ApplySchematicRuleDefinition : public GameRuleDefinition
{
private:
	LevelGenerationOptions *m_levelGenOptions;
	wstring m_schematicName;
	ConsoleSchematicFile *m_schematic;
	Vec3 *m_location;
	AABB *m_locationBox;
	ConsoleSchematicFile::ESchematicRotation m_rotation;
	int m_dimension;

	__int64 m_totalBlocksChanged;
	__int64 m_totalBlocksChangedLighting;
	bool m_completed;

	void updateLocationBox();
public:	
	ApplySchematicRuleDefinition(LevelGenerationOptions *levelGenOptions);
	~ApplySchematicRuleDefinition();

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_ApplySchematic; }
	
	virtual void writeAttributes(DataOutputStream *dos, UINT numAttrs);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	void processSchematic(AABB *chunkBox, LevelChunk *chunk);
	void processSchematicLighting(AABB *chunkBox, LevelChunk *chunk);

	bool checkIntersects(int x0, int y0, int z0, int x1, int y1, int z1);
	int getMinY();

	bool isComplete() { return m_completed; }

	wstring getSchematicName() { return m_schematicName; }

	/** 4J-JEV:
	 *  This GameRuleDefinition contains limited game state.
	 *	Reset any state to how it should be before a new game.
	 */
	void reset();
};