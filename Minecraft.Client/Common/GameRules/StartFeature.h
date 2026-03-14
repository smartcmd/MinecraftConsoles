#pragma once
using namespace std;

#include "GameRuleDefinition.h"
#include "../../../Minecraft.World/StructureFeature.h"

class StartFeature : public GameRuleDefinition
{
private:
	int m_chunkX, m_chunkZ, m_orientation;
	StructureFeature::EFeatureTypes m_feature;

public:
	StartFeature();

	virtual ConsoleGameRules::EGameRuleType getActionType() { return ConsoleGameRules::eGameRuleType_StartFeature; }
	
	virtual void writeAttributes(DataOutputStream *dos, UINT numAttrs);
	virtual void addAttribute(const wstring &attributeName, const wstring &attributeValue);

	bool isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes feature, int *orientation);
};