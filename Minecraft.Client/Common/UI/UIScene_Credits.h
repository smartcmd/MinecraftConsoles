#pragma once

#include "UIScene.h"

#define PS3_CREDITS_COUNT 80
#define PSVITA_CREDITS_COUNT 82
#define PS4_CREDITS_COUNT 80
#define XBOXONE_CREDITS_COUNT (80+318)
#define MILES_AND_IGGY_CREDITS_COUNT	8
#define DYNAMODE_FONT_CREDITS_COUNT	2
#define PS3_DOLBY_CREDIT 4


#ifdef __PS3__
#define MAX_CREDIT_STRINGS (PS3_CREDITS_COUNT + MILES_AND_IGGY_CREDITS_COUNT + DYNAMODE_FONT_CREDITS_COUNT + PS3_DOLBY_CREDIT)
#elif defined(__ORBIS__)
#define MAX_CREDIT_STRINGS (PS4_CREDITS_COUNT + MILES_AND_IGGY_CREDITS_COUNT + DYNAMODE_FONT_CREDITS_COUNT)
#elif defined(_DURANGO) || defined _WINDOWS64
#define MAX_CREDIT_STRINGS (XBOXONE_CREDITS_COUNT + MILES_AND_IGGY_CREDITS_COUNT)
#elif defined(__PSVITA__)
#define MAX_CREDIT_STRINGS (PSVITA_CREDITS_COUNT + MILES_AND_IGGY_CREDITS_COUNT + DYNAMODE_FONT_CREDITS_COUNT)
#endif

class UIScene_Credits : public UIScene
{
private:
	enum ECreditIcons
	{
		eCreditIcon_Iggy,
		eCreditIcon_Miles,
		eCreditIcon_Dolby,
	};

	static SCreditTextItemDef gs_aCreditDefs[MAX_CREDIT_STRINGS];
	
	int		m_iCurrDefIndex;		// Index of last created text def.
	int		m_iNumTextDefs;			// Total number of text defs in the credits.

	bool m_bAddNextLabel;

	IggyName m_funcSetNextLabel, m_funcAddImage;
	UI_BEGIN_MAP_ELEMENTS_AND_NAMES(UIScene)
		UI_MAP_NAME(m_funcSetNextLabel, L"SetNextLabel")
		UI_MAP_NAME(m_funcAddImage, L"AddImage")
	UI_END_MAP_ELEMENTS_AND_NAMES()
public:
	UIScene_Credits(int iPad, void *initData, UILayer *parentLayer);

	virtual EUIScene getSceneType() { return eUIScene_Credits;}
	
	virtual void updateTooltips();
	virtual void updateComponents();

	void handleReload();

	virtual void tick();

protected:
	// TODO: This should be pure virtual in this class
	virtual wstring getMoviePath();

public:
	// INPUT
	virtual void handleInput(int iPad, int key, bool repeat, bool pressed, bool released, bool &handled);

	virtual void handleRequestMoreData(F64 startIndex, bool up);

private:
	void setNextLabel(const wstring &label, ECreditTextTypes size);
	void addImage(ECreditIcons icon);
};
