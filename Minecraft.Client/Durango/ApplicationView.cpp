//
// ApplicationView.cpp -
//

#include "stdafx.h"
#include "ApplicationView.h"
#include "MinecraftServer.h"

using namespace Windows::Foundation;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;

ApplicationView::ApplicationView()
{
    m_activationComplete = false;
    m_windowClosed = false;
	m_inviteProcessed = false;
}

XALLOC_ATTRIBUTES ExpandAllocAttributes( _In_ LONGLONG dwAttributes )
{
	XALLOC_ATTRIBUTES attr;
	attr = *static_cast<XALLOC_ATTRIBUTES *>(&dwAttributes);
	return attr;
}

SIZE_T totalTracked = 0;
unordered_map<uintptr_t, SIZE_T> tracker;
static volatile bool memDump = false;
static volatile bool memReset = false;
static bool memDumpInit = false;
static CRITICAL_SECTION memTrackCS;
static volatile SIZE_T memSizeComp = 0;
static long long lastTrackTime = 0;

PVOID WINAPI XMemAllocLog( _In_ SIZE_T dwSize, _In_ ULONGLONG dwAttributes )
{
	if( !memDumpInit )
	{
		InitializeCriticalSection(&memTrackCS);
		memDumpInit = true;
	}

	PVOID           result              = XMemAllocDefault(dwSize,dwAttributes);
	XALLOC_ATTRIBUTES expanded_attributes = ExpandAllocAttributes(dwAttributes);
	EnterCriticalSection(&memTrackCS);
	tracker[(uintptr_t)result] = dwSize;
	totalTracked += dwSize;
	LeaveCriticalSection(&memTrackCS);
//	app.DebugPrintf("XMemAllocLog %p, Size: %d, AllocatorId: 0x%02x, ObjectType: 0x%04x, PageSize: %d, Alignment: %d, MemoryType: %d\n", result, dwSize, expanded_attributes.s.dwAllocatorId, expanded_attributes.s.dwObjectType, expanded_attributes.s.dwPageSize, expanded_attributes.s.dwAlignment, expanded_attributes.s.dwMemoryType  );
	return result;
}

VOID WINAPI XMemFreeLog( _In_ PVOID lpAddress, _In_ ULONGLONG dwAttributes )
{
	EnterCriticalSection(&memTrackCS);
	auto it = tracker.find((uintptr_t)lpAddress);
	if( it != tracker.end() )
	{
		totalTracked -= it->second;
		tracker.erase(it);		
	}
	LeaveCriticalSection(&memTrackCS);
	XALLOC_ATTRIBUTES expanded_attributes = ExpandAllocAttributes(dwAttributes);
//	app.DebugPrintf("XMemFreeLog %p, AllocatorId: 0x%02x, ObjectType: 0x%04x, PageSize: %d, Alignment: %d, MemoryType: %d\n", lpAddress, expanded_attributes.s.dwAllocatorId, expanded_attributes.s.dwObjectType, expanded_attributes.s.dwPageSize, expanded_attributes.s.dwAlignment, expanded_attributes.s.dwMemoryType  );
	XMemFreeDefault(lpAddress,dwAttributes);

	long long currentTime = System::currentTimeMillis();
	if( ( currentTime - lastTrackTime ) > 1000 )
	{
		app.DebugPrintf("Total memory tracked: %d\n", totalTracked);
		lastTrackTime = currentTime;
	}

	if( memReset )
	{
		tracker.clear();
		memReset = false;
		totalTracked = 0;
	}

	if( memDump )
	{
		for( auto it = tracker.begin(); it != tracker.end(); it++ )
		{
			app.DebugPrintf("0x%x : %d\n",it->first,it->second);
		}
		memDump = false;
	}
}

// Called by the system.  Perform application initialization here,
// hooking application wide events, etc.
void ApplicationView::Initialize(CoreApplicationView^ applicationView)
{
//	XMemSetAllocationHooks( &XMemAllocLog, &XMemFreeLog );
    applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &ApplicationView::OnActivated);

    CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &ApplicationView::OnSuspending);

    CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &ApplicationView::OnResuming);

	CoreApplication::ResourceAvailabilityChanged += ref new EventHandler< Platform::Object^ >( this, &ApplicationView::OnResourceAvailabilityChanged );
}

// Called when we are provided a window.
void ApplicationView::SetWindow(CoreWindow^ window)
{
    window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &ApplicationView::OnWindowClosed);
}

// The purpose of this method is to get the application entry point.
void ApplicationView::Load(Platform::String^ entryPoint)
{
}

void InitializeDurango(Windows::UI::Core::CoreWindow^ window);
void oldWinMainInit();
void oldWinMainTick();

// Called by the system after initialization is complete.  This
// implements the traditional game loop
void ApplicationView::Run()
{
//    m_game = ref new Game();
	InitializeDurango(CoreWindow::GetForCurrentThread());
    
    // Ensure we finish activation and let the system know we’re responsive
    while (!m_activationComplete)
    {
        CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
    }

#ifndef _CONTENT_PACKAGE
	char buf[256];
	sprintf(buf,"0x%llx this\n",this);
	OutputDebugStringA(buf);
#endif
	oldWinMainInit();


    while (!app.getShutdownFlag() && !m_windowClosed)
    {
        CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		oldWinMainTick();
    }

	Windows::ApplicationModel::Core::CoreApplication::Exit();
}

void ApplicationView::Uninitialize()
{
}

// Called when the application is activated.  For now, there is just one activation
// kind - Launch.
void ApplicationView::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
    
	Windows::Xbox::Multiplayer::PartyConfig::SuppressGameSessionReadyToast = true;
	Windows::Xbox::Multiplayer::PartyConfig::SuppressGameSessionReadyToastMode = Windows::Xbox::Multiplayer::SuppressGameSessionReadyToastMode::Always;

	// On starting up the game for the first time, we'll get one kind of onActivated message through. This will also
	// be called at other times though, so don't rely on it only happening once for anything
	if( !m_activationComplete )
	{
		if (args->Kind == Windows::ApplicationModel::Activation::ActivationKind::Launch)
		{
			// We get ActivationKind::Launch of we've just been started up from the user selecting us in the system UI, or if we are run normally from the debugger
			ILaunchActivatedEventArgs^ newArgs = static_cast< ILaunchActivatedEventArgs^>(args);

			
		}
		else if (args->Kind == Windows::ApplicationModel::Activation::ActivationKind::Protocol)
		{
			// We get Windows::ApplicationModel::Activation::ActivationKind::Protocol if we have been started from accepting a party invite (including joining an open party ourselves), or from a game session being ready
			IProtocolActivatedEventArgs^ proArgs = static_cast< IProtocolActivatedEventArgs^>(args);

			Platform::String^ uriType = proArgs->Uri->Host;
			if( uriType == L"partyInviteAccept")
			{
				if( !m_inviteProcessed )
				{
					DQRNetworkManager::SetPartyProcessJoinParty();
					DQRNetworkManager::SetInviteReceivedFlag();
					m_inviteProcessed = true;
				}
			}
			else if( uriType == L"gameSessionReady" )
			{
				Platform::String^ uriCommand = ref new Platform::String(proArgs->Uri->RawUri->Data());
				auto uri = ref new Windows::Foundation::Uri(uriCommand);
				WwwFormUrlDecoder^ query = uri->QueryParsed;

				DQRNetworkManager::SetPartyProcessJoinSession(0,query->GetFirstValueByName(L"sessionName"),query->GetFirstValueByName(L"serviceConfigurationId"),query->GetFirstValueByName(L"sessionTemplateName"));
			}
	   }

		m_activationComplete = true;
	}
	else
	{
		// This type of activation protocol can come in at any time during the game. Not sure of the best way to handle both this and gamesessionready events, so for now going to only process these
		// (1) once
		// (2) until the main menu is reached
		// To try and minimise any conflict of them coming in in the middle of the game when we are already handling this gamesessionready.
		if (args->Kind == Windows::ApplicationModel::Activation::ActivationKind::Protocol)
		{
			if( !app.HasReachedMainMenu() )
			{
				app.DebugPrintf("Accepting party invite after initial activation\n");
				// We get Windows::ApplicationModel::Activation::ActivationKind::Protocol if we have been started from accepting a party invite (including joining an open party ourselves), or from a game session being ready
				IProtocolActivatedEventArgs^ proArgs = static_cast< IProtocolActivatedEventArgs^>(args);

				Platform::String^ uriType = proArgs->Uri->Host;
				if( uriType == L"partyInviteAccept")
				{
					if( !m_inviteProcessed )
					{
						DQRNetworkManager::SetPartyProcessJoinParty();
						DQRNetworkManager::SetInviteReceivedFlag();
						m_inviteProcessed = true;
					}
				}
			}
			else
			{
				DQRNetworkManager::SetInviteReceivedFlag();		// This just informs the network manager that an invite has been received, not to start processing it
			}
		}
	}
}

// Called when the application is suspending.
void ApplicationView::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
    // TODO: Save game progress using the ConnectedStroage API.
	app.DebugPrintf("###################################\n\n\n");
	app.DebugPrintf("Suspending!");
	app.DebugPrintf("\n\n\n###################################");
	
	// Get the frequency of the timer
	LARGE_INTEGER qwTicksPerSec, qwTime, qwNewTime, qwDeltaTime;
	float fElapsedTime = 0.0f;
	QueryPerformanceFrequency( &qwTicksPerSec );
	float fSecsPerTick = 1.0f / static_cast<float>(qwTicksPerSec.QuadPart);
	// Save the start time
	QueryPerformanceCounter( &qwTime );

	// Request deferral
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	MinecraftServer *server = MinecraftServer::getInstance();
	if(server)
	{
		server->Suspend();
	}

	// Start suspend process for libraries
	StorageManager.Suspend();
	RenderManager.Suspend();	

	// Wait for libraries to finish suspending
	while(!StorageManager.Suspended() || !RenderManager.Suspended())
	{
		Sleep(10);
	}

	deferral->Complete();

	QueryPerformanceCounter( &qwNewTime );

	qwDeltaTime.QuadPart = qwNewTime.QuadPart - qwTime.QuadPart;
	fElapsedTime = fSecsPerTick * static_cast<FLOAT>(qwDeltaTime.QuadPart);

	app.DebugPrintf("Entire suspend process: Elapsed time %f\n", fElapsedTime);
}

// Called when the application is resuming from suspended.
void ApplicationView::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	app.DebugPrintf("###################################\n\n\n");
	app.DebugPrintf("Resuming!");
	app.DebugPrintf("\n\n\n###################################");

	// Start resume process for libraries
	RenderManager.Resume();
	InputManager.Resume();

	// Switch to offline if we were in an online game
	if (!g_NetworkManager.IsLocalGame() && g_NetworkManager.IsInGameplay())
	{
		// Bit specific but we want to avoid switching to offline if the primary player has signed out 
		// (and AFAIK there's no other way to find out until a few ticks time)
		if (app.GetXuiAction(0) != eAppAction_PrimaryPlayerSignedOut)
		{
			// We've lost the party so switch to offline, previously we exited the world here but this seems friendlier
			app.SetAction(ProfileManager.GetPrimaryPad(), eAppAction_EthernetDisconnected);
		}
	}

	// 4J-PB - clear all DLC info, and recheck, since someone may have suspended after starting a DLCinstall, and we won't have received any install/ license change event
	app.ClearDLCInstalled();
	ui.HandleDLCInstalled(ProfileManager.GetPrimaryPad());

}

void ApplicationView::OnResourceAvailabilityChanged( Platform::Object^ /*sender*/, Platform::Object^ /*args*/ )
{
	app.DebugPrintf("###################################\n\n\n");
	if( Windows::ApplicationModel::Core::CoreApplication::ResourceAvailability == Windows::ApplicationModel::Core::ResourceAvailability::Full )
	{
		app.DebugPrintf( "Entered the 'Full Running' state." );
	}
	else
	{
		app.DebugPrintf( "Entered the 'Constrained' PLM state." );
	}
	app.DebugPrintf("\n\n\n###################################");
}

void ApplicationView::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    m_windowClosed = true;
}

// Implements a IFrameworkView factory.
IFrameworkView^ ApplicationViewSource::CreateView()
{
    return ref new ApplicationView();
}

// Application entry point
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto applicationViewSource = ref new ApplicationViewSource();

    CoreApplication::Run(applicationViewSource);

    return 0;
}
