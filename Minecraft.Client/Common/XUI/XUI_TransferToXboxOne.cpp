#include "stdafx.h"
#include <xuiresource.h>
#include <xuiapp.h>
#include <assert.h>

#include "XUI_Ctrl_4JList.h"
#include "XUI_Ctrl_4JIcon.h"
#include "XUI_LoadSettings.h"
#include "../../ProgressRenderer.h"
#include "XUI_TransferToXboxOne.h"


//----------------------------------------------------------------------------------
// Performs initialization tasks - retrieves controls.
//----------------------------------------------------------------------------------
HRESULT CScene_TransferToXboxOne::OnInit( XUIMessageInit* pInitData, BOOL& bHandled )
{
	m_iX=-1;
	m_params = (LoadMenuInitData *)pInitData->pvInitData;

	m_iPad=m_params->iPad;

	m_bRetrievingSaveInfo=false;
	m_bIgnoreInput=false;
	MapChildControls();

	VOID *pObj;
	XuiObjectFromHandle( m_SavesSlotList, &pObj );
	m_pSavesSlotList = (CXuiCtrl4JList *)pObj;

	m_pbImageData=NULL;
	m_dwImageBytes=0;

	StorageManager.GetSaveCacheFileInfo(m_params->iSaveGameInfoIndex,m_XContentData);
	StorageManager.GetSaveCacheFileInfo(m_params->iSaveGameInfoIndex,&m_pbImageData,&m_dwImageBytes);


	m_SavesSlotListTimer.SetShow(TRUE);
	

	XuiControlSetText(m_SavesSlotList,app.GetString(IDS_XBONE_SELECTSLOT));

	// insert the current save slot names
	m_MaxSlotC=app.m_uiTransferSlotC;
	m_pSlotDataA = new SLOTDATA [m_MaxSlotC];
	ZeroMemory(m_pSlotDataA,sizeof(SLOTDATA)*m_MaxSlotC);


	// saves will be called slot1 to slotx
	// there will be a details file with the names and png of each slot
	m_pbSlotListFile=NULL;
	m_uiSlotListFileBytes=0;

	if(StorageManager.TMSPP_InFileList(C4JStorage::eGlobalStorage_TitleUser,m_iPad,L"XboxOne/SlotList"))
	{
		// there is a slot list file with details of the saves
		C4JStorage::ETMSStatus status=StorageManager.TMSPP_ReadFile(m_iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,"XboxOne/SlotList",&CScene_TransferToXboxOne::TMSPPSlotListReturned,this);
		m_iX=IDS_TOOLTIPS_CLEARSLOTS;
	}
	else
	{
		CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;

		ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));

		// create dummy slots
		for(int i=0;i<m_MaxSlotC;i++)
		{
			memcpy(m_pSlotDataA[i].wchSaveTitle,app.GetString(IDS_XBONE_EMPTYSLOT),sizeof(WCHAR)*XCONTENT_MAX_DISPLAYNAME_LENGTH);
			ListInfo.pwszText = app.GetString(IDS_XBONE_EMPTYSLOT);
			ListInfo.fEnabled = TRUE;
			ListInfo.iData = -1;
			m_pSavesSlotList->AddData(ListInfo);
		}
		m_SavesSlotListTimer.SetShow(FALSE);
	}


	CXuiSceneBase::SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, m_iX, -1,-1,-1,-1,-1,-1,true);

	return S_OK;
}

//----------------------------------------------------------------------------------
// TMSPPSlotListReturned callback
//----------------------------------------------------------------------------------
int CScene_TransferToXboxOne::TMSPPWriteReturned(LPVOID pParam,int iPad,int iUserData)
{
	CScene_TransferToXboxOne* pClass = (CScene_TransferToXboxOne *) pParam;
	pClass->m_bWaitingForWrite=false;

	return 0;
}

//----------------------------------------------------------------------------------
// TMSPPSlotListReturned callback
//----------------------------------------------------------------------------------
int CScene_TransferToXboxOne::TMSPPDeleteReturned(LPVOID pParam,int iPad,int iUserData)
{
	CScene_TransferToXboxOne* pClass = (CScene_TransferToXboxOne *) pParam;
	pClass->m_SavesSlotListTimer.SetShow(FALSE);
	pClass->m_bIgnoreInput=false;

	// update the slots
	delete pClass->m_pbSlotListFile;
	pClass->m_pbSlotListFile=NULL;
	pClass->m_uiSlotListFileBytes=0;
	pClass->m_pSavesSlotList->RemoveAllData();
	CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;

	ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));
	// clear our slot info
	ZeroMemory(pClass->m_pSlotDataA,sizeof(SLOTDATA)*pClass->m_MaxSlotC);

	for(int i=0;i<pClass->m_MaxSlotC;i++)
	{
		memcpy(pClass->m_pSlotDataA[i].wchSaveTitle,app.GetString(IDS_XBONE_EMPTYSLOT),sizeof(WCHAR)*XCONTENT_MAX_DISPLAYNAME_LENGTH);
		ListInfo.pwszText = app.GetString(IDS_XBONE_EMPTYSLOT);
		ListInfo.fEnabled = TRUE;
		ListInfo.iData = -1;
		pClass->m_pSavesSlotList->AddData(ListInfo);
	}

	return 0;
}

//----------------------------------------------------------------------------------
// TMSPPSlotListReturned callback
//----------------------------------------------------------------------------------
int CScene_TransferToXboxOne::TMSPPSlotListReturned(LPVOID pParam,int iPad,int iUserData,C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename)
{
	CScene_TransferToXboxOne* pClass = (CScene_TransferToXboxOne *) pParam;
	unsigned int uiSlotListFileSlots=*((unsigned int *)pFileData->pbData);
	pClass->m_pbSlotListFile=pFileData->pbData;
	pClass->m_uiSlotListFileBytes=pFileData->dwSize;

	// clear our slot info
	ZeroMemory(pClass->m_pSlotDataA,sizeof(SLOTDATA)*pClass->m_MaxSlotC);
	// set the empty slot strings
	for(int i=0;i<pClass->m_MaxSlotC;i++)
	{
		memcpy(pClass->m_pSlotDataA[i].wchSaveTitle,app.GetString(IDS_XBONE_EMPTYSLOT),sizeof(WCHAR)*XCONTENT_MAX_DISPLAYNAME_LENGTH);
	}
	// update our slot info with the data from the file - might have less slots
	unsigned int uiNewSlotsC=(pClass->m_MaxSlotC<uiSlotListFileSlots)?pClass->m_MaxSlotC:uiSlotListFileSlots;
	memcpy(pClass->m_pSlotDataA,pClass->m_pbSlotListFile + sizeof(unsigned int),sizeof(SLOTDATA)*uiNewSlotsC);

	CXuiCtrl4JList::LIST_ITEM_INFO ListInfo;

	ZeroMemory(&ListInfo,sizeof(CXuiCtrl4JList::LIST_ITEM_INFO));
	PBYTE pbImageData=pClass->m_pbSlotListFile +  sizeof(unsigned int) + sizeof(SLOTDATA)*uiSlotListFileSlots;

	// fill out the slot info
	for(int i=0;i<pClass->m_MaxSlotC;i++)
	{
		if(i<uiNewSlotsC)
		{
			ListInfo.pwszText = pClass->m_pSlotDataA[i].wchSaveTitle;
			ListInfo.fEnabled = TRUE;
			ListInfo.iData = -1;
			pClass->m_pSavesSlotList->AddData(ListInfo);

			if(pClass->m_pSlotDataA[i].uiImageLength!=0)
			{
				XuiCreateTextureBrushFromMemory(pbImageData,pClass->m_pSlotDataA[i].uiImageLength,&pClass->m_hXuiBrush);
				pClass->m_pSavesSlotList->UpdateGraphic(i,pClass->m_hXuiBrush);
				// increment the image data pointer
				pbImageData+=pClass->m_pSlotDataA[i].uiImageLength;
			}
		}
		else
		{
			// make it blank
			ListInfo.pwszText = app.GetString(IDS_XBONE_EMPTYSLOT);
			ListInfo.fEnabled = TRUE;
			ListInfo.iData = -1;
			pClass->m_pSavesSlotList->AddData(ListInfo);
		}
	}
	pClass->m_SavesSlotListTimer.SetShow(FALSE);
	return 0;

}

//----------------------------------------------------------------------------------
// Handler for OnDestroy
//----------------------------------------------------------------------------------
HRESULT CScene_TransferToXboxOne::OnDestroy()
{
	return S_OK;
}

//----------------------------------------------------------------------------------
// Handler for the button press message.
//----------------------------------------------------------------------------------
HRESULT CScene_TransferToXboxOne::OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// if we're retrieving save info, ignore key presses
	if(m_bRetrievingSaveInfo)
	{
		return S_OK;
	}

	// This assumes all buttons can only be pressed with the A button
	ui.AnimateKeyPress(pNotifyPressData->UserIndex, VK_PAD_A);

	if(hObjPressed==m_SavesSlotList)
	{
		m_bIgnoreInput=true;

		// update the info in the SlotList file
		CXuiControl pItem;
		int iIndex;

		// get the selected item
		iIndex=m_SavesSlotList.GetCurSel(&pItem);

		// check if there is a save there

		CXuiCtrl4JList::LIST_ITEM_INFO info = m_pSavesSlotList->GetData(iIndex);
		if(info.pwszImage!=NULL)
		{
			// we have a save here
			// Are you sure, etc.
		}

		// update the data 
		memcpy(m_pSlotDataA[iIndex].wchSaveTitle,m_XContentData.szDisplayName,sizeof(WCHAR)*XCONTENT_MAX_DISPLAYNAME_LENGTH);
		m_pSavesSlotList->UpdateText(iIndex,m_pSlotDataA[iIndex].wchSaveTitle);

		// if there is no thumbnail, retrieve the default one from the file. 
		// Don't delete the image data after creating the xuibrush, since we'll use it in the rename of the save
		bool bHostOptionsRead = false;
		unsigned int uiHostOptions = 0;

		XuiCreateTextureBrushFromMemory(m_pbImageData,m_dwImageBytes,&m_hXuiBrush);
		m_pSavesSlotList->UpdateGraphic(iIndex,m_hXuiBrush);

		m_pSlotDataA[iIndex].uiImageLength=m_dwImageBytes;

		m_bIgnoreInput=false;

		// finished so navigate back
		//app.NavigateBack(XUSER_INDEX_ANY);
		BuildSlotFile(iIndex,m_pbImageData,m_dwImageBytes);
	}
	
	return S_OK;
}

HRESULT CScene_TransferToXboxOne::BuildSlotFile(int iIndexBeingUpdated,PBYTE pbImageData,DWORD dwImageBytes )
{
	SLOTDATA *pCurrentSlotData=NULL;
	PBYTE pbCurrentSlotDataPtr=NULL;
	// there may be no slot file yet
	if(m_pbSlotListFile!=NULL)
	{
		pCurrentSlotData=(SLOTDATA *)(m_pbSlotListFile+sizeof(unsigned int));
		pbCurrentSlotDataPtr=m_pbSlotListFile + sizeof(unsigned int) + sizeof(SLOTDATA)*m_MaxSlotC;
	}

	m_uiSlotID=iIndexBeingUpdated;

	// memory required - first int is the number of slots in this file, in case that changes later
	unsigned int uiBytesRequired=sizeof(unsigned int) + sizeof(SLOTDATA)*m_MaxSlotC;
	for(int i=0;i<m_MaxSlotC;i++)
	{
		if(i==iIndexBeingUpdated)
		{
			uiBytesRequired+=dwImageBytes;
		}
		else
		{
			uiBytesRequired+=m_pSlotDataA[i].uiImageLength;
		}
	}

	PBYTE pbNewSlotFileData= new BYTE [uiBytesRequired];
	*((unsigned int *)pbNewSlotFileData)=m_MaxSlotC;
	PBYTE pbNewSlotFileDataSlots=pbNewSlotFileData + sizeof(unsigned int);
	PBYTE pbNewSlotImageDataPtr=pbNewSlotFileData + sizeof(unsigned int) + sizeof(SLOTDATA)*m_MaxSlotC;
	SLOTDATA *pNewSlotData=(SLOTDATA *)pbNewSlotFileDataSlots;

	// copy the slot info into the new memory, just after the first int
	memcpy(pbNewSlotFileDataSlots,m_pSlotDataA,sizeof(SLOTDATA)*m_MaxSlotC);

	for(int i=0;i<m_MaxSlotC;i++)
	{
		if(i==iIndexBeingUpdated)
		{
			memcpy(pbNewSlotImageDataPtr,pbImageData,dwImageBytes);		
			pbNewSlotImageDataPtr+=dwImageBytes;
			// update the fields
			pNewSlotData[i].uiImageLength=dwImageBytes;
			// save title is already correct
		}
		else
		{
			if(pbCurrentSlotDataPtr!=NULL)
			{
				memcpy(pbNewSlotImageDataPtr,pbCurrentSlotDataPtr,pCurrentSlotData[i].uiImageLength);		
				pbNewSlotImageDataPtr+=pCurrentSlotData[i].uiImageLength;
			}
			// fields are already correct
		}

		// move to the next image data in the current slot file
		if(pbCurrentSlotDataPtr!=NULL)
		{
			pbCurrentSlotDataPtr+=pCurrentSlotData[i].uiImageLength;
		}
	}

	// free the previous file data and the current file data
	delete m_pbSlotListFile;

	// reset the pointer until we have the exit from this scene running
	m_pbSlotListFile = pbNewSlotFileData;
	m_uiSlotListFileBytes = uiBytesRequired;
	m_pSlotDataA = (SLOTDATA *)pbNewSlotFileDataSlots;
	
	LoadingInputParams *loadingParams = new LoadingInputParams();
	loadingParams->func = &CScene_TransferToXboxOne::UploadSaveForXboxOneThreadProc;
	loadingParams->lpParam = (LPVOID)this;

	UIFullscreenProgressCompletionData *completionData = new UIFullscreenProgressCompletionData();
	completionData->bShowBackground=TRUE;
	completionData->bShowLogo=TRUE;
	completionData->iPad = m_iPad;
	completionData->type = e_ProgressCompletion_NavigateBackToScene;
	completionData->scene = eUIScene_LoadMenu;
	loadingParams->completionData = completionData;

	app.NavigateToScene(m_iPad,eUIScene_FullscreenProgress, loadingParams);
	
	return S_OK;
}

int CScene_TransferToXboxOne::UploadSaveForXboxOneThreadProc( LPVOID lpParameter )
{
	HRESULT hr = S_OK;
	char szFilename[32];
	CScene_TransferToXboxOne* pClass = (CScene_TransferToXboxOne *) lpParameter;
	Minecraft *pMinecraft = Minecraft::GetInstance();	
	unsigned int uiComplete=0;
	pClass->m_bWaitingForWrite=true;
	pMinecraft->progressRenderer->progressStart(IDS_XBONE_UPLOAD_SAVE_TITLE);
	pMinecraft->progressRenderer->progressStage( IDS_XBONE_UPLOAD_METADATA );
	// now write the new slot data file to global storage, and then write the save data
	C4JStorage::ETMSStatus eStatus=StorageManager.TMSPP_WriteFile(pClass->m_iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,C4JStorage::TMS_UGCTYPE_NONE,"XboxOne/SlotList",(PCHAR) pClass->m_pbSlotListFile, pClass->m_uiSlotListFileBytes,
		&CScene_TransferToXboxOne::TMSPPWriteReturned,lpParameter, 0);

	if(eStatus!=C4JStorage::ETMSStatus_WriteInProgress)
	{
		// failed
		pClass->m_bWaitingForWrite=false;
	}
	else
	{
		// loop waiting for the write to complete
		uiComplete=0;
		while(pClass->m_bWaitingForWrite && (hr == S_OK))
		{
			Sleep(50);
			uiComplete++;
			if(uiComplete>100) uiComplete=100;
	
			pMinecraft->progressRenderer->progressStagePercentage(uiComplete);

			if(app.GetChangingSessionType())
			{
				// 4J Stu - This causes the fullscreenprogress scene to ignore the action it was given
				hr = ERROR_CANCELLED;
			}			
		}

		if(hr!=S_OK) return -1;

		// finish the bar
		for(int i=uiComplete;i<100;i++)
		{
			Sleep(5);
			pMinecraft->progressRenderer->progressStagePercentage(i);
		}


		// now upload the save data
		pMinecraft->progressRenderer->progressStage( IDS_XBONE_UPLOAD_SAVE );

		// write the save file, and increment the progress percentage
		pMinecraft->progressRenderer->progressStagePercentage(25);
		pClass->m_bSaveDataReceived=false;
		C4JStorage::ELoadGameStatus eLoadStatus=StorageManager.LoadSaveData(&pClass->m_XContentData,CScene_TransferToXboxOne::LoadSaveDataReturned,lpParameter);

		// sleep until we have the data
		while(pClass->m_bSaveDataReceived==false)
		{
			Sleep(50);
		}

		// write the save to user TMS

		// break the file up into 256K chunks
		unsigned int uiChunkSize=262144;
		unsigned int uiBytesLeft=pClass->m_uiStorageLength;
		C4JStorage::ETMSStatus eStatus;
		// max file size would be 100*256K
		unsigned int uiPercentageChunk=100/(pClass->m_uiStorageLength/uiChunkSize);
		uiComplete=0;

		if(uiPercentageChunk==0) uiPercentageChunk=1;

		for(int i=0;i<(pClass->m_uiStorageLength/uiChunkSize)+1;i++)
		{
			sprintf( szFilename, "XboxOne/Slot%.2d%.2d", pClass->m_uiSlotID,i );
			PCHAR pchData=((PCHAR)pClass->m_pvSaveMem)+i*uiChunkSize;
		
			pClass->m_bWaitingForWrite=true;
			if(uiBytesLeft>=uiChunkSize)
			{
				eStatus=StorageManager.TMSPP_WriteFile(pClass->m_iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,C4JStorage::TMS_UGCTYPE_NONE,szFilename,pchData, uiChunkSize,
					&CScene_TransferToXboxOne::TMSPPWriteReturned,lpParameter, 0);
				uiBytesLeft-=uiChunkSize;
			}
			else
			{
				// last bit of the file to upload is less than uiChunkSize
				eStatus=StorageManager.TMSPP_WriteFile(pClass->m_iPad,C4JStorage::eGlobalStorage_TitleUser,C4JStorage::TMS_FILETYPE_BINARY,C4JStorage::TMS_UGCTYPE_NONE,szFilename,pchData, uiBytesLeft,
					&CScene_TransferToXboxOne::TMSPPWriteReturned,lpParameter, 0);
			}

			// wait until 
			if(eStatus!=C4JStorage::ETMSStatus_WriteInProgress)
			{
				// failed
				pClass->m_bWaitingForWrite=false;
			}
			else
			{
				// loop waiting for the write to complete
				while(pClass->m_bWaitingForWrite && (hr == S_OK))
				{
					Sleep(50);		
				}
				uiComplete+=uiPercentageChunk;
				if(uiComplete>100) uiComplete=100;

				// update the progress
				pMinecraft->progressRenderer->progressStagePercentage(uiComplete);
			}
		}



		delete pClass->m_pvSaveMem;
	}
	return hr;
}

int CScene_TransferToXboxOne::LoadSaveDataReturned(void *pParam,bool bContinue)
{
	CScene_TransferToXboxOne* pClass = (CScene_TransferToXboxOne*)pParam;

	if(bContinue==true)
	{
		unsigned int uiFileSize = StorageManager.GetSaveSize();
		pClass->m_pvSaveMem = new BYTE [uiFileSize];
		pClass->m_uiStorageLength=0;

		StorageManager.GetSaveData( pClass->m_pvSaveMem, &pClass->m_uiStorageLength );

		pClass->m_bSaveDataReceived=true;
	}

	return 0;
}

HRESULT CScene_TransferToXboxOne::OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled)
{
	if(m_bIgnoreInput) return S_OK;

	// if we're retrieving save info, ignore key presses
	if(m_bRetrievingSaveInfo)
	{
		return S_OK;
	}

	ui.AnimateKeyPress(pInputData->UserIndex, pInputData->dwKeyCode);

	HRESULT hr = S_OK;

	// Explicitly handle B button presses
	switch(pInputData->dwKeyCode)
	{
	case VK_PAD_B:
	case VK_ESCAPE:

		app.NavigateBack(XUSER_INDEX_ANY);
		rfHandled = TRUE;
		break;	
	case VK_PAD_X:
		// wipe the save slots
		if(m_pbSlotListFile!=NULL)
		{
			m_SavesSlotListTimer.SetShow(TRUE);
			m_bIgnoreInput=true;

			C4JStorage::ETMSStatus eStatus=StorageManager.TMSPP_DeleteFile(m_iPad,"XboxOne/SlotList",C4JStorage::TMS_FILETYPE_BINARY,&CScene_TransferToXboxOne::TMSPPDeleteReturned,this, 0);

		}

		break;
	}
	
	return hr;
}

HRESULT CScene_TransferToXboxOne::OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled)
{

	//if(m_bReady)
	{
		CXuiSceneBase::PlayUISFX(eSFX_Focus);
	}

	return S_OK;
}


HRESULT CScene_TransferToXboxOne::OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled )
{
	//if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ) return S_OK;

	if(pTransition->dwTransAction==XUI_TRANSITION_ACTION_DESTROY ||
		pTransition->dwTransType == XUI_TRANSITION_FROM  || pTransition->dwTransType == XUI_TRANSITION_BACKFROM)
	{
		// 4J Stu - We may have had to unload our font renderer in this scene if one of the save files
		// uses characters not in our font (eg asian chars) so restore our font renderer
		// This will not do anything if our font renderer is already loaded
		app.OverrideFontRenderer(true,true);
	}

	return S_OK;
}

HRESULT CScene_TransferToXboxOne::OnFontRendererChange()
{
	// update the tooltips
	CXuiSceneBase::SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, m_iX, -1,-1,-1,-1,-1,-1,true);
	
	return S_OK;
}

HRESULT CScene_TransferToXboxOne::OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	// update the tooltips
	CXuiSceneBase::SetTooltips( DEFAULT_XUI_MENU_USER, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, m_iX, -1,-1,-1,-1,-1,-1,true);

	return S_OK;
}

HRESULT CScene_TransferToXboxOne::OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled)
{
	return S_OK;
}


