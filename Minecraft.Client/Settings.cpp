#include "stdafx.h"
#include "Settings.h"
#include "..\Minecraft.World\File.h"
#include "..\Minecraft.World\StringHelpers.h"
#include <fstream>

static wstring ParsePropertyText(const string &text)
{
	return trimString(convStringToWstring(text));
}

static bool TryParseBoolean(const wstring &text, bool defaultValue)
{
	wstring lowered = toLower(trimString(text));
	if (lowered == L"true" || lowered == L"1" || lowered == L"yes" || lowered == L"on")
		return true;
	if (lowered == L"false" || lowered == L"0" || lowered == L"no" || lowered == L"off")
		return false;
	return defaultValue;
}

Settings::Settings(File *file)
{
	if (file != NULL)
	{
		filePath = file->getPath();
	}

	if (filePath.empty())
		return;

	std::ifstream stream(wstringtofilename(filePath), std::ios::in | std::ios::binary);
	if (!stream.is_open())
		return;

	string line;
	while (std::getline(stream, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);

		if (line.size() >= 3 &&
			(unsigned char)line[0] == 0xEF &&
			(unsigned char)line[1] == 0xBB &&
			(unsigned char)line[2] == 0xBF)
		{
			line.erase(0, 3);
		}

		size_t commentPos = line.find_first_of("#;");
		if (commentPos != string::npos && line.find_first_not_of(" \t") == commentPos)
			continue;

		size_t separatorPos = line.find('=');
		if (separatorPos == string::npos)
			continue;

		wstring key = ParsePropertyText(line.substr(0, separatorPos));
		if (key.empty())
			continue;

		wstring value = ParsePropertyText(line.substr(separatorPos + 1));
		properties[key] = value;
	}
}

void Settings::generateNewProperties()
{
	saveProperties();
}

void Settings::saveProperties()
{
	if (filePath.empty())
		return;

	std::ofstream stream(wstringtofilename(filePath), std::ios::out | std::ios::binary | std::ios::trunc);
	if (!stream.is_open())
		return;

	stream << "# MinecraftConsoles dedicated server properties\r\n";
	for (unordered_map<wstring, wstring>::const_iterator it = properties.begin(); it != properties.end(); ++it)
	{
		string key = string(wstringtochararray(it->first));
		string value = string(wstringtochararray(it->second));
		stream << key << "=" << value << "\r\n";
	}
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
		properties[key] = std::to_wstring(defaultValue);
		saveProperties();
	}
	return _fromString<int>(properties[key]);
}

bool Settings::getBoolean(const wstring& key, bool defaultValue)
{
	if(properties.find(key) == properties.end())
	{
		properties[key] = defaultValue ? L"true" : L"false";
		saveProperties();
	}
	MemSect(35);
	bool retval = TryParseBoolean(properties[key], defaultValue);
	MemSect(0);
	return retval;
}

void Settings::setBooleanAndSave(const wstring& key, bool value)
{
	properties[key] = value ? L"true" : L"false";
	saveProperties();
}