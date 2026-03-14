#pragma once
#include "../Minecraft.Client/Common/libs/bit7z/include/bitarchivereader.hpp"

class ZipEntry {
private:
	bit7z::BitArchiveItemOffset _itemInfo;

	// caching for faster load times
	std::string _name;
	std::string _path;

public:
	ZipEntry(bit7z::BitArchiveItemOffset itemInfo);
	DWORD getSize();
	bool isDir();
	bool isFile() { return !isDir(); }
	std::string getName();
	std::string path();
	unsigned int getIndex();
};