#include "stdafx.h"

#ifdef _DEBUG_MENUS_ENABLED
#include "UI.h"
#include "UIScene_DebugCreateSchematic.h"
#include "Minecraft.h"
#include "..\..\..\Minecraft.World\StringHelpers.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.h"

UIScene_DebugCreateSchematic::UIScene_DebugCreateSchematic(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	m_labelTitle.init(L"Name");
	m_labelStartX.init(L"StartX");
	m_labelStartY.init(L"StartY");
	m_labelStartZ.init(L"StartZ");
	m_labelEndX.init(L"EndX");
	m_labelEndY.init(L"EndY");
	m_labelEndZ.init(L"EndZ");

	m_textInputStartX.init(L"",eControl_StartX);
	m_textInputStartY.init(L"",eControl_StartY);
	m_textInputStartZ.init(L"",eControl_StartZ);
	m_textInputEndX.init(L"",eControl_EndX);
	m_textInputEndY.init(L"",eControl_EndY);
	m_textInputEndZ.init(L"",eControl_EndZ);
	m_textInputName.init(L"",eControl_Name);

	m_checkboxSaveMobs.init(L"Save Mobs", eControl_SaveMobs,false);
	m_checkboxUseCompression.init(L"Use Compression", eControl_UseCompression, false);

	m_buttonCreate.init(L"Create",eControl_Create);

	m_data = new ConsoleSchematicFile::XboxSchematicInitParam();
}

wstring UIScene_DebugCreateSchematic::getMoviePath()
{
	return L"DebugCreateSchematic";
}

UIControl_TextInput* UIScene_DebugCreateSchematic::getTextInputForControl(eControls ctrl)
{
	switch (ctrl)
	{
	case eControl_Name:   return &m_textInputName;
	case eControl_StartX: return &m_textInputStartX;
	case eControl_StartY: return &m_textInputStartY;
	case eControl_StartZ: return &m_textInputStartZ;
	case eControl_EndX:   return &m_textInputEndX;
	case eControl_EndY:   return &m_textInputEndY;
	case eControl_EndZ:   return &m_textInputEndZ;
	default: return NULL;
	}
}

#ifdef _WINDOWS64
void UIScene_DebugCreateSchematic::getDirectEditInputs(vector<UIControl_TextInput*> &inputs)
{
	inputs.push_back(&m_textInputName);
	inputs.push_back(&m_textInputStartX);
	inputs.push_back(&m_textInputStartY);
	inputs.push_back(&m_textInputStartZ);
	inputs.push_back(&m_textInputEndX);
	inputs.push_back(&m_textInputEndY);
	inputs.push_back(&m_textInputEndZ);
}

void UIScene_DebugCreateSchematic::onDirectEditFinished(UIControl_TextInput *input, UIControl_TextInput::EDirectEditResult result)
{
	wstring value = input->getEditBuffer();
	int iVal = 0;
	if (!value.empty())
		iVal = _fromString<int>(value);

	if (input == &m_textInputName)
	{
		if (!value.empty())
			swprintf(m_data->name, 64, L"%ls", value.c_str());
		else
			swprintf(m_data->name, 64, L"schematic");
	}
	else if (input == &m_textInputStartX) m_data->startX = iVal;
	else if (input == &m_textInputStartY) m_data->startY = iVal;
	else if (input == &m_textInputStartZ) m_data->startZ = iVal;
	else if (input == &m_textInputEndX)   m_data->endX = iVal;
	else if (input == &m_textInputEndY)   m_data->endY = iVal;
	else if (input == &m_textInputEndZ)   m_data->endZ = iVal;
}

bool UIScene_DebugCreateSchematic::handleMouseClick(F32 x, F32 y)
{
	UIScene::handleMouseClick(x, y);
	return true; // always consume to prevent Iggy re-entry on empty space
}
#endif

void UIScene_DebugCreateSchematic::tick()
{
	UIScene::tick();
}

void UIScene_DebugCreateSchematic::handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled)
{
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) return;
#endif
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
		sendInputToMovie(key, repeat, pressed, released);
		break;
	}
}

void UIScene_DebugCreateSchematic::handlePress(F64 controlId, F64 childId)
{
#ifdef _WINDOWS64
	if (isDirectEditBlocking()) return;
#endif
	switch((int)controlId)
	{
	case eControl_Create:
		{
			// We want the start to be even
			if(m_data->startX > 0 && m_data->startX%2 != 0)
				m_data->startX-=1;
			else if(m_data->startX < 0 && m_data->startX%2 !=0)
				m_data->startX-=1;
			if(m_data->startY < 0) m_data->startY = 0;
			else if(m_data->startY > 0 && m_data->startY%2 != 0)
				m_data->startY-=1;
			if(m_data->startZ > 0 && m_data->startZ%2 != 0)
				m_data->startZ-=1;
			else if(m_data->startZ < 0 && m_data->startZ%2 !=0)
				m_data->startZ-=1;

			// We want the end to be odd to have a total size that is even
			if(m_data->endX > 0 && m_data->endX%2 == 0)
				m_data->endX+=1;
			else if(m_data->endX < 0 && m_data->endX%2 ==0)
				m_data->endX+=1;
			if(m_data->endY > Level::maxBuildHeight)
				m_data->endY = Level::maxBuildHeight;
			else if(m_data->endY > 0 && m_data->endY%2 == 0)
				m_data->endY+=1;
			else if(m_data->endY < 0 && m_data->endY%2 ==0)
				m_data->endY+=1;
			if(m_data->endZ > 0 && m_data->endZ%2 == 0)
				m_data->endZ+=1;
			else if(m_data->endZ < 0 && m_data->endZ%2 ==0)
				m_data->endZ+=1;

			app.SetXuiServerAction(ProfileManager.GetPrimaryPad(), eXuiServerAction_ExportSchematic, (void *)m_data);

			navigateBack();
		}
		break;
	case eControl_Name:
	case eControl_StartX:
	case eControl_StartY:
	case eControl_StartZ:
	case eControl_EndX:
	case eControl_EndY:
	case eControl_EndZ:
		{
			m_keyboardCallbackControl = (eControls)((int)controlId);
#ifdef _WINDOWS64
			if (g_KBMInput.IsKBMActive())
			{
				UIControl_TextInput* input = getTextInputForControl(m_keyboardCallbackControl);
				if (input) input->beginDirectEdit(25);
			}
			else
			{
				UIKeyboardInitData kbData;
				kbData.title       = L"Enter something";
				kbData.defaultText = L"";
				kbData.maxChars    = 25;
				kbData.callback    = &UIScene_DebugCreateSchematic::KeyboardCompleteCallback;
				kbData.lpParam     = this;
				ui.NavigateToScene(m_iPad, eUIScene_Keyboard, &kbData, eUILayer_Fullscreen, eUIGroup_Fullscreen);
			}
#else
			InputManager.RequestKeyboard(L"Enter something",L"",(DWORD)0,25,&UIScene_DebugCreateSchematic::KeyboardCompleteCallback,this,C_4JInput::EKeyboardMode_Default);
#endif
		}
		break;
	};
}

void UIScene_DebugCreateSchematic::handleCheckboxToggled(F64 controlId, bool selected)
{
	switch((int)controlId)
	{
	case eControl_SaveMobs:
		m_data->bSaveMobs = selected;
		break;
	case eControl_UseCompression:
		if (selected)
			m_data->compressionType = APPROPRIATE_COMPRESSION_TYPE;
		else
			m_data->compressionType = Compression::eCompressionType_RLE;
		break;
	}
}

int UIScene_DebugCreateSchematic::KeyboardCompleteCallback(LPVOID lpParam,bool bRes)
{
	UIScene_DebugCreateSchematic *pClass=(UIScene_DebugCreateSchematic *)lpParam;

#ifdef _WINDOWS64
	uint16_t pchText[128];
	ZeroMemory(pchText, 128 * sizeof(uint16_t));
	Win64_GetKeyboardText(pchText, 128);
#else
	uint16_t pchText[128];
	ZeroMemory(pchText, 128 * sizeof(uint16_t) );
	InputManager.GetText(pchText);
#endif

	if(pchText[0]!=0)
	{
		wstring value = (wchar_t *)pchText;
		int iVal = 0; 
		if(!value.empty()) iVal = _fromString<int>( value );
		switch(pClass->m_keyboardCallbackControl)
		{
		case eControl_Name:
			pClass->m_textInputName.setLabel(value);
			if(!value.empty())
			{
				swprintf(pClass->m_data->name,64,L"%ls", value.c_str());
			}
			else
			{
				swprintf(pClass->m_data->name,64,L"schematic");
			}
			break;
		case eControl_StartX:
			pClass->m_textInputStartX.setLabel(value);

			if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
			{
				pClass->m_data->startX = iVal;
			}
			break;
		case eControl_StartY:
			pClass->m_textInputStartY.setLabel(value);

			if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
			{
				pClass->m_data->startY = iVal;
			}
			break;
		case eControl_StartZ:
			pClass->m_textInputStartZ.setLabel(value);

			if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
			{
				pClass->m_data->startZ = iVal;
			}
			break;
		case eControl_EndX:
			pClass->m_textInputEndX.setLabel(value);

			if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
			{
				pClass->m_data->endX = iVal;
			}
			break;
		case eControl_EndY:
			pClass->m_textInputEndY.setLabel(value);

			if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
			{
				pClass->m_data->endY = iVal;
			}
			break;
		case eControl_EndZ:
			pClass->m_textInputEndZ.setLabel(value);

			if( iVal >= (LEVEL_MAX_WIDTH * -16) || iVal < (LEVEL_MAX_WIDTH * 16))
			{
				pClass->m_data->endZ = iVal;
			}
			break;
		}
	}

	return 0;
}
#endif