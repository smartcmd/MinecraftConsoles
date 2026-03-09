//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once

//#include "UserController.h"

class DQRNetworkManager;

enum ChatPacketType
{
    IncomingPacket = 0,
    OutgoingPacket = 1
};

class ChatIntegrationLayer 
    : public std::enable_shared_from_this<ChatIntegrationLayer> // shared_from_this is needed to use a weak ref to 'this' when handling delegate callbacks
{
public:
    ChatIntegrationLayer();

	DQRNetworkManager *m_pDQRNet;
    /// <summary>
    /// Initializes the chat manager
    /// </summary>
    void InitializeChatManager( 
        __in bool combineCaptureBuffersIntoSinglePacketEnabled,
        __in bool useKinectAsCaptureSource,
        __in bool applySoundEffectToCapture,
        __in bool applySoundEffectToRender,
		DQRNetworkManager *pDQRNet
        );

    /// <summary>
    /// Shuts down the chat manager
    /// </summary>
    void Shutdown();

	class AddedUser
	{
	public:
		AddedUser(Windows::Xbox::System::IUser^ user, bool canCaptureAudio);
		Windows::Xbox::System::IUser^				m_user;
		bool										m_canCaptureAudio;
	};

    void AddLocalUser( __in Windows::Xbox::System::IUser^ user ); 
	void RemoveLocalUser( __in Windows::Xbox::System::IUser^ user ); 
	void EvaluateDevicesForUser(__in Windows::Xbox::System::IUser^ user );

	vector<AddedUser *> m_addedUsers;
	CRITICAL_SECTION m_csAddedUsers;

private:
    /// <summary>
    /// Adds a local user to a specific channel
    /// This is helper function waits for the task to complete so shouldn't be called from the UI thread 
    /// </summary>
    /// <param name="channelIndex">The channel to add the user to</param>
    /// <param name="user">The local user to add</param>
    void AddLocalUserToChatChannel( 
        __in uint8 channelIndex,
        __in Windows::Xbox::System::IUser^ user
        ); 

    /// <summary>
    /// Removes a local user from a specific channel
    /// This is helper function waits for the task to complete so shouldn't be called from the UI thread 
    /// </summary>
    /// <param name="channelIndex">The channel to remove the user from</param>
    /// <param name="user">The local user to remove</param>
    void RemoveUserFromChatChannel(
        __in uint8 channelIndex,
        __in Windows::Xbox::System::IUser^ user
        );
public:

    /// <summary>
    /// Removes a remote console from chat
    /// This is helper function waits for the task to complete so shouldn't be called from the UI thread 
    /// </summary>
    /// <param name="uniqueRemoteConsoleIdentifier">A unique ID for the remote console</param>
    void RemoveRemoteConsole( 
        unsigned int address
        );

    /// <summary>
    /// Handles incoming chat messages from the game's network layer
    /// </summary>
    /// <param name="chatPacket">A buffer containing the chat message</param>
    /// <param name="uniqueRemoteConsoleIdentifier">A unique ID for the remote console</param>
    void OnIncomingChatMessage( 
		unsigned int sessionAddress,
		Platform::Array<byte>^ message
        );

    /// <summary>
    /// Returns a list of chat users in the chat session
    /// </summary>
    Windows::Foundation::Collections::IVectorView<Microsoft::Xbox::GameChat::ChatUser^>^ GetChatUsers();

    /// <summary>
    /// Returns true if the game has mic focus.  Otherwise another app has mic focus
    /// </summary>
    bool HasMicFocus();

    /// <summary>
    /// Helper function to swap the mute state of a specific chat user
    /// </summary>
    void ChangeChatUserMuteState(
        __in Microsoft::Xbox::GameChat::ChatUser^ chatUser 
        );

    /// <summary>
    /// Helper function to change the channel of a specific chat user
    /// </summary>
    void HandleChatChannelChanged(
        __in uint8 oldChatChannelIndex,
        __in uint8 newChatChannelIndex,
        __in Microsoft::Xbox::GameChat::ChatUser^ chatUser 
        );

    /// <summary>
    /// Call this when a new console connects.  
    /// This adds this console to the chat layer
    /// </summary>
    void OnNewSessionAddressAdded( 
        __in unsigned int address 
        );

    /// <summary>
    /// Adds a list of locally signed in users that have intent to play to the chat session on a specific channel index.
    /// Avoid adding any user who is signed in that doesn't have intent to play otherwise users who are biometrically 
    /// signed in automatically will be added to the chat session
    /// </summary>
    /// <param name="channelIndex">The channel to add the users to</param>
    /// <param name="locallySignedInUsers">A list of locally signed in users that have intent to play</param>
    void AddAllLocallySignedInUsersToChatClient(
        __in uint8 channelIndex,
        __in Windows::Foundation::Collections::IVectorView<Windows::Xbox::System::User^>^ locallySignedInUsers
        );

    /// <summary>
    /// Handles when a debug message is received.  Send this to the UI and OutputDebugString.  Games should integrate with their existing log system.
    /// </summary>
    /// <param name="args">Contains the debug message to log</param>
    void OnDebugMessageReceived( 
        __in Microsoft::Xbox::GameChat::DebugMessageEventArgs^ args 
        );

    /// <summary>
    /// Send the chat packet to all connected consoles
    ///
    /// To integrate the Chat DLL in your game, change the following code to use your game's network layer.
    /// You will need to isolate chat messages to be unique from the rest of you game's other message types.
    /// When args->SendPacketToAllConnectedConsoles is true, your game should send the chat message to each connected console using the game's network layer.
    /// It should send the chat message with Reliable UDP if args->SendReliable is true.
    /// It should send the chat message in order (if that feature is available) if args->SendInOrder is true
    /// </summary>
    /// <param name="args">Describes the packet to send</param>
    void OnOutgoingChatPacketReady( 
        __in Microsoft::Xbox::GameChat::ChatPacketEventArgs^ args 
        );

    /// <summary>
    /// Example of how to cast an int to a Platform::Object^
    /// </summary>
    Platform::Object^ IntToPlatformObject(
        __in int val
        );

    /// <summary>
    /// Example of how to cast an Platform::Object^ to an int
    /// </summary>
    int PlatformObjectToInt(
        __in Platform::Object^ obj
        );

    /// <summary>
    /// Helper function to get specific ChatUser by xboxUserId
    /// </summary>
    Microsoft::Xbox::GameChat::ChatUser^ GetChatUserByXboxUserId(
        __in Platform::String^ xboxUserId
        );

    /// <summary>
    /// Helper function to get specific ChatUser by xboxUserId
    /// </summary>
    bool ChatIntegrationLayer::CompareUniqueConsoleIdentifiers( 
        __in Platform::Object^ uniqueRemoteConsoleIdentifier1, 
        __in Platform::Object^ uniqueRemoteConsoleIdentifier2 
        );

    /// <summary>
    /// Helper function to return the ChatPerformanceCounters^ from the ChatManager so perf numbers can be shown on the UI 
    /// These numbers will only be valid if m_chatManager->ChatSettings->PerformanceCountersEnabled is set to true.
    /// </summary>
    Microsoft::Xbox::GameChat::ChatPerformanceCounters^ GetChatPerformanceCounters();

    /// <summary>
    /// Returns a count of the number of chat packets of a specific type that have been either sent or received.
    /// It is useful to monitor this number in the UI / logs to debug network issues.
    /// </summary>
    int GameUI_GetPacketStatistic( 
        __in Microsoft::Xbox::GameChat::ChatMessageType messageType,
        __in ChatPacketType chatPacketType
        );
    
    void OnControllerPairingChanged( 
        __in Windows::Xbox::Input::ControllerPairingChangedEventArgs^ args 
        );

    void ToggleRenderTargetVolume();

private:
	void GetBufferBytes( __in Windows::Storage::Streams::IBuffer^ buffer, __out byte** ppOut );
	Windows::Storage::Streams::IBuffer^ ArrayToBuffer( __in Platform::Array<byte>^ array );

    Concurrency::critical_section m_lock;
    Microsoft::Xbox::GameChat::ChatManager^ m_chatManager;
    Windows::Foundation::EventRegistrationToken m_tokenOnDebugMessage;
    Windows::Foundation::EventRegistrationToken m_tokenOnOutgoingChatPacketReady;
    Windows::Foundation::EventRegistrationToken m_tokenOnCompareUniqueConsoleIdentifiers;
    Windows::Foundation::EventRegistrationToken m_tokenResourceAvailabilityChanged;
    Windows::Foundation::EventRegistrationToken m_tokenOnPreEncodeAudioBuffer;
    Windows::Foundation::EventRegistrationToken m_tokenOnPostDecodeAudioBuffer;
	Windows::Foundation::EventRegistrationToken m_tokenUserAudioDeviceAdded;
	Windows::Foundation::EventRegistrationToken m_tokenUserAudioDeviceRemoved;
	Windows::Foundation::EventRegistrationToken m_tokenUserAudioDeviceChanged;

    // Debug stats for chat packets. Use Debug_GetPacketStatistic() to get the values.
    /// It is useful to monitor this number in the UI / logs to debug network issues.
    void GameUI_RecordPacketStatistic( 
        __in Microsoft::Xbox::GameChat::ChatMessageType messageType,
        __in ChatPacketType chatPacketType
        );
    Concurrency::critical_section m_chatPacketStatsLock;
    int m_chatVoicePacketsStatistic[2][static_cast<int>(Microsoft::Xbox::GameChat::ChatMessageType::InvalidMessage)+1];
};

std::shared_ptr<ChatIntegrationLayer> GetChatIntegrationLayer();