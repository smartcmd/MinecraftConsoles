#pragma once

#include <cstdint>
#include <vector>
#include <string>

// 4J Stu - Represents java standard library class

class FileFilter;

class File
{
public:
	//The system-dependent path-separator character
	static const wchar_t pathSeparator;

	// 4J Jev, the start of the file root
	static const wstring pathRoot;

	File() { m_abstractPathName = L""; }

	File( const File &parent, const wstring& child );
	File( const wstring& pathname );
	File( const wstring& parent, const wstring& child  );
	bool _delete();
	bool mkdir() const;
	bool mkdirs() const;
	bool exists() const;
	bool isFile() const;
	bool renameTo(File dest);
	std::vector<File *> *listFiles() const; // Array
	std::vector<File *> *listFiles(FileFilter *filter) const;
	bool isDirectory() const;
	int64_t length();
	int64_t lastModified();
	const wstring getPath() const; // 4J Jev: TODO
	wstring getName() const;

	static int hash_fnct(const File &k);
	static bool eq_test(const File &x, const File &y);

private:
	void _init();
	wstring m_abstractPathName;

	// 4J Jev, just helper functions, change between paths and vector<string>
	//File(vector<wstring> *path);
};

struct FileKeyHash
{
	int operator() (const File &k) const;
};

struct FileKeyEq
{
	bool operator() (const File &x, const File &y) const;
};