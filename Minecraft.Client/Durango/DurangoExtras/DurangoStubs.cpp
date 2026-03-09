#include "stdafx.h"

#define DURANGO_STUBBED
//printf("missing function on Xbox One : %s\n", __FUNCTION__);

//static char sc_loadPath[] = {"/"};

//const char* getConsoleHomePath() { return sc_loadPath; }

DWORD GetFileSize(HANDLE fh,DWORD *pdwHigh)
{
	LARGE_INTEGER FileSize;

	BOOL bRes=GetFileSizeEx(fh,&FileSize);

	if(bRes==FALSE)
	{
		if(pdwHigh) *pdwHigh=0;
		return 0;
	}
	else
	{
		if(pdwHigh) *pdwHigh=FileSize.HighPart;
		return FileSize.LowPart;
	}
}

DWORD XGetLanguage() 
{ 
	//StringTable::eLocale eCurrentLocale=StringTable::eLocale_American;
	bool bLocaleFound;

	WCHAR wchLocaleName[LOCALE_NAME_MAX_LENGTH];
	GetUserDefaultLocaleName(wchLocaleName,LOCALE_NAME_MAX_LENGTH);
	eMCLang eLang=static_cast<eMCLang>(app.get_eMCLang(wchLocaleName));
	
#ifdef _DEBUG
	app.DebugPrintf("XGetLanguage() ==> '%ls'\n", wchLocaleName);
#endif

	// need to map locale to language
	switch(eLang)
	{
	case eMCLang_jaJP:
		return XC_LANGUAGE_JAPANESE;
	case eMCLang_deDE:
	case eMCLang_deAT:
		return XC_LANGUAGE_GERMAN;
	case eMCLang_frFR:
	case eMCLang_frCA:
		return XC_LANGUAGE_FRENCH;
	case eMCLang_esES:
	case eMCLang_esMX:
		return XC_LANGUAGE_SPANISH;
	case eMCLang_itIT:
		return XC_LANGUAGE_ITALIAN;
	case eMCLang_koKR:
		return XC_LANGUAGE_KOREAN;
	case eMCLang_zhHK:
	case eMCLang_zhSG:
	case eMCLang_zhTW:
	case eMCLang_zhCHT:
		return XC_LANGUAGE_TCHINESE;
	case eMCLang_zhCHS:
	case eMCLang_zhCN:
		return XC_LANGUAGE_SCHINESE;
	case eMCLang_ptPT:
	case eMCLang_ptBR:
		return XC_LANGUAGE_PORTUGUESE;
	case eMCLang_ruRU:
		return XC_LANGUAGE_RUSSIAN;
	case eMCLang_nlNL:
	case eMCLang_nlBE:
		return XC_LANGUAGE_DUTCH;
	case eMCLang_fiFI:
		return XC_LANGUAGE_FINISH;
	case eMCLang_svSV:
	case eMCLang_svSE:
		return XC_LANGUAGE_SWEDISH;
	case eMCLang_daDA:
	case eMCLang_daDK:
		return XC_LANGUAGE_DANISH;
	case eMCLang_noNO:
	case eMCLang_nnNO:
	case eMCLang_nbNO:
		return XC_LANGUAGE_BNORWEGIAN;
	case eMCLang_plPL:
		return XC_LANGUAGE_POLISH;
	case eMCLang_trTR:
		return XC_LANGUAGE_TURKISH;
	case eMCLang_laLAS:
		return XC_LANGUAGE_LATINAMERICANSPANISH;
	case eMCLang_elEL:
	case eMCLang_elGR:
	case eMCLang_enGR: // Hack redirect
		return XC_LANGUAGE_GREEK;
	case eMCLang_csCZ:
	case eMCLang_enCZ: // Hack redirect
		return XC_LANGUAGE_CZECH;
	case eMCLang_skSK:
	case eMCLang_enSK: // Hack redirect
		return XC_LANGUAGE_SLOVAK;
	case eMCLang_enUS:
	case eMCLang_enGB:
	case eMCLang_enIE:
	case eMCLang_enAU:
	case eMCLang_enNZ:
	case eMCLang_enCA:
	default:
		return XC_LANGUAGE_ENGLISH;
	}
}

DWORD XGetLocale() 
{ 
	//return XC_LOCALE_SWEDEN;

	WCHAR wchLocaleName[LOCALE_NAME_MAX_LENGTH];
	GetUserDefaultLocaleName(wchLocaleName,LOCALE_NAME_MAX_LENGTH);
	
	return app.get_xcLang(wchLocaleName);
}

DWORD XEnableGuestSignin(BOOL fEnable) 
{ 
	return 0; 
}

