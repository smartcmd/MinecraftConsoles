//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#include "stdafx.h"
#include "ChatIntegrationLayer.h"
#include "DQRNetworkManager.h"
#include <robuffer.h>

using namespace Windows::Foundation;
using namespace Windows::Xbox::System;

// To integrate the Chat DLL in your game, you can use this ChatIntegrationLayer class with modifications, 
// or create your own design your own class using the code in this file a guide.
std::shared_ptr<ChatIntegrationLayer> GetChatIntegrationLayer()
{
    static std::shared_ptr<ChatIntegrationLayer> chatIntegrationLayerInstance;
    if (chatIntegrationLayerInstance == nullptr)
    {
        chatIntegrationLayerInstance.reset( new ChatIntegrationLayer() );
    }

    return chatIntegrationLayerInstance;
}

ChatIntegrationLayer::ChatIntegrationLayer() 
{
    ZeroMemory( m_chatVoicePacketsStatistic, sizeof(m_chatVoicePacketsStatistic) );
}

void ChatIntegrationLayer::InitializeChatManager( 
    __in bool combineCaptureBuffersIntoSinglePacket,
    __in bool useKinectAsCaptureSource,
    __in bool applySoundEffectsToCapturedAudio,
    __in bool applySoundEffectsToChatRenderedAudio,
	DQRNetworkManager *pDQRNet
    )
{
	m_pDQRNet = pDQRNet;
    {
        Concurrency::critical_section::scoped_lock lock(m_chatPacketStatsLock);
        ZeroMemory( m_chatVoicePacketsStatistic, sizeof(m_chatVoicePacketsStatistic) );
    }

    m_chatManager = ref new Microsoft::Xbox::GameChat::ChatManager();
	m_chatManager->ChatSettings->DiagnosticsTraceLevel = Microsoft::Xbox::GameChat::GameChatDiagnosticsTraceLevel::Verbose;

    // Optionally, change the default settings below as desired by commenting out and editing any of the following lines
    // Otherwise these defaults are used.
    //
    // m_chatManager = ref new Microsoft::Xbox::GameChat::ChatManager( ChatSessionPeriod::ChatPeriodOf40Milliseconds );
    // m_chatManager->ChatSettings->AudioThreadPeriodInMilliseconds = 40;
    // m_chatManager->ChatSettings->AudioThreadAffinityMask = XAUDIO2_DEFAULT_PROCESSOR; // <- this means is core 5, same as the default XAudio2 core
    // m_chatManager->ChatSettings->AudioThreadPriority = THREAD_PRIORITY_TIME_CRITICAL;
    // m_chatManager->ChatSettings->AudioEncodingQuality = Windows::Xbox::Chat::EncodingQuality::Normal;
    // m_chatManager->ChatSettings->DiagnosticsTraceLevel = Microsoft::Xbox::GameChat::GameChatDiagnosticsTraceLevel::Verbose;
    m_chatManager->ChatSettings->CombineCaptureBuffersIntoSinglePacket = combineCaptureBuffersIntoSinglePacket; // if unset, it defaults to TRUE
    m_chatManager->ChatSettings->UseKinectAsCaptureSource = useKinectAsCaptureSource; // if unset, it defaults to FALSE
    m_chatManager->ChatSettings->PreEncodeCallbackEnabled = applySoundEffectsToCapturedAudio; // if unset, it defaults to FALSE
    m_chatManager->ChatSettings->PostDecodeCallbackEnabled = applySoundEffectsToChatRenderedAudio; // if unset, it defaults to FALSE

	InitializeCriticalSection(&m_csAddedUsers);

    std::weak_ptr<ChatIntegrationLayer> weakPtrToThis = shared_from_this();

#ifdef PROFILE
    m_chatManager->ChatSettings->PerformanceCountersEnabled = true;
#endif

    // Upon enter constrained mode, mute everyone.  
    // Upon leaving constrained mode, unmute everyone who was previously muted.
    m_tokenResourceAvailabilityChanged = Windows::ApplicationModel::Core::CoreApplication::ResourceAvailabilityChanged += 
        ref new EventHandler< Platform::Object^ >( [weakPtrToThis] (Platform::Object^, Platform::Object^ )
    {
        // Using a std::weak_ptr instead of 'this' to avoid dangling pointer if caller class is released.
        // Simply unregistering the callback in the destructor isn't enough to prevent a dangling pointer
        std::shared_ptr<ChatIntegrationLayer> sharedPtrToThis(weakPtrToThis.lock());
        if( sharedPtrToThis != nullptr )
        {
            if (Windows::ApplicationModel::Core::CoreApplication::ResourceAvailability == Windows::ApplicationModel::Core::ResourceAvailability::Constrained)
            {
                if( sharedPtrToThis->m_chatManager != nullptr )
                {
                    sharedPtrToThis->m_chatManager->MuteAllUsersFromAllChannels();
                }
            }
            else if(Windows::ApplicationModel::Core::CoreApplication::ResourceAvailability == Windows::ApplicationModel::Core::ResourceAvailability::Full)
            {
                if( sharedPtrToThis->m_chatManager != nullptr )
                {
                    sharedPtrToThis->m_chatManager->UnmuteAllUsersFromAllChannels();

                    // The title should remember who was muted so when the Resume even occurs
                    // to avoid unmuting users who has been previously muted.  Simply re-mute them here
                }
            }
        }
    });

    m_tokenOnDebugMessage = m_chatManager->OnDebugMessage += 
        ref new Windows::Foundation::EventHandler<Microsoft::Xbox::GameChat::DebugMessageEventArgs^>(
            [weakPtrToThis] ( Platform::Object^, Microsoft::Xbox::GameChat::DebugMessageEventArgs^ args )
    {
        // Using a std::weak_ptr instead of 'this' to avoid dangling pointer if caller class is released.
        // Simply unregistering the callback in the destructor isn't enough to prevent a dangling pointer
        std::shared_ptr<ChatIntegrationLayer> sharedPtrToThis(weakPtrToThis.lock());
        if( sharedPtrToThis != nullptr )
        {
            sharedPtrToThis->OnDebugMessageReceived(args);
        }
    });

    m_tokenOnOutgoingChatPacketReady = m_chatManager->OnOutgoingChatPacketReady += 
        ref new Windows::Foundation::EventHandler<Microsoft::Xbox::GameChat::ChatPacketEventArgs^>( 
            [weakPtrToThis] ( Platform::Object^, Microsoft::Xbox::GameChat::ChatPacketEventArgs^ args )
    {
        // Using a std::weak_ptr instead of 'this' to avoid dangling pointer if caller class is released.
        // Simply unregistering the callback in the destructor isn't enough to prevent a dangling pointer
        std::shared_ptr<ChatIntegrationLayer> sharedPtrToThis(weakPtrToThis.lock());
        if( sharedPtrToThis != nullptr )
        {
            sharedPtrToThis->OnOutgoingChatPacketReady(args);
        }
    });

    m_tokenOnCompareUniqueConsoleIdentifiers = m_chatManager->OnCompareUniqueConsoleIdentifiers += 
        ref new Microsoft::Xbox::GameChat::CompareUniqueConsoleIdentifiersHandler( 
            [weakPtrToThis] ( Platform::Object^ obj1, Platform::Object^ obj2 ) 
    { 
        // Using a std::weak_ptr instead of 'this' to avoid dangling pointer if caller class is released.
        // Simply unregistering the callback in the destructor isn't enough to prevent a dangling pointer
        std::shared_ptr<ChatIntegrationLayer> sharedPtrToThis(weakPtrToThis.lock());
        if( sharedPtrToThis != nullptr )
        {
            return sharedPtrToThis->CompareUniqueConsoleIdentifiers(obj1, obj2); 
        }
        else
        {
            return false;
        }
    });

	m_tokenUserAudioDeviceAdded = WXS::User::AudioDeviceAdded += 
		ref new Windows::Foundation::EventHandler<WXS::AudioDeviceAddedEventArgs^>(
            [weakPtrToThis] ( Platform::Object^, WXS::AudioDeviceAddedEventArgs^ value ) 
    { 
		std::shared_ptr<ChatIntegrationLayer> sharedPtrToThis(weakPtrToThis.lock());
		if( sharedPtrToThis != nullptr )
		{
			sharedPtrToThis->EvaluateDevicesForUser(value->User);
		}
    });

	m_tokenUserAudioDeviceRemoved = WXS::User::AudioDeviceRemoved += 
		ref new Windows::Foundation::EventHandler<WXS::AudioDeviceRemovedEventArgs^>(
            [weakPtrToThis] ( Platform::Object^, WXS::AudioDeviceRemovedEventArgs^ value ) 
    { 
		std::shared_ptr<ChatIntegrationLayer> sharedPtrToThis(weakPtrToThis.lock());
		if( sharedPtrToThis != nullptr )
		{
			sharedPtrToThis->EvaluateDevicesForUser(value->User);
		}
    });

	m_tokenUserAudioDeviceChanged = WXS::User::AudioDeviceChanged += 
		ref new Windows::Foundation::EventHandler<WXS::AudioDeviceChangedEventArgs^>(
            [weakPtrToThis] ( Platform::Object^, WXS::AudioDeviceChangedEventArgs^ value ) 
    { 
		std::shared_ptr<ChatIntegrationLayer> sharedPtrToThis(weakPtrToThis.lock());
		if( sharedPtrToThis != nullptr )
		{
			sharedPtrToThis->EvaluateDevicesForUser(value->User);
		}
    });

    //m_tokenSuspending = Windows::ApplicationModel::Core::CoreApplication::Suspending += 
    //    ref new EventHandler< Windows::ApplicationModel::SuspendingEventArgs^ >( [weakPtrToThis] (Platform::Object^, Windows::ApplicationModel::SuspendingEventArgs^ args)
    //{
    //    // Upon Suspending, nothing needs to be done  
    //});

    //m_tokenResuming = Windows::ApplicationModel::Core::CoreApplication::Resuming += 
    //    ref new EventHandler< Platform::Object^ >( [weakPtrToThis] (Platform::Object^, Windows::ApplicationModel::SuspendingEventArgs^ args)
    //{
    //    // Upon Resuming, re-initialize the network, and reinitialize the chat session 
    //});
}


void ChatIntegrationLayer::Shutdown()
{
    if( m_chatManager != nullptr )
    {
        m_chatManager->OnDebugMessage -= m_tokenOnDebugMessage;
        m_chatManager->OnOutgoingChatPacketReady -= m_tokenOnOutgoingChatPacketReady;
        m_chatManager->OnCompareUniqueConsoleIdentifiers -= m_tokenOnCompareUniqueConsoleIdentifiers;
        Windows::ApplicationModel::Core::CoreApplication::ResourceAvailabilityChanged -= m_tokenResourceAvailabilityChanged;
        if( m_chatManager->ChatSettings->PreEncodeCallbackEnabled )
        {
            m_chatManager->OnPreEncodeAudioBuffer -= m_tokenOnPreEncodeAudioBuffer;
        }
        if( m_chatManager->ChatSettings->PostDecodeCallbackEnabled )
        {
            m_chatManager->OnPostDecodeAudioBuffer -= m_tokenOnPostDecodeAudioBuffer;
        }
		WXS::User::AudioDeviceAdded -= m_tokenUserAudioDeviceAdded;
		WXS::User::AudioDeviceRemoved -= m_tokenUserAudioDeviceRemoved;
		WXS::User::AudioDeviceChanged -= m_tokenUserAudioDeviceChanged;

		DeleteCriticalSection(&m_csAddedUsers);

        m_chatManager = nullptr;
    }
}

void ChatIntegrationLayer::OnDebugMessageReceived( 
    __in Microsoft::Xbox::GameChat::DebugMessageEventArgs^ args 
    )
{
    // To integrate the Chat DLL in your game, 
    // change this to false and remove the LogComment calls, 
    // or integrate with your game's own UI/debug message logging system
    bool outputToUI = false; 

    if( outputToUI )
    {
        if (args->ErrorCode == S_OK )
        {
            m_pDQRNet->LogComment(L"GameChat: " + args->Message);
        }
        else
        {
            m_pDQRNet->LogCommentWithError(L"GameChat: " + args->Message, args->ErrorCode);
        }
    }
    else
    {
        // The string appear in the Visual Studio Output window 
#ifndef _CONTENT_PACKAGE
        OutputDebugString( args->Message->Data() );
#endif
    }
}

void ChatIntegrationLayer::GameUI_RecordPacketStatistic( 
    __in Microsoft::Xbox::GameChat::ChatMessageType messageType,
    __in ChatPacketType chatPacketType
    )
{
    uint32 messageTypeInt = static_cast<uint32>(messageType);
    if( messageType > Microsoft::Xbox::GameChat::ChatMessageType::InvalidMessage )
    {
        return;
    }

    {
        Concurrency::critical_section::scoped_lock lock(m_chatPacketStatsLock);
        m_chatVoicePacketsStatistic[static_cast<int>(chatPacketType)][messageTypeInt]++;
    }
}

int ChatIntegrationLayer::GameUI_GetPacketStatistic( 
    __in Microsoft::Xbox::GameChat::ChatMessageType messageType,
    __in ChatPacketType chatPacketType
    )
{
    uint32 messageTypeInt = static_cast<uint32>(messageType);
    if( messageType > Microsoft::Xbox::GameChat::ChatMessageType::InvalidMessage )
    {
        return 0;
    }

    {
        Concurrency::critical_section::scoped_lock lock(m_chatPacketStatsLock);
        return m_chatVoicePacketsStatistic[static_cast<int>(chatPacketType)][messageTypeInt];
    }
}

void ChatIntegrationLayer::OnOutgoingChatPacketReady( 
    __in Microsoft::Xbox::GameChat::ChatPacketEventArgs^ args 
    )
{
	byte *bytes;
	int byteCount;

	GetBufferBytes(args->PacketBuffer, &bytes);
	byteCount = args->PacketBuffer->Length;
	unsigned int address = 0;
	if( !args->SendPacketToAllConnectedConsoles )
	{
		address = safe_cast<unsigned int>(args->UniqueTargetConsoleIdentifier);
	}
	m_pDQRNet->SendBytesChat(address, bytes, byteCount, args->SendReliable, args->SendInOrder, args->SendPacketToAllConnectedConsoles);

    GameUI_RecordPacketStatistic( args->ChatMessageType, ChatPacketType::OutgoingPacket );

}

void ChatIntegrationLayer::OnIncomingChatMessage( 
	unsigned int sessionAddress,
	Platform::Array<byte>^ message
    )
{
    // To integrate the Chat DLL in your game, change the following code to use your game's network layer.
    // Ignore the OnChatMessageReceived event as that is specific to this sample's simple network layer.
    // Instead your title should upon receiving a packet, extract the chat message from it, and then call m_chatManager->ProcessIncomingChatMessage as shown below
    // You will need to isolate chat messages to be unique from the rest of you game's other message types.

    // uniqueRemoteConsoleIdentifier is a Platform::Object^ and can be cast or unboxed to most types. 
    // What exactly you use doesn't matter, but optimally it would be something that uniquely identifies a console on in the session. 
    // A Windows::Xbox::Networking::SecureDeviceAssociation^ is perfect to use if you have access to it.

    // This is how you would convert from byte array to a IBuffer^
    //
    //    Windows::Storage::Streams::IBuffer^ destBuffer = ref new Windows::Storage::Streams::Buffer( sourceByteBufferSize );
    //    byte* destBufferBytes = nullptr;
    //    GetBufferBytes( destBuffer, &destBufferBytes );
    //    errno_t err = memcpy_s( destBufferBytes, destBuffer->Capacity, sourceByteBuffer, sourceByteBufferSize );
    //    THROW_HR_IF(err != 0, E_FAIL);
    //    destBuffer->Length = sourceByteBufferSize;

    // This is how you would convert from an int to a Platform::Object^
    // Platform::Object obj = IntToPlatformObject(5);

    Windows::Storage::Streams::IBuffer^ chatMessage = ArrayToBuffer(message);
    Platform::Object^ uniqueRemoteConsoleIdentifier = (Platform::Object^)sessionAddress;

    if( m_chatManager != nullptr )
    {
        Microsoft::Xbox::GameChat::ChatMessageType chatMessageType = m_chatManager->ProcessIncomingChatMessage(chatMessage, uniqueRemoteConsoleIdentifier);

        GameUI_RecordPacketStatistic( chatMessageType, ChatPacketType::IncomingPacket ); 
    }
}

// Only add people who intend to play.
void ChatIntegrationLayer::AddAllLocallySignedInUsersToChatClient(
    __in uint8 channelIndex,
    __in Windows::Foundation::Collections::IVectorView<Windows::Xbox::System::User^>^ locallySignedInUsers
    )
{
    // To integrate the Chat DLL in your game, 
    // add all locally signed in users to the chat client
    for each( Windows::Xbox::System::User^ user in locallySignedInUsers )
    {
        if( user != nullptr )
        {
//            LogComment(L"Adding Local User to Chat Client");
            AddLocalUserToChatChannel( channelIndex, user );
        }
    }
}

ChatIntegrationLayer::AddedUser::AddedUser(Windows::Xbox::System::IUser^ user, bool canCaptureAudio)
{
	m_user = user;
	m_canCaptureAudio = canCaptureAudio;
}


void ChatIntegrationLayer::AddLocalUser( __in Windows::Xbox::System::IUser^ user )
{
	// Check we haven't added already
	for( int i = 0; i < m_addedUsers.size(); i++ )
	{
		if( m_addedUsers[i]->m_user->XboxUserId == user->XboxUserId )
		{
			return;
		}
	}

	bool kinectAvailable = false;
	Windows::Kinect::KinectSensor^ sensor = Windows::Kinect::KinectSensor::GetDefault();
	if( sensor )
	{
		sensor->Open();
		if( sensor->IsAvailable )
		{
			kinectAvailable = true;
			m_pDQRNet->LogComment(L"Evaluated that kinect is available\n");
		}
		sensor->Close();
	}

	EnterCriticalSection(&m_csAddedUsers);
	// First establish whether we have an appropriate audio device at this time
	bool canCaptureAudio = false;
	for each( WXS::IAudioDeviceInfo^ audioDevice in user->AudioDevices )
	{
		m_pDQRNet->LogComment(L"Evaluating device " + audioDevice->DeviceCategory.ToString() + L" " +
													   audioDevice->DeviceType.ToString() +  L" " +
													   audioDevice->Id +  L" " +
													   audioDevice->Sharing.ToString() +  L" " +
													   audioDevice->IsMicrophoneMuted.ToString() + L"\n");

		// Consider shared devices only if kinect is actually available - every machine seems to claim a shared device whether kinect is attached or not
		if( ( audioDevice->DeviceType == WXS::AudioDeviceType::Capture ) && ( kinectAvailable  || ( audioDevice->Sharing != WXS::AudioDeviceSharing::Shared) ) )
		{
			canCaptureAudio = true;
		}
	}

	// If we can capture audio initially, then register with the chat session. Otherwise we'll reevaluate this situation when audio devices change
	if( canCaptureAudio )
	{
		AddLocalUserToChatChannel( 0 , user );
	}

	// Add to vector of users that we are tracking in the chat system
	m_addedUsers.push_back(new AddedUser(user, canCaptureAudio));
	LeaveCriticalSection(&m_csAddedUsers);
}

// Remove from our list of tracked users, if the user is already there
void ChatIntegrationLayer::RemoveLocalUser( __in Windows::Xbox::System::IUser^ user )
{
	EnterCriticalSection(&m_csAddedUsers);
	for( auto it = m_addedUsers.begin(); it != m_addedUsers.end(); it++ )
	{
		if( (*it)->m_user->XboxUserId == user->XboxUserId )
		{
			delete (*it);
			m_addedUsers.erase(it);
			LeaveCriticalSection(&m_csAddedUsers);
			return;
		}
	}
	LeaveCriticalSection(&m_csAddedUsers);
}

// This is called when the audio devices for a user change in any way, and establishes whether we can now capture audio. Any change in this status from before will cause the user to be added/removed from the chat session.
void ChatIntegrationLayer::EvaluateDevicesForUser(__in Windows::Xbox::System::IUser^ user )
{
	bool kinectAvailable = false;
	Windows::Kinect::KinectSensor^ sensor = Windows::Kinect::KinectSensor::GetDefault();
	if( sensor )
	{
		sensor->Open();
		if( sensor->IsAvailable )
		{
			kinectAvailable = true;
			m_pDQRNet->LogComment(L"Evaluated that kinect is available\n");
		}
		sensor->Close();
	}

	EnterCriticalSection(&m_csAddedUsers);
	for( int i = 0; i < m_addedUsers.size(); i++ )
	{
		AddedUser *addedUser = m_addedUsers[i];
		if( addedUser->m_user->XboxUserId == user->XboxUserId )
		{
			bool canCaptureAudio = false;
			for each( WXS::IAudioDeviceInfo^ audioDevice in addedUser->m_user->AudioDevices )
			{
				// Consider shared devices only if kinect is actually available - every machine seems to claim a shared device whether kinect is attached or not
				if( ( audioDevice->DeviceType == WXS::AudioDeviceType::Capture ) && ( kinectAvailable  || ( audioDevice->Sharing != WXS::AudioDeviceSharing::Shared) ) )
				{
					canCaptureAudio = true;
					break;
				}
			}
			if( canCaptureAudio != addedUser->m_canCaptureAudio )
			{
				if( canCaptureAudio )
				{
					AddLocalUserToChatChannel(0, addedUser->m_user );
				}
				else
				{
					RemoveUserFromChatChannel(0, addedUser->m_user );
				}
				addedUser->m_canCaptureAudio = canCaptureAudio;
				LeaveCriticalSection(&m_csAddedUsers);
				return;
			}
		}
	}
	LeaveCriticalSection(&m_csAddedUsers);
}

void ChatIntegrationLayer::AddLocalUserToChatChannel(
    __in uint8 channelIndex,
    __in Windows::Xbox::System::IUser^ user
    )
{
    // Adds a local user to a specific channel.  

    // This is helper function waits for the task to cm_chatManageromplete so shouldn't be called from the UI thread 
    // Remove the .wait() and return the result of concurrency::create_task() if you want to call it from the UI thread 
    // and chain PPL tasks together

	m_pDQRNet->LogComment( L">>>>>>>>>>>>> AddLocalUserToChatChannel" );
    if( m_chatManager != nullptr )
    {
        auto asyncOp = m_chatManager->AddLocalUserToChatChannelAsync( channelIndex, user );
        concurrency::create_task( asyncOp )
        .then( [this] ( concurrency::task<void> t )
        {
            // Error handling
            try
            {
                t.get();                
            }
            catch ( Platform::Exception^ ex )
            {
                m_pDQRNet->LogCommentWithError( L"AddLocalUserToChatChannelAsync failed", ex->HResult );
            }
        })
        .wait();
    }
}

void ChatIntegrationLayer::RemoveRemoteConsole( 
    unsigned int address
    )
{
    // uniqueConsoleIdentifier is a Platform::Object^ and can be cast or unboxed to most types. 
    // What exactly you use doesn't matter, but optimally it would be something that uniquely identifies a console on in the session. 
    // A Windows::Xbox::Networking::SecureDeviceAssociation^ is perfect to use if you have access to it.

    // This is how you would convert from an int to a Platform::Object^
    // Platform::Object obj = IntToPlatformObject(5);

    // This is helper function waits for the task to complete so shouldn't be called from the UI thread 
    // Remove the .wait() and return the result of concurrency::create_task() if you want to call it from the UI thread 
    // and chain PPL tasks together

	Platform::Object^ uniqueRemoteConsoleIdentifier = (Platform::Object^)address;

    if( m_chatManager != nullptr )
    {
        auto asyncOp = m_chatManager->RemoveRemoteConsoleAsync( uniqueRemoteConsoleIdentifier );
        concurrency::create_task( asyncOp ).then( [this] ( concurrency::task<void> t )
        {
            // Error handling
            try
            {
                t.get();                
            }
            catch ( Platform::Exception^ ex )
            {
                m_pDQRNet->LogCommentWithError( L"RemoveRemoteConsoleAsync failed", ex->HResult );
            }
        })
        .wait();
    }
}

void ChatIntegrationLayer::RemoveUserFromChatChannel( 
    __in uint8 channelIndex,
    __in Windows::Xbox::System::IUser^ user 
    )
{
    if( m_chatManager != nullptr )
    {
        // This is helper function waits for the task to complete so shouldn't be called from the UI thread 
        // Remove the .wait() and return the result of concurrency::create_task() if you want to call it from the UI thread 
        // and chain PPL tasks together

        auto asyncOp = m_chatManager->RemoveLocalUserFromChatChannelAsync( channelIndex, user );
        concurrency::create_task( asyncOp ).then( [this] ( concurrency::task<void> t )
        {
            // Error handling
            try
            {
                t.get();
            }
            catch ( Platform::Exception^ ex )
            {
                m_pDQRNet->LogCommentWithError( L"RemoveLocalUserFromChatChannelAsync failed", ex->HResult );
            }
        })
        .wait();
    }
}

void ChatIntegrationLayer::OnNewSessionAddressAdded( 
    __in unsigned int address
    )
{
	m_pDQRNet->LogCommentFormat( L">>>>>>>>>>>>> OnNewSessionAddressAdded (%d)",address );
	Platform::Object^ uniqueConsoleIdentifier = (Platform::Object^)address;
    /// Call this when a new console connects.  
    /// This adds this console to the chat layer
    if( m_chatManager != nullptr )
    {
        m_chatManager->HandleNewRemoteConsole(uniqueConsoleIdentifier );
    }
}

Windows::Foundation::Collections::IVectorView<Microsoft::Xbox::GameChat::ChatUser^>^ ChatIntegrationLayer::GetChatUsers()
{
    if( m_chatManager != nullptr )
    {
        return m_chatManager->GetChatUsers();
    }

    return nullptr;
}

bool ChatIntegrationLayer::HasMicFocus()
{
    if( m_chatManager != nullptr )
    {
        return m_chatManager->HasMicFocus;
    }

    return false;
}

Platform::Object^ ChatIntegrationLayer::IntToPlatformObject( 
    __in int val 
    )
{
    return (Platform::Object^)val;

    // You can also do the same using a PropertyValue.
    //return Windows::Foundation::PropertyValue::CreateInt32(val);
}

int ChatIntegrationLayer::PlatformObjectToInt( 
    __in Platform::Object^ uniqueRemoteConsoleIdentifier 
    )
{
    return safe_cast<int>( uniqueRemoteConsoleIdentifier ); 

    // You can also do the same using a PropertyValue.
    //return safe_cast<Windows::Foundation::IPropertyValue^>(uniqueRemoteConsoleIdentifier)->GetInt32();
}

void ChatIntegrationLayer::HandleChatChannelChanged( 
    __in uint8 oldChatChannelIndex, 
    __in uint8 newChatChannelIndex, 
    __in Microsoft::Xbox::GameChat::ChatUser^ chatUser 
    )
{
    // We remember if the local user was currently muted from all channels. And when we switch channels, 
    // we ensure that the state persists. For remote users, title should implement this themselves
    // based on title game design if they want to persist the muting state.

    bool wasUserMuted = false;
    IUser^ userBeingRemoved = nullptr;

    if (chatUser != nullptr && chatUser->IsLocal)
    {
        wasUserMuted = chatUser->IsMuted;
        userBeingRemoved = chatUser->User;
        if (userBeingRemoved != nullptr)
        {
            RemoveUserFromChatChannel(oldChatChannelIndex, userBeingRemoved);
            AddLocalUserToChatChannel(newChatChannelIndex, userBeingRemoved);
        }
    }

    // If the local user was muted earlier, get the latest chat users and mute him again on the newly added channel.
    if (wasUserMuted && userBeingRemoved != nullptr)
    {
        auto chatUsers = GetChatUsers();
        if (chatUsers != nullptr )
        {
            for (UINT chatUserIndex = 0; chatUserIndex < chatUsers->Size; chatUserIndex++)
            {
                Microsoft::Xbox::GameChat::ChatUser^ chatUser = chatUsers->GetAt(chatUserIndex);
                if( chatUser != nullptr && (chatUser->XboxUserId == userBeingRemoved->XboxUserId) )
                {
                    m_chatManager->MuteUserFromAllChannels(chatUser);
                    break;
                }
            }
        }
    }
}

void ChatIntegrationLayer::ChangeChatUserMuteState( 
    __in Microsoft::Xbox::GameChat::ChatUser^ chatUser 
    )
{
    /// Helper function to swap the mute state of a specific chat user
    if( m_chatManager != nullptr && chatUser != nullptr)
    {
        if (chatUser->IsMuted)
        {
            m_chatManager->UnmuteUserFromAllChannels(chatUser);
        }
        else
        {
            m_chatManager->MuteUserFromAllChannels(chatUser);
        }
    }
}

Microsoft::Xbox::GameChat::ChatUser^ ChatIntegrationLayer::GetChatUserByXboxUserId( 
    __in Platform::String^ xboxUserId 
    )
{
    Windows::Foundation::Collections::IVectorView<Microsoft::Xbox::GameChat::ChatUser^>^ chatUsers = GetChatUsers();
    for each (Microsoft::Xbox::GameChat::ChatUser^ chatUser in chatUsers)
    {
        if (chatUser != nullptr && ( chatUser->XboxUserId == xboxUserId ) )
        {
            return chatUser;
        }
    }
    
    return nullptr;
}


bool ChatIntegrationLayer::CompareUniqueConsoleIdentifiers( 
    __in Platform::Object^ uniqueRemoteConsoleIdentifier1, 
    __in Platform::Object^ uniqueRemoteConsoleIdentifier2 
    )
{
    if (uniqueRemoteConsoleIdentifier1 == nullptr || uniqueRemoteConsoleIdentifier2 == nullptr)
    {
        return false;
    }

    // uniqueRemoteConsoleIdentifier is a Platform::Object^ and can be cast or unboxed to most types. 
	// We're using XRNS addresses, which are unsigned ints
    unsigned int address1 = safe_cast<unsigned int>(uniqueRemoteConsoleIdentifier1);
    unsigned int address2 = safe_cast<unsigned int>(uniqueRemoteConsoleIdentifier2);
    return address1 == address2;
}

Microsoft::Xbox::GameChat::ChatPerformanceCounters^ ChatIntegrationLayer::GetChatPerformanceCounters()
{
    if( m_chatManager != nullptr &&
        m_chatManager->ChatSettings != nullptr &&
        m_chatManager->ChatSettings->PerformanceCountersEnabled )
    {
        return m_chatManager->ChatPerformanceCounters;
    }

    return nullptr;
}

void ChatIntegrationLayer::OnControllerPairingChanged( Windows::Xbox::Input::ControllerPairingChangedEventArgs^ args )
{
#if 0
    auto controller = args->Controller;
    if ( controller )
    {
        if ( controller->Type == L"Windows.Xbox.Input.Gamepad" )
        {
            // Either add the user or sign one in
            User^ user = args->User;
            if ( user != nullptr )
            {
                g_sampleInstance->GetLoggingUI()->LogCommentToUI("OnControllerPairingChanged: " + user->DisplayInfo->Gamertag);
                AddLocalUserToChatChannel(
                    g_sampleInstance->GetUISelectionChatChannelIndex(),
                    user
                    );
            }
        }
    }
#endif
}

void ChatIntegrationLayer::ToggleRenderTargetVolume()
{
    // Simple toggle logic to just show usage of LocalRenderTargetVolume property
    static bool makeRenderTargetVolumeQuiet = false;
    makeRenderTargetVolumeQuiet = !makeRenderTargetVolumeQuiet;

    auto chatUsers = GetChatUsers();
    if (chatUsers != nullptr )
    {
        for (UINT chatUserIndex = 0; chatUserIndex < chatUsers->Size; chatUserIndex++)
        {
            Microsoft::Xbox::GameChat::ChatUser^ chatUser = chatUsers->GetAt(chatUserIndex);
            if( chatUser != nullptr && chatUser->IsLocal )
            {
                chatUser->LocalRenderTargetVolume = ( makeRenderTargetVolumeQuiet ) ? 0.1f : 1.0f;
            }
        }
    }
}


void ChatIntegrationLayer::GetBufferBytes( __in Windows::Storage::Streams::IBuffer^ buffer, __out byte** ppOut )
{
    if ( ppOut == nullptr || buffer == nullptr )
    {
        throw ref new Platform::InvalidArgumentException();
    }

    *ppOut = nullptr;

    Microsoft::WRL::ComPtr<IInspectable> srcBufferInspectable(reinterpret_cast<IInspectable*>( buffer ));
    Microsoft::WRL::ComPtr<Windows::Storage::Streams::IBufferByteAccess> srcBufferByteAccess;
    srcBufferInspectable.As(&srcBufferByteAccess);
    srcBufferByteAccess->Buffer(ppOut);
}

Windows::Storage::Streams::IBuffer^ ChatIntegrationLayer::ArrayToBuffer( __in Platform::Array<byte>^ array )
{
    Windows::Storage::Streams::DataWriter^ writer = ref new Windows::Storage::Streams::DataWriter();
    writer->WriteBytes(array);
    return writer->DetachBuffer();
}