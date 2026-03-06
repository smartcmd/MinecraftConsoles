#include "stdafx.h"
#include "TexturePackRepository.h"
#include "DefaultTexturePack.h"
#include "FileTexturePack.h"
#include "FolderTexturePack.h"
#include "DLCTexturePack.h"
#include "Options.h"
#include "..\Minecraft.World\File.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "Minimap.h"

TexturePack *TexturePackRepository::DEFAULT_TEXTURE_PACK = NULL;

TexturePackRepository::TexturePackRepository(File workingDirectory, Minecraft *minecraft)
{
	if(!DEFAULT_TEXTURE_PACK) DEFAULT_TEXTURE_PACK = new DefaultTexturePack();

	// 4J - added
	usingWeb = false;
	selected = NULL;
	texturePacks = new vector<TexturePack *>;

    this->minecraft = minecraft;

	texturePacks->push_back(DEFAULT_TEXTURE_PACK);
	cacheById[DEFAULT_TEXTURE_PACK->getId()] = DEFAULT_TEXTURE_PACK;
    selected = DEFAULT_TEXTURE_PACK;

	DEFAULT_TEXTURE_PACK->loadColourTable();

	m_dummyTexturePack = NULL;
	m_dummyDLCTexturePack = NULL;
	lastSelected = NULL;

    updateList();
}

void TexturePackRepository::addDebugPacks()
{
#ifndef _CONTENT_PACKAGE
	//File *file = new File(L"DummyTexturePack"); // Path to the test texture pack
	//m_dummyTexturePack = new FolderTexturePack(FOLDER_TEST_TEXTURE_PACK_ID, L"FolderTestPack", file, DEFAULT_TEXTURE_PACK);
	//texturePacks->push_back(m_dummyTexturePack);
	//cacheById[m_dummyTexturePack->getId()] = m_dummyTexturePack;

#ifdef _XBOX
	File packedTestFile(L"GAME:\\DummyTexturePack\\TexturePack.pck");
	if(packedTestFile.exists())
	{
		DLCPack *pack = app.m_dlcManager.getPack(L"DLCTestPack");

		if( pack != NULL && pack->IsCorrupt() )
		{
			app.m_dlcManager.removePack(pack);
			pack = NULL;
		}

		if(pack == NULL)
		{
			wprintf(L"Pack \"%ls\" is not installed, so adding it\n", L"DLCTestPack");
			pack = new DLCPack(L"DLCTestPack",0xffffffff);
			DWORD dwFilesProcessed = 0;
			if( app.m_dlcManager.readDLCDataFile(dwFilesProcessed, "GAME:\\DummyTexturePack\\TexturePack.pck",pack))
			{
				// 4J Stu - Don't need to do this, as the readDLCDataFile now adds texture packs
				//m_dummyDLCTexturePack = addTexturePackFromDLC(pack, DLC_TEST_TEXTURE_PACK_ID); //new DLCTexturePack(0xFFFFFFFE, L"DLCTestPack", pack, DEFAULT_TEXTURE_PACK);
				app.m_dlcManager.addPack(pack);
			}
			else
			{
				delete pack;
			}
		}
	}

#endif // _XBOX
#endif // _CONTENT_PACKAGE
}

void TexturePackRepository::createWorkingDirecoryUnlessExists()
{
	// 4J Unused
#if 0
	if (!workDir.isDirectory()) {
		workDir.delete();
		workDir.mkdirs();
	}

	if (!multiplayerDir.isDirectory()) {
		multiplayerDir.delete();
		multiplayerDir.mkdirs();
	}
#endif
}

bool TexturePackRepository::selectSkin(TexturePack *skin)
{
    if (skin==selected) return false;

	lastSelected = selected;
    usingWeb = false;
    selected = skin;
    //minecraft->options->skin = skin->getName();
    //minecraft->options->save();
    return true;
}

void TexturePackRepository::selectWebSkin(const wstring &url)
{
	app.DebugPrintf("TexturePackRepository::selectWebSkin is not implemented\n");
#if 0
	String filename = url.substring(url.lastIndexOf("/") + 1);
	if (filename.contains("?")) filename = filename.substring(0, filename.indexOf("?"));
	if (!filename.endsWith(".zip")) return;
	File file = new File(multiplayerDir, filename);
	downloadWebSkin(url, file);
#endif
}

void TexturePackRepository::downloadWebSkin(const wstring &url, File file)
{
	app.DebugPrintf("TexturePackRepository::selectWebSkin is not implemented\n");
#if 0
	Map<String, String> headers = new HashMap<String, String>();
	final ProgressScreen listener = new ProgressScreen();
	headers.put("X-Minecraft-Username", minecraft.user.name);
	headers.put("X-Minecraft-Version", SharedConstants.VERSION_STRING);
	headers.put("X-Minecraft-Supported-Resolutions", "16");
	usingWeb = true;

	minecraft.setScreen(listener);

	HttpUtil.downloadTo(file, url, new HttpUtil.DownloadSuccessRunnable() {
		public void onDownloadSuccess(File file) {
			if (!usingWeb) return;

			selected = new FileTexturePack(getIdOrNull(file), file, DEFAULT_TEXTURE_PACK);
			minecraft.delayTextureReload();
		}
	}, headers, MAX_WEB_FILESIZE, listener);
#endif
}

bool TexturePackRepository::isUsingWebSkin()
{
	return usingWeb;
}

void TexturePackRepository::resetWebSkin()
{
	usingWeb = false;
	updateList();
	minecraft->delayTextureReload();
}

void TexturePackRepository::updateList()
{
	// 4J Stu - We don't ever want to completely refresh the lists, we keep them up-to-date as we go
#if 0
    vector<TexturePack *> *currentPacks = new vector<TexturePack *>;
	currentPacks->push_back(DEFAULT_TEXTURE_PACK);
	cacheById[DEFAULT_TEXTURE_PACK->getId()] = DEFAULT_TEXTURE_PACK;
#ifndef _CONTENT_PACKAGE
	currentPacks->push_back(m_dummyTexturePack);
	cacheById[m_dummyTexturePack->getId()] = m_dummyTexturePack;

	if(m_dummyDLCTexturePack != NULL)
	{
		currentPacks->push_back(m_dummyDLCTexturePack);
		cacheById[m_dummyDLCTexturePack->getId()] = m_dummyDLCTexturePack;
	}

    //selected = m_dummyTexturePack;
#endif
    selected = DEFAULT_TEXTURE_PACK;


	// 4J Unused
	for (File file : getWorkDirContents())
	{
		final String id = getIdOrNull(file);
		if (id == null) continue;

		TexturePack pack = cacheById.get(id);
		if (pack == null) {
			pack = file.isDirectory() ? new FolderTexturePack(id, file, DEFAULT_TEXTURE_PACK) : new FileTexturePack(id, file, DEFAULT_TEXTURE_PACK);
			cacheById.put(id, pack);
		}

		if (pack.getName().equals(minecraft.options.skin)) {
			selected = pack;
		}
		currentPacks.add(pack);
	}

	// 4J - was texturePacks.removeAll(currentPacks);
	for( auto it1 = currentPacks->begin(); it1 != currentPacks->end(); it1++ )
	{
		for( auto it2 = texturePacks->begin(); it2 != texturePacks->end(); it2++ )
		{
			if( *it1 == *it2 )
			{
				it2 = texturePacks->erase(it2);
			}
		}
	}

	for( auto it = texturePacks->begin(); it != texturePacks->end(); it++ )
	{
		TexturePack *pack = *it;
		pack->unload(minecraft->textures);
		cacheById.erase(pack->getId());
	}

	delete texturePacks;
    texturePacks = currentPacks;
#endif
}

wstring TexturePackRepository::getIdOrNull(File file)
{
	app.DebugPrintf("TexturePackRepository::getIdOrNull is not implemented\n");
#if 0
	if (file.isFile() && file.getName().toLowerCase().endsWith(".zip")) {
		return file.getName() + ":" + file.length() + ":" + file.lastModified();
	} else if (file.isDirectory() && new File(file, "pack.txt").exists()) {
		return file.getName() + ":folder:" + file.lastModified();
	}

	return NULL;
#endif
	return L"";
}

vector<File> TexturePackRepository::getWorkDirContents()
{
	app.DebugPrintf("TexturePackRepository::getWorkDirContents is not implemented\n");
#if 0
	if (workDir.exists() && workDir.isDirectory()) {
		return Arrays.asList(workDir.listFiles());
	}

	return Collections.emptyList();
#endif
	return vector<File>();
}

vector<TexturePack *> *TexturePackRepository::getAll()
{
	// 4J - note that original constucted a copy of texturePacks here
	return texturePacks;
}

TexturePack *TexturePackRepository::getSelected()
{
	if(selected->hasData())	return selected;
	else return DEFAULT_TEXTURE_PACK;
}

bool TexturePackRepository::shouldPromptForWebSkin()
{
	app.DebugPrintf("TexturePackRepository::shouldPromptForWebSkin is not implemented\n");
#if 0
	if (!minecraft.options.serverTextures) return false;
	ServerData data = minecraft.getCurrentServer();

	if (data == null) {
		return true;
	} else {
		return data.promptOnTextures();
	}
#endif
	return false;
}

bool TexturePackRepository::canUseWebSkin()
{
	app.DebugPrintf("TexturePackRepository::canUseWebSkin is not implemented\n");
#if 0
	if (!minecraft.options.serverTextures) return false;
	ServerData data = minecraft.getCurrentServer();

	if (data == null) {
		return false;
	} else {
		return data.allowTextures();
	}
#endif
	return false;
}

vector< pair<DWORD,wstring> > *TexturePackRepository::getTexturePackIdNames()
{
	vector< pair<DWORD,wstring> > *packList = new vector< pair<DWORD,wstring> >();

	for(auto& pack : *texturePacks)
	{
		packList->emplace_back(pack->getId(), pack->getName());
	}
	return packList;
}

bool TexturePackRepository::selectTexturePackById(DWORD id)
{
	bool bDidSelect = false;

	//4J-PB - add in a store of the texture pack required, so that join from invite games
	// (where they don't have the texture pack) can check this when the texture pack is installed
	app.SetRequiredTexturePackID(id);

    auto it = cacheById.find(id);
    if(it != cacheById.end())
	{
		TexturePack *newPack = it->second;
		if(newPack != selected)
		{
			selectSkin(newPack);

			if(newPack->hasData())
			{
				app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadTexturePack);
			}
			else
			{
				newPack->loadData();
			}
			//Minecraft *pMinecraft = Minecraft::GetInstance();
			//pMinecraft->textures->reloadAll();
		}
		else
		{
			app.DebugPrintf("TexturePack with id %d is already selected\n",id);
		}
		bDidSelect = true;
	}
	else
	{
		app.DebugPrintf("Failed to select texture pack %d as it is not in the list\n", id);
#ifndef _CONTENT_PACKAGE
		// TODO - 4J Stu: We should report this to the player in some way
		//__debugbreak();
#endif
		// Fail safely
		if( selectSkin( DEFAULT_TEXTURE_PACK ) )
		{
			app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_ReloadTexturePack);
		}
	}
	return bDidSelect;
}

TexturePack *TexturePackRepository::getTexturePackById(DWORD id)
{
    auto it = cacheById.find(id);
    if(it != cacheById.end())
	{
		return it->second;
	}

	return NULL;
}

TexturePack *TexturePackRepository::addTexturePackFromDLC(DLCPack *dlcPack, DWORD id)
{
	TexturePack *newPack = NULL;
	// 4J-PB - The City texture pack went out with a child id for the texture pack of 1 instead of zero
	// we need to mask off the child id here to deal with this
	DWORD dwParentID=id&0xFFFFFF; // child id is <<24 and Or'd with parent

	if(dlcPack != NULL)
	{
		newPack = new DLCTexturePack(dwParentID, dlcPack, DEFAULT_TEXTURE_PACK);
		texturePacks->push_back(newPack);
		cacheById[dwParentID] = newPack;

#ifndef _CONTENT_PACKAGE
		if(dlcPack->hasPurchasedFile(DLCManager::e_DLCType_TexturePack,L""))
		{
			wprintf(L"Added new FULL DLCTexturePack: %ls - id=%d\n", dlcPack->getName().c_str(),dwParentID );
		}
		else
		{
			wprintf(L"Added new TRIAL DLCTexturePack: %ls - id=%d\n", dlcPack->getName().c_str(),dwParentID );
		}
#endif
	}
	return newPack;
}

void TexturePackRepository::clearInvalidTexturePacks()
{
	for(auto& it : m_texturePacksToDelete)
	{
		delete it;
	}
}

void TexturePackRepository::removeTexturePackById(DWORD id)
{
    auto it = cacheById.find(id);
    if(it != cacheById.end())
	{
		TexturePack *oldPack = it->second;

        auto it2 = find(texturePacks->begin(), texturePacks->end(), oldPack);
        if(it2 != texturePacks->end())
		{
			texturePacks->erase(it2);
			if(lastSelected == oldPack)
			{
				lastSelected = NULL;
			}
		}
		m_texturePacksToDelete.push_back(oldPack);
	}
}

void TexturePackRepository::updateUI()
{
	if(lastSelected != NULL && lastSelected != selected)
	{
		lastSelected->unloadUI();
		selected->loadUI();
		Minimap::reloadColours();
		ui.StartReloadSkinThread();
		lastSelected = NULL;
	}
}

bool TexturePackRepository::needsUIUpdate()
{
	return lastSelected != NULL && lastSelected != selected;
}

unsigned int TexturePackRepository::getTexturePackCount()
{
	return texturePacks->size();
}

TexturePack *TexturePackRepository::getTexturePackByIndex(unsigned int index)
{
	TexturePack *pack = NULL;
	if(index < texturePacks->size())
	{
		pack = texturePacks->at(index);
	}
	return pack;
}

unsigned int TexturePackRepository::getTexturePackIndex(unsigned int id)
{
	int currentIndex = 0;
	for(auto& pack : *texturePacks)
	{
		if(pack->getId() == id)
			break;
		++currentIndex;
	}
	if(currentIndex >= texturePacks->size()) currentIndex = 0;
	return currentIndex;
}