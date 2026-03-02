#pragma once

#include "UIScene.h"


class UIScene_HowToPlay : public UIScene
{
public:
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
		eHowToPlay_LabelBeacon_PrimaryPower,
		eHowToPlay_LabelBeacon_SecondaryPower,
		eHowToPlay_LabelFireworksText,
		eHowToPlay_LabelFireworksInventory,
		eHowToPlay_LabelHopperText,
		eHowToPlay_LabelHopperInventory,
		eHowToPlay_LabelDropperText,
		eHowToPlay_LabelDropperInventory,
		eHowToPlay_NumLabels
	};

	struct SHowToPlayPageDef
	{
		int		m_iTextStringID;			// -1 if not used.
		int		m_iLabelStartIndex;			// index of the labels if there are any for the page
		int		m_iLabelCount;
	};

private:
	EHowToPlayPage	m_eCurrPage;

	IggyName m_funcLoadPage;
	UIControl_DynamicLabel m_DynamicLabel;
	UIControl_Label m_labels[ eHowToPlay_NumLabels ];
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_ELEMENT( m_DynamicLabel , "DynamicHtmlText" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelCTGroup ] , "Label1_9" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelCTItem ] , "Label2_9")
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelCTInventory3x3 ] , "Label3_9" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelCGroup ] , "Label1_8" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelCItem ] , "Label2_8" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelCInventory2x2 ] , "Label3_8" )
		
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelFChest ] , "Label1_10" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelFIngredient ] , "Label2_10" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelFFuel ] , "Label3_10" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelFInventory ] , "Label4_10" )
		
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelLCChest ] , "Label1_7" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelLCInventory ] , "Label2_7" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelCreativeInventory ] , "Label1_4" )
		
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelSCChest ] , "Label1_6" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelSCInventory ] , "Label2_6" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelIInventory ] , "Label1_5" )
		
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelDText ] , "Label1_11" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelDInventory ] , "Label2_11" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelEEnchant ] , "Label1_13" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelEInventory ] , "Label2_13" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelBBrew ] , "Label1_12" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelBInventory ] , "Label2_12" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelTrading_VillagerOffers ] , "Label1_22" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelTrading_NeededForTrade ] , "Label2_22" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelTrading_Inventory ] , "Label3_22" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelTrading_Offer1 ] , "Label4_22" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelTrading_Offer2 ] , "Label5_22" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelAnvil_ARepairAndName ] , "Label1_21" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelAnvil_Cost ] , "Label2_21" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelAnvil_Inventory ] , "Label3_21" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelBeacon_PrimaryPower ] , "Label1_25" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelBeacon_SecondaryPower ] , "Label2_25" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelFireworksText ] , "Label1_26" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelFireworksInventory ] , "Label2_26" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelHopperText ] , "Label1_27" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelHopperInventory ] , "Label2_27" )

		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelDropperText ] , "Label1_28" )
		UI_MAP_ELEMENT( m_labels[ eHowToPlay_LabelDropperInventory ] , "Label2_28" )
		
		UI_MAP_NAME(m_funcLoadPage, L"LoadHowToPlayPage")
	UI_END_MAP_ELEMENTS_AND_NAMES()
public:
	UIScene_HowToPlay(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_HowToPlay;}
	virtual void updateTooltips();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	virtual void handleReload();

	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

private:
	void StartPage( EHowToPlayPage ePage );
};
