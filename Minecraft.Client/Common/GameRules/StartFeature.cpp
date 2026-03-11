#include "stdafx.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "StartFeature.h"

StartFeature::StartFeature()
{
	m_chunkX = 0;
	m_chunkZ = 0;
	m_orientation = 0;
	m_feature = StructureFeature::eFeature_Temples;
}

void StartFeature::writeAttributes(DataOutputStream *dos, UINT numAttrs)
{
	GameRuleDefinition::writeAttributes(dos, numAttrs + 4);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_chunkX);
	dos->writeUTF(std::to_wstring(m_chunkX));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_chunkZ);
	dos->writeUTF(std::to_wstring(m_chunkZ));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_feature);
	dos->writeUTF(std::to_wstring((int)m_feature));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_orientation);
	dos->writeUTF(std::to_wstring(m_orientation));
}

void StartFeature::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"chunkX") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_chunkX = value;
		app.DebugPrintf("StartFeature: Adding parameter chunkX=%d\n",m_chunkX);
	}
	else if(attributeName.compare(L"chunkZ") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_chunkZ = value;
		app.DebugPrintf("StartFeature: Adding parameter chunkZ=%d\n",m_chunkZ);
	}
	else if(attributeName.compare(L"orientation") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_orientation = value;
		app.DebugPrintf("StartFeature: Adding parameter orientation=%d\n",m_orientation);
	}
	else if(attributeName.compare(L"feature") == 0)
	{
		int value = _fromString<int>(attributeValue);
		m_feature = static_cast<StructureFeature::EFeatureTypes>(value);
		app.DebugPrintf("StartFeature: Adding parameter feature=%d\n",m_feature);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}

bool StartFeature::isFeatureChunk(int chunkX, int chunkZ, StructureFeature::EFeatureTypes feature, int *orientation)
{
	if(orientation != nullptr) *orientation = m_orientation;
	return chunkX == m_chunkX && chunkZ == m_chunkZ && feature == m_feature;
}