#include "stdafx.h"

#ifdef _DEBUG_MENUS_ENABLED
#include "UI.h"
#include "UIScene_DebugOverlay.h"
#include "..\..\Minecraft.h"
#include "..\..\MinecraftServer.h"
#include "..\..\GameRenderer.h"
#include "..\..\MultiPlayerLevel.h"
#include "ClientConnection.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.storage.h"
#include "..\..\..\Minecraft.World\net.minecraft.commands.common.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.item.enchantment.h"

UIScene_DebugOverlay::UIScene_DebugOverlay(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	Minecraft *pMinecraft = Minecraft::GetInstance();
	WCHAR TempString[256];
	swprintf( (WCHAR *)TempString, 256, L"Set fov (%d)", (int)pMinecraft->gameRenderer->GetFovVal());
	m_sliderFov.init(TempString,eControl_FOV,0,100,(int)pMinecraft->gameRenderer->GetFovVal());

	float currentTime = pMinecraft->level->getLevelData()->getTime() % 24000;
	swprintf( (WCHAR *)TempString, 256, L"Set time (unsafe) (%d)", (int)currentTime);
	m_sliderTime.init(TempString,eControl_Time,0,240,currentTime/100);

	m_buttonRain.init(L"Toggle Rain",eControl_Rain);
	m_buttonThunder.init(L"Toggle Thunder",eControl_Thunder);
	m_buttonSchematic.init(L"Create Schematic",eControl_Schematic);
	m_buttonResetTutorial.init(L"Reset profile tutorial progress",eControl_ResetTutorial);
	m_buttonSetCamera.init(L"Set camera",eControl_SetCamera);
	m_buttonSetDay.init(L"Set Day", eControl_SetDay);
	m_buttonSetNight.init(L"Set Night", eControl_SetNight);

	m_buttonListItems.init(eControl_Items);

	int listId = 0;
	for(unsigned int i = 0; i < Item::items.length; ++i)
	{
		if(Item::items[i] != NULL)
		{
			m_itemIds.push_back(i);
			m_buttonListItems.addItem(app.GetString(Item::items[i]->getDescriptionId()), listId);
			++listId;
		}
	}

	m_buttonListEnchantments.init(eControl_Enchantments);

	for(unsigned int i = 0; i < Enchantment::validEnchantments.size(); ++i )
	{
		Enchantment *ench = Enchantment::validEnchantments.at(i);

		for(unsigned int level = ench->getMinLevel(); level <= ench->getMaxLevel(); ++level)
		{
			m_enchantmentIdAndLevels.push_back(pair<int,int>(ench->id,level));
			m_buttonListEnchantments.addItem(app.GetString( ench->getDescriptionId() ) + _toString<int>(level) );
		}
	}

	m_buttonListMobs.init(eControl_Mobs);
	m_buttonListMobs.addItem( L"Chicken" );
	m_mobFactories.push_back(eTYPE_CHICKEN);
	m_buttonListMobs.addItem( L"Cow" );
	m_mobFactories.push_back(eTYPE_COW);
	m_buttonListMobs.addItem( L"Pig" );
	m_mobFactories.push_back(eTYPE_PIG);
	m_buttonListMobs.addItem( L"Sheep" );
	m_mobFactories.push_back(eTYPE_SHEEP);
	m_buttonListMobs.addItem( L"Squid" );
	m_mobFactories.push_back(eTYPE_SQUID);
	m_buttonListMobs.addItem( L"Wolf" );
	m_mobFactories.push_back(eTYPE_WOLF);
	m_buttonListMobs.addItem( L"Creeper" );
	m_mobFactories.push_back(eTYPE_CREEPER);
	m_buttonListMobs.addItem( L"Ghast" );
	m_mobFactories.push_back(eTYPE_GHAST);
	m_buttonListMobs.addItem( L"Pig Zombie" );
	m_mobFactories.push_back(eTYPE_PIGZOMBIE);
	m_buttonListMobs.addItem( L"Skeleton" );
	m_mobFactories.push_back(eTYPE_SKELETON);
	m_buttonListMobs.addItem( L"Slime" );
	m_mobFactories.push_back(eTYPE_SLIME);
	m_buttonListMobs.addItem( L"Spider" );
	m_mobFactories.push_back(eTYPE_SPIDER);
	m_buttonListMobs.addItem( L"Zombie" );
	m_mobFactories.push_back(eTYPE_ZOMBIE);
	m_buttonListMobs.addItem( L"Enderman" );
	m_mobFactories.push_back(eTYPE_ENDERMAN);
	m_buttonListMobs.addItem( L"Silverfish" );
	m_mobFactories.push_back(eTYPE_SILVERFISH);
	m_buttonListMobs.addItem( L"Cave Spider" );
	m_mobFactories.push_back(eTYPE_CAVESPIDER);
	m_buttonListMobs.addItem( L"Mooshroom" );
	m_mobFactories.push_back(eTYPE_MUSHROOMCOW);
	m_buttonListMobs.addItem( L"Snow Golem" );
	m_mobFactories.push_back(eTYPE_SNOWMAN);
	m_buttonListMobs.addItem( L"Ender Dragon" );
	m_mobFactories.push_back(eTYPE_ENDERDRAGON);
	m_buttonListMobs.addItem( L"Blaze" );
	m_mobFactories.push_back(eTYPE_BLAZE);
	m_buttonListMobs.addItem( L"Magma Cube" );
	m_mobFactories.push_back(eTYPE_LAVASLIME);
}

wstring UIScene_DebugOverlay::getMoviePath()
{
	return L"DebugMenu";
}

void UIScene_DebugOverlay::customDraw(IggyCustomDrawCallbackRegion *region)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localplayers[m_iPad] == NULL || pMinecraft->localgameModes[m_iPad] == NULL) return;

	int itemId = -1;
	swscanf((wchar_t*)region->name,L"item_%d",&itemId);
	if (itemId == -1 || itemId > Item::ITEM_NUM_COUNT || Item::items[itemId] == NULL)
	{
		app.DebugPrintf("This is not the control we are looking for\n");
	}
	else
	{
		shared_ptr<ItemInstance> item = shared_ptr<ItemInstance>( new ItemInstance(itemId,1,0) );
		if(item != NULL) customDrawSlotControl(region,m_iPad,item,1.0f,false,false);
	}
}

void UIScene_DebugOverlay::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
		}
		break;
	case ACTION_MENU_OK:
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
	case ACTION_MENU_LEFT:
	case ACTION_MENU_RIGHT:
		if(pressed)
		{
			sendInputToMovie(key, repeat, pressed, released);
		}
		break;
	}
}

void UIScene_DebugOverlay::handlePress(F64 controlId, F64 childId)
{
	switch((int)controlId)
	{
	case eControl_Items:
		{
			app.DebugPrintf("UIScene_DebugOverlay::handlePress for itemsList: %f\n", childId);
			int id = childId;
			//app.SetXuiServerAction(m_iPad, eXuiServerAction_DropItem, (void *)m_itemIds[id]);
			ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
			conn->send( GiveItemCommand::preparePacket(dynamic_pointer_cast<Player>(Minecraft::GetInstance()->localplayers[ProfileManager.GetPrimaryPad()]), m_itemIds[id]) );
		}
		break;
	case eControl_Mobs:
		{
			int id = childId;
			if(id<m_mobFactories.size())
			{			
				app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_SpawnMob,(void *)m_mobFactories[id]);
			}
		}
		break;
	case eControl_Enchantments:
		{
			int id = childId;
			ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
			conn->send( EnchantItemCommand::preparePacket(dynamic_pointer_cast<Player>(Minecraft::GetInstance()->localplayers[ProfileManager.GetPrimaryPad()]), m_enchantmentIdAndLevels[id].first, m_enchantmentIdAndLevels[id].second) );
		}
		break;
	case eControl_Schematic:
		{
#ifndef _CONTENT_PACKAGE
			ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DebugCreateSchematic,NULL,eUILayer_Debug);
#endif
		}
		break;
	case eControl_SetCamera:
		{
#ifndef _CONTENT_PACKAGE
			ui.NavigateToScene(ProfileManager.GetPrimaryPad(),eUIScene_DebugSetCamera,NULL,eUILayer_Debug);
#endif
		}
		break;
	case eControl_Rain:
		{
			//app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_ToggleRain);
			ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
			conn->send( ToggleDownfallCommand::preparePacket() );
		}
		break;
	case eControl_Thunder:
		app.SetXuiServerAction(ProfileManager.GetPrimaryPad(),eXuiServerAction_ToggleThunder);
		break;
	case eControl_ResetTutorial:
		Tutorial::debugResetPlayerSavedProgress( ProfileManager.GetPrimaryPad() );
		break;
	case eControl_SetDay:
		{
			ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
			conn->send( TimeCommand::preparePacket(false) );
		}
		break;
	case eControl_SetNight:
		{
			ClientConnection *conn = Minecraft::GetInstance()->getConnection(ProfileManager.GetPrimaryPad());
			conn->send( TimeCommand::preparePacket(true) );
		}
		break;
	};
}

void UIScene_DebugOverlay::handleSliderMove(F64 sliderId, F64 currentValue)
{
	switch((int)sliderId)
	{
	case eControl_Time:
		{
			Minecraft *pMinecraft = Minecraft::GetInstance();

			// Need to set the time on both levels to stop the flickering as the local level
			// tries to predict the time
			// Only works if we are on the host machine, but shouldn't break if not
			MinecraftServer::SetTime(currentValue * 100);
			pMinecraft->level->getLevelData()->setTime(currentValue * 100);

			WCHAR TempString[256];
			float currentTime = currentValue * 100;
			swprintf( (WCHAR *)TempString, 256, L"Set time (unsafe) (%d)", (int)currentTime);
			m_sliderTime.setLabel(TempString);
		}
		break;
	case eControl_FOV:
		{
			Minecraft *pMinecraft = Minecraft::GetInstance();
			pMinecraft->gameRenderer->SetFovVal((float)currentValue);

			WCHAR TempString[256];
			swprintf( (WCHAR *)TempString, 256, L"Set fov (%d)", (int)currentValue);
			m_sliderFov.setLabel(TempString);
		}
		break;
	};
}
#endif
