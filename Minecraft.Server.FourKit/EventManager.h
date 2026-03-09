#pragma once

#include "Events.h"
#include "Listener.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Collections::Generic;
using namespace System::Threading;

public ref class EventManager
{
public:
	static void RegisterListener(Listener^ listener);
	
	static void UnregisterListener(Listener^ listener);
	
	static void FireEvent(Object^ eventObject);
	
	static property List<Listener^>^ RegisteredListeners
	{
		List<Listener^>^ get();
	}
	
	static void SetCurrentPlugin(String^ pluginName);
	static void ClearCurrentPlugin();

private:
	static List<Listener^>^ listeners;
	static Dictionary<Listener^, String^>^ listenerPluginMap;
	static ThreadLocal<String^>^ currentPlugin;
	
	static EventManager()
	{
		// No visual Studio... this Compiles!
		listeners = gcnew List<Listener^>();
		listenerPluginMap = gcnew Dictionary<Listener^, String^>();
		currentPlugin = gcnew ThreadLocal<String^>();
	}
	
	static String^ GetPluginNameForListener(Listener^ listener);
};
