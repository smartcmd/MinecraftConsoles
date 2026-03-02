#include "stdafx.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiplayerLocalPlayer.h"
#include "..\..\MultiplayerLevel.h"
#include "..\..\GameMode.h"
#include "..\..\SurvivalMode.h"
#include "..\..\CreativeMode.h"
#include "ClientConnection.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\ArrayWithLength.h"
#include "..\..\..\Minecraft.World\com.mojang.nbt.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.saveddata.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.chunk.storage.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\..\..\Minecraft.World\InputOutputStream.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFileIO.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.enchantment.h"
#include "XUI_DebugOverlay.h"
#include "..\..\..\Minecraft.Client\GameRenderer.h"
#include "..\..\MinecraftServer.h"
#include "..\..\Common\Tutorial\Tutorial.h"
#include "..\..\..\Minecraft.World\net.minecraft.commands.common.h"
#include "..\..\..\Minecraft.World\ConsoleSaveFileOriginal.h"

#ifdef _DEBUG_MENUS_ENABLED	
HRESULT CScene_DebugOverlay::OnInit( XUIMessageInit *pInitData, BOOL &bHandled )
{
	MapChildControls();

	m_items.InsertItems( 0, 512 );

	for(unsigned int i = 0; i < Item::items.length; ++i)
	{
		if(Item::items[i] != NULL)
		{
			//m_items.InsertItems(m_items.GetItemCount(),1);
			m_itemIds.push_back(i);
			m_items.SetText( m_itemIds.size() - 1, app.GetString( Item::items[i]->getDescriptionId() ) );
		}
	}

	m_enchantments.InsertItems( 0, Enchantment::validEnchantments.size() );
	for(unsigned int i = 0; i < Enchantment::validEnchantments.size(); ++i )
	{
		Enchantment *ench = Enchantment::validEnchantments.at(i);

		m_enchantmentIds.push_back(ench->id);
		m_enchantments.SetText( i, app.GetString( ench->getDescriptionId() ) );
	}
	
	m_mobs.InsertItems( 0, 21 );

	m_mobs.SetText( m_mobFactories.size(), L"Chicken" );
	m_mobFactories.push_back(eTYPE_CHICKEN);
	m_mobs.SetText( m_mobFactories.size(), L"Cow" );
	m_mobFactories.push_back(eTYPE_COW);
	m_mobs.SetText( m_mobFactories.size(), L"Pig" );
	m_mobFactories.push_back(eTYPE_PIG);
	m_mobs.SetText( m_mobFactories.size(), L"Sheep" );
	m_mobFactories.push_back(eTYPE_SHEEP);
	m_mobs.SetText( m_mobFactories.size(), L"Squid" );
	m_mobFactories.push_back(eTYPE_SQUID);
	m_mobs.SetText( m_mobFactories.size(), L"Wolf" );
	m_mobFactories.push_back(eTYPE_WOLF);
	m_mobs.SetText( m_mobFactories.size(), L"Creeper" );
	m_mobFactories.push_back(eTYPE_CREEPER);
	m_mobs.SetText( m_mobFactories.size(), L"Ghast" );
	m_mobFactories.push_back(eTYPE_GHAST);
	m_mobs.SetText( m_mobFactories.size(), L"Pig Zombie" );
	m_mobFactories.push_back(eTYPE_PIGZOMBIE);
	m_mobs.SetText( m_mobFactories.size(), L"Skeleton" );
	m_mobFactories.push_back(eTYPE_SKELETON);
	m_mobs.SetText( m_mobFactories.size(), L"Slime" );
	m_mobFactories.push_back(eTYPE_SLIME);
	m_mobs.SetText( m_mobFactories.size(), L"Spider" );
	m_mobFactories.push_back(eTYPE_SPIDER);
	m_mobs.SetText( m_mobFactories.size(), L"Zombie" );
	m_mobFactories.push_back(eTYPE_ZOMBIE);
	m_mobs.SetText( m_mobFactories.size(), L"Enderman" );
	m_mobFactories.push_back(eTYPE_ENDERMAN);
	m_mobs.SetText( m_mobFactories.size(), L"Silverfish" );
	m_mobFactories.push_back(eTYPE_SILVERFISH);
	m_mobs.SetText( m_mobFactories.size(), L"Cave Spider" );
	m_mobFactories.push_back(eTYPE_CAVESPIDER);
	m_mobs.SetText( m_mobFactories.size(), L"Mooshroom" );
	m_mobFactories.push_back(eTYPE_MUSHROOMCOW);
	m_mobs.SetText( m_mobFactories.size(), L"Snow Golem" );
	m_mobFactories.push_back(eTYPE_SNOWMAN);
	m_mobs.SetText( m_mobFactories.size(), L"Ender Dragon" );
	m_mobFactories.push_back(eTYPE_ENDERDRAGON);
	m_mobs.SetText( m_mobFactories.size(), L"Blaze" );
	m_mobFactories.push_back(eTYPE_BLAZE);
	m_mobs.SetText( m_mobFactories.size(), L"Magma Cube" );
	m_mobFactories.push_back(eTYPE_LAVASLIME);
	

	Minecraft *pMinecraft = Minecraft::GetInstance();
	m_setTime.SetValue( pMinecraft->level->getLevelData()->getTime() % 24000 );
	m_setFov.SetValue( (int)pMinecraft->gameRenderer->GetFovVal());

	XuiSetTimer(m_hObj,0,DEBUG_OVERLAY_UPDATE_TIME_PERIOD);

	bHandled = TRUE;
	return S_OK;
}

// Handler for the XM_NOTIFY message
HRESULT CScene_DebugOverlay::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

    unsigned int nIndex;

	Minecraft *pMinecraft = Minecraft::GetInstance();

    if ( hObjPressed == m_items )
    {
        nIndex = m_items.GetCurSel();
		if(nIndex<m_itemIds.size())
		{
			int id = m_itemIds[nIndex];
			//app.SetXuiServerAction(pNotifyPressData->UserIndex, eXuiServerAction_DropItem, (void *)id);
			ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
			conn->send( GiveItemCommand::preparePacket(dynamic_pointer_cast<Player>(Minecraft::GetInstance()->localplayers[ProfileManager.GetPrimaryPad()]), id) );
		}
    }
	else if ( hObjPressed == m_mobs )
    {
        nIndex = m_mobs.GetCurSel();
		if(nIndex<m_mobFactories.size())
		{			
			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_SpawnMob,(void *)m_mobFactories[nIndex]);
		}
    }
	else if ( hObjPressed == m_enchantments )
    {
        nIndex = m_enchantments.GetCurSel();
		ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
		conn->send( EnchantItemCommand::preparePacket(dynamic_pointer_cast<Player>(Minecraft::GetInstance()->localplayers[ProfileManager.GetPrimaryPad()]), m_enchantmentIds[nIndex]) );
    }
	/*else if( hObjPressed == m_saveToDisc ) // 4J-JEV: Doesn't look like we use this debug option anymore.
	{
#ifndef _CONTENT_PACKAGE
		pMinecraft->level->save(true, NULL);

		int radius;
		m_chunkRadius.GetValue(&radius);
		if( radius > 0 )
		{
			SaveLimitedFile(radius);
		}
		else
		{
			pMinecraft->level->getLevelStorage()->getSaveFile()->DebugFlushToFile();
		}
#endif
	}*/
	else if( hObjPressed == m_createSchematic )
	{
#ifndef _CONTENT_PACKAGE
		// load from the .xzp file
		const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

		HXUIOBJ hScene;
		HRESULT hr;
		//const WCHAR XZP_SEPARATOR  = L'#';
		const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
		WCHAR szResourceLocator[ LOCATOR_SIZE ];

		swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/");
		hr = XuiSceneCreate(szResourceLocator,app.GetSceneName(eUIScene_DebugCreateSchematic,false, false), NULL, &hScene);
		this->NavigateForward(hScene);
		//app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DebugCreateSchematic);
#endif
	}
	else if ( hObjPressed == m_setCamera )
	{
#ifndef _CONTENT_PACKAGE
		// load from the .xzp file
		const ULONG_PTR c_ModuleHandle = (ULONG_PTR)GetModuleHandle(NULL);

		HXUIOBJ hScene;
		HRESULT hr;
		//const WCHAR XZP_SEPARATOR  = L'#';
		const DWORD LOCATOR_SIZE = 256; // Use this to allocate space to hold a ResourceLocator string 
		WCHAR szResourceLocator[ LOCATOR_SIZE ];

		swprintf(szResourceLocator, LOCATOR_SIZE, L"section://%X,%ls#%ls",c_ModuleHandle,L"media", L"media/");
		hr = XuiSceneCreate(szResourceLocator,app.GetSceneName(eUIScene_DebugSetCamera, false, false), NULL, &hScene);
		this->NavigateForward(hScene);
		//app.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DebugCreateSchematic);
#endif
	}
	else if( hObjPressed == m_toggleRain )
	{
		//app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_ToggleRain);
		ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
		conn->send( ToggleDownfallCommand::preparePacket() );
	}
	else if( hObjPressed == m_toggleThunder )
	{
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_ToggleThunder);
	}
	else if( hObjPressed == m_resetTutorial )
	{
		Tutorial::debugResetPlayerSavedProgress( ProfileManager.GetPrimaryPad() );
	}
	else if( hObjPressed == m_setDay )
	{
		ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
		conn->send( TimeCommand::preparePacket(false) );
	}
	else if( hObjPressed == m_setNight )
	{
		ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
		conn->send( TimeCommand::preparePacket(true) );
	}

    rfHandled = TRUE;
    return S_OK;
}

HRESULT CScene_DebugOverlay::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);


	switch(pInputData->dwKeyCode)
	{

	case VK_PAD_B:
	case VK_PAD_START:
	case VK_PAD_BACK:

		// kill the crafting xui
		app.EnableDebugOverlay(false,pInputData->UserIndex);

		rfHandled = TRUE;

		break;

	}

	return S_OK;
}

HRESULT CScene_DebugOverlay::OnNotifyValueChanged( HXUIOBJ hObjSource, XUINotifyValueChanged *pNotifyValueChangedData, BOOL &bHandled)
{
	if( hObjSource == m_setTime )
	{
		Minecraft *pMinecraft = Minecraft::GetInstance();
		
		// Need to set the time on both levels to stop the flickering as the local level
		// tries to predict the time
		// Only works if we are on the host machine, but shouldn't break if not
		MinecraftServer::SetTime(pNotifyValueChangedData->nValue);
		pMinecraft->level->getLevelData()->setTime(pNotifyValueChangedData->nValue);
	}
	if( hObjSource == m_setFov )
	{
		Minecraft *pMinecraft = Minecraft::GetInstance();
		pMinecraft->gameRenderer->SetFovVal((float)pNotifyValueChangedData->nValue);
	}
	return S_OK;
}

HRESULT CScene_DebugOverlay::OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled )
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->level != NULL)
	{
		m_setTime.SetValue( pMinecraft->level->getLevelData()->getTime() % 24000 );
		m_setFov.SetValue( (int)pMinecraft->gameRenderer->GetFovVal());
	}
	return S_OK;
}

void CScene_DebugOverlay::SetSpawnToPlayerPos()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	pMinecraft->level->getLevelData()->setXSpawn((int)pMinecraft->player->x);
	pMinecraft->level->getLevelData()->setYSpawn((int)pMinecraft->player->y);
	pMinecraft->level->getLevelData()->setZSpawn((int)pMinecraft->player->z);
}

#ifndef _CONTENT_PACKAGE
void CScene_DebugOverlay::SaveLimitedFile(int chunkRadius)
{
	unordered_map<File, RegionFile *, FileKeyHash, FileKeyEq> newFileCache;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	ConsoleSaveFile *currentSave = pMinecraft->level->getLevelStorage()->getSaveFile();

	// With a size of 0 but a value in the data pointer we should create a new save
	ConsoleSaveFileOriginal newSave( currentSave->getFilename(), NULL, 0, true );

	// TODO Make this only happen for the new save
	//SetSpawnToPlayerPos();
	FileEntry *origFileEntry = currentSave->createFile( wstring( L"level.dat" ) );
	byteArray levelData( origFileEntry->getFileSize() );
	DWORD bytesRead;
	currentSave->setFilePointer(origFileEntry,0,NULL,FILE_BEGIN);
	currentSave->readFile(
         origFileEntry,
         levelData.data, // data buffer
         origFileEntry->getFileSize(), // number of bytes to read
         &bytesRead // number of bytes read
		 );

	FileEntry *newFileEntry = newSave.createFile( wstring( L"level.dat" ) );
	DWORD bytesWritten;
	newSave.writeFile(	newFileEntry,
					levelData.data, // data buffer
					origFileEntry->getFileSize(), // number of bytes to write
					&bytesWritten // number of bytes written
					);

	int playerChunkX = pMinecraft->player->xChunk;
	int playerChunkZ = pMinecraft->player->zChunk;

	for(int xPos = playerChunkX - chunkRadius; xPos < playerChunkX + chunkRadius; ++xPos)
	{
		for(int zPos = playerChunkZ - chunkRadius; zPos < playerChunkZ + chunkRadius; ++zPos)
		{
			CompoundTag *chunkData=NULL;

			DataInputStream *is = RegionFileCache::getChunkDataInputStream(currentSave, L"", xPos, zPos);
			if (is != NULL)
			{
				chunkData = NbtIo::read((DataInput *)is);
				is->deleteChildStream();
				delete is;
			}
			app.DebugPrintf("Processing chunk (%d, %d)\n", xPos, zPos);
			DataOutputStream *os = getChunkDataOutputStream(newFileCache, &newSave, L"", xPos, zPos);
			if(os != NULL)
			{
				NbtIo::write(chunkData, os);
				os->close();

				// 4J Stu - getChunkDataOutputStream makes a new DataOutputStream that points to a new ChunkBuffer( ByteArrayOutputStream )
				// We should clean these up when we are done
				os->deleteChildStream();
				delete os;
			}
			if(chunkData != NULL)
			{
				delete chunkData;
			}
		}
	}

	newSave.DebugFlushToFile();
}
#endif

RegionFile *CScene_DebugOverlay::getRegionFile(unordered_map<File, RegionFile *, FileKeyHash, FileKeyEq> &newFileCache, ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ)		// 4J - TODO was synchronized
{
	File file( prefix + wstring(L"r.") + _toString(chunkX>>5) + L"." + _toString(chunkZ>>5) + L".mcr" );

	RegionFile *ref = NULL;
	AUTO_VAR(it, newFileCache.find(file));
	if( it != newFileCache.end() )
		ref = it->second;

	// 4J Jev, put back in.
    if (ref != NULL)
	{
		return ref;
    }

	RegionFile *reg = new RegionFile(saveFile, &file);
    newFileCache[file] = reg;	   // 4J - this was originally a softReferenc
    return reg;
}

DataOutputStream *CScene_DebugOverlay::getChunkDataOutputStream(unordered_map<File, RegionFile *, FileKeyHash, FileKeyEq> &newFileCache, ConsoleSaveFile *saveFile, const wstring &prefix, int chunkX, int chunkZ)
{
	RegionFile *r = getRegionFile(newFileCache, saveFile, prefix, chunkX, chunkZ);
    return r->getChunkDataOutputStream(chunkX & 31, chunkZ & 31);
}
#endif