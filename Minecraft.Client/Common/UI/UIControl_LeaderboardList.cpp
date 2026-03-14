#include "stdafx.h"
#include "UI.h"
#include "UIControl_LeaderboardList.h"

UIControl_LeaderboardList::UIControl_LeaderboardList()
{
}

bool UIControl_LeaderboardList::setupControl(UIScene *scene, IggyValuePath *parent, const string &controlName)
{
	UIControl::setControlType(UIControl::eLeaderboardList);
	bool success = UIControl_Base::setupControl(scene,parent,controlName);

	//UIControl_LeaderboardList specific initialisers
	m_funcInitLeaderboard = registerFastName(L"InitLeaderboard");
	m_funcAddDataSet = registerFastName(L"AddDataSet");
	m_funcResetLeaderboard = registerFastName(L"ResetLeaderboard");
	m_funcSetupTitles = registerFastName(L"SetupTitles");
	m_funcSetColumnIcon = registerFastName(L"SetColumnIcon");
#ifdef __PSVITA__
	m_funcSetTouchFocus = registerFastName(L"SetTouchFocus");
	m_bTouchInitialised = false;
#endif

	return success;
}

void UIControl_LeaderboardList::init(int id)
{
	m_id = id;

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = id;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath() , m_initFunc , 1 , value );
}

void UIControl_LeaderboardList::ReInit()
{
	UIControl_Base::ReInit();
	init(m_id);
}

void UIControl_LeaderboardList::clearList()
{
	IggyDataValue result;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcResetLeaderboard , 0 , nullptr );
}

void UIControl_LeaderboardList::setupTitles(const wstring &rank, const wstring &gamertag)
{
	IggyDataValue result;
	IggyDataValue value[2];

	IggyStringUTF16 stringVal0;
	stringVal0.string = (IggyUTF16*)rank.c_str();
	stringVal0.length = rank.length();
	value[0].type = IGGY_DATATYPE_string_UTF16;
	value[0].string16 = stringVal0;

	IggyStringUTF16 stringVal1;
	stringVal1.string = (IggyUTF16*)gamertag.c_str();
	stringVal1.length = gamertag.length();
	value[1].type = IGGY_DATATYPE_string_UTF16;
	value[1].string16 = stringVal1;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcSetupTitles , 2 , value );
}

void UIControl_LeaderboardList::initLeaderboard(int iFirstFocus, int iTotalEntries, int iNumColumns)
{
	IggyDataValue result;
	IggyDataValue value[3];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iFirstFocus;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iTotalEntries;

	value[2].type = IGGY_DATATYPE_number;
	value[2].number = iNumColumns;
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcInitLeaderboard , 3 , value );

#ifdef __PSVITA__
	// 4J-PB - add this button to the vita touch box list
	if(!m_bTouchInitialised)
	{
		switch(m_parentScene->GetParentLayer()->m_iLayer)
		{
		case eUILayer_Fullscreen:
		case eUILayer_Scene:
			ui.TouchBoxAdd(this,m_parentScene);
			break;
		}
		m_bTouchInitialised = true;
	}
#endif
}

void UIControl_LeaderboardList::setColumnIcon(int iColumn, int iType)
{
	IggyDataValue result;
	IggyDataValue value[2];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iColumn;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = (iType<=32000)?0:(iType-32000);

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcSetColumnIcon , 2 , value );
}

void UIControl_LeaderboardList::addDataSet(bool bLast, int iId, int iRank, const wstring &gamertag, bool bDisplayMessage, const wstring &col0, const wstring &col1, const wstring &col2, const wstring &col3, const wstring &col4, const wstring &col5, const wstring &col6)
{
	IggyDataValue result;
	IggyDataValue value[12];

	value[0].type = IGGY_DATATYPE_boolean;
	value[0].boolval = bLast;

	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iId;

	value[2].type = IGGY_DATATYPE_number;
	value[2].number = iRank;

	IggyStringUTF16 stringVal0;
	stringVal0.string = (IggyUTF16*)gamertag.c_str();
	stringVal0.length = gamertag.length();
	value[3].type = IGGY_DATATYPE_string_UTF16;
	value[3].string16 = stringVal0;

	value[4].type = IGGY_DATATYPE_boolean;
	value[4].boolval = bDisplayMessage;

	IggyStringUTF16 stringVal1;
	stringVal1.string = (IggyUTF16*)col0.c_str();
	stringVal1.length = col0.length();
	value[5].type = IGGY_DATATYPE_string_UTF16;
	value[5].string16 = stringVal1;

	if(col1.empty())
	{
		value[6].type = IGGY_DATATYPE_null;
	}
	else
	{
		IggyStringUTF16 stringVal2;
		stringVal2.string = (IggyUTF16*)col1.c_str();
		stringVal2.length = col1.length();
		value[6].type = IGGY_DATATYPE_string_UTF16;
		value[6].string16 = stringVal2;
	}

	if(col2.empty())
	{
		value[7].type = IGGY_DATATYPE_null;
	}
	else
	{
		IggyStringUTF16 stringVal3;
		stringVal3.string = (IggyUTF16*)col2.c_str();
		stringVal3.length = col2.length();
		value[7].type = IGGY_DATATYPE_string_UTF16;
		value[7].string16 = stringVal3;
	}

	if(col3.empty())
	{
		value[8].type = IGGY_DATATYPE_null;
	}
	else
	{
		IggyStringUTF16 stringVal4;
		stringVal4.string = (IggyUTF16*)col3.c_str();
		stringVal4.length = col3.length();
		value[8].type = IGGY_DATATYPE_string_UTF16;
		value[8].string16 = stringVal4;
	}

	if(col4.empty())
	{
		value[9].type = IGGY_DATATYPE_null;
	}
	else
	{
		IggyStringUTF16 stringVal5;
		stringVal5.string = (IggyUTF16*)col4.c_str();
		stringVal5.length = col4.length();
		value[9].type = IGGY_DATATYPE_string_UTF16;
		value[9].string16 = stringVal5;
	}

	if(col5.empty())
	{
		value[10].type = IGGY_DATATYPE_null;
	}
	else
	{
		IggyStringUTF16 stringVal6;
		stringVal6.string = (IggyUTF16*)col5.c_str();
		stringVal6.length = col5.length();
		value[10].type = IGGY_DATATYPE_string_UTF16;
		value[10].string16 = stringVal6;
	}

	if(col6.empty())
	{
		value[11].type = IGGY_DATATYPE_null;
	}
	else
	{
		IggyStringUTF16 stringVal7;
		stringVal7.string = (IggyUTF16*)col6.c_str();
		stringVal7.length = col6.length();
		value[11].type = IGGY_DATATYPE_string_UTF16;
		value[11].string16 = stringVal7;
	}
	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie() , &result, getIggyValuePath(), m_funcAddDataSet , 12 , value );
}

#ifdef __PSVITA__
void UIControl_LeaderboardList::SetTouchFocus(S32 iX, S32 iY, bool bRepeat)
{
	IggyDataValue result;
	IggyDataValue value[3];

	value[0].type = IGGY_DATATYPE_number;
	value[0].number = iX;
	value[1].type = IGGY_DATATYPE_number;
	value[1].number = iY;
	value[2].type = IGGY_DATATYPE_boolean;
	value[2].boolval = bRepeat;

	IggyResult out = IggyPlayerCallMethodRS ( m_parentScene->getMovie(), &result, getIggyValuePath(), m_funcSetTouchFocus, 3 , value );
}
#endif