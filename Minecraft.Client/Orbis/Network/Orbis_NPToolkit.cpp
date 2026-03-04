#include "stdafx.h"

#include "Orbis_NPToolkit.h"
#include "Orbis\ps4__np_conf.h"
#include "Orbis/Network/SonyCommerce_Orbis.h"

// #define NP_TITLE_ID "CUSA00265_00"
// #define NP_TITLE_SECRET_HEX "c37e30fa1f7fd29e3534834d62781143ae29aa7b51d02320e7aa0b45116ad600e4d309e8431bc37977d98b8db480e721876e7d736e11fd906778c0033bbb6370903477b1dc1e65106afc62007a5feee3158844d721b88c3f4bff2e56417b6910cedfdec78b130d2e0dd35a35a9e2ae31d5889f9398c1d62b52a3630bb03faa5b"
// #define CLIENT_ID_FOR_SAMPLE	"c8c483e7-f0b4-420b-877b-307fcb4c3cdc"

//#define _USE_STANDARD_ALLOC

// Singleton
OrbisNPToolkit NPToolkit;
sce::Toolkit::NP::Utilities::Future< sce::Toolkit::NP::NpSessionInformation > OrbisNPToolkit::sm_createJoinFuture;
sce::Toolkit::NP::NpSessionInformation OrbisNPToolkit::m_currentSessionInfo;
sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::MessageAttachment> OrbisNPToolkit::m_messageData;


void OrbisNPToolkit::presenceCallback( const sce::Toolkit::NP::Event& event )
{
	switch(event.event)
	{
	case sce::Toolkit::NP::Event::presenceSet:
		app.DebugPrintf("presenceSet Successfully\n");
		break;
	case sce::Toolkit::NP::Event::presenceSetFailed:
		app.DebugPrintf("presenceSetFailed event received = 0x%x\n", event.returnCode);
		SQRNetworkManager_Orbis::SetPresenceFailedCallback();
		break;
	default:
		break;
	}
}

void OrbisNPToolkit::coreCallback( const sce::Toolkit::NP::Event& event )
{
	switch (event.event)
	{
	case sce::Toolkit::NP::Event::enetDown:
		app.DebugPrintf("Online: Received core callback: Network down \n");
		ProfileManager.SetNetworkStatus(false);
		break;
	case sce::Toolkit::NP::Event::enetUp:
		app.DebugPrintf("Online: Received core callback: Network up \n");
		ProfileManager.SetNetworkStatus(true);
		break;
	case sce::Toolkit::NP::Event::loggedIn:
		app.DebugPrintf("Online: Received core callback: PSN sign in \n");
		assert(event.userInformation.state == SCE_NP_STATE_SIGNED_IN);
		ProfileManager.SignedInPSNStateCallback(event.userInformation.userId, event.userInformation.state, &event.userInformation.npId);
		break;
	case sce::Toolkit::NP::Event::loggedOut:
		app.DebugPrintf("Online: Received core callback: PSN sign out \n");
		assert(event.userInformation.state == SCE_NP_STATE_SIGNED_OUT);
		ProfileManager.SignedInPSNStateCallback(event.userInformation.userId, event.userInformation.state, &event.userInformation.npId);
		break;
	default:
		app.DebugPrintf("Online: Received core callback: event Num: %d \n", event.event);
		break;
	}
}

void OrbisNPToolkit::sceNpToolkitCallback( const sce::Toolkit::NP::Event& event)
{
	switch(event.service)
	{
	case sce::Toolkit::NP::ServiceType::core:
		coreCallback(event);
		break;
// 	case sce::Toolkit::NP::ServiceType::netInfo:
// 		Menu::NetInfo::sceNpToolkitCallback(event);
// 		break;
 	case sce::Toolkit::NP::ServiceType::sessions:
 		sessionsCallback(event);
 		break;
// 	case sce::Toolkit::NP::ServiceType::tss:
// 		Menu::Tss::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::ranking:
// 		Menu::Ranking::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::tus:
// 		Menu::Tus::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::profile:
// 		Menu::Profile::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::friends:
// 		Menu::Friends::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::auth:
// 		Menu::Auth::sceNpToolkitCallback(event);
// 		break;
	case sce::Toolkit::NP::ServiceType::trophy:
		ProfileManager.trophySystemCallback(event);
		break;
  	case sce::Toolkit::NP::ServiceType::messaging:
 		messagingCallback(event);
		break;
// 	case sce::Toolkit::NP::ServiceType::inGameMessage:
// 		Menu::Messaging::sceNpToolkitCallback(event);
// 		break;

 	case sce::Toolkit::NP::ServiceType::commerce:
 		SonyCommerce_Orbis::commerce2Handler(event);
 		break;
	case sce::Toolkit::NP::ServiceType::presence:
 		presenceCallback(event);
		break;
// 	case sce::Toolkit::NP::ServiceType::wordFilter:
// 		Menu::WordFilter::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::sns:
// 		Menu::Sns::sceNpToolkitCallback(event);
// 		break;

	case sce::Toolkit::NP::ServiceType::gameCustomData:
		gameCustomDataCallback(event);
	default:
		break;
	}
}



void OrbisNPToolkit::sessionsCallback( const sce::Toolkit::NP::Event& event)
{
	switch(event.event)
	{
	case sce::Toolkit::NP::Event::npSessionCreateResult:			///< An event generated when the %Np session creation process has been completed.
		app.DebugPrintf("npSessionCreateResult");
		if(sm_createJoinFuture.hasResult()) 
		{
			app.DebugPrintf("Session Created Successfully\n");
			m_currentSessionInfo = *sm_createJoinFuture.get();
		} 
		else 
		{
			app.DebugPrintf("Session Creation Failed 0x%x\n",sm_createJoinFuture.getError());
		}
		sm_createJoinFuture.reset();
		break;
	case sce::Toolkit::NP::Event::npSessionJoinResult:				///< An event generated when the join %Np session process has been completed.
		app.DebugPrintf("npSessionJoinResult");
		if(sm_createJoinFuture.hasResult()) 
		{
			app.DebugPrintf("Session joined successfully\n");
			m_currentSessionInfo = *sm_createJoinFuture.get();
		} 
		else 
		{
			app.DebugPrintf("Session join Failed 0x%x\n",sm_createJoinFuture.getError());
		}
		sm_createJoinFuture.reset();
		break;
	case sce::Toolkit::NP::Event::npSessionError:					///< An event generated when there was error performing the current %Np session process.
		app.DebugPrintf("npSessionError");
		break;
	case sce::Toolkit::NP::Event::npSessionLeaveResult:				///< An event generated when the user has left the current %Np session.
		app.DebugPrintf("npSessionLeaveResult");
		break;
	case sce::Toolkit::NP::Event::npSessionModified:				///< An event generated when the %Np session has been modified.
		app.DebugPrintf("npSessionModified");
		break;
	case sce::Toolkit::NP::Event::npSessionUpdateResult:			///< An event generated when the %Np session has been updated. 
		app.DebugPrintf("npSessionUpdateResult");
		break;
	case sce::Toolkit::NP::Event::npSessionGetInfoResult:			///< An event generated when the %Np session info has been retrieved.
		app.DebugPrintf("npSessionGetInfoResult");
		break;
	case sce::Toolkit::NP::Event::npSessionGetInfoListResult:		///< An event generated when the %Np session info has been retrieved.
		app.DebugPrintf("npSessionGetInfoListResult");
		break;
	case sce::Toolkit::NP::Event::npSessionGetSessionDataResult:	///< An event generated when the %Np session data has been retrieved.
		app.DebugPrintf("npSessionGetSessionDataResult");
		break;
	case sce::Toolkit::NP::Event::npSessionSearchResult:			///< An event generated when the %Np session search request has been completed.
		app.DebugPrintf("npSessionSearchResult");
		break;
	case sce::Toolkit::NP::Event::npSessionInviteNotification:		///< An event generated when the %Np session push notification is received.
		app.DebugPrintf("npSessionInviteNotification");
		break;
	case sce::Toolkit::NP::Event::npSessionInviteGetInfoResult:		///< An event generated when the %Np session info has been retrieved.
		app.DebugPrintf("npSessionInviteGetInfoResult");
		break;
	case sce::Toolkit::NP::Event::npSessionInviteGetInfoListResult:	///< An event generated when the %Np session info has been retrieved.
		app.DebugPrintf("npSessionInviteGetInfoListResult");
		break;
	case sce::Toolkit::NP::Event::npSessionInviteGetDataResult:		///< An event generated when the %Np session data has been retrieved.
		app.DebugPrintf("npSessionInviteGetDataResult");
		break;
	default:
		assert(0);
		break;
	}

}

void OrbisNPToolkit::gameCustomDataCallback( const sce::Toolkit::NP::Event& event)
{
	switch(event.event)
	{

	case sce::Toolkit::NP::Event::gameCustomDataItemListResult:
		app.DebugPrintf("gameCustomDataItemListResult");
		break;
	case sce::Toolkit::NP::Event::gameCustomDataGameDataResult:
		app.DebugPrintf("gameCustomDataGameDataResult");
		if(m_messageData.hasResult())
		{
			SQRNetworkManager_Orbis::GetInviteDataAndProcess(m_messageData.get());
		}
		else
		{
			app.DebugPrintf("gameCustomDataMessageResult error 0x%08x\n", m_messageData.getError());

			UINT uiIDA[1] = { IDS_OK };

			switch(m_messageData.getError())
			{
			case SCE_NP_ERROR_LATEST_PATCH_PKG_EXIST:
			case SCE_NP_ERROR_LATEST_PATCH_PKG_DOWNLOADED:
				app.ShowPatchAvailableError();
				break;
			case SCE_NP_ERROR_AGE_RESTRICTION:
			default:
				ui.RequestMessageBox(IDS_ONLINE_SERVICE_TITLE, IDS_CONTENT_RESTRICTION, uiIDA, 1);
				break;
			}
		}
		break;
	case sce::Toolkit::NP::Event::gameCustomDataMessageResult:
		app.DebugPrintf("gameCustomDataMessageResult");
		break;
	case sce::Toolkit::NP::Event::gameCustomDataSetUseFlagResult:
		app.DebugPrintf("gameCustomDataSetUseFlagResult");
		break;
	case sce::Toolkit::NP::Event::gameCustomDataGameThumbnailResult:
		app.DebugPrintf("gameCustomDataGameThumbnailResult");
		break;
	case sce::Toolkit::NP::Event::messageError:
		app.DebugPrintf("messageError : 0x%08x\n", event.returnCode);
		assert(0);
		break;
	default:
		assert(0);
		break;
	}
}

void OrbisNPToolkit::messagingCallback( const sce::Toolkit::NP::Event& event)
{
	switch(event.event)
	{
	case sce::Toolkit::NP::Event::messageSent:						///< An event generated when a message has been sent.
		app.DebugPrintf("sce::Toolkit::NP::Event::messageSent\n");
		SQRNetworkManager_Orbis::s_bInviteDialogRunning = false;
		break;
	case sce::Toolkit::NP::Event::messageError:						///< An event generated when a message failed to be received or sent. 
		app.DebugPrintf("sce::Toolkit::NP::Event::messageError\n");
		SQRNetworkManager_Orbis::s_bInviteDialogRunning = false;
		break;
	case sce::Toolkit::NP::Event::messageDialogTerminated:			///< An event generated when a message dialog box is terminated.
		app.DebugPrintf("sce::Toolkit::NP::Event::messageDialogTerminated\n");
		SQRNetworkManager_Orbis::s_bInviteDialogRunning = false;
		break;
	//case sce::Toolkit::NP::Event::messageRetrieved:					///< An event generated when a message attachment has been retrieved.
	//case sce::Toolkit::NP::Event::messageInGameDataReceived:			///< An event generated when in-game data is received.
	//case sce::Toolkit::NP::Event::messageInGameDataRetrievalDone:		///< An event generated when in-game data retrieval is complete.

	// case sce::Toolkit::NP::Event::messageAttachmentReceived:			///< An event generated when a message with a data attachment has been received.
	// case sce::Toolkit::NP::Event::messageAttachmentOpened:			///< An event generated when a message with a data attachment has been opened (and the sysutil GUI is closed).
	// case sce::Toolkit::NP::Event::messageInviteReceived:				///< An event generated when a message with an invite has been received.
	// case sce::Toolkit::NP::Event::messageInviteAccepted:				///< An event generated when a message with an invite has been accepted via ToolkitNp (and the sysutil GUI is closed).
	}
}

static uint8_t hexCharToUint(char ch)
{
	uint8_t val = 0;

	if ( isdigit(ch) ){
		val  = (ch - '0');
	}
	else if ( isupper(ch) ){
		val  = (ch - 'A' + 10);
	}
	else{
		val  = (ch - 'a' + 10);
	}

	return val;
}

void hexStrToBin(
	const char *pHexStr, 
	uint8_t *pBinBuf, 
	size_t binBufSize
	)
{
	uint8_t val = 0;
	int hexStrLen = strlen(pHexStr);

	int binOffset = 0;
	for (int i = 0; i < hexStrLen; i++) {
		val |= hexCharToUint(*(pHexStr + i));
		if (i % 2 == 0) {
			val <<= 4;
		}
		else {
			if (pBinBuf != NULL && binOffset < binBufSize) {
				memcpy(pBinBuf + binOffset, &val, 1);
				val = 0;
			}
			binOffset++;
		}		
	}

	if (val != 0 && pBinBuf != NULL && binOffset < binBufSize) {
		memcpy(pBinBuf + binOffset, &val, 1);
	}

	return;
}

void OrbisNPToolkit::init()
{
//	MenuApp menuApp;

	sce::Toolkit::NP::NpTitleId nptTitleId;
	nptTitleId.setTitleSecret(*SQRNetworkManager_Orbis::GetSceNpTitleId(), *SQRNetworkManager_Orbis::GetSceNpTitleSecret());
	sce::Toolkit::NP::CommunicationId commsIds(s_npCommunicationId, s_npCommunicationPassphrase, s_npCommunicationSignature);
	sce::Toolkit::NP::Parameters params(sceNpToolkitCallback,nptTitleId);


	int ret = sce::Toolkit::NP::Interface::init(params);
	if (ret != SCE_OK) 
	{
		app.DebugPrintf("Failed to initialize NP Toolkit Library : 0x%x\n", ret);
		assert(0);
	}


	ret = sce::Toolkit::NP::Interface::registerNpCommsId(commsIds, sce::Toolkit::NP::matching);
	if (ret < 0) 
	{
		app.DebugPrintf("Failed to register TSS Comms ID : 0x%x\n", ret);
		assert(0);
	}

	// Get network status and inform library
	sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::NetStateBasic> netStateFuture = sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::NetStateBasic>();
	sce::Toolkit::NP::NetInfo::Interface::getNetInfo(&netStateFuture);

	// Wait for the net state (< 5ms)
	while (netStateFuture.isBusy())
	{
		Sleep(1);
	}

	if (netStateFuture.hasResult())
	{
		sce::Toolkit::NP::NetStateBasic *netState = netStateFuture.get();
		ProfileManager.SetNetworkStatus(netState->connectionStatus == SCE_NET_CTL_STATE_IPOBTAINED);
	}
	else
	{
		// Error message means we're disconnected
		ProfileManager.SetNetworkStatus(false);
	}

// 	// Register Client ID for Auth
// 	ret = sce::Toolkit::NP::Interface::registerClientId(CLIENT_ID_FOR_SAMPLE);
// 	if (ret < 0) 
// 	{
// 		app.DebugPrintf("Failed to register Auth Client ID : 0x%x\n", ret);
// 		assert(0);
// 	}

}



void OrbisNPToolkit::createNPSession()
{
#define CURRENT_SESSION_ATTR_NUMS		5
#define SESSION_IMAGE_PATH				"/app0/orbis/session_image.png"
#define SESSION_STATUS					"Minecraft online game (this text needs defined and localised)"
#define SESSION_NAME					"Minecraft(this text needs defined and localised)"

	static const int maxSlots = 8;

	SceUserServiceUserId userId = SCE_USER_SERVICE_USER_ID_INVALID;
	int ret = sceUserServiceGetInitialUser(&userId); 
	if( ret < 0 ) 
	{
		app.DebugPrintf("Couldn't retrieve user ID 0x%x ...\n",ret);
	}

	sce::Toolkit::NP::CreateNpSessionRequest createSessionRequest;
	memset(&createSessionRequest,0,sizeof(createSessionRequest));
	strncpy(createSessionRequest.sessionName,SESSION_NAME,strlen(SESSION_NAME));
	createSessionRequest.sessionTypeFlag = SCE_TOOLKIT_NP_CREATE_SESSION_TYPE_PUBLIC;
	createSessionRequest.maxSlots = maxSlots;
	strncpy(createSessionRequest.sessionImgPath,SESSION_IMAGE_PATH,strlen(SESSION_IMAGE_PATH));
	strncpy(createSessionRequest.sessionStatus,SESSION_STATUS,strlen(SESSION_STATUS));
	createSessionRequest.userInfo.userId = userId;
	char test[3] = {'R','K','B'};
	createSessionRequest.sessionData= test;
	createSessionRequest.sessionDataSize = 3;
	ret = sce::Toolkit::NP::Sessions::Interface::create(&createSessionRequest,&sm_createJoinFuture);
}


void OrbisNPToolkit::joinNPSession()
{
	SceUserServiceUserId userId = SCE_USER_SERVICE_USER_ID_INVALID;
	int ret = sceUserServiceGetInitialUser(&userId); 
	if( ret < 0 ) 
	{
		app.DebugPrintf("Couldn't retrieve user ID 0x%x ...\n",ret);
	}

	sce::Toolkit::NP::JoinNpSessionRequest  joinSessionRequest;
	memset(&joinSessionRequest,0,sizeof(joinSessionRequest));
	// still to sort this out
	ORBIS_STUBBED;
}

void OrbisNPToolkit::leaveNPSession()
{

}



void OrbisNPToolkit::getMessageData(SceNpGameCustomDataEventParam* paramData)
{

	sce::Toolkit::NP::GameCustomDataGameDataRequest req; 
	SceUserServiceUserId userId = SCE_USER_SERVICE_USER_ID_INVALID;
	int ret = sceUserServiceGetInitialUser(&userId); 
	if( ret < 0 ) {
		//Error handling 
	}

	req.itemId = paramData->itemId;
	req.userInfo.userId = userId;
	ret = sce::Toolkit::NP::GameCustomData::Interface::getGameData(&req, &m_messageData);
	if( ret < 0 ) {
		//Error handling
	} else {
		//Error handling
	}


// 
// 
// 	sce::Toolkit::NP::GameDataRequest req; 
// 	SceUserServiceUserId userId = SCE_USER_SERVICE_USER_ID_INVALID;
// 	int ret = sceUserServiceGetInitialUser(&userId); 
// 	if( ret < 0 ) {
// 		//Error handling 
// 	}
// 
// 	req.itemId = ItemID ;
// 	req.userInfo.userId = userId;
// 	sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::MessageAttachment> actualMessage;
// 	ret = sce::Toolkit::NP::GameCustomData::Interface::getGameData(&req,&actualMessage,false);
// 	if( ret < 0 ) {
// 		//Error handling
// 	} else {
// 		//Error handling
// 	}
// 
// 
// 
// 
// 
// 
// 
// 
// 	app.DebugPrintf("Session Invitation \n");
// 	sce::Toolkit::NP::ReceiveMessageRequest request; 
// 	request.eventParamData = paramData;
// 	request.msgType = SCE_TOOLKIT_NP_MESSAGE_TYPE_CUSTOM_DATA;
// 	sceUserServiceGetInitialUser(&request.userInfo.userId);
// 	sce::Toolkit::NP::Messaging::Interface::retrieveMessageAttachmentFromEvent(&request,&m_messageData);
}