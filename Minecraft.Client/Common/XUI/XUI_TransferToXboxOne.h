#pragma once
using namespace std;
//#include <vector>

#include "../Media/xuiscene_TransferToXboxOne.h"


class CXuiCtrl4JList;

class CScene_TransferToXboxOne : public CXuiSceneImpl
{
protected:

	typedef struct
	{
		WCHAR wchSaveTitle[XCONTENT_MAX_DISPLAYNAME_LENGTH];
		unsigned int uiImageLength;
	}
	SLOTDATA;

	CXuiCtrl4JList *m_pSavesSlotList;
	CXuiList m_SavesSlotList;
	CXuiControl m_SavesSlotListTimer;

	XUI_BEGIN_MSG_MAP()
		XUI_ON_XM_INIT( OnInit )
		XUI_ON_XM_DESTROY(OnDestroy)
		XUI_ON_XM_KEYDOWN(OnKeyDown)
		XUI_ON_XM_NOTIFY_PRESS_EX(OnNotifyPressEx)
		XUI_ON_XM_NOTIFY_SELCHANGED(OnNotifySelChanged)
		XUI_ON_XM_NOTIFY_SET_FOCUS(OnNotifySetFocus)
		XUI_ON_XM_NOTIFY_KILL_FOCUS(OnNotifyKillFocus)
		XUI_ON_XM_TRANSITION_START(OnTransitionStart)
		XUI_ON_XM_FONTRENDERERCHANGE_MESSAGE(OnFontRendererChange)

	XUI_END_MSG_MAP()

		BEGIN_CONTROL_MAP()
			MAP_CONTROL(IDC_SavesSlotTimer, m_SavesSlotListTimer)
			MAP_CONTROL(IDC_SavesSlotsList, m_SavesSlotList)
		END_CONTROL_MAP()


		HRESULT OnInit( XUIMessageInit* pInitData, BOOL& bHandled );
		HRESULT OnDestroy();
		HRESULT OnNotifyPressEx(HXUIOBJ hObjPressed, XUINotifyPress* pNotifyPressData,BOOL& rfHandled);
		HRESULT OnKeyDown(XUIMessageInput* pInputData, BOOL& rfHandled);
		HRESULT OnNavReturn(HXUIOBJ hObj,BOOL& rfHandled);
		HRESULT OnNotifySelChanged(HXUIOBJ hObjSource, XUINotifySelChanged *pNotifySelChangedData, BOOL& bHandled);
		HRESULT OnNotifySetFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
		HRESULT OnNotifyKillFocus(HXUIOBJ hObjSource, XUINotifyFocus *pNotifyFocusData, BOOL& bHandled);
		HRESULT OnTimer( XUIMessageTimer *pTimer, BOOL& bHandled );
		HRESULT OnTransitionStart( XUIMessageTransition *pTransition, BOOL& bHandled );
		HRESULT OnFontRendererChange();

public:

	// Define the class. The class name must match the ClassOverride property
	// set for the scene in the UI Authoring tool.
	XUI_IMPLEMENT_CLASS( CScene_TransferToXboxOne, L"CScene_TransferToXboxOne", XUI_CLASS_SCENE )

	static int TMSPPSlotListReturned(LPVOID pParam,int iPad,int iUserData,C4JStorage::PTMSPP_FILEDATA pFileData, LPCSTR szFilename);
	static int TMSPPWriteReturned(LPVOID pParam,int iPad,int iUserData);
	static int TMSPPDeleteReturned(LPVOID pParam,int iPad,int iUserData);
	static int UploadSaveForXboxOneThreadProc( LPVOID lpParameter );
	static int LoadSaveDataReturned(void *pParam,bool bContinue);
private:	
	HRESULT BuildSlotFile(int iIndexBeingUpdated,PBYTE pbImageData,DWORD dwImageBytes);

	bool m_bIgnoreInput;
	bool m_bRetrievingSaveInfo;
	int m_iPad;
	int m_MaxSlotC;
	int m_iX; // tooltip for clearing all slots if there are saves in them
	LoadMenuInitData *m_params;
	XCONTENT_DATA m_XContentData;
	PBYTE m_pbImageData;
	DWORD m_dwImageBytes;
	HXUIBRUSH m_hXuiBrush;
	PBYTE m_pbSlotListFile;
	unsigned int m_uiSlotListFileBytes;
	SLOTDATA *m_pSlotDataA;
	bool m_bWaitingForWrite;
	void *m_pvSaveMem;
	unsigned int m_uiStorageLength;
	bool m_bSaveDataReceived;
	unsigned int m_uiSlotID;
};