#pragma once

#define XM_SPLITSCREENPLAYER_MESSAGE		XM_USER
#define XM_FONTRENDERERCHANGE_MESSAGE		XM_USER + 1
#define XM_DLCMOUNTED_MESSAGE				XM_USER + 2
#define XM_BASE_POSITION_CHANGED_MESSAGE	XM_USER + 3
#define XM_DLCSINSTALLED_MESSAGE			XM_USER + 4
#define XM_INVENTORYUPDATED_MESSAGE			XM_USER + 5
#define XM_TMS_DLCFILE_RETRIEVED_MESSAGE	XM_USER + 6
#define XM_TMS_BANFILE_RETRIEVED_MESSAGE	XM_USER + 7
#define XM_TMS_ALLFILES_RETRIEVED_MESSAGE	XM_USER + 8	
#define XM_CUSTOMTICKSCENE_MESSAGE			XM_USER + 9
#define XM_GETSLOTITEM_MESSAGE				XM_USER + 10

typedef struct
{
	shared_ptr<ItemInstance> item;

	// Legacy values for compatibility
	int iDataBitField;
	int iItemBitField;
	LPCWSTR szPath;
	BOOL bDirty;
}
CustomMessage_GetSlotItem_Struct;


// Define the prototype for your handler function
// Sig: HRESULT OnCustomMessage_GetSlotItem(CustomMessage_GetSlotItem_Struct *pData, BOOL& bHandled)

// Define the message map macro
#define XUI_ON_XM_GETSLOTITEM_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_GETSLOTITEM_MESSAGE)\
{\
	CustomMessage_GetSlotItem_Struct *pData = (CustomMessage_GetSlotItem_Struct *) pMessage->pvData;\
	return MemberFunc(pData, pMessage->bHandled);\
}

static __declspec(noinline) void CustomMessage_GetSlotItem(XUIMessage *pMsg, CustomMessage_GetSlotItem_Struct* pData, int iDataBitField, int iItemBitField)
{
	XuiMessage(pMsg,XM_GETSLOTITEM_MESSAGE);
	_XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->item = nullptr;
	pData->iDataBitField = iDataBitField;
	pData->iItemBitField = iItemBitField;
	pData->szPath = NULL;
	pData->bDirty = false;
}

typedef struct
{
	bool bJoining; // if you're not joining, your leaving
}
CustomMessage_Splitscreenplayer_Struct;


// Define the prototype for your handler function
// Sig: HRESULT OnCustomMessage_Splitscreenplayer(bool bJoining, BOOL& bHandled)

// Define the message map macro
#define XUI_ON_XM_SPLITSCREENPLAYER_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_SPLITSCREENPLAYER_MESSAGE)\
{\
	CustomMessage_Splitscreenplayer_Struct *pData = (CustomMessage_Splitscreenplayer_Struct *) pMessage->pvData;\
	return MemberFunc(pData->bJoining, pMessage->bHandled);\
}

static __declspec(noinline) void CustomMessage_Splitscreenplayer(XUIMessage *pMsg, CustomMessage_Splitscreenplayer_Struct* pData, bool bJoining)
{
	XuiMessage(pMsg,XM_SPLITSCREENPLAYER_MESSAGE);
	_XuiMessageExtra(pMsg,(XUIMessageData*) pData, sizeof(*pData));
	pData->bJoining = bJoining;
}

// Define the prototype for your handler function
// Sig: HRESULT OnFontRendererChange()

// Define the message map macro
#define XUI_ON_XM_FONTRENDERERCHANGE_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_FONTRENDERERCHANGE_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_FontRendererChange(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_FONTRENDERERCHANGE_MESSAGE);
}

// Define the prototype for your handler function
// Sig: HRESULT OnDLCMounted()

// Define the message map macro
#define XUI_ON_XM_DLCLOADED_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_DLCMOUNTED_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_DLCMountingComplete(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_DLCMOUNTED_MESSAGE);
}

// Define the prototype for your handler function
// Sig: HRESULT OnBasePositionChanged()

// Define the message map macro
#define XUI_ON_XM_BASE_POSITION_CHANGED_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_BASE_POSITION_CHANGED_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_BasePositionChanged(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_BASE_POSITION_CHANGED_MESSAGE);
}

// the prototype for your handler function
// Sig: HRESULT OnDLCInstalled()

// Define the message map macro
#define XUI_ON_XM_DLCINSTALLED_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_DLCSINSTALLED_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_DLCInstalled(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_DLCSINSTALLED_MESSAGE);
}

// the prototype for your handler function
// Sig: HRESULT OnCustomMessage_InventoryUpdated()

// Define the message map macro
#define XUI_ON_XM_INVENTORYUPDATED_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_INVENTORYUPDATED_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_InventoryUpdated(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_INVENTORYUPDATED_MESSAGE);
}

// the prototype for your handler function
// Sig: HRESULT OnCustomMessage_()

// Define the message map macro
#define XUI_ON_XM_TMS_DLCFILE_RETRIEVED_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_TMS_DLCFILE_RETRIEVED_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_TMS_DLCFileRetrieved(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_TMS_DLCFILE_RETRIEVED_MESSAGE);
}

// the prototype for your handler function
// Sig: HRESULT OnCustomMessage_()

// Define the message map macro
#define XUI_ON_XM_TMS_BANFILE_RETRIEVED_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_TMS_BANFILE_RETRIEVED_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_TMS_BanFileRetrieved(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_TMS_BANFILE_RETRIEVED_MESSAGE);
}

// Define the prototype for your handler function
// Sig: HRESULT OnCustomMessage_TickScene()

// Define the message map macro
#define XUI_ON_XM_CUSTOMTICKSCENE_MESSAGE(MemberFunc)\
	if (pMessage->dwMessage == XM_CUSTOMTICKSCENE_MESSAGE)\
{\
	return MemberFunc();\
}

static __declspec(noinline) void CustomMessage_TickScene(XUIMessage *pMsg)
{
	XuiMessage(pMsg,XM_CUSTOMTICKSCENE_MESSAGE);
}
