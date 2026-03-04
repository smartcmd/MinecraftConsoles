#include "stdafx.h"

#include "PSVita_NPToolkit.h"
#include "PSVita/PSVitaExtras/Conf.h"
#include "PSVita/Network/SonyCommerce_Vita.h"
 
// #define NP_TITLE_ID "CUSA00265_00"
// #define NP_TITLE_SECRET_HEX "c37e30fa1f7fd29e3534834d62781143ae29aa7b51d02320e7aa0b45116ad600e4d309e8431bc37977d98b8db480e721876e7d736e11fd906778c0033bbb6370903477b1dc1e65106afc62007a5feee3158844d721b88c3f4bff2e56417b6910cedfdec78b130d2e0dd35a35a9e2ae31d5889f9398c1d62b52a3630bb03faa5b"
// #define CLIENT_ID_FOR_SAMPLE	"c8c483e7-f0b4-420b-877b-307fcb4c3cdc"

//#define _USE_STANDARD_ALLOC

// sce::Toolkit::NP::Utilities::Future< sce::Toolkit::NP::NpSessionInformation > PSVitaNPToolkit::sm_createJoinFuture;
// sce::Toolkit::NP::NpSessionInformation PSVitaNPToolkit::m_currentSessionInfo;
sce::Toolkit::NP::Utilities::Future<sce::Toolkit::NP::MessageAttachment> PSVitaNPToolkit::m_messageData;


void PSVitaNPToolkit::presenceCallback( const sce::Toolkit::NP::Event& event )
{
	switch(event.event)
	{
	case sce::Toolkit::NP::Event::presenceSet:
		app.DebugPrintf("presenceSet Successfully\n");
		break;
	case sce::Toolkit::NP::Event::presenceSetFailed:
		app.DebugPrintf("presenceSetFailed event received = 0x%x\n", event.returnCode);
		SQRNetworkManager_Vita::SetPresenceFailedCallback();
// 		assert(0);
		break;
	default:
		break;
	}
}

void PSVitaNPToolkit::profileCallback( const sce::Toolkit::NP::Event& event )
{
	switch(event.event)
	{
	case sce::Toolkit::NP::Event::profileError:
		app.DebugPrintf("User profile error: 0x%x\n", event.returnCode);
		break;
	default:
		app.DebugPrintf("User profile event: %i\n", event.event);
		break;
	}
}

void PSVitaNPToolkit::messagingCallback( const sce::Toolkit::NP::Event& event )
{
	switch(event.event)
	{
	case sce::Toolkit::NP::Event::serviceError:
		app.DebugPrintf("NP messagingCallback - serviceError: 0x%x\n", event.returnCode);
		ProfileManager.SetSysUIShowing( false );
		break;
	case sce::Toolkit::NP::Event::messageSent:
		app.DebugPrintf("NP messagingCallback - messageSent: 0x%x\n", event.returnCode);
		ProfileManager.SetSysUIShowing( false );
		break;
	case sce::Toolkit::NP::Event::messageError:
		app.DebugPrintf("NP messagingCallback - messageError: 0x%x\n", event.returnCode);
		if(SQRNetworkManager_Vita::m_bSendingInviteMessage)		// MGH - added to fix a sysUI lockup on startup - devtrack #5883 
			ProfileManager.SetSysUIShowing( false );
		break;
	case sce::Toolkit::NP::Event::messageDialogTerminated:
		app.DebugPrintf("NP messagingCallback - messageDialogTerminated: 0x%x\n", event.returnCode);
		ProfileManager.SetSysUIShowing( false );
		break;
	case sce::Toolkit::NP::Event::messageRetrieved:
		app.DebugPrintf("NP messagingCallback - messageRetrieved: 0x%x\n", event.returnCode);
		if(m_messageData.hasResult())
		{
			SQRNetworkManager_Vita::GetInviteDataAndProcess(m_messageData.get());
		}
		else
		{
			app.DebugPrintf("messageRetrieved error 0x%08x\n", m_messageData.getError());
		}
		break;
	case sce::Toolkit::NP::Event::messageInGameDataReceived:
		app.DebugPrintf("NP messagingCallback - messageInGameDataReceived: 0x%x\n", event.returnCode);
		break;
	case sce::Toolkit::NP::Event::messageInGameDataRetrievalDone:
		app.DebugPrintf("NP messagingCallback - messageInGameDataRetrievalDone: 0x%x\n", event.returnCode);
		break;

	default:
		assert(0);
		break;
	}
}

void PSVitaNPToolkit::coreCallback( const sce::Toolkit::NP::Event& event )
{
	switch (event.event)
	{
	case sce::Toolkit::NP::Event::enetUp:		///< An event from the NetCtl service generated when a connection has been established.
		app.DebugPrintf("Received core callback: Network Up \n");
		break;
	case sce::Toolkit::NP::Event::enetDown:		///< An event from the NetCtl service generated when the connection layer has gone down.
		app.DebugPrintf("Received core callback: Network down \n");
		break;
	case sce::Toolkit::NP::Event::loggedIn:		///< An event from the NetCtl service generated when a connection to the PSN has been established.
 		app.DebugPrintf("Received core callback: PSN sign in \n");
		SceNetCtlInfo info;
		sceNetCtlInetGetInfo(SCE_NET_CTL_INFO_DEVICE, &info);
		if(info.device == SCE_NET_CTL_DEVICE_PHONE) // 3G connection, we're not going to allow this
		{
			ProfileManager.SetNetworkStatus(false, true);
		}
		else
		{
			ProfileManager.SetNetworkStatus(true, true);
		}
		break;
	case sce::Toolkit::NP::Event::loggedOut:	///< An event from the NetCtl service generated when a connection to the PSN has been lost.
 		app.DebugPrintf("Received core callback: PSN sign out \n");
		ProfileManager.SetNetworkStatus(false, true);
		break;
	default:
		app.DebugPrintf("Received core callback: event Num: %d \n", event.event);
		break;
	}
}

void PSVitaNPToolkit::sceNpToolkitCallback( const sce::Toolkit::NP::Event& event)
{
	switch(event.service)
	{
	case sce::Toolkit::NP::ServiceType::core:
		coreCallback(event);
		break;
// 	case sce::Toolkit::NP::ServiceType::netInfo:
// 		Menu::NetInfo::sceNpToolkitCallback(event);
// 		break;
//  	case sce::Toolkit::NP::ServiceType::sessions:
//  		sessionsCallback(event);
//  		break;
// 	case sce::Toolkit::NP::ServiceType::tss:
// 		Menu::Tss::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::ranking:
// 		Menu::Ranking::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::tus:
// 		Menu::Tus::sceNpToolkitCallback(event);
// 		break;
	case sce::Toolkit::NP::ServiceType::profile:
		profileCallback(event);
		break;
	case sce::Toolkit::NP::ServiceType::messaging:
		messagingCallback(event);
// 	case sce::Toolkit::NP::ServiceType::friends:
// 		Menu::Friends::sceNpToolkitCallback(event);
// 		break;
// 	case sce::Toolkit::NP::ServiceType::auth:
// 		Menu::Auth::sceNpToolkitCallback(event);
// 		break;
	case sce::Toolkit::NP::ServiceType::trophy:
// 		ProfileManager.trophySystemCallback(event);
		break;
//  	case sce::Toolkit::NP::ServiceType::messaging:
// 		messagingCallback(event);
// 	case sce::Toolkit::NP::ServiceType::inGameMessage:
// 		Menu::Messaging::sceNpToolkitCallback(event);
// 		break;

 	case sce::Toolkit::NP::ServiceType::commerce:
 		SonyCommerce_Vita::commerce2Handler(event);
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

// 	case sce::Toolkit::NP::ServiceType::gameCustomData:
// 		gameCustomDataCallback(event);
	default:
		break;
	}
}


// 
// void PSVitaNPToolkit::sessionsCallback( const sce::Toolkit::NP::Event& event)
// {
// 	switch(event.event)
// 	{
// 	case sce::Toolkit::NP::Event::npSessionCreateResult:			///< An event generated when the %Np session creation process has been completed.
// 		app.DebugPrintf("npSessionCreateResult");
// 		if(sm_createJoinFuture.hasResult()) 
// 		{
// 			app.DebugPrintf("Session Created Successfully\n");
// 			m_currentSessionInfo = *sm_createJoinFuture.get();
// 		} 
// 		else 
// 		{
// 			app.DebugPrintf("Session Creation Failed 0x%x\n",sm_createJoinFuture.getError());
// 		}
// 		sm_createJoinFuture.reset();
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionJoinResult:				///< An event generated when the join %Np session process has been completed.
// 		app.DebugPrintf("npSessionJoinResult");
// 		if(sm_createJoinFuture.hasResult()) 
// 		{
// 			app.DebugPrintf("Session joined successfully\n");
// 			m_currentSessionInfo = *sm_createJoinFuture.get();
// 		} 
// 		else 
// 		{
// 			app.DebugPrintf("Session join Failed 0x%x\n",sm_createJoinFuture.getError());
// 		}
// 		sm_createJoinFuture.reset();
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionError:					///< An event generated when there was error performing the current %Np session process.
// 		app.DebugPrintf("npSessionError");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionLeaveResult:				///< An event generated when the user has left the current %Np session.
// 		app.DebugPrintf("npSessionLeaveResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionModified:				///< An event generated when the %Np session has been modified.
// 		app.DebugPrintf("npSessionModified");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionUpdateResult:			///< An event generated when the %Np session has been updated. 
// 		app.DebugPrintf("npSessionUpdateResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionGetInfoResult:			///< An event generated when the %Np session info has been retrieved.
// 		app.DebugPrintf("npSessionGetInfoResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionGetInfoListResult:		///< An event generated when the %Np session info has been retrieved.
// 		app.DebugPrintf("npSessionGetInfoListResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionGetSessionDataResult:	///< An event generated when the %Np session data has been retrieved.
// 		app.DebugPrintf("npSessionGetSessionDataResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionSearchResult:			///< An event generated when the %Np session search request has been completed.
// 		app.DebugPrintf("npSessionSearchResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionInviteNotification:		///< An event generated when the %Np session push notification is received.
// 		app.DebugPrintf("npSessionInviteNotification");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionInviteGetInfoResult:		///< An event generated when the %Np session info has been retrieved.
// 		app.DebugPrintf("npSessionInviteGetInfoResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionInviteGetInfoListResult:	///< An event generated when the %Np session info has been retrieved.
// 		app.DebugPrintf("npSessionInviteGetInfoListResult");
// 		break;
// 	case sce::Toolkit::NP::Event::npSessionInviteGetDataResult:		///< An event generated when the %Np session data has been retrieved.
// 		app.DebugPrintf("npSessionInviteGetDataResult");
// 		break;
// 	default:
// 		assert(0);
// 		break;
// 	}
// 
// }

void PSVitaNPToolkit::gameCustomDataCallback( const sce::Toolkit::NP::Event& event)
{
// 	switch(event.event)
// 	{
// 
// 	case sce::Toolkit::NP::Event::gameCustomDataItemListResult:
// 		app.DebugPrintf("gameCustomDataItemListResult");
// 		break;
// 	case sce::Toolkit::NP::Event::gameCustomDataGameDataResult:
// 		app.DebugPrintf("gameCustomDataGameDataResult");
// 		if(m_messageData.hasResult())
// 		{
// 			SQRNetworkManager_Orbis::GetInviteDataAndProcess(m_messageData.get());
// 		}
// 		else
// 		{
// 			app.DebugPrintf("gameCustomDataMessageResult error 0x%08x\n", m_messageData.getError());
// 		}
// 		break;
// 	case sce::Toolkit::NP::Event::gameCustomDataMessageResult:
// 		app.DebugPrintf("gameCustomDataMessageResult");
// 		break;
// 	case sce::Toolkit::NP::Event::gameCustomDataSetUseFlagResult:
// 		app.DebugPrintf("gameCustomDataSetUseFlagResult");
// 		break;
// 	case sce::Toolkit::NP::Event::gameCustomDataGameThumbnailResult:
// 		app.DebugPrintf("gameCustomDataGameThumbnailResult");
// 		break;
// 	case sce::Toolkit::NP::Event::messageError:
// 		app.DebugPrintf("messageError : 0x%08x\n", event.returnCode);
// 		assert(0);
// 		break;
// 	default:
// 		assert(0);
// 		break;
// 	}
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


static void npStateCallback(SceNpServiceState state, int retCode, void *userdata)
{
	//CD - Updates the online status of player
	switch(state)
	{
	case SCE_NP_SERVICE_STATE_SIGNED_OUT:
		ProfileManager.SetNetworkStatus(false, false);
		break;
	case SCE_NP_SERVICE_STATE_SIGNED_IN:
		ProfileManager.SetNetworkStatus(false, true);
		break;
	case SCE_NP_SERVICE_STATE_ONLINE:
		SceNetCtlInfo info;
		sceNetCtlInetGetInfo(SCE_NET_CTL_INFO_DEVICE, &info);
		if(info.device == SCE_NET_CTL_DEVICE_PHONE) // 3G connection, we're not going to allow this
		{
			app.DebugPrintf("Online with 3G connection!!\n");
			ProfileManager.SetNetworkStatus(false, true);
		}
		else
		{
			ProfileManager.SetNetworkStatus(true, true);
		}
		break;
	default:
		break;
	}
}

void PSVitaNPToolkit::init()
{
//	MenuApp menuApp;
// 	sce::Toolkit::NP::NpTitleId nptTitleId;
// 	nptTitleId.setTitleSecret(*SQRNetworkManager_Vita::GetSceNpTitleId(), *SQRNetworkManager_Vita::GetSceNpTitleSecret());
	sce::Toolkit::NP::CommunicationId commsIds(s_npCommunicationId, s_npCommunicationPassphrase, s_npCommunicationSignature);
	sce::Toolkit::NP::Parameters params(sceNpToolkitCallback,commsIds);
 	params.m_title.setId(app.GetCommerceCategory());


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

// 	extern void npStateCallback(SceNpServiceState state, int retCode, void *userdata);

	ret = sceNpRegisterServiceStateCallback(npStateCallback, NULL);
	if (ret < 0) 
	{
		app.DebugPrintf("sceNpRegisterServiceStateCallback() failed. ret = 0x%x\n", ret);
	}




// 	// Register Client ID for Auth
// 	ret = sce::Toolkit::NP::Interface::registerClientId(CLIENT_ID_FOR_SAMPLE);
// 	if (ret < 0) 
// 	{
// 		app.DebugPrintf("Failed to register Auth Client ID : 0x%x\n", ret);
// 		assert(0);
// 	}

}



// void PSVitaNPToolkit::createNPSession()
// {
// #define CURRENT_SESSION_ATTR_NUMS		5
// #define SESSION_IMAGE_PATH				"/app0/orbis/session_image.png"
// #define SESSION_STATUS					"Minecraft online game (this text needs defined and localised)"
// #define SESSION_NAME					"Minecraft(this text needs defined and localised)"
// 
// 	static const int maxSlots = 8;
// 
// 	SceUserServiceUserId userId = SCE_USER_SERVICE_USER_ID_INVALID;
// 	int ret = sceUserServiceGetInitialUser(&userId); 
// 	if( ret < 0 ) 
// 	{
// 		app.DebugPrintf("Couldn't retrieve user ID 0x%x ...\n",ret);
// 	}
// 
// 	sce::Toolkit::NP::CreateNpSessionRequest createSessionRequest;
// 	memset(&createSessionRequest,0,sizeof(createSessionRequest));
// 	strncpy(createSessionRequest.sessionName,SESSION_NAME,strlen(SESSION_NAME));
// 	createSessionRequest.sessionTypeFlag = SCE_TOOLKIT_NP_CREATE_SESSION_TYPE_PUBLIC;
// 	createSessionRequest.maxSlots = maxSlots;
// 	strncpy(createSessionRequest.sessionImgPath,SESSION_IMAGE_PATH,strlen(SESSION_IMAGE_PATH));
// 	strncpy(createSessionRequest.sessionStatus,SESSION_STATUS,strlen(SESSION_STATUS));
// 	createSessionRequest.userInfo.userId = userId;
// 	char test[3] = {'R','K','B'};
// 	createSessionRequest.sessionData= test;
// 	createSessionRequest.sessionDataSize = 3;
// 	ret = sce::Toolkit::NP::Sessions::Interface::create(&createSessionRequest,&sm_createJoinFuture);
// }
// 
// 
// void PSVitaNPToolkit::joinNPSession()
// {
// 	SceUserServiceUserId userId = SCE_USER_SERVICE_USER_ID_INVALID;
// 	int ret = sceUserServiceGetInitialUser(&userId); 
// 	if( ret < 0 ) 
// 	{
// 		app.DebugPrintf("Couldn't retrieve user ID 0x%x ...\n",ret);
// 	}
// 
// 	sce::Toolkit::NP::JoinNpSessionRequest  joinSessionRequest;
// 	memset(&joinSessionRequest,0,sizeof(joinSessionRequest));
// 	// still to sort this out
// 	ORBIS_STUBBED;
// }
// 
// void PSVitaNPToolkit::leaveNPSession()
// {
// 
// }
// 


void PSVitaNPToolkit::getMessageData(SceAppUtilAppEventParam* paramData)
{

	if (SCE_APPUTIL_APPEVENT_TYPE_NP_INVITE_MESSAGE == paramData->type)
	{
		sce::Toolkit::NP::Messaging::Interface::retrieveMessageAttachment(paramData,&m_messageData);
	}
	else if (SCE_APPUTIL_APPEVENT_TYPE_NP_APP_DATA_MESSAGE == paramData->type)
	{
		sce::Toolkit::NP::Messaging::Interface::retrieveMessageAttachment(paramData,&m_messageData);
	}
	else if (SCE_APPUTIL_APPEVENT_TYPE_NP_BASIC_JOINABLE_PRESENCE == paramData->type)
	{
		SceAppUtilNpBasicJoinablePresenceParam joinParam = {0};
		int ret = sceAppUtilAppEventParseNpBasicJoinablePresence(paramData, &joinParam);
		if (ret < 0) 
		{
			app.DebugPrintf("sceAppUtilAppEventParseNpBasicJoinablePresence() failed: 0x%x\n", ret);
		}
		else
		{
			SQRNetworkManager_Vita::GetJoinablePresenceDataAndProcess(&joinParam);
		}

	}
	else
	{
		assert(0);
	}

}