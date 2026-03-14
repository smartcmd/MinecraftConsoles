#include "StdAfx.h"
#include "StringTable.h"

StringTable::StringTable(void)
{

}

// Load string table from a binary blob, filling out with the current localisation data only
StringTable::StringTable(PBYTE pbData, DWORD dwSize)
{
	src = byteArray(pbData, dwSize);

	ProcessStringTableData();
}


void StringTable::ReloadStringTable()
{
	m_stringsMap.clear();
	m_stringsVec.clear();

	ProcessStringTableData();
}

void StringTable::ProcessStringTableData(void)
{
	ByteArrayInputStream bais(src);
	DataInputStream dis(&bais);

	int versionNumber = dis.readInt();
	int languagesCount = dis.readInt();

	vector< pair<wstring, int> > langSizeMap;
	for(int i = 0; i < languagesCount; ++i)
	{
		wstring langId = dis.readUTF();
		int langSize = dis.readInt();

		langSizeMap.push_back( vector< pair<wstring, int> >::value_type(langId, langSize));
	}

	vector<wstring> locales;
	app.getLocale(locales);

	bool foundLang = false;
	int64_t bytesToSkip = 0;
	int dataSize = 0;

	//
    for (auto it_locales = locales.begin();
         it_locales != locales.end() && (!foundLang);
         ++it_locales)
    {
		bytesToSkip = 0;

		for(auto& it : langSizeMap)
		{
			if(it.first.compare(*it_locales) == 0)
			{
				app.DebugPrintf("StringTable:: Found language '%ls'.\n", it_locales->c_str());
				dataSize = it.second;
				foundLang = true;
				break;
			}

			bytesToSkip += it.second;
		}

		if (!foundLang)
			app.DebugPrintf("StringTable:: Can't find language '%ls'.\n", it_locales->c_str());
	}

	if(foundLang)
	{
		dis.skip(bytesToSkip);

		byteArray langData(dataSize);
		dis.read(langData);

		dis.close();

		ByteArrayInputStream bais2(langData);
		DataInputStream dis2(&bais2);

		// Read the language file for the selected language
		int langVersion = dis2.readInt();

		isStatic = false;     // 4J-JEV: Versions 1 and up could use
		if (langVersion > 0)  // integers rather than wstrings as keys.
			isStatic = dis2.readBoolean();

		wstring langId = dis2.readUTF();
		int totalStrings = dis2.readInt();

		app.DebugPrintf("IsStatic=%d totalStrings = %d\n",isStatic?1:0,totalStrings);

		if (!isStatic)
		{
			for(int i = 0; i < totalStrings; ++i)
			{
				wstring stringId = dis2.readUTF();
				wstring stringValue = dis2.readUTF();

				m_stringsMap.insert( unordered_map<wstring, wstring>::value_type(stringId, stringValue) );
			}
		}
		else
		{
			for(int i = 0; i < totalStrings; ++i)
				m_stringsVec.push_back( dis2.readUTF() );
		}
		dis2.close();

		// We can't delete this data in the dtor, so clear the reference
		bais2.reset();
	}
	else
	{
		app.DebugPrintf("Failed to get language\n");


		isStatic = false;
	}

	// We can't delete this data in the dtor, so clear the reference
	bais.reset();
}


StringTable::~StringTable(void)
{
	// delete src.data; TODO 4J-JEV: ?
}

void StringTable::getData(PBYTE *ppData, UINT *pSize)
{
	*ppData = src.data;
	*pSize = src.length;
}

LPCWSTR StringTable::getString(const wstring &id)
{
#ifndef _CONTENT_PACKAGE
	if (isStatic)
	{
		__debugbreak();
		return L"";
	}
#endif

    auto it = m_stringsMap.find(id);

    if(it != m_stringsMap.end())
	{
		return it->second.c_str();
	}
	else
	{
		return L"";
	}
}

LPCWSTR StringTable::getString(int id)
{
#ifndef _CONTENT_PACKAGE
	if (!isStatic)
	{
		__debugbreak();
		return L"";
	}
#endif

	if (id < m_stringsVec.size())
	{
		LPCWSTR pwchString=m_stringsVec.at(id).c_str();
		return pwchString;
	}
	else
		return L"";
}





