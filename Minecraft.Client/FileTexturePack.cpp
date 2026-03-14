#include "stdafx.h"
#include "FileTexturePack.h"
#include "TexturePackRepository.h"
#include "..\Minecraft.World\StringHelpers.h"

FileTexturePack::FileTexturePack(DWORD id, File *file, TexturePack *fallback) : AbstractTexturePack(id, file, file->getName(), fallback)
{
	// 4J Stu - These calls need to be in the most derived version of the class
	loadZipFile(file);
	if (!zipFile) {
		return;
	}

	if (id)
		this->id = id;

	loadColourTable();
	loadIcon();
	loadName(file);
	loadDescription();
	checkTexSize();
}


DLCPack* FileTexturePack::getDLCPack()
{
	return NULL;
}

bool FileTexturePack::hasAudio() { // todo - add audio replacement support
	return false;
}

bool FileTexturePack::hasData()
{
	return !_iconData.empty() && zipFile != nullptr;
}

bool FileTexturePack::isLoadingData()
{
	return false;
}

void FileTexturePack::unload(Textures *textures)
{
#if 0
	super.unload(textures);

	try {
		if (zipFile != null) zipFile.close();
	}
	catch (IOException ignored)
	{
	}
	zipFile = null;
#endif
}

wstring FileTexturePack::getAnimationString(const wstring& textureName, const wstring& path, bool allowFallback) {
	// copy paste of base class method, except I return null and check for existence in getResource
	// so it is fine to skip checking if the file exists and this allows a decent speedup
	
	std::wstring result = L"";
	InputStream* fileStream = getResource(L"\\" + path + textureName + L".txt", true);

	if (fileStream) {
		InputStreamReader isr(fileStream);
		BufferedReader br(&isr);

		wstring line = br.readLine();
		while (!line.empty())
		{
			line = trimString(line);
			if (line.length() > 0)
			{
				result.append(L",");
				result.append(line);
			}
			line = br.readLine();
		}
		delete fileStream;
	}

	return result;
}

InputStream *FileTexturePack::getResourceImplementation(const wstring &name) {
#if 0
	loadZipFile();

	ZipEntry entry = zipFile.getEntry(name.substring(1));
	if (entry == null) {
		throw new FileNotFoundException(name);
	}

	return zipFile.getInputStream(entry);
#endif
	app.DebugPrintf("ZIP: resource - %ls\n", name.c_str());

	auto entry = zipFile->getEntry(&name);
	if (entry) {
		return zipFile->getInputStream(entry.get());
	}
	return nullptr;
}

bool FileTexturePack::hasFile(const wstring &name)
{
#if 0
	try {
		loadZipFile();

		return zipFile.getEntry(name.substring(1)) != null;
	} catch (Exception e) {
		return false;
	}
#endif
#if (defined _WIN64)
	

	return zipFile->hasFile(&name);
#endif
	return false;
}

BufferedImage* FileTexturePack::getImageResource(const wstring& file, bool filenameHasExtension, bool bTitleUpdateTexture, const wstring& drive){
	app.DebugPrintf("ZIP: image - %ls\n", file.c_str());

	if (file == L"terrain.png" && terrainAtlas.get() != nullptr)
		return terrainAtlas.release();

	if (file == L"items.png" && itemAtlas.get() != nullptr)
		return itemAtlas.release();

	std::wstring f = file;
	auto it = AbstractTexturePack::INDEXED_TO_JAVA_MAP.find(L"res/" + file);
	if (it != INDEXED_TO_JAVA_MAP.end()) {
		f = L"assets/minecraft/textures/" + it->second;
	}

	auto folderSep = file.find_last_of(L"/") + 1;
	auto find = file.find(L"res/");
	if (folderSep != file.npos && file.find(L"res/") == 0)
		f = file.substr(folderSep, file.length() - folderSep);

	//f = L"assets/minecraft/textures/" + f;
	auto imageData = zipFile->extract(&f);
	if (!imageData.empty()) {
		auto image = new BufferedImage(imageData.data(), imageData.size());
		return image;
	}

	app.DebugPrintf("ZIP: failed to find %ls fetching from default\n", file.c_str());

	TexturePackRepository* repo = Minecraft::GetInstance()->skins;
	return repo->getDefault()->getImageResource(file, filenameHasExtension, bTitleUpdateTexture);
}

void FileTexturePack::loadIcon() {
	if (zipFile->hasFile("pack.png")) {
		app.DebugPrintf("Found pack.png in zip file, loading as icon\n");
		// auto zipFileEntry = zipFile->getEntry(&std::wstring(L"pack.png"));
		_iconData = zipFile->extract(&std::wstring(L"pack.png"));
		if (!_iconData.empty()) {
			iconImage = new BufferedImage(_iconData.data(), _iconData.size());
			m_iconData = _iconData.data();
			m_iconSize = _iconData.size();
		} else
			app.DebugPrintf("Failed to load pack.png from zip file\n");
	}
}

PBYTE FileTexturePack::getPackIcon(DWORD& dwImageBytes) {
	if (_iconData.empty()) loadIcon();
	if (!_iconData.empty()) { 
		dwImageBytes = m_iconSize;
		return _iconData.data(); 
	}
	return NULL;
}

void FileTexturePack::loadName(File *file) {
	std::wstring name = file->getName();
	int pathSep = max(name.find_last_of('/'), name.find_last_of('\\')) + 1;
	int extSep = name.find_last_of('.');
	if (extSep < 0) {
		extSep = name.length();
	}
	name = name.substr(pathSep, extSep - pathSep);
	texname = name;
}

void FileTexturePack::loadZipFile(File* file) //throws IOException
{
	if (file == NULL || file->getName() == L"") {
		return;
	}

	zipFile = std::make_unique<ZipFile>(file);
}

bool FileTexturePack::isTerrainUpdateCompatible()
{
#if 0
	try {
		loadZipFile();

		Enumeration<? extends ZipEntry> entries = zipFile.entries();
		while (entries.hasMoreElements()) {
			ZipEntry entry = entries.nextElement();
			if (entry.getName().startsWith("textures/")) {
				return true;
			}
		}
	} catch (Exception ignored) {
	}
	boolean hasOldFiles = hasFile("terrain.png") || hasFile("gui/items.png");
	return !hasOldFiles;
#endif
	return false;
}