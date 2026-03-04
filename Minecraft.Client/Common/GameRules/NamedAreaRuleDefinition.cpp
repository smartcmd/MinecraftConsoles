#include "stdafx.h"
#include "NamedAreaRuleDefinition.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.phys.h"

NamedAreaRuleDefinition::NamedAreaRuleDefinition()
{
	m_name = L"";
	m_area = AABB::newPermanent(0,0,0,0,0,0);
}

NamedAreaRuleDefinition::~NamedAreaRuleDefinition()
{
	delete m_area;
}

void NamedAreaRuleDefinition::writeAttributes(DataOutputStream *dos, UINT numAttributes)
{
	GameRuleDefinition::writeAttributes(dos, numAttributes + 7);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_name);
	dos->writeUTF(m_name);

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x0);
	dos->writeUTF(_toString(m_area->x0));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y0);
	dos->writeUTF(_toString(m_area->y0));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z0);
	dos->writeUTF(_toString(m_area->z0));

	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_x1);
	dos->writeUTF(_toString(m_area->x1));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_y1);
	dos->writeUTF(_toString(m_area->y1));
	ConsoleGameRules::write(dos, ConsoleGameRules::eGameRuleAttr_z1);
	dos->writeUTF(_toString(m_area->z1));
}

void NamedAreaRuleDefinition::addAttribute(const wstring &attributeName, const wstring &attributeValue)
{
	if(attributeName.compare(L"name") == 0)
	{
		m_name = attributeValue;
#ifndef _CONTENT_PACKAGE
		wprintf(L"NamedAreaRuleDefinition: Adding parameter name=%ls\n",m_name.c_str());
#endif
	}
	else if(attributeName.compare(L"x0") == 0)
	{
		m_area->x0 = _fromString<int>(attributeValue);
		app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter x0=%f\n",m_area->x0);
	}
	else if(attributeName.compare(L"y0") == 0)
	{
		m_area->y0 = _fromString<int>(attributeValue);
		if(m_area->y0 < 0) m_area->y0 = 0;
		app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter y0=%f\n",m_area->y0);
	}
	else if(attributeName.compare(L"z0") == 0)
	{
		m_area->z0 = _fromString<int>(attributeValue);
		app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter z0=%f\n",m_area->z0);
	}
	else if(attributeName.compare(L"x1") == 0)
	{
		m_area->x1 = _fromString<int>(attributeValue);
		app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter x1=%f\n",m_area->x1);
	}
	else if(attributeName.compare(L"y1") == 0)
	{
		m_area->y1 = _fromString<int>(attributeValue);
		if(m_area->y1 < 0) m_area->y1 = 0;
		app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter y1=%f\n",m_area->y1);
	}
	else if(attributeName.compare(L"z1") == 0)
	{
		m_area->z1 = _fromString<int>(attributeValue);
		app.DebugPrintf("NamedAreaRuleDefinition: Adding parameter z1=%f\n",m_area->z1);
	}
	else
	{
		GameRuleDefinition::addAttribute(attributeName, attributeValue);
	}
}
