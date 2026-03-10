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

	SignEntryScreenInput* initData = static_cast<SignEntryScreenInput *>(_initData);
	m_sign = initData->sign;

	m_bConfirmed = false;
	m_bIgnoreInput = false;
	m_iSignCursorFrame = 0;
#ifdef _WINDOWS64
	m_iActiveDirectEditLine = -1;
	m_bNeedsInitialEdit = true;
	m_bSkipTickNav = false;
#endif

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
	m_sign->SetSelectedLine(-1);
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

#ifdef _WINDOWS64
	// On first tick, auto-start editing line 1 if KBM is active (Java-style flow)
	if (m_bNeedsInitialEdit)
	{
		m_bNeedsInitialEdit = false;
		if (g_KBMInput.IsKBMActive())
		{
			SetFocusToElement(eControl_Line1);
			m_iActiveDirectEditLine = 0;
			m_textInputLines[0].beginDirectEdit(15);
		}
	}

	// UP/DOWN navigation — must happen after tickDirectEdit (so typed chars are consumed)
	// and before sign cursor update (so the cursor is correct for this frame's render)
	// m_bSkipTickNav prevents double-processing when handleInput auto-started editing this frame
	if (m_iActiveDirectEditLine >= 0 && !m_bSkipTickNav)
	{
		int navDir = 0;
		if (g_KBMInput.IsKeyPressed(VK_DOWN)) navDir = 1;
		else if (g_KBMInput.IsKeyPressed(VK_UP)) navDir = -1;

		if (navDir != 0)
		{
			int newLine = m_iActiveDirectEditLine + navDir;
			if (newLine >= eControl_Line1 && newLine <= eControl_Line4)
			{
				m_textInputLines[m_iActiveDirectEditLine].confirmDirectEdit();
				SetFocusToElement(newLine);
				m_iActiveDirectEditLine = newLine;
				m_textInputLines[newLine].beginDirectEdit(15);
			}
			else if (navDir > 0)
			{
				m_textInputLines[m_iActiveDirectEditLine].confirmDirectEdit();
				SetFocusToElement(eControl_Confirm);
				m_iActiveDirectEditLine = -1;
			}
		}
	}
	m_bSkipTickNav = false;

	if (m_iActiveDirectEditLine >= 0 && !m_textInputLines[m_iActiveDirectEditLine].isDirectEditing())
		m_iActiveDirectEditLine = -1;
#endif

	// Blinking > text < cursor on the 3D sign
	m_iSignCursorFrame++;
	if (m_iSignCursorFrame / 6 % 2 == 0)
	{
#ifdef _WINDOWS64
		if (m_iActiveDirectEditLine >= 0)
			m_sign->SetSelectedLine(m_iActiveDirectEditLine);
		else
#endif
		{
			int focusedLine = -1;
			for (int i = eControl_Line1; i <= eControl_Line4; i++)
			{
				if (controlHasFocus(i))
				{
					focusedLine = i;
					break;
				}
			}
			m_sign->SetSelectedLine(focusedLine);
		}
	}
	else
	{
		m_sign->SetSelectedLine(-1);
	}

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
			if(player != nullptr && player->connection && player->connection->isStarted())
			{
				player->connection->send(std::make_shared<SignUpdatePacket>(m_sign->x, m_sign->y, m_sign->z, m_sign->IsVerified(), m_sign->IsCensored(), m_sign->GetMessages()));
			}
		}
		ui.CloseUIScenes(m_iPad);
	}
}

void UIScene_SignEntryMenu::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	if(m_bConfirmed || m_bIgnoreInput) return;
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) { handled = true; return; }
#endif

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
		sendInputToMovie(key, repeat, pressed, released);
		handled = true;
		break;
	case ACTION_MENU_UP:
	case ACTION_MENU_DOWN:
		sendInputToMovie(key, repeat, pressed, released);
#ifdef _WINDOWS64
		// Auto-start editing if focus moved to a line (e.g. UP from Confirm)
		if (g_KBMInput.IsKBMActive())
		{
			for (int i = eControl_Line1; i <= eControl_Line4; i++)
			{
				if (controlHasFocus(i))
				{
					m_iActiveDirectEditLine = i;
					m_textInputLines[i].beginDirectEdit(15);
					m_bSkipTickNav = true;
					break;
				}
			}
		}
#endif
		handled = true;
		break;
	}
}

#ifdef _WINDOWS64
void UIScene_SignEntryMenu::getDirectEditInputs(vector<UIControl_TextInput*> &inputs)
{
	for (int i = 0; i < 4; i++)
		inputs.push_back(&m_textInputLines[i]);
}

void UIScene_SignEntryMenu::onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result)
{
	int line = -1;
	for (int i = 0; i < 4; i++)
	{
		if (input == &m_textInputLines[i]) { line = i; break; }
	}
	if (line != m_iActiveDirectEditLine) return;

	if (result == UIControl_TextInput::eDirectEdit_Confirmed)
	{
		int newLine = line + 1;
		if (newLine <= eControl_Line4)
		{
			SetFocusToElement(newLine);
			m_iActiveDirectEditLine = newLine;
			m_textInputLines[newLine].beginDirectEdit(15);
		}
		else
		{
			m_iActiveDirectEditLine = -1;
			m_bConfirmed = true;
		}
	}
	else if (result == UIControl_TextInput::eDirectEdit_Cancelled)
	{
		m_iActiveDirectEditLine = -1;
		wstring temp = L"";
		for (int j = 0; j < 4; j++)
			m_sign->SetMessage(j, temp);
		navigateBack();
		ui.PlayUISFX(eSFX_Back);
	}
}

bool UIScene_SignEntryMenu::handleMouseClick(F32 x, F32 y)
{
	if (m_iActiveDirectEditLine >= 0)
	{
		// During direct edit, only the Done button is clickable.
		// Hit-test it manually — all other clicks are consumed but ignored.
		m_buttonConfirm.UpdateControl();
		S32 cx = m_buttonConfirm.getXPos();
		S32 cy = m_buttonConfirm.getYPos();
		S32 cw = m_buttonConfirm.getWidth();
		S32 ch = m_buttonConfirm.getHeight();
		if (cw > 0 && ch > 0 && x >= cx && x <= cx + cw && y >= cy && y <= cy + ch)
		{
			m_textInputLines[m_iActiveDirectEditLine].confirmDirectEdit();
			m_iActiveDirectEditLine = -1;
			m_bConfirmed = true;
		}
		return true;
	}
	return UIScene::handleMouseClick(x, y);
}
#endif

int UIScene_SignEntryMenu::KeyboardCompleteCallback(LPVOID lpParam,bool bRes)
{
	const auto pClass=static_cast<UIScene_SignEntryMenu *>(lpParam);
	pClass->m_bIgnoreInput = false;
	if (bRes)
	{
#ifdef _WINDOWS64
		uint16_t pchText[128];
		ZeroMemory(pchText, 128 * sizeof(uint16_t));
		Win64_GetKeyboardText(pchText, 128);
		pClass->m_textInputLines[pClass->m_iEditingLine].setLabel(reinterpret_cast<wchar_t *>(pchText));
#else
		uint16_t pchText[128];
		ZeroMemory(pchText, 128 * sizeof(uint16_t) );
		InputManager.GetText(pchText);
		pClass->m_textInputLines[pClass->m_iEditingLine].setLabel((wchar_t *)pchText);
#endif
	}
	return 0;
}

void UIScene_SignEntryMenu::handlePress(F64 controlId, F64 childId)
{
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) return;
#endif
	switch(static_cast<int>(controlId))
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
			m_iEditingLine = static_cast<int>(controlId);
#ifdef _WINDOWS64
			if (g_KBMInput.IsKBMActive())
			{
				// Only start editing from keyboard (Enter on focused line), not mouse clicks
				if (!g_KBMInput.IsMouseButtonPressed(KeyboardMouseInput::MOUSE_LEFT))
				{
					m_iActiveDirectEditLine = m_iEditingLine;
					m_textInputLines[m_iEditingLine].beginDirectEdit(15);
				}
			}
			else
			{
				m_bIgnoreInput = true;
				UIKeyboardInitData kbData;
				kbData.title       = app.GetString(IDS_SIGN_TITLE);
				kbData.defaultText = m_textInputLines[m_iEditingLine].getLabel();
				kbData.maxChars    = 15;
				kbData.callback    = &UIScene_SignEntryMenu::KeyboardCompleteCallback;
				kbData.lpParam     = this;
				ui.NavigateToScene(m_iPad, eUIScene_Keyboard, &kbData, eUILayer_Fullscreen, eUIGroup_Fullscreen);
			}
#else
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
			InputManager.RequestKeyboard(app.GetString(IDS_SIGN_TITLE),m_textInputLines[m_iEditingLine].getLabel(),static_cast<DWORD>(m_iPad),15,&UIScene_SignEntryMenu::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Alphabet);
#endif
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