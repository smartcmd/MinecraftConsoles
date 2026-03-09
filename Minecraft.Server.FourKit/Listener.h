#pragma once

using namespace System;

public interface class ServerPlugin
{
public:
	void OnEnable();
	
	void OnDisable();
	
	String^ GetName();
	String^ GetVersion();
	String^ GetAuthor();
};

public interface class Listener
{
};

[AttributeUsage(AttributeTargets::Method)]
public ref class EventHandlerAttribute : public Attribute
{
};
