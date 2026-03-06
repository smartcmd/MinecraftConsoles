#include "stdafx.h"
#include "XUI_Ctrl_4JList.h"

static bool TimeSortFn(const void *a, const void *b);

HRESULT CXuiCtrl4JList::OnInit(XUIMessageInit *pInitData, BOOL& bHandled)
{
	InitializeCriticalSection(&m_AccessListData);

	m_hSelectionChangedHandlerObj = NULL;

	return S_OK;
}

void CXuiCtrl4JList::AddData( const LIST_ITEM_INFO& ItemInfo , int iSortListFromIndex, int iSortFunction)
{
	// need to allocate memory for the structure and its strings
	// and remap the string pointers
	DWORD dwBytes=0;
	DWORD dwLen1=0;
	DWORD dwLen2=0;

	if(ItemInfo.pwszText)
	{
		dwLen1=(int)wcslen(ItemInfo.pwszText)*sizeof(WCHAR);
		dwBytes+=dwLen1+sizeof(WCHAR);
	}

	if(ItemInfo.pwszImage)
	{
		dwLen2=(int)(wcslen(ItemInfo.pwszImage))*sizeof(WCHAR);
		dwBytes+=dwLen2+sizeof(WCHAR);
	}

	dwBytes+=sizeof( LIST_ITEM_INFO );
	LIST_ITEM_INFO *pItemInfo = (LIST_ITEM_INFO *)new BYTE[dwBytes];
	ZeroMemory(pItemInfo,dwBytes);

	XMemCpy( pItemInfo, &ItemInfo, sizeof( LIST_ITEM_INFO ) );
	if(dwLen1!=0)
	{
		XMemCpy( &pItemInfo[1], ItemInfo.pwszText, dwLen1 );
		pItemInfo->pwszText=(LPCWSTR)&pItemInfo[1];
		if(dwLen2!=0)
		{
			BYTE *pwszImage = ((BYTE *)&pItemInfo[1])+dwLen1+sizeof(WCHAR);
			XMemCpy( pwszImage, ItemInfo.pwszImage, dwLen2 );
			pItemInfo->pwszImage=(LPCWSTR)pwszImage;
		}
	}
	else if(dwLen2!=0)
	{
		XMemCpy( &pItemInfo[1], ItemInfo.pwszImage, dwLen2 );
		pItemInfo->pwszImage=(LPCWSTR)&pItemInfo[1];
	}

	EnterCriticalSection(&m_AccessListData);

	// need to remember the original index of this addition before it gets sorted - this will get used to load the game
	if(iSortListFromIndex!=-1)
	{
		pItemInfo->iIndex=(int)m_vListData.size()-iSortListFromIndex;
	}
	else
	{
		pItemInfo->iIndex=(int)m_vListData.size();
	}

	// added to force a sort order for DLC
	//pItemInfo->iSortIndex=iSortIndex;

	m_vListData.push_back(pItemInfo);

#ifdef _DEBUG

	int iCount=0;
	for ( auto it : m_vListData )
	{
		PLIST_ITEM_INFO pInfo=(PLIST_ITEM_INFO)*it;
		app.DebugPrintf("%d. ",iCount++);
		OutputDebugStringW(pInfo->pwszText);
		app.DebugPrintf(" - %d\n",pInfo->iSortIndex);

	}
#endif


	if(iSortListFromIndex!=-1)
	{
		switch(iSortFunction)
		{
		case eSortList_Date:
			// sort from the index passed (to leave create world and tutorial in the saves list)
			sort(m_vListData.begin()+iSortListFromIndex, m_vListData.end(),CXuiCtrl4JList::TimeSortFn);
			break;
		case eSortList_Alphabetical:
			// alphabetical sort
			sort(m_vListData.begin()+iSortListFromIndex, m_vListData.end(),CXuiCtrl4JList::AlphabeticSortFn);
			break;
		case eSortList_Index:
			sort(m_vListData.begin()+iSortListFromIndex, m_vListData.end(),CXuiCtrl4JList::IndexSortFn);
			break;
		}
	}
	LeaveCriticalSection(&m_AccessListData);
	InsertItems( 0, 1 );
}

void CXuiCtrl4JList::RemoveAllData( )
{
	EnterCriticalSection(&m_AccessListData);

	int iSize=(int)m_vListData.size();
	for(int i=0;i<iSize;i++)
	{
		LIST_ITEM_INFO *pBack = m_vListData.back();
		if( pBack->hXuiBrush )
		{
			XuiDestroyBrush(pBack->hXuiBrush);
		}
		m_vListData.pop_back();
		DeleteItems( 0, 1 );
	}

	LeaveCriticalSection(&m_AccessListData);
}

void CXuiCtrl4JList::SelectByUserData(int iData)
{
	for(unsigned int i = 0; i < m_vListData.size(); ++i)
	{
		if(m_vListData.at(i)->iData == iData)
		{
			SetCurSel(i);
			SetTopItem(i); // scroll the item into view if it's not visible
			break;
		}
	}
}

int CXuiCtrl4JList::GetIndexByUserData(int iData)
{
	for(unsigned int i = 0; i < m_vListData.size(); ++i)
	{
		if(m_vListData.at(i)->iData == iData)
		{
			return i;
		}
	}
	return 0;
}

CXuiCtrl4JList::LIST_ITEM_INFO&	CXuiCtrl4JList::GetData(DWORD dw)
{
	return *m_vListData[dw];
}

CXuiCtrl4JList::LIST_ITEM_INFO&	CXuiCtrl4JList::GetDataiData(int iData)
{
	LIST_ITEM_INFO info;

	for(unsigned int i=0;i<m_vListData.size();i++)
	{
		info=*m_vListData[i];
		if(info.iData==iData)
		{
			return *m_vListData[i];
		}
	}

	return *m_vListData[0];
}

CXuiCtrl4JList::LIST_ITEM_INFO&	CXuiCtrl4JList::GetData(FILETIME *pFileTime)
{
	LIST_ITEM_INFO info;

	for(unsigned int i=0;i<m_vListData.size();i++)
	{
		info=*m_vListData[i];
		if((info.fTime.dwHighDateTime==pFileTime->dwHighDateTime)&&(info.fTime.dwLowDateTime==pFileTime->dwLowDateTime))
		{
			return *m_vListData[i];
		}
	}

	return *m_vListData[0];
}

bool CXuiCtrl4JList::TimeSortFn(const void *a, const void *b)
{
	CXuiCtrl4JList::LIST_ITEM_INFO *SaveDetailsA=(CXuiCtrl4JList::LIST_ITEM_INFO *)a;
	CXuiCtrl4JList::LIST_ITEM_INFO *SaveDetailsB=(CXuiCtrl4JList::LIST_ITEM_INFO *)b;

	if(SaveDetailsA->fTime.dwHighDateTime > SaveDetailsB->fTime.dwHighDateTime)
	{
		return true;
	}
	else if(SaveDetailsA->fTime.dwHighDateTime == SaveDetailsB->fTime.dwHighDateTime)
	{
		if(SaveDetailsA->fTime.dwLowDateTime > SaveDetailsB->fTime.dwLowDateTime)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}

bool CXuiCtrl4JList::AlphabeticSortFn(const void *a, const void *b)
{
	CXuiCtrl4JList::LIST_ITEM_INFO *SaveDetailsA=(CXuiCtrl4JList::LIST_ITEM_INFO *)a;
	CXuiCtrl4JList::LIST_ITEM_INFO *SaveDetailsB=(CXuiCtrl4JList::LIST_ITEM_INFO *)b;

	wstring wstr1=SaveDetailsA->pwszText;
	wstring wstr2=SaveDetailsB->pwszText;
	if(wstr1.compare(wstr2)<0)
	{
		return true;
	}

	return false;
}

bool CXuiCtrl4JList::IndexSortFn(const void *a, const void *b)
{
	CXuiCtrl4JList::LIST_ITEM_INFO *SaveDetailsA=(CXuiCtrl4JList::LIST_ITEM_INFO *)a;
	CXuiCtrl4JList::LIST_ITEM_INFO *SaveDetailsB=(CXuiCtrl4JList::LIST_ITEM_INFO *)b;

	int iA=SaveDetailsA->iSortIndex;
	int iB=SaveDetailsB->iSortIndex;
	if(iA>iB)
	{
		return true;
	}

	return false;
}

void CXuiCtrl4JList::UpdateGraphic(int iItem,HXUIBRUSH hXuiBrush )
{
	// need to update the one with the matching filetime
	EnterCriticalSection(&m_AccessListData);
	if( GetData(iItem).hXuiBrush )
	{
		XuiDestroyBrush( GetData(iItem).hXuiBrush );
	}
	GetData(iItem).hXuiBrush=hXuiBrush;
	LeaveCriticalSection(&m_AccessListData);
}

void CXuiCtrl4JList::UpdateText(int iItem,LPCWSTR pwszText )
{
	// need to update the one with the matching filetime
	EnterCriticalSection(&m_AccessListData);
	GetData(iItem).pwszText=pwszText;
	LeaveCriticalSection(&m_AccessListData);
}

void CXuiCtrl4JList::UpdateGraphicFromiData(int iData,HXUIBRUSH hXuiBrush )
{
	// need to update the one with the matching iData
	EnterCriticalSection(&m_AccessListData);
	if( GetDataiData(iData).hXuiBrush )
	{
		XuiDestroyBrush( GetDataiData(iData).hXuiBrush );
	}
	GetDataiData(iData).hXuiBrush=hXuiBrush;
	LeaveCriticalSection(&m_AccessListData);
}

void CXuiCtrl4JList::UpdateGraphic(FILETIME *pfTime,HXUIBRUSH hXuiBrush )
{
	// need to update the one with the matching filetime
	EnterCriticalSection(&m_AccessListData);
	if( GetData(pfTime).hXuiBrush )
	{
		XuiDestroyBrush( GetData(pfTime).hXuiBrush );
	}
	GetData(pfTime).hXuiBrush=hXuiBrush;
	LeaveCriticalSection(&m_AccessListData);
}

// Gets called every frame
HRESULT CXuiCtrl4JList::OnGetSourceDataText(XUIMessageGetSourceText *pGetSourceTextData,BOOL& bHandled)
{
	if( ( 0 == pGetSourceTextData->iData ) && ( ( pGetSourceTextData->bItemData ) ) )
	{
		EnterCriticalSection(&m_AccessListData);
		pGetSourceTextData->szText =
			GetData(pGetSourceTextData->iItem).pwszText;
		LeaveCriticalSection(&m_AccessListData);
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CXuiCtrl4JList::OnGetItemCountAll(XUIMessageGetItemCount *pGetItemCountData,BOOL& bHandled)
{
	pGetItemCountData->cItems = (int)m_vListData.size();
	bHandled = TRUE;
	return S_OK;
}

HRESULT CXuiCtrl4JList::OnGetSourceDataImage(XUIMessageGetSourceImage *pGetSourceImageData,BOOL& bHandled)
{
	if( ( 0 == pGetSourceImageData->iData ) && ( pGetSourceImageData->bItemData ) )
	{
		// Check for a brush
		EnterCriticalSection(&m_AccessListData);
		if(GetData(pGetSourceImageData->iItem).hXuiBrush!=NULL)
		{
			pGetSourceImageData->hBrush=GetData(pGetSourceImageData->iItem).hXuiBrush;
		}
		else
		{
			pGetSourceImageData->szPath =
				GetData(pGetSourceImageData->iItem).pwszImage;
		}
		LeaveCriticalSection(&m_AccessListData);
		bHandled = TRUE;
	}
	return S_OK;
}

HRESULT CXuiCtrl4JList::OnGetItemEnable(XUIMessageGetItemEnable *pGetItemEnableData,BOOL& bHandled)
{
	if(m_vListData.size()!=0)
	{
		EnterCriticalSection(&m_AccessListData);
		pGetItemEnableData->bEnabled =
			GetData(pGetItemEnableData->iItem).fEnabled;
		LeaveCriticalSection(&m_AccessListData);
	}
	bHandled = TRUE;
	return S_OK;
}


HRESULT CXuiCtrl4JList::SetBorder(DWORD dw,BOOL bShow)
{
	CXuiControl Control;
	HXUIOBJ hVisual,hBorder;
	GetItemControl(dw,&Control);
	Control.GetVisual(&hVisual);
	XuiElementGetChildById(hVisual,L"Border",&hBorder);
	return XuiElementSetShow(hBorder,bShow);
}

void CXuiCtrl4JList::SetSelectionChangedHandle(HXUIOBJ hObj)
{
	m_hSelectionChangedHandlerObj = hObj;
}

HRESULT CXuiCtrl4JList::OnDestroy()
{
	DeleteCriticalSection(&m_AccessListData);

	if(m_vListData.size()!=0)
	{
		for (unsigned i = 0; i < m_vListData.size(); ++i)
		{
			if( m_vListData[i]->hXuiBrush )
			{
				XuiDestroyBrush( m_vListData[i]->hXuiBrush );
			}
			delete [] (BYTE *)m_vListData[i];
		}
	}
	return S_OK;
}

HRESULT CXuiCtrl4JList::OnNotifySelChanged( HXUIOBJ hObjSource, XUINotifySelChanged* pNotifySelChangedData, BOOL& bHandled )
{
	if(m_hSelectionChangedHandlerObj)
	{
		XUIMessage xuiMsg;
		XUINotify xuiNotify;
		XUINotifySelChanged xuiNotifySel;
		XuiNotifySelChanged( &xuiMsg, &xuiNotify, &xuiNotifySel, hObjSource, pNotifySelChangedData->iItem, pNotifySelChangedData->iOldItem );
		XuiSendMessage( m_hSelectionChangedHandlerObj, &xuiMsg );

		bHandled = xuiMsg.bHandled;
	}
	return S_OK;
}