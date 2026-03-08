#include "stdafx.h"

#include <assert.h>
#include "../XUI/XUI_HelpHowToPlay.h"
#include "../../../Minecraft.World/StringHelpers.h"

static SHowToPlayPageDef gs_aPageDefs[ eHowToPlay_NumPages ] =
{
	{ eHowToPlay_WhatsNew,			IDS_HOW_TO_PLAY_WHATSNEW,		eHowToPlay_ImageNone,				0,									0},			// eHowToPlay_WhatsNew
	{ eHowToPlay_TextBasics,		IDS_HOW_TO_PLAY_BASICS,			eHowToPlay_ImageNone,				0,									0},			// eHowToPlay_Basics
	{ eHowToPlay_TextMultiplayer,	IDS_HOW_TO_PLAY_MULTIPLAYER,	eHowToPlay_ImageNone,				0,									0},			// eHowToPlay_Multiplayer
	{ eHowToPlay_TextHUD,			IDS_HOW_TO_PLAY_HUD,			eHowToPlay_ImageHUD,				0,									0},			// eHowToPlay_HUD
	{ eHowToPlay_TextCreative,		IDS_HOW_TO_PLAY_CREATIVE,		eHowToPlay_ImageCreative,			eHowToPlay_LabelCreativeInventory,	1},			// eHowToPlay_Creative
	{ eHowToPlay_TextInventory,		IDS_HOW_TO_PLAY_INVENTORY,		eHowToPlay_ImageInventory,			eHowToPlay_LabelIInventory,			1},			// eHowToPlay_Inventory
	{ eHowToPlay_TextSmallChest,	IDS_HOW_TO_PLAY_CHEST,			eHowToPlay_ImageChest,				eHowToPlay_LabelSCInventory,		2},			// eHowToPlay_Chest
	{ eHowToPlay_TextLargeChest,	IDS_HOW_TO_PLAY_LARGECHEST,		eHowToPlay_ImageLargeChest,			eHowToPlay_LabelLCInventory,		2},			// eHowToPlay_LargeChest
	{ eHowToPlay_TextEnderchest,	IDS_HOW_TO_PLAY_ENDERCHEST,		eHowToPlay_ImageEnderChest,			0,									0},			// eHowToPlay_EnderChest
	{ eHowToPlay_TextCrafting,		IDS_HOW_TO_PLAY_CRAFTING,		eHowToPlay_ImageInventoryCrafting,	eHowToPlay_LabelCItem,				3},			// eHowToPlay_InventoryCrafting
	{ eHowToPlay_TextCraftTable,	IDS_HOW_TO_PLAY_CRAFT_TABLE,	eHowToPlay_ImageCraftingTable,		eHowToPlay_LabelCTItem,				3},			// eHowToPlay_CraftTable
	{ eHowToPlay_TextFurnace,		IDS_HOW_TO_PLAY_FURNACE,		eHowToPlay_ImageFurnace,			eHowToPlay_LabelFFuel,				4},			// eHowToPlay_Furnace
	{ eHowToPlay_TextDispenser,		IDS_HOW_TO_PLAY_DISPENSER,		eHowToPlay_ImageDispenser,			eHowToPlay_LabelDText,				2},			// eHowToPlay_Dispenser
	{ eHowToPlay_TextBrewing,		IDS_HOW_TO_PLAY_BREWING,		eHowToPlay_ImageBrewing,			eHowToPlay_LabelBBrew,				2},			// eHowToPlay_Brewing
	{ eHowToPlay_TextEnchantment,	IDS_HOW_TO_PLAY_ENCHANTMENT,	eHowToPlay_ImageEnchantment,		eHowToPlay_LabelEEnchant,			2},			// eHowToPlay_Enchantment
	{ eHowToPlay_TextAnvil,			IDS_HOW_TO_PLAY_ANVIL,			eHowToPlay_ImageAnvil,				eHowToPlay_LabelAnvil_Inventory,	3},			// eHowToPlay_Anvil
	{ eHowToPlay_TextFarmingAnimals,IDS_HOW_TO_PLAY_FARMANIMALS,	eHowToPlay_ImageFarmingAnimals,		0,									0},			// eHowToPlay_Farming
	{ eHowToPlay_TextBreeding,		IDS_HOW_TO_PLAY_BREEDANIMALS,	eHowToPlay_ImageBreeding,			0,									0},			// eHowToPlay_Breeding
	{ eHowToPlay_TextTrading,		IDS_HOW_TO_PLAY_TRADING,		eHowToPlay_ImageTrading,			eHowToPlay_LabelTrading_Inventory,	5},			// eHowToPlay_Trading
	{ eHowToPlay_TextNetherPortal,	IDS_HOW_TO_PLAY_NETHERPORTAL,	eHowToPlay_ImageNetherPortal,		0,									0},			// eHowToPlay_NetherPortal
	{ eHowToPlay_TextTheEnd,		IDS_HOW_TO_PLAY_THEEND,			eHowToPlay_ImageTheEnd,				0,									0},			// eHowToPlay_TheEnd
	{ eHowToPlay_TextSocialMedia,	IDS_HOW_TO_PLAY_SOCIALMEDIA,	eHowToPlay_ImageNone,				0,									0},			// eHowToPlay_SocialMedia
	{ eHowToPlay_TextBanList,		IDS_HOW_TO_PLAY_BANLIST,		eHowToPlay_ImageNone,				0,									0},			// eHowToPlay_BanList
	{ eHowToPlay_TextHostOptions,	IDS_HOW_TO_PLAY_HOSTOPTIONS,	eHowToPlay_ImageNone,				0,									0},			// eHowToPlay_HostOptions
};


//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_HowToPlay::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	// Extract pad and required page from init data. We just put the data into the pointer rather than using it as an address.
	size_t uiInitData = ( size_t )( pInitData->pvInitData );

	m_iPad = ( int )( ( short )( uiInitData & 0xFFFF ) );
	EHowToPlayPage eStartPage = ( EHowToPlayPage )( ( uiInitData >> 16 ) & 0xFFF );		// Ignores MSB which is set to 1!

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_HowToPlay, (ETelemetry_HowToPlay_SubMenuId)eStartPage);

	MapChildControls();

	wstring wsTemp, inventoryString = app.GetString(IDS_INVENTORY);
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelCTItem],app.GetString(IDS_ITEM_HATCHET_WOOD));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelCTGroup],app.GetString(IDS_GROUPNAME_TOOLS));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelCTInventory3x3],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelCItem],app.GetString(IDS_TILE_WORKBENCH));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelCGroup],app.GetString(IDS_GROUPNAME_STRUCTURES));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelCInventory2x2],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelFFuel],app.GetString(IDS_FUEL));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelFInventory],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelFIngredient],app.GetString(IDS_INGREDIENT));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelFChest],app.GetString(IDS_FURNACE));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelLCInventory],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelCreativeInventory],app.GetString(IDS_GROUPNAME_BUILDING_BLOCKS));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelLCChest],app.GetString(IDS_CHEST));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelSCInventory],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelSCChest],app.GetString(IDS_CHEST));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelIInventory],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelDInventory],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelDText],app.GetString(IDS_DISPENSER));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelEEnchant],app.GetString(IDS_ENCHANT));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelEInventory],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelBBrew],app.GetString(IDS_BREWING_STAND));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelBInventory],inventoryString.c_str());
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelAnvil_Inventory], inventoryString.c_str());
	
	wsTemp = app.GetString(IDS_REPAIR_COST);
	wsTemp.replace( wsTemp.find(L"%d"), 2, wstring(L"8") );
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelAnvil_Cost], wsTemp.c_str());

	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelAnvil_ARepairAndName], app.GetString(IDS_REPAIR_AND_NAME));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelTrading_Inventory], inventoryString.c_str());
	//XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelTrading_Offer2], app.GetString(IDS_ITEM_HATCHET_DIAMOND));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelTrading_Offer1], app.GetString(IDS_ITEM_EMERALD));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelTrading_NeededForTrade], app.GetString(IDS_REQUIRED_ITEMS_FOR_TRADE));

	wsTemp = app.GetString(IDS_VILLAGER_OFFERS_ITEM);
	wsTemp = replaceAll(wsTemp,L"{*VILLAGER_TYPE*}",app.GetString(IDS_VILLAGER_PRIEST));
	wsTemp.replace(wsTemp.find(L"%s"),2, app.GetString(IDS_TILE_LIGHT_GEM));
	XuiControlSetText(m_aLabelControls[ eHowToPlay_LabelTrading_VillagerOffers], wsTemp.c_str());

	if(app.GetLocalPlayerCount()>1)
	{
		app.AdjustSplitscreenScene(m_hObj,&m_OriginalPosition,m_iPad);
	}


	StartPage( eStartPage );

	return S_OK;
}

HRESULT CScene_HowToPlay::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);
	
	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:
	{
		app.NavigateBack(pInputData->UserIndex);
		rfHandled = TRUE;
	}
	break;
	case VK_PAD_A:
	{
		// Next page
		int iNextPage = ( int )( m_eCurrPage ) + 1;
		if ( iNextPage != eHowToPlay_NumPages )
		{
			StartPage( ( EHowToPlayPage )( iNextPage ) );
			CXuiSceneBase::PlayUISFX(eSFX_Press);
		}
		rfHandled = TRUE;
	}
	break;
	case VK_PAD_X:
	{
		// Next page
		int iPrevPage = ( int )( m_eCurrPage ) - 1;
		if ( iPrevPage >= 0 )
		{
			StartPage( ( EHowToPlayPage )( iPrevPage ) );
			CXuiSceneBase::PlayUISFX(eSFX_Press);
		}
		rfHandled = TRUE;
	}
	break;
	}
	return S_OK;
}


void CScene_HowToPlay::StartPage( EHowToPlayPage ePage )
{
	int iBaseSceneUser;
	// if we're not in the game, we need to use basescene 0 
	if(Minecraft::GetInstance()->level==NULL)
	{
		iBaseSceneUser=DEFAULT_XUI_MENU_USER;
	}
	else
	{
		iBaseSceneUser=m_iPad;
	}
	m_eCurrPage = ePage;

	// Turn off everything.
	for ( int i = 0; i < eHowToPlay_NumTexts; ++i )
	{
		m_aTextControls[ i ].SetShow( FALSE );
	}
	for ( int i = 0; i < eHowToPlay_NumImages; ++i )
	{
		m_aImageControls[ i ].SetShow( FALSE );
	}
	for ( int i = 0; i < eHowToPlay_NumLabels; ++i )
	{
		m_aLabelControls[ i ].SetShow( FALSE );
	}

	// Turn on just what we need for this screen.
	SHowToPlayPageDef* pDef = &( gs_aPageDefs[ m_eCurrPage ] );

	if ( pDef->m_iTextControlIndex != eHowToPlay_TextNone )
	{
		// Replace button identifiers in the text with actual button images.
		wstring replacedText = app.FormatHTMLString(m_iPad, app.GetString( pDef->m_iTextStringID ));

		// 4J-PB - replace the title with the platform specific title, and the platform name
		replacedText = replaceAll(replacedText,L"{*PLATFORM_NAME*}",app.GetString(IDS_PLATFORM_NAME));
		replacedText = replaceAll(replacedText,L"{*BACK_BUTTON*}",app.GetString(IDS_BACK_BUTTON));
		replacedText = replaceAll(replacedText,L"{*DISABLES_ACHIEVEMENTS*}",app.GetString(IDS_HOST_OPTION_DISABLES_ACHIEVEMENTS));

		// Set the text colour
		wstring finalText(replacedText.c_str() );
		wchar_t startTags[64];
		swprintf(startTags,64,L"<font color=\"#%08x\">",app.GetHTMLColour(eHTMLColor_White));
		finalText = startTags + finalText;

		// Set the text in the xui scene.
		m_aTextControls[ pDef->m_iTextControlIndex ].SetText( finalText.c_str() );

		// Make it visible.
		m_aTextControls[ pDef->m_iTextControlIndex ].SetShow( TRUE );

		XuiElementSetUserFocus(m_aTextControls[ pDef->m_iTextControlIndex ].m_hObj, m_iPad);
	}

	if(pDef->m_iLabelCount!=0)
	{
		for(int i=pDef->m_iLabelStartIndex;i<(pDef->m_iLabelStartIndex+pDef->m_iLabelCount);i++)
		{
			m_aLabelControls[i].SetShow( TRUE );
		}
	}

	if ( pDef->m_iImageControlIndex != eHowToPlay_ImageNone )
	{
		m_aImageControls[ pDef->m_iImageControlIndex ].SetShow( TRUE );
	}

	// Tool tips.
	int iPage = ( int )( m_eCurrPage );
	if ( iPage == 0 )
	{
		// No previous page.
		ui.SetTooltips( iBaseSceneUser, IDS_HOW_TO_PLAY_NEXT, IDS_TOOLTIPS_BACK, -1 );
	}
	else if  ( ( iPage + 1 ) == eHowToPlay_NumPages )
	{
		// No next page.
		ui.SetTooltips( iBaseSceneUser, -1, IDS_TOOLTIPS_BACK, IDS_HOW_TO_PLAY_PREV );
	}
	else
	{
		ui.SetTooltips( iBaseSceneUser, IDS_HOW_TO_PLAY_NEXT, IDS_TOOLTIPS_BACK, IDS_HOW_TO_PLAY_PREV );
	}

	TelemetryManager->RecordMenuShown(m_iPad, eUIScene_HowToPlay, (ETelemetry_HowToPlay_SubMenuId)ePage);
}

HRESULT CScene_HowToPlay::OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)
{
	bHandled=true;
	return app.AdjustSplitscreenScene_PlayerChanged(m_hObj,&m_OriginalPosition,m_iPad,bJoining);
}
