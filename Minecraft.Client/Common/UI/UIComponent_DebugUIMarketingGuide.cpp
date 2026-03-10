#include "stdafx.h"
#include "UI.h"
#include "UIComponent_DebugUIMarketingGuide.h"

UIComponent_DebugUIMarketingGuide::UIComponent_DebugUIMarketingGuide(int iPad, void *initData, UILayer *parentLayer) : UIScene(iPad, parentLayer)
{
	// Setup all the Iggy references we need for this scene
	initialiseMovie();

	IggyDataValue result;
	IggyDataValue value[1];
	value[0].type = IGGY_DATATYPE_number;
	value[0].number = static_cast<F64>(0);	// WIN64
#if defined _XBOX
	value[0].number = (F64)1;
#elif defined _DURANGO
	value[0].number = (F64)2;
#elif defined __PS3__
	value[0].number = (F64)3;
#elif defined __ORBIS__
	value[0].number = (F64)4;
#elif defined __PSVITA__
	value[0].number = (F64)5;
#elif defined _WINDOWS64
	value[0].number = static_cast<F64>(0);
#endif
	IggyResult out = IggyPlayerCallMethodRS ( getMovie() , &result, IggyPlayerRootPath( getMovie() ), m_funcSetPlatform , 1 , value );
}

wstring UIComponent_DebugUIMarketingGuide::getMoviePath()
{
	return L"DebugUIMarketingGuide";
}