#pragma once

class ColourTable
{
private:
	unsigned int m_colourValues[eMinecraftColour_COUNT];

	static const wchar_t *ColourTableElements[eMinecraftColour_COUNT];
	static unordered_map<wstring,eMinecraftColour> s_colourNamesMap;

public:
	static void staticCtor();

	ColourTable(PBYTE pbData, DWORD dwLength);
	ColourTable(ColourTable *defaultColours, PBYTE pbData, DWORD dwLength);

	unsigned int getColour(eMinecraftColour id);
	unsigned int getColor(eMinecraftColour id) { return getColour(id); }

	void loadColoursFromData(PBYTE pbData, DWORD dwLength);
	void setColour(const wstring &colourName, int value);
	void setColour(const wstring &colourName, const wstring &value);
};