#pragma once

#include <vector>
#include <unordered_map>

#include "..\Minecraft.World\File.h"
#include "..\Minecraft.World\ArrayWithLength.h"

using namespace std;

class ArchiveFile
{
protected:
	File m_sourcefile;
	BYTE *m_cachedData;

	typedef struct _MetaData
	{
		wstring filename;
		int ptr;
		int filesize;
		bool isCompressed;

	} MetaData, *PMetaData;
	
	unordered_map<wstring, PMetaData> m_index;
	
public:
	void _readHeader(DataInputStream *dis);

	ArchiveFile(File file);
	~ArchiveFile();

	vector<wstring> *getFileList();
	bool hasFile(const wstring &filename);
	int getFileSize(const wstring &filename);
	byteArray getFile(const wstring &filename);
};