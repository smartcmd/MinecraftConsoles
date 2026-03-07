#include "stdafx.h"
#include "UI.h"
#include "UIScene_LanguageSelector.h"

// strings for buttons in the list				  
const unsigned int UIScene_LanguageSelector::m_uiHTPButtonNameA[]=
{
	HAS_LANGUAGE_SYSTEM(IDS_LANG_SYSTEM)
	HAS_LANGUAGE_EN_US(IDS_LANG_ENGLISH)
	HAS_LANGUAGE_DE_DE(IDS_LANG_GERMAN)
	HAS_LANGUAGE_ES_ES(IDS_LANG_SPANISH_SPAIN)
	HAS_LANGUAGE_ES_MX(IDS_LANG_SPANISH_LATIN_AMERICA)
	HAS_LANGUAGE_FR_FR(IDS_LANG_FRENCH)
	HAS_LANGUAGE_IT_IT(IDS_LANG_ITALIAN)
	HAS_LANGUAGE_PT_PT(IDS_LANG_PORTUGUESE_PORTUGAL)
	HAS_LANGUAGE_PT_BR(IDS_LANG_PORTUGUESE_BRAZIL)
	HAS_LANGUAGE_JA_JP(IDS_LANG_JAPANESE)
	HAS_LANGUAGE_KO_KR(IDS_LANG_KOREAN)
	HAS_LANGUAGE_CN_TW(IDS_LANG_CHINESE_TRADITIONAL)
	HAS_LANGUAGE_CN_CN(IDS_LANG_CHINESE_SIMPLIFIED)
	HAS_LANGUAGE_DA_DK(IDS_LANG_DANISH)
	HAS_LANGUAGE_FI_FI(IDS_LANG_FINISH)
	HAS_LANGUAGE_NL_NL(IDS_LANG_DUTCH)
	HAS_LANGUAGE_PL_PL(IDS_LANG_POLISH)
	HAS_LANGUAGE_RU_RU(IDS_LANG_RUSSIAN)
	HAS_LANGUAGE_SV_SE(IDS_LANG_SWEDISH)
	HAS_LANGUAGE_NB_NO(IDS_LANG_NORWEGIAN)
	HAS_LANGUAGE_SK_SK(IDS_LANG_SLOVAK)
	HAS_LANGUAGE_CZ_CZ(IDS_LANG_CZECH)
	HAS_LANGUAGE_EL_GR(IDS_LANG_GREEK)
	HAS_LANGUAGE_TR_TR(IDS_LANG_TURKISH)
};


UIScene_LanguageSelector::UIScene_LanguageSelector(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_buttonListHowTo.init(eControl_Buttons);

	for(unsigned int i = 0; i < eLanguageSelector_MAX; ++i)
	{
		m_buttonListHowTo.addItem( m_uiHTPButtonNameA[i] , i);
	}
}

wstring UIScene_LanguageSelector::getMoviePath()
{
	if (app.GetLocalPlayerCount() > 1)	return L"LanguagesMenuSplit";
	else								return L"LanguagesMenu";
}

void UIScene_LanguageSelector::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK);
}

void UIScene_LanguageSelector::updateComponents()
{
	bool bNotInGame=(Minecraft::GetInstance()->level==NULL);
	if(bNotInGame)
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,true);
		m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
	}
	else
	{
		m_parentLayer->showComponent(m_iPad,eUIComponent_Panorama,false);

		if( app.GetLocalPlayerCount() == 1 ) m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,true);
		else m_parentLayer->showComponent(m_iPad,eUIComponent_Logo,false);
	}
}

void UIScene_LanguageSelector::handleReload()
{
	for (unsigned int i = 0; i < eLanguageSelector_MAX; ++i)
	{
		m_buttonListHowTo.addItem( m_uiHTPButtonNameA[i], i);
	}
}

void UIScene_LanguageSelector::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
			//ui.NavigateToScene(m_iPad, eUIScene_SettingsOptionsMenu);
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		sendInputToMovie(key, repeat, pressed, released);
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_LanguageSelector::handlePress(F64 controlId, F64 childId)
{
	if( (int)controlId == eControl_Buttons )
	{
		//CD - Added for audio
		ui.PlayUISFX(eSFX_Press);

		int newLanguage, newLocale;
		newLanguage = uiLangMap[(int)childId];
		newLocale =	uiLocaleMap[(int)childId];

		app.SetMinecraftLanguage(m_iPad, newLanguage);
		app.SetMinecraftLocale(m_iPad, newLocale);

		app.CheckGameSettingsChanged(true, m_iPad);
	}
}