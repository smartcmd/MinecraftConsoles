#include "stdafx.h"
#include "FolderTexturePack.h"

FolderTexturePack::FolderTexturePack(DWORD id, const wstring &name, File *folder, TexturePack *fallback) : AbstractTexturePack(id, folder, name, fallback)
{
	// 4J Stu - These calls need to be in the most derived version of the class
	loadIcon();
	loadName();
	loadDescription();

	bUILoaded = false;
}

InputStream *FolderTexturePack::getResourceImplementation(const wstring &name) //throws IOException
{
#if 0
	final File file = new File(this.file, name.substring(1));
	if (!file.exists()) {
		throw new FileNotFoundException(name);
	}

	return new BufferedInputStream(new FileInputStream(file));
#endif

	wstring wDrive = L"";
	// Make the content package point to to the UPDATE: drive is needed
#ifdef _XBOX
	wDrive=L"GAME:\\DummyTexturePack\\res";
#else
	wDrive = L"Common\\DummyTexturePack\\res";
#endif
	InputStream *resource = InputStream::getResourceAsStream(wDrive + name);
	//InputStream *stream = DefaultTexturePack::class->getResourceAsStream(name);
	//if (stream == NULL)
	//{
	//	throw new FileNotFoundException(name);
	//}

	//return stream;
	return resource;
}

bool FolderTexturePack::hasFile(const wstring &name)
{
	File file = File( getPath() + name);
	return file.exists() && file.isFile();
	//return true;
}

bool FolderTexturePack::isTerrainUpdateCompatible()
{
#if 0
	final File dir = new File(this.file, "textures/");
	final boolean hasTexturesFolder = dir.exists() && dir.isDirectory();
	final boolean hasOldFiles = hasFile("terrain.png") || hasFile("gui/items.png");
	return hasTexturesFolder || !hasOldFiles;
#endif
	return true;
}

wstring FolderTexturePack::getPath(bool bTitleUpdateTexture /*= false*/,const char *pchBDPatchFilename)
{
	wstring wDrive;
#ifdef _XBOX
		wDrive=L"GAME:\\" + file->getPath() + L"\\";
#else
		wDrive=L"Common\\" + file->getPath() + L"\\";
#endif
	return wDrive;
}

void FolderTexturePack::loadUI()
{
#ifdef _XBOX
	//"file://" + Drive + PathToXZP + "#" + PathInsideXZP

	//L"file://game:/ui.xzp#skin_default.xur"

	// Load new skin
	if(hasFile(L"TexturePack.xzp"))
	{
		const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
		WCHAR szResourceLocator[ LOCATOR_SIZE ];

		swprintf(szResourceLocator, LOCATOR_SIZE,L"file://%lsTexturePack.xzp#skin_Minecraft.xur",getPath().c_str());

		XuiFreeVisuals(L"");
		app.LoadSkin(szResourceLocator,NULL);//L"TexturePack");
		bUILoaded = true;
		//CXuiSceneBase::GetInstance()->SetVisualPrefix(L"TexturePack");
	}

	AbstractTexturePack::loadUI();
#endif
}

void FolderTexturePack::unloadUI()
{
#ifdef _XBOX
	// Unload skin
	if(bUILoaded)
	{
		XuiFreeVisuals(L"TexturePack");
		XuiFreeVisuals(L"");
		CXuiSceneBase::GetInstance()->SetVisualPrefix(L"");
		CXuiSceneBase::GetInstance()->SkinChanged(CXuiSceneBase::GetInstance()->m_hObj);
	}
	AbstractTexturePack::unloadUI();
#endif
}