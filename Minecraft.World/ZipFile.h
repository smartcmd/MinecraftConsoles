#pragma once
#include <string>
#include <fstream>
#include "File.h"
#include "ZipEntry.h"
#include "../Minecraft.Client/Common/libs/bit7z/include/bitarchivereader.hpp"

class ZipFile {
public:
	ZipFile(File* file);
	ZipFile(std::string name);
	std::vector<std::wstring> listFiles();
	bool hasFile(const std::wstring* name);
	bool hasFile(char* str);
	std::unique_ptr<ZipEntry> getEntry(const std::wstring* name);
	std::vector<BYTE> extract(const std::wstring* name);
	InputStream* getInputStream(ZipEntry* entry);
	InputStream* getInputStream(int entryId);


private:
	static bit7z::Bit7zLibrary* _library;
	std::unique_ptr<bit7z::BitArchiveReader> _reader;
	unordered_map<std::wstring, unsigned int> _fileCache;


	bool open(File* file);


};