#pragma once

using namespace System;

public ref class InetAddress
{
public:
	InetAddress(array<Byte>^ address, String^ hostAddress, String^ hostName, String^ canonicalHostName);

	array<Byte>^ getAddress();
	String^ getHostAddress();
	String^ getHostName();
	String^ getCanonicalHostName();
	bool isAnyLocalAddress();
	bool isLinkLocalAddress();
	bool isLoopbackAddress();
	bool isMulticastAddress();
	bool isSiteLocalAddress();
	bool isReachable(int timeout);

private:
	array<Byte>^ m_address;
	String^ m_hostAddress;
	String^ m_hostName;
	String^ m_canonicalHostName;
};
