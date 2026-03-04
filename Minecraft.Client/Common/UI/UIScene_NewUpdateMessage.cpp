#include "stdafx.h"
#include "UI.h"
#include "UIScene_NewUpdateMessage.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"

UIScene_NewUpdateMessage::UIScene_NewUpdateMessage(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	parentLayer->addComponent(iPad,eUIComponent_Panorama);
	parentLayer->addComponent(iPad,eUIComponent_Logo);

	m_buttonConfirm.init(app.GetString(IDS_TOOLTIPS_ACCEPT),eControl_Confirm);

	wstring message = app.GetString(IDS_TITLEUPDATE);
	message.append(L"\r\n");

	message=app.FormatHTMLString(m_iPad,message);

	vector<wstring> paragraphs;
	int lastIndex = 0;
	for (	int index = message.find(L"\r\n", lastIndex, 2);
			index != wstring::npos; 
			index = message.find(L"\r\n", lastIndex, 2)
		)
	{
		paragraphs.push_back( message.substr(lastIndex, index-lastIndex) + L" " );
		lastIndex = index + 2;
	}
	paragraphs.push_back( message.substr( lastIndex, message.length() - lastIndex ) );

	for(unsigned int i = 0; i < paragraphs.size(); ++i)
	{
		m_labelDescription.addText(paragraphs[i],i == (paragraphs.size() - 1) );
	}

	m_bIgnoreInput=false;

#ifdef __PSVITA__
	ui.TouchBoxRebuild(this);
#endif
}

UIScene_NewUpdateMessage::~UIScene_NewUpdateMessage()
{
	m_parentLayer->removeComponent(eUIComponent_Panorama);
	m_parentLayer->removeComponent(eUIComponent_Logo);
}

wstring UIScene_NewUpdateMessage::getMoviePath()
{
	return L"EULA";
}

void UIScene_NewUpdateMessage::updateTooltips()
{
	ui.SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT );
}

void UIScene_NewUpdateMessage::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bIgnoreInput) return;

#ifdef __ORBIS__
	// ignore all players except player 0 - it's their profile that is currently being used
	if(iPad!=0) return;
#endif
	
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_B:
		{
			int iVal=app.GetGameSettings(m_iPad,eGameSetting_DisplayUpdateMessage);
			if(iVal>0) iVal--;

			// set the update text as seen, by clearing the flag
			app.SetGameSettings(m_iPad,eGameSetting_DisplayUpdateMessage,iVal);
			// force a profile write
			app.CheckGameSettingsChanged(true,m_iPad);
			ui.NavigateBack(m_iPad);
		}
		break;
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_OK:
	case ACTION_MENU_DOWN:
	case ACTION_MENU_UP:
	case ACTION_MENU_PAGEUP:
	case ACTION_MENU_PAGEDOWN:
	case ACTION_MENU_OTHER_STICK_DOWN:
	case ACTION_MENU_OTHER_STICK_UP:
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_NewUpdateMessage::handlePress(F64 controlId, F64 childId)
{
	switch((int)controlId)
	{
	case eControl_Confirm:
		{
			//CD - Added for audio
			ui.PlayUISFX(eSFX_Press);

			int iVal=app.GetGameSettings(m_iPad,eGameSetting_DisplayUpdateMessage);
			if(iVal>0) iVal--;

			// set the update text as seen, by clearing the flag
			app.SetGameSettings(m_iPad,eGameSetting_DisplayUpdateMessage,iVal);
			// force a profile write
			app.CheckGameSettingsChanged(true,m_iPad);
			ui.NavigateBack(m_iPad);
		}
		break;
	};
}
