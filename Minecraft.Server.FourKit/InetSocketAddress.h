#pragma once

#include "InetAddress.h"

using namespace System;

public ref class InetSocketAddress
{
public:
	InetSocketAddress(String^ hostName, String^ hostString, int port, bool unresolved, InetAddress^ address);

	String^ getHostName();
	int getPort();
	String^ getHostString();
	bool isUnresolved();
	InetAddress^ getAddress();

private:
	String^ m_hostName;
	String^ m_hostString;
	int m_port;
	bool m_unresolved;
	InetAddress^ m_address;
};
