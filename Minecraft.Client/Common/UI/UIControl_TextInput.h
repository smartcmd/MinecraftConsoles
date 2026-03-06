#pragma once

#include "UIControl_Base.h"

class UIControl_TextInput : public UIControl_Base
{
private:
	IggyName m_textName, m_funcChangeState, m_funcSetCharLimit;
	IggyName m_funcSetCaretIndex;
	bool m_bHasFocus;
	bool m_bHasCaret;
	bool m_bCaretChecked;

#ifdef _WINDOWS64
	bool m_bDirectEditing;
	wstring m_textBeforeEdit;
	wstring m_editBuffer;
	int m_iCursorPos;
	int m_iCharLimit;
	int m_iDirectEditCooldown;
	int m_iCaretBlinkTimer;
#endif

public:
	UIControl_TextInput();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	void init(UIString label, int id);
	void ReInit();

	virtual void setFocus(bool focus);

	void SetCharLimit(int iLimit);

	void setCaretVisible(bool visible);
	void setCaretIndex(int index);

#ifdef _WINDOWS64
	enum EDirectEditResult
	{
		eDirectEdit_Continue,
		eDirectEdit_Confirmed,
		eDirectEdit_Cancelled,
	};

	void beginDirectEdit(int charLimit = 0);
	EDirectEditResult tickDirectEdit();
	void cancelDirectEdit();
	void confirmDirectEdit();
	bool isDirectEditing() const { return m_bDirectEditing; }
	int getDirectEditCooldown() const { return m_iDirectEditCooldown; }
	const wstring& getEditBuffer() const { return m_editBuffer; }
#endif
};