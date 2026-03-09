#include "stdafx.h"
#include "FileTexturePack.h"

FileTexturePack::FileTexturePack(DWORD id, File *file, TexturePack *fallback) : AbstractTexturePack(id, file, file->getName(), fallback)
{
	// 4J Stu - These calls need to be in the most derived version of the class
	loadIcon();
	loadName();
	loadDescription();
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

InputStream *FileTexturePack::getResourceImplementation(const wstring &name) //throws IOException
{
#if 0
	loadZipFile();

	ZipEntry entry = zipFile.getEntry(name.substring(1));
	if (entry == null) {
		throw new FileNotFoundException(name);
	}

	return zipFile.getInputStream(entry);
#endif
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
	return false;
}

void FileTexturePack::loadZipFile() //throws IOException
{
#if 0
	if (zipFile != null) {
		return;
	}

	zipFile = new ZipFile(file);
#endif
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