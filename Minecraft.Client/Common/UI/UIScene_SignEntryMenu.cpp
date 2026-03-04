#include "stdafx.h"
#include "UI.h"
#include "UIScene_SignEntryMenu.h"
#include "..\..\Minecraft.h"
#include "..\..\MultiPlayerLocalPlayer.h"
#include "..\..\MultiPlayerLevel.h"
#include "..\..\ClientConnection.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.entity.h"

UIScene_SignEntryMenu::UIScene_SignEntryMenu(int iPad, void *_initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	SignEntryScreenInput* initData = (SignEntryScreenInput*)_initData;
	m_sign = initData->sign;

	m_bConfirmed = false;
	m_bIgnoreInput = false;

	m_buttonConfirm.init(app.GetString(IDS_DONE), eControl_Confirm);
	m_labelMessage.init(app.GetString(IDS_EDIT_SIGN_MESSAGE));

	for(unsigned int i = 0; i<4; ++i)
	{
#if TO_BE_IMPLEMENTED
		// Have to have the Latin alphabet here, since that's what we have on the sign in-game
		// but because the JAP/KOR/CHN fonts don't have extended European characters, let's restrict those languages to not having the extended character set, since they can't see what they are typing
		switch(XGetLanguage())
		{
		case XC_LANGUAGE_JAPANESE:
		case XC_LANGUAGE_TCHINESE:
		case XC_LANGUAGE_KOREAN:
		case XC_LANGUAGE_RUSSIAN:
			m_signRows[i].SetKeyboardType(C_4JInput::EKeyboardMode_Alphabet);
			break;
		default:
			m_signRows[i].SetKeyboardType(C_4JInput::EKeyboardMode_Full);
			break;
		}

		m_signRows[i].SetText( m_sign->GetMessage(i).c_str() );
		m_signRows[i].SetTextLimit(15);
		// Set the title and desc for the edit keyboard popup
		m_signRows[i].SetTitleAndText(IDS_SIGN_TITLE,IDS_SIGN_TITLE_TEXT);
#endif
		m_textInputLines[i].init(m_sign->GetMessage(i).c_str(), i);
	}

	parentLayer->addComponent(iPad,eUIComponent_MenuBackground);
}

UIScene_SignEntryMenu::~UIScene_SignEntryMenu()
{
	m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

wstring UIScene_SignEntryMenu::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1)
	{
		return L"SignEntryMenuSplit";
	}
	else
	{
		return L"SignEntryMenu";
	}
}

void UIScene_SignEntryMenu::updateTooltips()
{
	ui.SetTooltips( m_iPad, IDS_TOOLTIPS_SELECT,IDS_TOOLTIPS_BACK);
}

void UIScene_SignEntryMenu::tick()
{
	UIScene::tick();

	if(m_bConfirmed)
	{
		m_bConfirmed = false;

		// Set the sign text here so we on;y call the verify once it has been set, not while we're typing in to it
		for(int i=0;i<4;i++)
		{
			wstring temp=m_textInputLines[i].getLabel();
			m_sign->SetMessage(i,temp);		
		}

		m_sign->setChanged();

		Minecraft *pMinecraft=Minecraft::GetInstance();
		// need to send the new data
		if (pMinecraft->level->isClientSide)
		{
			shared_ptr<MultiplayerLocalPlayer> player = pMinecraft->localplayers[m_iPad];
			if(player != NULL && player->connection && player->connection->isStarted())
			{
				player->connection->send( shared_ptr<SignUpdatePacket>( new SignUpdatePacket(m_sign->x, m_sign->y, m_sign->z, m_sign->IsVerified(), m_sign->IsCensored(), m_sign->GetMessages()) ) );
			}
		}
		ui.CloseUIScenes(m_iPad);
	}
}

void UIScene_SignEntryMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bConfirmed || m_bIgnoreInput) return;

	ui.AnimateKeyPress(iPad, key, repeat, pressed, released);

	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			// user backed out, so wipe the sign
			wstring temp=L"";

			for(int i=0;i<4;i++)
			{
				m_sign->SetMessage(i,temp);
			}

			navigateBack();
			ui.PlayUISFX(eSFX_Back);
			handled = true;
		}
		break;
	case ACTION_MENU_OK:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	}
}

int UIScene_SignEntryMenu::KeyboardCompleteCallback(LPVOID lpParam,bool bRes)
{
	// 4J HEG - No reason to set value if keyboard was cancelled
	UIScene_SignEntryMenu *pClass=(UIScene_SignEntryMenu *)lpParam;
	pClass->m_bIgnoreInput = false;
	if (bRes)
	{
		uint16_t pchText[128];
		ZeroMemory(pchText, 128 * sizeof(uint16_t) );
		InputManager.GetText(pchText);
		pClass->m_textInputLines[pClass->m_iEditingLine].setLabel((wchar_t *)pchText);
	}
	return 0;
}

void UIScene_SignEntryMenu::handlePress(F64 controlId, F64 childId)
{
	switch((int)controlId)
	{
	case eControl_Confirm:
		{
			m_bConfirmed = true;
		}
		break;
	case eControl_Line1:
	case eControl_Line2:
	case eControl_Line3:
	case eControl_Line4:
		{
			m_iEditingLine = (int)controlId;
			m_bIgnoreInput = true;
#ifdef _XBOX_ONE
			// 4J-PB - Xbox One uses the Windows virtual keyboard, and doesn't have the Xbox 360 Latin keyboard type, so we can't restrict the input set to alphanumeric. The closest we get is the emailSmtpAddress type.
			int language = XGetLanguage();
			switch(language)
			{
			case XC_LANGUAGE_JAPANESE:
			case XC_LANGUAGE_KOREAN:
			case XC_LANGUAGE_TCHINESE:
				InputManager.RequestKeyboard(app.GetString(IDS_SIGN_TITLE),m_textInputLines[m_iEditingLine].getLabel(),(DWORD)m_iPad,15,&UIScene_SignEntryMenu::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Email);
				break;
			default:
				InputManager.RequestKeyboard(app.GetString(IDS_SIGN_TITLE),m_textInputLines[m_iEditingLine].getLabel(),(DWORD)m_iPad,15,&UIScene_SignEntryMenu::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Alphabet);
				break;
			}
#else
			InputManager.RequestKeyboard(app.GetString(IDS_SIGN_TITLE),m_textInputLines[m_iEditingLine].getLabel(),(DWORD)m_iPad,15,&UIScene_SignEntryMenu::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Alphabet);
#endif
		}
		break;
	}
}

void UIScene_SignEntryMenu::handleDestroy()
{
#ifdef __PSVITA__
	app.DebugPrintf("missing InputManager.DestroyKeyboard on Vita !!!!!!\n");
#endif

	// another player destroyed the anvil, so shut down the keyboard if it is displayed
#if ( defined __PS3__ || defined __ORBIS__ || defined _DURANGO)
	InputManager.DestroyKeyboard();
#endif
}