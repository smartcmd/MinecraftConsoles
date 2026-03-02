#pragma once
// 4J Stu - Implements the Java InputStream but rather than writing directly to disc it writes through the save file

#include "InputStream.h"

#include "ConsoleSavePath.h"

class ConsoleSaveFile;
class FileEntry;

class ConsoleSaveFileInputStream : public InputStream
{
public:
	ConsoleSaveFileInputStream(ConsoleSaveFile *saveFile, const ConsoleSavePath &file);
	ConsoleSaveFileInputStream(ConsoleSaveFile *saveFile, FileEntry *file);
	virtual int read();
	virtual int read(byteArray b);
	virtual int read(byteArray b, unsigned int offset, unsigned int length);
	virtual void close();
	virtual __int64 skip(__int64 n) { return n; }

private:
	ConsoleSaveFile *m_saveFile;
	FileEntry *m_file;

};