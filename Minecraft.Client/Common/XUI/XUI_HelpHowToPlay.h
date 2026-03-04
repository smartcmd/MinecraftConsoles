#pragma once

#include "../media/xuiscene_howtoplay.h"
#include "XUI_CustomMessages.h"

enum EHowToPlayTextControls
{
	eHowToPlay_TextNone = -1,
	eHowToPlay_TextWhatsNew = 0,
	eHowToPlay_TextBasics,
	eHowToPlay_TextMultiplayer,
	eHowToPlay_TextHUD,
	eHowToPlay_TextCreative,
	eHowToPlay_TextInventory,
	eHowToPlay_TextSmallChest,
	eHowToPlay_TextLargeChest,
	eHowToPlay_TextEnderchest,
	eHowToPlay_TextCrafting,
	eHowToPlay_TextCraftTable,
	eHowToPlay_TextFurnace,
	eHowToPlay_TextDispenser,
	eHowToPlay_TextBrewing,
	eHowToPlay_TextEnchantment,
	eHowToPlay_TextAnvil,
	eHowToPlay_TextFarmingAnimals,
	eHowToPlay_TextBreeding,
	eHowToPlay_TextTrading,
	eHowToPlay_TextNetherPortal,
	eHowToPlay_TextTheEnd,
	eHowToPlay_TextSocialMedia,
	eHowToPlay_TextBanList,
	eHowToPlay_TextHostOptions,
	eHowToPlay_NumTexts
};

enum EHowToPlayImageControls
{
	eHowToPlay_ImageNone = -1,
	eHowToPlay_ImageHUD = 0,
	eHowToPlay_ImageCreative,
	eHowToPlay_ImageInventory,
	eHowToPlay_ImageChest,
	eHowToPlay_ImageLargeChest,
	eHowToPlay_ImageEnderChest,
	eHowToPlay_ImageInventoryCrafting,
	eHowToPlay_ImageCraftingTable,
	eHowToPlay_ImageFurnace,
	eHowToPlay_ImageDispenser,
	eHowToPlay_ImageBrewing,
	eHowToPlay_ImageEnchantment,
	eHowToPlay_ImageAnvil,
	eHowToPlay_ImageFarmingAnimals,
	eHowToPlay_ImageBreeding,
	eHowToPlay_ImageTrading,
	eHowToPlay_ImageNetherPortal,
	eHowToPlay_ImageTheEnd,
	eHowToPlay_NumImages
};

enum EHowToPlayLabelControls
{
	eHowToPlay_LabelNone			 = -1,
	eHowToPlay_LabelIInventory			=0,
	eHowToPlay_LabelSCInventory			,
	eHowToPlay_LabelSCChest				,
	eHowToPlay_LabelLCInventory			,
	eHowToPlay_LabelLCChest				,
	eHowToPlay_LabelCItem				,
	eHowToPlay_LabelCGroup				,
	eHowToPlay_LabelCInventory2x2		,
	eHowToPlay_LabelCTItem				,
	eHowToPlay_LabelCTGroup				,
	eHowToPlay_LabelCTInventory3x3		,
	eHowToPlay_LabelFFuel				,
	eHowToPlay_LabelFInventory			,
	eHowToPlay_LabelFIngredient			,
	eHowToPlay_LabelFChest				,
	eHowToPlay_LabelDText				,
	eHowToPlay_LabelDInventory			,
	eHowToPlay_LabelCreativeInventory,
	eHowToPlay_LabelEEnchant,
	eHowToPlay_LabelEInventory,
	eHowToPlay_LabelBBrew,
	eHowToPlay_LabelBInventory,
	eHowToPlay_LabelAnvil_Inventory,
	eHowToPlay_LabelAnvil_Cost,
	eHowToPlay_LabelAnvil_ARepairAndName,
	eHowToPlay_LabelTrading_Inventory,
	eHowToPlay_LabelTrading_Offer2,
	eHowToPlay_LabelTrading_Offer1,
	eHowToPlay_LabelTrading_NeededForTrade,
	eHowToPlay_LabelTrading_VillagerOffers,
	eHowToPlay_NumLabels
};

struct SHowToPlayPageDef
{
	int		m_iTextControlIndex;		// eHowToPlay_TextNone if not used.
	int		m_iTextStringID;			// -1 if not used.
	int		m_iImageControlIndex;		// eHowToPlay_ImageNone if not used.
	int		m_iLabelStartIndex;			// index of the labels if there are any for the page
	int		m_iLabelCount;
};

class CScene_HowToPlay : public CXuiSceneImpl
{
protected:
	int					m_iPad;
	D3DXVECTOR3 m_OriginalPosition;
	EHowToPlayPage		m_eCurrPage;

	// Control and Element wrapper objects.
	CXuiHtmlElement		m_aTextControls[ eHowToPlay_NumTexts ];
	CXuiControl			m_aImageControls[ eHowToPlay_NumImages ];
	CXuiControl			m_aLabelControls[ eHowToPlay_NumLabels ];

	// Message map. Here we tie messages to message handlers.
	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(OnCustomMessage_Splitscreenplayer)
	XUI_END_MSG_MAP()

		   // Control mapping to objects
	BEGIN_CONTROL_MAP()
		MAP_CONTROL(IDC_XuiHtmlControlMultiplayer,		m_aTextControls[ eHowToPlay_TextMultiplayer ] )
		MAP_CONTROL(IDC_XuiHtmlControlBasics,			m_aTextControls[ eHowToPlay_TextBasics ] )
		MAP_CONTROL(IDC_XuiHtmlControlHUD,				m_aTextControls[ eHowToPlay_TextHUD ] )
		MAP_CONTROL(IDC_XuiHtmlControlCreative,			m_aTextControls[ eHowToPlay_TextCreative ] )
		MAP_CONTROL(IDC_XuiHtmlControlInventory,		m_aTextControls[ eHowToPlay_TextInventory ] )
		MAP_CONTROL(IDC_XuiHtmlControlChest,			m_aTextControls[ eHowToPlay_TextSmallChest ] )
		MAP_CONTROL(IDC_XuiHtmlControlLargeChest,		m_aTextControls[ eHowToPlay_TextLargeChest ] )
		MAP_CONTROL(IDC_XuiHtmlControlEnderchest,		m_aTextControls[ eHowToPlay_TextEnderchest ] )
		MAP_CONTROL(IDC_XuiHtmlControlCrafting,			m_aTextControls[ eHowToPlay_TextCrafting ] )
		MAP_CONTROL(IDC_XuiHtmlControlCraftingTable,	m_aTextControls[ eHowToPlay_TextCraftTable ] )
		MAP_CONTROL(IDC_XuiHtmlControlFurnace,			m_aTextControls[ eHowToPlay_TextFurnace ] )
		MAP_CONTROL(IDC_XuiHtmlControlDispenser,		m_aTextControls[ eHowToPlay_TextDispenser ] )
		MAP_CONTROL(IDC_XuiHtmlControlBrewing,			m_aTextControls[ eHowToPlay_TextBrewing ] )
		MAP_CONTROL(IDC_XuiHtmlControlEnchantment,		m_aTextControls[ eHowToPlay_TextEnchantment ] )
		MAP_CONTROL(IDC_XuiHtmlControlAnvil,			m_aTextControls[ eHowToPlay_TextAnvil ] )
		MAP_CONTROL(IDC_XuiHtmlControlFarmingAnimals,	m_aTextControls[ eHowToPlay_TextFarmingAnimals ] )
		MAP_CONTROL(IDC_XuiHtmlControlBreeding,			m_aTextControls[ eHowToPlay_TextBreeding ] )
		MAP_CONTROL(IDC_XuiHtmlControlTrading,			m_aTextControls[ eHowToPlay_TextTrading ] )
		MAP_CONTROL(IDC_XuiHtmlControlNetherPortal,		m_aTextControls[ eHowToPlay_TextNetherPortal ] )
		MAP_CONTROL(IDC_XuiHtmlControlTheEnd,			m_aTextControls[ eHowToPlay_TextTheEnd ] )
		MAP_CONTROL(IDC_XuiHtmlControlSocialMedia,		m_aTextControls[ eHowToPlay_TextSocialMedia ] )
		MAP_CONTROL(IDC_XuiHtmlControlBanList,			m_aTextControls[ eHowToPlay_TextBanList ] )
		MAP_CONTROL(IDC_XuiHtmlControlWhatsNew,			m_aTextControls[ eHowToPlay_TextWhatsNew ] )
		MAP_CONTROL(IDC_XuiHtmlControlHostOptions,		m_aTextControls[ eHowToPlay_TextHostOptions] )

		MAP_CONTROL(IDC_XuiImageHUD,					m_aImageControls[ eHowToPlay_ImageHUD ] )
		MAP_CONTROL(IDC_XuiImageCreative,				m_aImageControls[ eHowToPlay_ImageCreative ] )
		MAP_CONTROL(IDC_XuiImageInventory,				m_aImageControls[ eHowToPlay_ImageInventory ] )
		MAP_CONTROL(IDC_XuiImageChest,					m_aImageControls[ eHowToPlay_ImageChest ] )
		MAP_CONTROL(IDC_XuiImageLargeChest,				m_aImageControls[ eHowToPlay_ImageLargeChest ] )
		MAP_CONTROL(IDC_XuiImageEnderchest,				m_aImageControls[ eHowToPlay_ImageEnderChest ] )
		MAP_CONTROL(IDC_XuiImageCrafting,				m_aImageControls[ eHowToPlay_ImageInventoryCrafting ] )
		MAP_CONTROL(IDC_XuiImageCraftingTable,			m_aImageControls[ eHowToPlay_ImageCraftingTable ] )
		MAP_CONTROL(IDC_XuiImageFurnace,				m_aImageControls[ eHowToPlay_ImageFurnace ] )
		MAP_CONTROL(IDC_XuiImageDispenser,				m_aImageControls[ eHowToPlay_ImageDispenser ] )
		MAP_CONTROL(IDC_XuiImageBrewing,				m_aImageControls[ eHowToPlay_ImageBrewing ] )
		MAP_CONTROL(IDC_XuiImageEnchantment,			m_aImageControls[ eHowToPlay_ImageEnchantment ] )
		MAP_CONTROL(IDC_XuiImageAnvil,					m_aImageControls[ eHowToPlay_ImageAnvil ] )
		MAP_CONTROL(IDC_XuiImageBreeding,				m_aImageControls[ eHowToPlay_ImageBreeding ] )
		MAP_CONTROL(IDC_XuiImageFarmingAnimals,			m_aImageControls[ eHowToPlay_ImageFarmingAnimals ] )
		MAP_CONTROL(IDC_XuiImageTrading,				m_aImageControls[ eHowToPlay_ImageTrading ] )
		MAP_CONTROL(IDC_XuiImageNetherPortal,			m_aImageControls[ eHowToPlay_ImageNetherPortal ] )
		MAP_CONTROL(IDC_XuiImageTheEnd,					m_aImageControls[ eHowToPlay_ImageTheEnd ] )

		MAP_CONTROL(IDC_CTItem,							m_aLabelControls[ eHowToPlay_LabelCTItem		] )
		MAP_CONTROL(IDC_CTGroup,						m_aLabelControls[ eHowToPlay_LabelCTGroup		] )
		MAP_CONTROL(IDC_CTInventory3x3,					m_aLabelControls[ eHowToPlay_LabelCTInventory3x3	] )
		MAP_CONTROL(IDC_CItem,							m_aLabelControls[ eHowToPlay_LabelCItem		] )
		MAP_CONTROL(IDC_CGroup,							m_aLabelControls[ eHowToPlay_LabelCGroup		] )
		MAP_CONTROL(IDC_CInventory,						m_aLabelControls[ eHowToPlay_LabelCInventory2x2	] )
		MAP_CONTROL(IDC_FFuel,							m_aLabelControls[ eHowToPlay_LabelFFuel			] )
		MAP_CONTROL(IDC_FInventory,						m_aLabelControls[ eHowToPlay_LabelFInventory		] )
		MAP_CONTROL(IDC_FIngredient,					m_aLabelControls[ eHowToPlay_LabelFIngredient	] )
		MAP_CONTROL(IDC_FChest,							m_aLabelControls[ eHowToPlay_LabelFChest			] )
		MAP_CONTROL(IDC_LCInventory,					m_aLabelControls[ eHowToPlay_LabelLCInventory	] )
		MAP_CONTROL(IDC_CIGroup,						m_aLabelControls[ eHowToPlay_LabelCreativeInventory	] )
		MAP_CONTROL(IDC_LCChest,						m_aLabelControls[ eHowToPlay_LabelLCChest		] )
		MAP_CONTROL(IDC_SCInventory,					m_aLabelControls[ eHowToPlay_LabelSCInventory	] )
		MAP_CONTROL(IDC_SCChest,						m_aLabelControls[ eHowToPlay_LabelSCChest		] )
		MAP_CONTROL(IDC_IInventory,						m_aLabelControls[ eHowToPlay_LabelIInventory		] )
		MAP_CONTROL(IDC_DInventory,						m_aLabelControls[ eHowToPlay_LabelDInventory		] )
		MAP_CONTROL(IDC_DText,							m_aLabelControls[ eHowToPlay_LabelDText			] )
		MAP_CONTROL(IDC_EEnchant,						m_aLabelControls[ eHowToPlay_LabelEEnchant			] )
		MAP_CONTROL(IDC_EInventory,						m_aLabelControls[ eHowToPlay_LabelEInventory			] )
		MAP_CONTROL(IDC_BBrew,							m_aLabelControls[ eHowToPlay_LabelBBrew			] )
		MAP_CONTROL(IDC_BInventory,						m_aLabelControls[ eHowToPlay_LabelBInventory			] )	
		MAP_CONTROL(IDC_AInventory,						m_aLabelControls[ eHowToPlay_LabelAnvil_Inventory ] )
		MAP_CONTROL(IDC_ACost,							m_aLabelControls[ eHowToPlay_LabelAnvil_Cost ] )
		MAP_CONTROL(IDC_ARepairAndName,					m_aLabelControls[ eHowToPlay_LabelAnvil_ARepairAndName ] )
		MAP_CONTROL(IDC_TInventory,						m_aLabelControls[ eHowToPlay_LabelTrading_Inventory ] )
		//MAP_CONTROL(IDC_TOffer2Label,					m_aLabelControls[ eHowToPlay_LabelTrading_Offer2 ] )
		MAP_CONTROL(IDC_TOffer1Label,					m_aLabelControls[ eHowToPlay_LabelTrading_Offer1 ] )
		MAP_CONTROL(IDC_TNeededForTrade,				m_aLabelControls[ eHowToPlay_LabelTrading_NeededForTrade ] )
		MAP_CONTROL(IDC_TVillagerOffers,				m_aLabelControls[ eHowToPlay_LabelTrading_VillagerOffers ] )

		
	END_CONTROL_MAP()

	HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
	HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
	HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled);

	void StartPage( EHowToPlayPage ePage );
public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_HowToPlay, L"CScene_HowToPlay", XUI_CLASS_SCENE )



};
