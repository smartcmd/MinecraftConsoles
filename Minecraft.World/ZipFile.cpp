#include "stdafx.h"
#include "ZipFile.h"

// static library
bit7z::Bit7zLibrary* ZipFile::_library;

ZipFile::ZipFile(File* file) {
	if (file != NULL && file->exists()) {
		open(file);
	}
}

ZipFile::ZipFile(std::string name) {
	File file(convStringToWstring(name));
	if (file.exists()) {
		open(&file);
	}
}

std::vector<std::wstring> ZipFile::listFiles() {
	std::vector<std::wstring> files;
	files.reserve(_fileCache.size());

	for (auto item : _fileCache) {
		files.emplace_back(item.first);
	}
	return files;
}

// returns -1 if not found
std::unique_ptr<ZipEntry> ZipFile::getEntry(const std::wstring* name) {
	bool path = name->find('/') != name->npos || name->find('\\') != name->npos;
	std::wstring nameCopy = *name;
#ifdef _WIN64
	if (path) { // 
		for (int i = 0; i < nameCopy.length(); i++)
			if (nameCopy[i] == L'/')
				nameCopy[i] = L'\\';
	}
#endif

	auto it = _fileCache.find(nameCopy);
	if (it != _fileCache.end()) {
		auto entryInfo = _reader->itemAt(it->second);
		return std::make_unique<ZipEntry>(entryInfo);
	}

	return nullptr;
}

std::vector<BYTE> ZipFile::extract(const std::wstring* name) {
	auto entry = getEntry(name);
	if (!entry)
		return {};

	try {
		auto size = entry->getSize();
		std::vector<BYTE> buffer(size);
		_reader->extractTo(buffer.data(), size, entry->getIndex());
		return buffer;
	}
	catch (exception e) {
		OutputDebugString(wstringtochararray(L"Error extracting file from zip: " + *name + L'\n'));
		OutputDebugString(wstringtochararray(L"Exception: " + convStringToWstring(e.what()) + L'\n'));
		__debugbreak();
		return {};
	}
}

bool ZipFile::hasFile(const std::wstring* name) {
	bool path = name->find('/') != name->npos || name->find('\\') != name->npos;
	std::wstring nameCopy = *name;
#ifdef _WIN64
	if (path) { // 
		for (int i = 0; i < nameCopy.length(); i++)
			if (nameCopy[i] == L'/')
				nameCopy[i] = L'\\';
	}
#endif

	return _fileCache.find(nameCopy) != _fileCache.end();
}

bool ZipFile::hasFile(char* str) {
	try {
		std::wstring wstr = convStringToWstring(std::string(str));
		return hasFile(&wstr);
	}
	catch (exception e) {
		return false;
	}
}

InputStream* ZipFile::getInputStream(ZipEntry* entry) {
	std::vector<BYTE> data(entry->getSize());
	_reader->extractTo(data.data(), data.size(), entry->getIndex());
	byteArray* buf = new byteArray(data.data(), data.size());
	ByteArrayInputStream* stream = new ByteArrayInputStream(*buf, 0, data.size());
	return stream;
}

InputStream* ZipFile::getInputStream(int entryId) {
	auto entry = _reader->itemAt(entryId);
	std::vector<BYTE> data(entry.size());
	_reader->extractTo(data.data(), data.size(), entry.index());
	byteArray* buf = new byteArray(data.data(), data.size());
	ByteArrayInputStream* stream = new ByteArrayInputStream(*buf, 0, data.size());
	return stream;
}

bool ZipFile::open(File* file) {
	try {
		std::wstring wpath = file->getPath();
		int charCount = WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, NULL, 0, NULL, NULL);
		std::string path(charCount - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wpath.c_str(), -1, &path[0], charCount, NULL, NULL);

		if (_library == nullptr) {
			OutputDebugString("Initializing bit7z library");
			_library = new bit7z::Bit7zLibrary("Windows64Media\\7z.dll");
		}
		
		_reader = std::make_unique<bit7z::BitArchiveReader>(*_library, path, bit7z::BitFormat::Zip);
		auto _libentries = _reader->items();
		_fileCache.clear();

		for (auto &item : _libentries) {
			if (!item.isDir()) { // We never unpack directories, this also saves time comparing against them
				_fileCache[item.nativePath()] = item.index();
				_fileCache[convStringToWstring(item.name())] = item.index();
			}
		}
	}
	catch (exception e) {
		OutputDebugString(wstringtochararray(L"Error opening zip file: " + file->getPath()));
		OutputDebugString(wstringtochararray(L"\nException: " + convStringToWstring(e.what())));
		__debugbreak();
	}
	return false;
}