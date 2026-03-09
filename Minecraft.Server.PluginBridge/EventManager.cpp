#include "EventManager.h"
#include "Listener.h"
#include "PluginLogger.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Collections::Generic;

void EventManager::RegisterListener(Listener^ listener)
{
	if (listener == nullptr)
	{
		throw gcnew ArgumentNullException("listener");
	}

	listeners->Add(listener);
	
	String^ pluginName = currentPlugin->Value;

	// ????
    if (!String::IsNullOrEmpty(pluginName))
    {
		if (listenerPluginMap->ContainsKey(listener))
		{
			listenerPluginMap->Remove(listener);
		}
		listenerPluginMap->Add(listener, pluginName);
    }

	//PluginLogger::LogDebug("eventmanager", String::Format("Registered listener: {0}", listener->GetType()->Name));
}

void EventManager::UnregisterListener(Listener^ listener)
{
	if (listener == nullptr)
	{
		throw gcnew ArgumentNullException("listener");
	}

	listeners->Remove(listener);
	listenerPluginMap->Remove(listener);
	PluginLogger::LogDebug("eventmanager", String::Format("Unregistered listener: {0}", listener->GetType()->Name));
}

String^ EventManager::GetPluginNameForListener(Listener^ listener)
{
	String^ pluginName = nullptr;
	if (listenerPluginMap->TryGetValue(listener, pluginName))
	{
		return pluginName;
	}
	return listener->GetType()->Name;
}

void EventManager::FireEvent(Object^ eventObject)
{
	if (eventObject == nullptr)
	{
		throw gcnew ArgumentNullException("eventObject");
	}

	Type^ eventType = eventObject->GetType();
	String^ eventTypeName = eventType->Name;

	//PluginLogger::LogDebug("eventmanager", String::Format("Firing event: {0} to {1} listeners", eventTypeName, listeners->Count));

	for each(Listener^ listener in listeners)
	{
		Type^ listenerType = listener->GetType();
		cli::array<MethodInfo^>^ methods = listenerType->GetMethods(
			BindingFlags::Public | BindingFlags::Instance | BindingFlags::IgnoreCase);

		for each(MethodInfo^ method in methods)
		{
			cli::array<Object^>^ attributes = method->GetCustomAttributes(EventHandlerAttribute::typeid, false);
			
			if (attributes->Length > 0)
			{
				cli::array<ParameterInfo^>^ parameters = method->GetParameters();
				
				if (parameters->Length == 1 && parameters[0]->ParameterType == eventType)
				{
					try
					{
						String^ pluginName = GetPluginNameForListener(listener);
						
						method->Invoke(listener, gcnew cli::array<Object^> { eventObject });
					}
					catch (Exception^ ex)
					{
						PluginLogger::LogError("eventmanager", 
							String::Format("Error invoking {0}::{1}: {2}", 
								listenerType->Name, method->Name, ex->InnerException->Message));
					}
				}
			}
		}
	}
}

void EventManager::SetCurrentPlugin(String^ pluginName)
{
	currentPlugin->Value = pluginName;
}

void EventManager::ClearCurrentPlugin()
{
	currentPlugin->Value = nullptr;
}

List<Listener^>^ EventManager::RegisteredListeners::get()
{
	return listeners;
}

