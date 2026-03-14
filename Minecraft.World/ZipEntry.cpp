#include "stdafx.h"
#include "ZipEntry.h"

ZipEntry::ZipEntry(bit7z::BitArchiveItemOffset itemInfo) : _itemInfo(itemInfo) {
	_name = itemInfo.name();
	_path = itemInfo.path();
}

DWORD ZipEntry::getSize() {
	return _itemInfo.size();
}

bool ZipEntry::isDir() {
	return _itemInfo.isDir();
}

std::string ZipEntry::getName() {
	return _name;
}

std::string ZipEntry::path() {
	return _path;
}

unsigned int ZipEntry::getIndex() {
	return (unsigned int)_itemInfo.index();
}