#include "stdafx.h"
#include "Settings.h"
#include "..\Minecraft.World\StringHelpers.h"

// 4J - TODO - serialise/deserialise from file
Settings::Settings(File *file)
{
}

void Settings::generateNewProperties()
{
}

void Settings::saveProperties()
{
}

wstring Settings::getString(const wstring& key, const wstring& defaultValue)
{
	if(properties.find(key) == properties.end())
	{
		properties[key] = defaultValue;
		saveProperties();
	}
	return properties[key];
}

int Settings::getInt(const wstring& key, int defaultValue)
{
	if(properties.find(key) == properties.end())
	{
		properties[key] = _toString<int>(defaultValue);
		saveProperties();
	}
	return _fromString<int>(properties[key]);
}

bool Settings::getBoolean(const wstring& key, bool defaultValue)
{
	if(properties.find(key) == properties.end())
	{
		properties[key] = _toString<bool>(defaultValue);
		saveProperties();
	}
	MemSect(35);
	bool retval = _fromString<bool>(properties[key]);
	MemSect(0);
	return retval;
}

void Settings::setBooleanAndSave(const wstring& key, bool value)
{
	properties[key] = _toString<bool>(value);
	saveProperties();
}