//
// Class to handle and manage integration with social networks.
// 4J Studios Ltd, 2011.
// Andy West
//


#include "stdafx.h"
#include "SocialManager.h"
#include <assert.h>
#include <xgraphics.h>
#include "../Sentient/SentientManager.h"

///////////////////////////////////////////////////////////////////////////
// Constants.
///////////////////////////////////////////////////////////////////////////

// Table of SDK IDs for the social networks.
static const DWORD		kaSocialNetworkIDS[ eNumSocialNetworks ] =
{
	2,					// eFacebook
};

// WESTY : Not sure if we even need to get social access key!
/*
// Table of social network permissions that we will required to post.
static const LPCWSTR	kaSocialNetworkRequiredPermissions[ eNumSocialNetworks ] = 
{
	L"publish_stream",		// eFacebook
};


// Default size to use for social network access key text.
static const DWORD		kDefaultAccessKeyTextSize = 64;
*/


///////////////////////////////////////////////////////////////////////////
// Static members intialisation.
///////////////////////////////////////////////////////////////////////////

// Initialise static instance pointer.
CSocialManager* CSocialManager::m_pInstance = 0;




///////////////////////////////////////////////////////////////////////////
// Class functions.
///////////////////////////////////////////////////////////////////////////

// Get singleton instance.
CSocialManager* CSocialManager::Instance()
{
	if ( m_pInstance == 0 )			// Is this the first time?
	{
		m_pInstance = new CSocialManager;		// Create the singleton instance.
	}
	return m_pInstance;
}

// Default constructor.
CSocialManager::CSocialManager()
{
	m_eCurrState = eStateUnitialised;
	m_dwSocialPostingCapability = 0;

// WESTY : Not sure if we even need to get social access key!
/*
	m_pAccessKeyText = NULL; 
	m_dwAccessKeyTextSize = 0;
*/

	m_pMainImageBuffer = NULL;
	m_dwMainImageBufferSize = 0;

	m_PostPreviewImage.pBytes = NULL;
	m_dwCurrRequestUser = -1;

	ZeroMemory(m_wchTitleA,sizeof(WCHAR)*MAX_SOCIALPOST_CAPTION);
	ZeroMemory(m_wchCaptionA,sizeof(WCHAR)*MAX_SOCIALPOST_CAPTION);
	ZeroMemory(m_wchDescA,sizeof(WCHAR)*MAX_SOCIALPOST_DESC);

}


// To be called once during game init.
void CSocialManager::Initialise()
{
	assert( m_eCurrState == eStateUnitialised );
	m_eCurrState = eStateReady;

}


void CSocialManager::Tick()
{
	switch( m_eCurrState )
	{
		case eStateUnitialised:
			// Nothing to do.
			break;

		case eStateReady:
			// Nothing to do.
			break;

		case eStateGetPostingCapability:
			{
				// Poll for result.
				DWORD dwResult = XGetOverlappedResult( &( m_Overlapped ), &( m_dwOverlappedResultCode ), false );

				switch( dwResult )
				{
					case ERROR_SUCCESS:
						app.DebugPrintf( "XSocialGetCapabilities - SUCCESS - FLAGS = %d\n", m_dwSocialPostingCapability );
						m_eCurrState = eStateReady;
						break;

					case ERROR_IO_INCOMPLETE:
						// Still on going.
						break;

					default:
						// Some kind of error occured.
						app.DebugPrintf( "XSocialGetCapabilities - FAILED\n", m_dwSocialPostingCapability );
						// WESTY: TO DO: Get error code, and possibly extened error code to figure out what went wrong.
						//assert( false );
						m_eCurrState = eStateReady;
						break;
				}
			}
			break;
		

		case eStatePostingImage:
			{ 
				// Poll for result.
				DWORD dwResult = XGetOverlappedResult( &( m_Overlapped ), &( m_dwOverlappedResultCode ), false );

				switch( dwResult )
				{
					case ERROR_SUCCESS:
						TelemetryManager->RecordMediaShareUpload(m_dwCurrRequestUser, ESen_MediaDestination_Facebook, eSen_MediaType_Picture);

						app.DebugPrintf( "Facebook image post success!!!\n" );
						DestroyMainPostImage();
						DestroyPreviewPostImage();

						// Award for successful post.
						ProfileManager.Award( m_dwCurrRequestUser, eAward_socialPost );
						
						m_eCurrState = eStateReady;
						break;

					case ERROR_IO_INCOMPLETE:
						// Still on going.
						break;

					default:
						// For some reason, the operation did not complete.
						// SDK docs give no indication why this may be, but could be user cancelled the operation.
						app.DebugPrintf( "Facebook image post failure!!!\n" );
						DestroyMainPostImage();
						DestroyPreviewPostImage();
						m_eCurrState = eStateReady;
						break;
				}
			}
			break;

		case eStatePostingLink:
			{ 
				// Poll for result.
				DWORD dwResult = XGetOverlappedResult( &( m_Overlapped ), &( m_dwOverlappedResultCode ), false );

				switch( dwResult )
				{
					case ERROR_SUCCESS:
						app.DebugPrintf( "Facebook link post success!!!\n" );

						// Award for successful post.
						ProfileManager.Award( m_dwCurrRequestUser, eAward_socialPost );

						m_eCurrState = eStateReady;
						break;

					case ERROR_IO_INCOMPLETE:
						// Still on going.
						break;

					default:
						// For some reason, the operation did not complete.
						// SDK docs give no indication why this may be, but could be user cancelled the operation.
						app.DebugPrintf( "Facebook link post failure!!!\n" );
						m_eCurrState = eStateReady;
						break;
				}
			}
			break;

		default:
			// Unhandled state!
			assert( false );
			break;
	}
}


// Find out if we are allowed to post to social networks.
bool CSocialManager::RefreshPostingCapability()
{
	bool bResult = false;

	if ( m_eCurrState != eStateReady )
	{
		return bResult;
	}

	// Test for titles ability to post to social networks.

	// Test for titles ability to post to social networks.

	XMemSet( &m_Overlapped, 0, sizeof(XOVERLAPPED) );

	DWORD dwResult = XSocialGetCapabilities( &( m_dwSocialPostingCapability ), &( m_Overlapped ) );
	
	switch( dwResult )
	{
		case ERROR_IO_PENDING:
			bResult = true;
			m_eCurrState = eStateGetPostingCapability;
			break;

		case ERROR_FUNCTION_FAILED:
			{
				// Hmm ... why would it fail, and what are we meant to do about it?
				DWORD dwExtendedError = XGetOverlappedExtendedError( &( m_Overlapped ) );
				app.DebugPrintf( "XSocialGetCapabilities ASYNC : FAILED : Extended Error %d\n", dwExtendedError );
				//assert( false );
			}
			break;

		default:
			{
				// Should never get in here.
				DWORD dwExtendedError = XGetOverlappedExtendedError( &( m_Overlapped ) );
				app.DebugPrintf( "XSocialGetCapabilities ASYNC : FAILED : Extended Error %d\n", dwExtendedError );
				//assert( false );
			}
			break;
	}

	return bResult;
}



bool CSocialManager::IsTitleAllowedToPostAnything()
{
	assert( m_eCurrState != eStateUnitialised );

	return( m_dwSocialPostingCapability != 0 );
}


bool CSocialManager::IsTitleAllowedToPostImages()
{
	assert( m_eCurrState != eStateUnitialised );

	return( ( m_dwSocialPostingCapability & XSOCIAL_CAPABILITY_POSTIMAGE ) != 0 );
}


bool CSocialManager::IsTitleAllowedToPostLinks()
{
	assert( m_eCurrState != eStateUnitialised );

	return( ( m_dwSocialPostingCapability & XSOCIAL_CAPABILITY_POSTLINK ) != 0 );
}

bool CSocialManager::AreAllUsersAllowedToPostImages()
{
	// catch no user being signed in to LIVE here too (or a local disconnect)
	BOOL bUserAllowedToPost=FALSE;

	for(int i=0;i<XUSER_MAX_COUNT;i++)
	{
		if(ProfileManager.IsSignedInLive(i)) 
		{
			XUserCheckPrivilege(i,XPRIVILEGE_SOCIAL_NETWORK_SHARING ,&bUserAllowedToPost);
			if(bUserAllowedToPost==FALSE) 
			{
				return false;
			}
		}
	}
	return bUserAllowedToPost?true:false;
}



// Ensure this is only done after posting is complete.
void CSocialManager::DestroyMainPostImage()
{
	delete [] m_PostImageParams.pFullImage;
	m_PostImageParams.pFullImage=NULL;
	m_dwMainImageBufferSize=0;
 }

void CSocialManager::DestroyPreviewPostImage()
{
	XPhysicalFree( (void *)m_PostPreviewImage.pBytes );
	m_PostPreviewImage.pBytes = NULL;
}

void CSocialManager::SetSocialPostText(LPCWSTR pwchTitle, LPCWSTR pwchCaption, LPCWSTR pwchDesc)
{
	wstring wTemp;

	ZeroMemory(m_wchTitleA,sizeof(WCHAR)*(MAX_SOCIALPOST_CAPTION+1));
	ZeroMemory(m_wchCaptionA,sizeof(WCHAR)*(MAX_SOCIALPOST_CAPTION+1));
	ZeroMemory(m_wchDescA,sizeof(WCHAR)*(MAX_SOCIALPOST_DESC+1));
	wTemp = pwchTitle;
	memcpy(m_wchTitleA,wTemp.c_str(),wTemp.length()*sizeof(WCHAR));
	wTemp = pwchCaption;
	memcpy(m_wchCaptionA,wTemp.c_str(),wTemp.length()*sizeof(WCHAR));
	wTemp = pwchDesc;
	memcpy(m_wchDescA,wTemp.c_str(),wTemp.length()*sizeof(WCHAR));
}

bool CSocialManager::PostLinkToSocialNetwork( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect )
{
	if ( m_eCurrState != eStateReady )
	{
		return false;
	}

	// Ensure title is actually allowed to post links ... should always be the case, but we have to check for TCR.
	if ( IsTitleAllowedToPostLinks() == false )
	{
		//assert( false );
		return false;
	}
	
	// Ensure dwUserIndex is sensible.
	if ( dwUserIndex > 3 )
	{
		//assert( false );
		return false;
	}

	// Store user index for award on successful post.
	m_dwCurrRequestUser = dwUserIndex;

	// Required for kinect only.
	DWORD dwUserTrackingIndex = 0;
	if ( bUsingKinect )
	{
		// WESTY - Will need a method to get this.
		dwUserTrackingIndex = 0;
	}

	bool bResult = false;

//	CreatePreviewImage();

	// SDK social post flags....
	// XSOCIAL_POST_USERGENERATEDCONTENT
	// XSOCIAL_POST_KINECTCONTENT
	// XSOCIAL_POST_GAMECONTENT
	// XSOCIAL_POST_ACHIEVEMENTCONTENT
	// XSOCIAL_POST_MEDIACONTENT

	// 4J Stu - MessageText is not a member of the struct in the header of the latest XDK 20871.6
	// The documentation hasn't changed though...
	//m_PostLinkParams.MessageText = L"This is a test link post.";
	m_PostLinkParams.TitleText = L"Test Title";
	m_PostLinkParams.TitleURL = L"http:\\www.4jstudios.com";
	m_PostLinkParams.PictureCaption = L"Test Picture Caption";
	m_PostLinkParams.PictureDescription = L"Test Picture Description";
	m_PostLinkParams.PictureURL = L"http:\\www.4jstudios.com";
	m_PostLinkParams.PreviewImage = m_PostPreviewImage;
	m_PostLinkParams.Flags = ( XSOCIAL_POST_GAMECONTENT | XSOCIAL_POST_USERGENERATEDCONTENT );	// WESTY: Need to set correct flags for content type.
	m_PostLinkParams.Size = sizeof( m_PostLinkParams );
	

	// BEGIN: Asyncronous function call version
	DWORD dwResult;

	XMemSet( &m_Overlapped, 0, sizeof(XOVERLAPPED) );

	if ( bUsingKinect )
	{
		dwResult = XShowNuiSocialNetworkLinkPostUI( dwUserTrackingIndex, dwUserIndex, &( m_PostLinkParams ), &( m_Overlapped ) );
	}
	else // using normal controller UI
	{
		dwResult = XShowSocialNetworkLinkPostUI( dwUserIndex, &( m_PostLinkParams ), &( m_Overlapped ) );
	}
	
	switch( dwResult )
	{
		case ERROR_IO_PENDING:
			bResult = true;
			m_eCurrState = eStatePostingLink;
			break;

		case ERROR_FUNCTION_FAILED:
			// Hmm ... why would it fail, and what are we meant to do about it? Get extended error code!?
			DestroyMainPostImage();
			DestroyPreviewPostImage();
			//assert( false );
			break;

		default:
			// Should never get in here.
			DestroyMainPostImage();
			DestroyPreviewPostImage();
			assert( false );
			break;
	}
	// END: Asyncronous function call version
	
	return bResult;
}

bool CSocialManager::PostImageToSocialNetwork( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect )
{
	if ( m_eCurrState != eStateReady )
	{
		return false;
	}

	// Ensure title is actually allowed to post images ... should always be the case, but we have to check for TCR.
	if ( IsTitleAllowedToPostImages() == false )
	{
		//assert( false );
		return false;
	}
	
	// Ensure dwUserIndex is sensible.
	if ( dwUserIndex > 3 )
	{
		//assert( false );
		return false;
	}

	// Store user index for award on successful post.
	m_dwCurrRequestUser = dwUserIndex;

	// Required for kinect only.
	DWORD dwUserTrackingIndex = 0;
	if ( bUsingKinect )
	{
		// WESTY - Will need a method to get this.
		dwUserTrackingIndex = 0;
	}

	bool bResult = false;

	PBYTE pbData=NULL;
	DWORD dwDataSize;

	app.GetScreenshot(dwUserIndex,&pbData,&dwDataSize);
	app.GetPreviewImage(dwUserIndex,&m_PostPreviewImage);

	// SDK social post flags....
	// XSOCIAL_POST_USERGENERATEDCONTENT
	// XSOCIAL_POST_KINECTCONTENT
	// XSOCIAL_POST_GAMECONTENT
	// XSOCIAL_POST_ACHIEVEMENTCONTENT
	// XSOCIAL_POST_MEDIACONTENT

	// 4J Stu - MessageText is not a member of the struct in the header of the latest XDK 20871.6
	// The documentation hasn't changed though...
	//m_PostImageParams.MessageText = L"This is a test image post.";
	m_PostImageParams.TitleText = m_wchTitleA;
	m_PostImageParams.PictureCaption = m_wchCaptionA;
	m_PostImageParams.PictureDescription = m_wchDescA;
	m_PostImageParams.PreviewImage = m_PostPreviewImage;
	m_PostImageParams.Flags = ( XSOCIAL_POST_GAMECONTENT | XSOCIAL_POST_USERGENERATEDCONTENT );	// WESTY: Need to set correct flags for content type.
	m_PostImageParams.pFullImage = pbData;//m_pMainImageBuffer;
	m_PostImageParams.FullImageByteCount = dwDataSize;//m_dwMainImageBufferSize;
	m_PostImageParams.Size = sizeof( m_PostImageParams );

	// BEGIN: Asyncronous function call version
	DWORD dwResult;

	XMemSet( &m_Overlapped, 0, sizeof(XOVERLAPPED) );

	if ( bUsingKinect )
	{
		dwResult = XShowNuiSocialNetworkImagePostUI( dwUserTrackingIndex, dwUserIndex, &( m_PostImageParams ), &( m_Overlapped ) );
	}
	else // using normal controller UI
	{
		dwResult = XShowSocialNetworkImagePostUI( dwUserIndex, &( m_PostImageParams ), &( m_Overlapped ) );
	}
	
	switch( dwResult )
	{
		case ERROR_IO_PENDING:
			bResult = true;
			m_eCurrState = eStatePostingImage;
			break;

		case ERROR_FUNCTION_FAILED:
			// Hmm ... why would it fail, and what are we meant to do about it? Get extended error code!?
			DestroyMainPostImage();
			DestroyPreviewPostImage();
			//assert( false );
			break;

		default:
			// Should never get in here.
			DestroyMainPostImage();
			DestroyPreviewPostImage();
			assert( false );
			break;
	}
	// END: Asyncronous function call version

	
	return bResult;
}

// WESTY : Not sure if we even need to get social access key!
/*
// It is unclear from the sdk docs what this is actually used for.
// So far we have only created a syncronous version of this function, and the xbox functionality does not seem to work yet!
bool CSocialManager::GetSocialNetworkAccessKey( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect, DWORD dwUserTrackingIndex, bool bShowNetworkSignin )
{
	bool bReturn = false;

	DWORD dwResult;

	// Ensure that we free any previously used access key buffer.
	if ( m_pAccessKeyText != NULL )
	{
		delete [] m_pAccessKeyText;
		m_pAccessKeyText = NULL;
	}

	// Get social network ID and permissions.
	DWORD dwSocialNetworkID = kaSocialNetworkIDS[ eSocialNetwork ];
	LPCWSTR	pRequiredPermissions = kaSocialNetworkRequiredPermissions[ eSocialNetwork ]; 

	// Allocate buffer for social network access key text.
	m_dwAccessKeyTextSize = kDefaultAccessKeyTextSize;
	m_pAccessKeyText = new wchar_t [ m_dwAccessKeyTextSize ];

	if ( bShowNetworkSignin )
	{
		if ( bUsingKinect )
		{
			dwResult = XShowNuiSocialGetUserToken( dwUserTrackingIndex, dwUserIndex, dwSocialNetworkID, pRequiredPermissions, m_pAccessKeyText, &( m_dwAccessKeyTextSize ), NULL );

			// If buffer for key text was not large enough, reallocate buffer that is large enough and try again.
			if ( dwResult == ERROR_INSUFFICIENT_BUFFER )
			{
				delete [] m_pAccessKeyText;
				m_pAccessKeyText = new wchar_t[ m_dwAccessKeyTextSize ];
				dwResult = XShowNuiSocialGetUserToken( dwUserTrackingIndex, dwUserIndex, dwSocialNetworkID, pRequiredPermissions, m_pAccessKeyText, &( m_dwAccessKeyTextSize ), NULL );
			}
		}
		else // using standard controller interface.
		{
			dwResult = XShowSocialGetUserToken( dwUserIndex, dwSocialNetworkID, pRequiredPermissions, m_pAccessKeyText, &( m_dwAccessKeyTextSize ), NULL );

			// If buffer for key text was not large enough, reallocate buffer that is large enough and try again.
			if ( dwResult == ERROR_INSUFFICIENT_BUFFER )
			{
				delete [] m_pAccessKeyText;
				m_pAccessKeyText = new wchar_t[ m_dwAccessKeyTextSize ];
				dwResult = XShowSocialGetUserToken( dwUserIndex, dwSocialNetworkID, pRequiredPermissions, m_pAccessKeyText, &( m_dwAccessKeyTextSize ), NULL );
			}
		}
	}
	else // we are trying to obtain cached user access key.
	{
		dwResult = XSocialGetUserToken( dwUserIndex, dwSocialNetworkID, pRequiredPermissions, m_pAccessKeyText, &( m_dwAccessKeyTextSize ), NULL );

		// If buffer for key text was not large enough, reallocate buffer that is large enough and try again.
		if ( dwResult == ERROR_INSUFFICIENT_BUFFER )
		{
			delete [] m_pAccessKeyText;
			m_pAccessKeyText = new wchar_t[ m_dwAccessKeyTextSize ];
			dwResult = XSocialGetUserToken( dwUserIndex, dwSocialNetworkID, pRequiredPermissions, m_pAccessKeyText, &( m_dwAccessKeyTextSize ), NULL );
		}
	}

	// Check result.
	if ( dwResult != ERROR_SUCCESS )
	{
		switch( dwResult )
		{
			// ERROR_OUT_MEMORY does not seem to exist, despite being mentioned in the XDK docs.
			//case ERROR_OUT_MEMORY:			
			//	// Not much we can do about this?!
			//	assert( false );
			//	break;

			case ERROR_INVALID_PARAMETER:
				// dwUserIndex or dwSocialNetworkID are incorrect!
				assert( false );
				break;

			case ERROR_ACCESS_DENIED:
				// Either no user is signed in, or title not allowed to use social posting (the latter should never occur!).
				break;

			case ERROR_FUNCTION_FAILED:
				// dwTrackingID is not valid. Kinect only. Not sure what we would do about this one?
				assert( false );
				break;

			case ERROR_INSUFFICIENT_BUFFER:
				// This should not be possible, we already resized the buffer and retried if it was too small.
				assert( false );
				break;

			case ERROR_GRACEFUL_DISCONNECT:
				// User disconnected before process was complete.
				break;

			case ERROR_CANCELLED:
				// User cancelled the request.
				break;

			case ERROR_BAD_NET_RESP:
				// Error in access, or some kind of network related error.
				// Need to display warning??
				assert( false );
				break;

			default:
				// Should never get here.
				assert( false );
				break;
		}
	}
	else // success
	{
		bReturn = true;
	}

	return bReturn;
}
*/

// WESTY : Not sure if we even need to get social access key!
/*

// It is unclear from the sdk docs what this is actually used for.
// So far we have only created a syncronous version of this function, and the xbox functionality does not seem to work yet!
bool CSocialManager::ObtainSocialNetworkAccessKey( ESocialNetwork eSocialNetwork, DWORD dwUserIndex, bool bUsingKinect )
{
	if ( m_eCurrState != eStateReady )
	{
		return false;
	}
	
	// Ensure dwUserIndex is sensible.
	if ( dwUserIndex > 3 )
	{
		assert( false );
		return false;
	}

	// Kinect user tracking index.
	DWORD dwUserTrackingIndex = 0;
	if ( bUsingKinect )
	{
		// WESTY - Will need a method to get this.
		dwUserTrackingIndex = 0;
	}
	
	// Try first of all to obtain cached user key.
	bool bSocialNetworkAccessKeyAvailable = GetSocialNetworkAccessKey( eSocialNetwork, dwUserIndex, bUsingKinect, dwUserTrackingIndex, false );

	if ( bSocialNetworkAccessKeyAvailable == false )
	{
		// No chached user key, so we will try the UI version to signin to social network?
		bSocialNetworkAccessKeyAvailable = GetSocialNetworkAccessKey( eSocialNetwork, dwUserIndex, bUsingKinect, dwUserTrackingIndex, true );
	}

	return bSocialNetworkAccessKeyAvailable;
}
*/
