#pragma once

class UITTFFont
{
private:
	const string m_strFontName;

	PBYTE pbData;
	//DWORD dwDataSize;

public:
	UITTFFont(const string &name, const string &path, S32 fallbackCharacter);
	~UITTFFont();

	string getFontName();
};
