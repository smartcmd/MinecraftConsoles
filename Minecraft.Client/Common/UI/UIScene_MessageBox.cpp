#include "stdafx.h"
#include "UI.h"
#include "UIScene_MessageBox.h"

UIScene_MessageBox::UIScene_MessageBox(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	MessageBoxInfo *param = static_cast<MessageBoxInfo *>(initData);

	m_buttonCount = param->uiOptionC;

	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = param->uiOptionC;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = param->dwFocusButton;
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcInit , 2 , value );

	int buttonIndex = 0;
	if(param->uiOptionC > 3)
	{
		m_buttonButtons[eControl_Button0].init(app.GetString(param->uiOptionA[buttonIndex]),buttonIndex);
		++buttonIndex;
	}
	if(param->uiOptionC > 2)
	{
		m_buttonButtons[eControl_Button1].init(app.GetString(param->uiOptionA[buttonIndex]),buttonIndex);
		++buttonIndex;
	}
	if(param->uiOptionC > 1)
	{
	m_buttonButtons[eControl_Button2].init(app.GetString(param->uiOptionA[buttonIndex]),buttonIndex);
		++buttonIndex;
	}
	m_buttonButtons[eControl_Button3].init(app.GetString(param->uiOptionA[buttonIndex]),buttonIndex);

	m_labelTitle.init(app.GetString(param->uiTitle));
	m_labelContent.init(app.GetString(param->uiText));

	out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcAutoResize , 0 , nullptr );

	m_Func = param->Func;
	m_lpParam = param->lpParam;

	parentLayer->addComponent(iPad,eUIComponent_MenuBackground);

	// 4J-TomK - rebuild touch after auto resize
#ifdef __PSVITA__
	ui.TouchBoxRebuild(this);
#endif
}

UIScene_MessageBox::~UIScene_MessageBox()
{
	m_parentLayer->removeComponent(eUIComponent_MenuBackground);
}

wstring UIScene_MessageBox::getMoviePath()
{
	if(app.GetLocalPlayerCount() > 1 && !m_parentLayer->IsFullscreenGroup())
	{
		return L"MessageBoxSplit";
	}
	else
	{
		return L"MessageBox";
	}
}

void UIScene_MessageBox::updateTooltips()
{
	ui.SetTooltips( m_parentLayer->IsFullscreenGroup()?XUSER_INDEX_ANY:m_iPad, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_CANCEL);
}

void UIScene_MessageBox::handleReload()
{
	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = m_buttonCount;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = static_cast<F64>(getControlFocus());
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcInit , 2 , value );

	out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcAutoResize , 0 , nullptr );
}

void UIScene_MessageBox::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
	//app.DebugPrintf("UIScene_DebugOverlay handling input for pad %d, key %d, down- %s, pressed- %s, released- %s\n", iPad, key, down?"TRUE":"FALSE", pressed?"TRUE":"FALSE", released?"TRUE":"FALSE");
	ui.AnimateKeyPress(m_iPad, key, repeat, pressed, released);
	switch(key)
	{
	case ACTION_MENU_CANCEL:
		if(pressed)
		{
			navigateBack();
			if(m_Func) m_Func(m_lpParam, iPad, C4JStorage::EMessage_Cancelled);
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
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
	handled = true;
}

void UIScene_MessageBox::handlePress(F64 controlId, F64 childId)
{
	C4JStorage::EMessageResult result = C4JStorage::EMessage_Cancelled;
	switch(static_cast<int>(controlId))
	{
	case 0:
		result = C4JStorage::EMessage_ResultAccept;
		break;
	case 1:
		result = C4JStorage::EMessage_ResultDecline;
		break;
	case 2:
		result = C4JStorage::EMessage_ResultThirdOption;
		break;
	case 3:
		result = C4JStorage::EMessage_ResultFourthOption;
		break;
	}

	navigateBack();
	if(m_Func) m_Func(m_lpParam, m_iPad, result);
}

bool UIScene_MessageBox::hasFocus(int iPad)
{
	// 4J-JEV: Fix for PS4 #5204 - [TRC][R4033] The application can be locked up by second user logging out of the system. 
	if (m_iPad == 255)
	{
		// Message box is for everyone
		return bHasFocus;
	}
	else if (ProfileManager.IsSignedIn(m_iPad))
	{
		// Owner is still present
		return bHasFocus && (iPad == m_iPad);
	}
	else
	{
		// Original owner has left so let everyone interact
		return bHasFocus;
	}
}