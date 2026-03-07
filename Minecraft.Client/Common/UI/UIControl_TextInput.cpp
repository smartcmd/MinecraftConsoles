#include "stdafx.h"
#include "UI.h"
#include "UIControl_TextInput.h"

UIControl_TextInput::UIControl_TextInput()
{
	m_bHasFocus = false;
	m_bHasCaret = false;
	m_bCaretChecked = false;
#ifdef _WINDOWS64
	m_bDirectEditing = false;
	m_iCursorPos = 0;
	m_iCharLimit = 0;
	m_iDirectEditCooldown = 0;
	m_iCaretBlinkTimer = 0;
#endif
}

bool UIControl_TextInput::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eTextInput);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//TextInput specific initialisers
	m_textName = registerFastName(L"text");
	m_funcChangeState = registerFastName(L"ChangeState");
	m_funcSetCharLimit = registerFastName(L"SetCharLimit");
	m_funcSetCaretIndex = registerFastName(L"SetCaretIndex");

	return success;
}

void UIControl_TextInput::init(UIString label, int id)
{
	m_label = label;
	m_id = id;

	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_string_UTF16;
	IggyStringUTF16 stringVal;

	stringVal.string = (IggyUTF16*)label.c_str();
	stringVal.length = label.length();
	value[0].string16 = stringVal;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = id;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 2 , value );

 #ifdef __PSVITA__
 	// 4J-TomK - add this buttonlist to the vita touch box list
 
 	switch(m_parentScene->GetParentLayer()->m_iLayer)
 	{
 	case eUILayer_Fullscreen:
 	case eUILayer_Scene:
 	case eUILayer_HUD:
 		ui.TouchBoxAdd(this,m_parentScene);
 		break;
 	}
 #endif
}

void UIControl_TextInput::ReInit()
{
	UIControl_Base::ReInit();

	init(m_label, m_id);
}

void UIControl_TextInput::setFocus(bool focus)
{
	if(m_bHasFocus != focus)
	{
		m_bHasFocus = focus;

		IggyDataValue result;
		IggyDataValue value[1];
		value[0].type = IGGY_DATATYPE_number;
		value[0].number = focus?0:1;
		IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcChangeState , 1 , value );
	}
}

void UIControl_TextInput::SetCharLimit(int iLimit)
{
	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iLimit;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcSetCharLimit , 1 , value );
}

void UIControl_TextInput::setCaretVisible(bool visible)
{
	if (!m_parentScene || !m_parentScene->getMovie())
		return;

	// Check once whether this SWF's FJ_TextInput actually has a m_mcCaret child.
	// IggyValuePathMakeNameRef always succeeds (creates a ref to undefined),
	// so we validate by trying to read a property from the resolved path.
	if (!m_bCaretChecked)
	{
		IggyValuePath caretPath;
		if (IggyValuePathMakeNameRef(&caretPath, getIggyValuePath(), "m_mcCaret"))
		{
			rrbool test = false;
			IggyResult res = IggyValueGetBooleanRS(&caretPath, m_nameVisible, NULL, &test);
			m_bHasCaret = (res == 0);
		}
		else
		{
			m_bHasCaret = false;
		}
		m_bCaretChecked = true;
	}
	if (!m_bHasCaret)
		return;

	IggyValuePath caretPath;
	if (IggyValuePathMakeNameRef(&caretPath, getIggyValuePath(), "m_mcCaret"))
	{
		IggyValueSetBooleanRS(&caretPath, m_nameVisible, NULL, visible);
	}
}

void UIControl_TextInput::setCaretIndex(int index)
{
	if (!m_parentScene || !m_parentScene->getMovie())
		return;

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = index;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_funcSetCaretIndex , 1 , value );
}

#ifdef _WINDOWS64

void UIControl_TextInput::beginDirectEdit(int charLimit)
{
	const wchar_t* current = getLabel();
	m_editBuffer = current ? current : L"";
	m_textBeforeEdit = m_editBuffer;
	m_iCursorPos = (int)m_editBuffer.length();
	m_iCharLimit = charLimit;
	m_bDirectEditing = true;
	m_iDirectEditCooldown = 0;
	m_iCaretBlinkTimer = 0;
	g_KBMInput.ClearCharBuffer();
	setCaretVisible(true);
	setCaretIndex(m_iCursorPos);
}

UIControl_TextInput::EDirectEditResult UIControl_TextInput::tickDirectEdit()
{
	if (m_iDirectEditCooldown > 0)
		m_iDirectEditCooldown--;

	if (!m_bDirectEditing)
	{
		setCaretVisible(false);
		return eDirectEdit_Continue;
	}

	// Enforce caret visibility and position every tick — setLabel() and Flash
	// focus changes can reset both at any time.
	setCaretVisible(true);
	setCaretIndex(m_iCursorPos);

	// For SWFs without m_mcCaret, insert '_' at the cursor position.
	// All characters remain visible — '_' sits between them like a cursor.
	if (!m_bHasCaret)
	{
		wstring display = m_editBuffer;
		display.insert(m_iCursorPos, 1, L'_');
		setLabel(display.c_str());
	}

	EDirectEditResult result = eDirectEdit_Continue;
	bool changed = false;

	// Consume typed characters from the KBM buffer
	wchar_t ch;
	while (g_KBMInput.ConsumeChar(ch))
	{
		if (ch == 0x08) // Backspace
		{
			if (m_iCursorPos > 0)
			{
				m_editBuffer.erase(m_iCursorPos - 1, 1);
				m_iCursorPos--;
				changed = true;
			}
		}
		else if (ch == 0x0D) // Enter — confirm edit
		{
			m_bDirectEditing = false;
			m_iDirectEditCooldown = 4;
			setLabel(m_editBuffer.c_str(), true);
			setCaretVisible(false);
			return eDirectEdit_Confirmed;
		}
		else if (m_iCharLimit <= 0 || (int)m_editBuffer.length() < m_iCharLimit)
		{
			m_editBuffer.insert(m_iCursorPos, 1, ch);
			m_iCursorPos++;
			changed = true;
		}
	}

	// Arrow keys, Home, End, Delete for cursor movement
	if (g_KBMInput.IsKeyPressed(VK_LEFT) && m_iCursorPos > 0)
	{
		m_iCursorPos--;
		setCaretIndex(m_iCursorPos);
	}
	if (g_KBMInput.IsKeyPressed(VK_RIGHT) && m_iCursorPos < (int)m_editBuffer.length())
	{
		m_iCursorPos++;
		setCaretIndex(m_iCursorPos);
	}
	if (g_KBMInput.IsKeyPressed(VK_HOME))
	{
		m_iCursorPos = 0;
		setCaretIndex(m_iCursorPos);
	}
	if (g_KBMInput.IsKeyPressed(VK_END))
	{
		m_iCursorPos = (int)m_editBuffer.length();
		setCaretIndex(m_iCursorPos);
	}
	if (g_KBMInput.IsKeyPressed(VK_DELETE) && m_iCursorPos < (int)m_editBuffer.length())
	{
		m_editBuffer.erase(m_iCursorPos, 1);
		changed = true;
	}

	// Escape — cancel edit and restore original text
	if (g_KBMInput.IsKeyPressed(VK_ESCAPE))
	{
		m_editBuffer = m_textBeforeEdit;
		m_bDirectEditing = false;
		m_iDirectEditCooldown = 4;
		setLabel(m_editBuffer.c_str());
		setCaretVisible(false);
		return eDirectEdit_Cancelled;
	}

	if (changed)
	{
		if (m_bHasCaret)
		{
			setLabel(m_editBuffer.c_str());
			setCaretIndex(m_iCursorPos);
		}
		// SWFs without caret: the cursor block above already updates the label every tick
	}

	return eDirectEdit_Continue;
}

void UIControl_TextInput::cancelDirectEdit()
{
	if (m_bDirectEditing)
	{
		m_editBuffer = m_textBeforeEdit;
		m_bDirectEditing = false;
		m_iDirectEditCooldown = 4;
		setLabel(m_editBuffer.c_str(), true);
		setCaretVisible(false);
	}
}

void UIControl_TextInput::confirmDirectEdit()
{
	if (m_bDirectEditing)
	{
		m_bDirectEditing = false;
		setLabel(m_editBuffer.c_str(), true);
		setCaretVisible(false);
	}
}

#endif
