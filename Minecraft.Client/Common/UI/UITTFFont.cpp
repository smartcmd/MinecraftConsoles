#include "stdafx.h"
#include "UI.h"
#include "../../../Minecraft.World/StringHelpers.h"
#include "../../../Minecraft.World/File.h"
#include "UITTFFont.h"

UITTFFont::UITTFFont(const string &name, const string &path, S32 fallbackCharacter) 
	: m_strFontName(name)
{
	app.DebugPrintf("UITTFFont opening %s\n",path.c_str());

#ifdef _UNICODE
	wstring wPath = convStringToWstring(path);
	HANDLE file = CreateFile(wPath.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
	HANDLE file = CreateFile(path.c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#endif
	if( file == INVALID_HANDLE_VALUE )
	{
		DWORD error = GetLastError();
		app.DebugPrintf("Failed to open TTF file with error code %d (%x)\n", error, error);
		assert(false);
	}

	DWORD dwHigh=0;
	DWORD dwFileSize = GetFileSize(file,&dwHigh);

	if(dwFileSize!=0)
	{
		DWORD bytesRead;

		pbData =  (PBYTE) new BYTE[dwFileSize];
		BOOL bSuccess = ReadFile(file,pbData,dwFileSize,&bytesRead,nullptr);
		if(bSuccess==FALSE)
		{
			app.FatalLoadError();
		}
		CloseHandle(file);

		IggyFontInstallTruetypeUTF8 ( (void *)pbData, IGGY_TTC_INDEX_none, m_strFontName.c_str(), -1, IGGY_FONTFLAG_none );

		IggyFontInstallTruetypeFallbackCodepointUTF8( m_strFontName.c_str(), -1, IGGY_FONTFLAG_none, fallbackCharacter );

		// 4J Stu - These are so we can use the default flash controls
		IggyFontInstallTruetypeUTF8 ( (void *)pbData, IGGY_TTC_INDEX_none, "Times New Roman", -1, IGGY_FONTFLAG_none );
		IggyFontInstallTruetypeUTF8 ( (void *)pbData, IGGY_TTC_INDEX_none, "Arial", -1, IGGY_FONTFLAG_none );
	}
}

UITTFFont::~UITTFFont()
{
}


string UITTFFont::getFontName()
{
	return m_strFontName;
}