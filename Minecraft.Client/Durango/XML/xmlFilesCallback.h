
#pragma once
#ifndef XMLMOJANGCALLBACK_H
#define XMLMOJANGCALLBACK_H
// xml reading

using namespace ATG;
/*
class xmlMojangCallback : public ATG::ISAXCallback
{
public:
	virtual HRESULT  StartDocument() { return S_OK; };
	virtual HRESULT  EndDocument() { return S_OK; };

	virtual HRESULT  ElementBegin( CONST WCHAR* strName, UINT NameLen, CONST XMLAttribute *pAttributes, UINT NumAttributes )
	{
		WCHAR wTemp[35] = L"";
		WCHAR wAttName[32] = L"";
		WCHAR wNameXUID[32] = L"";
		WCHAR wNameSkin[32] = L"";
		WCHAR wNameCloak[32] = L"";
		PlayerUID xuid=0LL;


		if (NameLen >31)
			return S_FALSE;
		else
			wcsncpy( wAttName, strName, NameLen);

		if  ( _wcsicmp(wAttName,L"root") == 0)
		{
			return S_OK;
		}
		else if ( _wcsicmp(wAttName,L"data") == 0)
		{
			for(UINT i = 0; i < NumAttributes; i++)
			{
				wcsncpy_s( wAttName, pAttributes[i].strName, pAttributes[i].NameLen);
				if (_wcsicmp(wAttName,L"name")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
						wcsncpy_s( wNameXUID, pAttributes[i].strValue, pAttributes[i].ValueLen);
				}
				else if (_wcsicmp(wAttName,L"xuid")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						ZeroMemory(wTemp,sizeof(WCHAR)*35);
						wcsncpy_s( wTemp, pAttributes[i].strValue, pAttributes[i].ValueLen);
						xuid=_wcstoui64(wTemp,nullptr,10);					
					}
				}
				else if (_wcsicmp(wAttName,L"cape")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wNameCloak, pAttributes[i].strValue, pAttributes[i].ValueLen);
					}
				}
				else if (_wcsicmp(wAttName,L"skin")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wNameSkin, pAttributes[i].strValue, pAttributes[i].ValueLen);
					}
				}

			}

			// if the xuid hasn't been defined, then we can't use the data
			if(xuid!=0LL)
			{
				return CConsoleMinecraftApp::RegisterMojangData(wNameXUID , xuid, wNameSkin, wNameCloak );
			}
			else return S_FALSE;
		}
		else
		{
			return S_FALSE;
		}
	};

	virtual HRESULT  ElementContent( CONST WCHAR *strData, UINT DataLen, BOOL More ) {    return S_OK;   };

	virtual HRESULT  ElementEnd( CONST WCHAR *strName, UINT NameLen ){       return S_OK;    };

	virtual HRESULT  CDATABegin( )  { return S_OK; };

	virtual HRESULT  CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore ){ return S_OK; };

	virtual HRESULT  CDATAEnd( ){ return S_OK; };

	virtual VOID     Error( HRESULT hError, CONST CHAR *strMessage )    {     app.DebugPrintf("Error when Parsing xuids.XML\n");    };

};
*/
class xmlConfigCallback : public ATG::ISAXCallback
{
public:
	virtual HRESULT  StartDocument() { return S_OK; };
	virtual HRESULT  EndDocument() { return S_OK; };

	virtual HRESULT  ElementBegin( CONST WCHAR* strName, UINT NameLen, CONST XMLAttribute *pAttributes, UINT NumAttributes )
	{
		WCHAR wTemp[35] = L"";
		WCHAR wType[32] = L"";
		WCHAR wAttName[32] = L"";
		WCHAR wValue[32] = L"";
		int iValue=-1;

		if (NameLen >31)
			return S_FALSE;
		else
			wcsncpy_s( wAttName, strName, NameLen);

		if  ( _wcsicmp(wAttName,L"root") == 0)
		{
			return S_OK;
		}
		else if ( _wcsicmp(wAttName,L"data") == 0)
		{
			for(UINT i = 0; i < NumAttributes; i++)
			{
				wcsncpy_s( wAttName, pAttributes[i].strName, pAttributes[i].NameLen);
				if (_wcsicmp(wAttName,L"Type")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wType, pAttributes[i].strValue, pAttributes[i].ValueLen);			
					}
				}
				else if(_wcsicmp(wAttName,L"Value")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wValue, pAttributes[i].strValue, pAttributes[i].ValueLen);			

#ifdef _XBOX
						iValue=_wtoi(wValue);
#else
						iValue=wcstol(wValue, nullptr, 10);
#endif
					}
				}
			}
		
			// if the xuid hasn't been defined, then we can't use the data
			if(iValue!=-1)
			{
	#ifdef _DEBUG
				wprintf(L"Type - %s, Value - %d, ",wType, iValue);
	#endif

				return CConsoleMinecraftApp::RegisterConfigValues(wType, iValue );
			}
			else 
			{
				return S_FALSE;
			}
		}
		else
		{
			return S_FALSE;
		}
	}


	virtual HRESULT  ElementContent( CONST WCHAR *strData, UINT DataLen, BOOL More ) {    return S_OK;   };

	virtual HRESULT  ElementEnd( CONST WCHAR *strName, UINT NameLen ){       return S_OK;    };

	virtual HRESULT  CDATABegin( )  { return S_OK; };

	virtual HRESULT  CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore ){ return S_OK; };

	virtual HRESULT  CDATAEnd( ){ return S_OK; };

	virtual VOID     Error( HRESULT hError, CONST CHAR *strMessage )    {     app.DebugPrintf("Error when Parsing xuids.XML\n");    };

};

class xmlDLCInfoCallback : public ATG::ISAXCallback
{
public:
	virtual HRESULT  StartDocument() { return S_OK; };
	virtual HRESULT  EndDocument() { return S_OK; };

	virtual HRESULT  ElementBegin( CONST WCHAR* strName, UINT NameLen, CONST XMLAttribute *pAttributes, UINT NumAttributes )
	{
		WCHAR wTemp[35] = L"";
		WCHAR wAttName[32] = L"";
		WCHAR wNameBanner[32] = L"";
		WCHAR wDataFile[32] = L"";
		WCHAR wType[32] = L"";
		WCHAR wFirstSkin[32] = L"";
		WCHAR wConfig[32] = L"";
		WCHAR wUID[64] = L"";
		WCHAR wName[64] = L"";
		unsigned int uiSortIndex=0L;
		int iGender=0;
		int iConfig=0;

		if (NameLen >31)
			return S_FALSE;
		else
			wcsncpy_s( wAttName, strName, NameLen);

		if  ( _wcsicmp(wAttName,L"root") == 0)
		{
			return S_OK;
		}
		else if ( _wcsicmp(wAttName,L"data") == 0)
		{
			for(UINT i = 0; i < NumAttributes; i++)
			{
				wcsncpy_s( wAttName, pAttributes[i].strName, pAttributes[i].NameLen);
				if (_wcsicmp(wAttName,L"SortIndex")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						ZeroMemory(wTemp,sizeof(WCHAR)*35);
						wcsncpy_s( wTemp, pAttributes[i].strValue, pAttributes[i].ValueLen);
						uiSortIndex=wcstoul(wTemp,nullptr,10);
					}
				}
				else if (_wcsicmp(wAttName,L"Banner")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wNameBanner, pAttributes[i].strValue, pAttributes[i].ValueLen);
					}
				}
				else if (_wcsicmp(wAttName,L"Full")==0)
				{
					if (pAttributes[i].ValueLen <= 64)
					{		
						ZeroMemory(wUID,sizeof(WCHAR)*64);
						wcsncpy_s( wUID, pAttributes[i].strValue, pAttributes[i].ValueLen);
					}
				}
				else if (_wcsicmp(wAttName,L"Name")==0)
				{
					if (pAttributes[i].ValueLen <= 64)
					{		
						ZeroMemory(wName,sizeof(WCHAR)*64);
						wcsncpy_s( wName, pAttributes[i].strValue, pAttributes[i].ValueLen);
					}
				}
				else if (_wcsicmp(wAttName,L"FirstSkin")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wFirstSkin, pAttributes[i].strValue, pAttributes[i].ValueLen);			
					}
				}	
				else if (_wcsicmp(wAttName,L"Type")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wType, pAttributes[i].strValue, pAttributes[i].ValueLen);			
					}
				}
				else if(_wcsicmp(wAttName,L"Config")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wConfig, pAttributes[i].strValue, pAttributes[i].ValueLen);			

#ifdef _XBOX
						iConfig=_wtoi(wConfig);
#else
						iConfig=wcstol(wConfig, nullptr, 10);
#endif
					}
				}
				else if (_wcsicmp(wAttName,L"DataFile")==0)
				{
					if (pAttributes[i].ValueLen <= 32)
					{
						wcsncpy_s( wDataFile, pAttributes[i].strValue, pAttributes[i].ValueLen);			
					}
				}	

			}

			// if the xuid hasn't been defined, then we can't use the data
			if(wUID[0]!=0)
			{
#ifdef _DEBUG
				wprintf(L"Type - %ls, Name - %ls, ",wType, wNameBanner);
#endif

				eDLCContentType eDLCType=e_DLC_NotDefined;
				if(wType[0]!=0)
				{	
					if(wcscmp(wType,L"Skin")==0)
					{
						eDLCType=e_DLC_SkinPack;
					}
					else if(wcscmp(wType,L"MashUpPack")==0)
					{
						eDLCType=e_DLC_MashupPacks;
					}
					else if(wcscmp(wType,L"TexturePack")==0)
					{
						eDLCType=e_DLC_TexturePacks;
					}
				}
				return CConsoleMinecraftApp::RegisterDLCData(eDLCType, wNameBanner , wUID, wName, wFirstSkin, iConfig, uiSortIndex  );
			}
			else 
			{
				return S_FALSE;
			}
		}
		else
		{
			return S_FALSE;
		}
	};

	virtual HRESULT  ElementContent( CONST WCHAR *strData, UINT DataLen, BOOL More ) {    return S_OK;   };

	virtual HRESULT  ElementEnd( CONST WCHAR *strName, UINT NameLen ){       return S_OK;    };

	virtual HRESULT  CDATABegin( )  { return S_OK; };

	virtual HRESULT  CDATAData( CONST WCHAR *strCDATA, UINT CDATALen, BOOL bMore ){ return S_OK; };

	virtual HRESULT  CDATAEnd( ){ return S_OK; };

	virtual VOID     Error( HRESULT hError, CONST CHAR *strMessage )    {     app.DebugPrintf("Error when Parsing DLC.XML\n");    };

};


#endif