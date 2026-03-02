#pragma once

#include "UIControl_Button.h"

class UIControl_EnchantmentButton : public UIControl_Button
{
private:
	// Maps to values in AS
	enum EState
	{
		eState_Inactive = 0,
		eState_Active = 1,
		eState_Selected = 2,
	};

	EState m_lastState;
	int m_lastCost;
	int m_index;
	wstring m_enchantmentString;
	bool m_bHasFocus;

	IggyName m_funcChangeState;

	unsigned int m_textColour, m_textFocusColour, m_textDisabledColour;

	class EnchantmentNames
	{
	public:
		static EnchantmentNames instance;

	private:
		Random random;
		vector<wstring> words;

		EnchantmentNames();

	public:
		wstring getRandomName();
	};

public:
	UIControl_EnchantmentButton();

	virtual bool setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName);

	virtual void tick();

	void init(int index);
	virtual void ReInit();
	void render(IggyCustomDrawCallbackRegion *region);

	void updateState();

	virtual void setFocus(bool focus);
};