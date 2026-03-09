#include "InetSocketAddress.h"

using namespace System;

InetSocketAddress::InetSocketAddress(String^ hostName, String^ hostString, int port, bool unresolved, InetAddress^ address)
	: m_hostName(String::IsNullOrEmpty(hostName) ? String::Empty : hostName)
	, m_hostString(String::IsNullOrEmpty(hostString) ? String::Empty : hostString)
	, m_port(port)
	, m_unresolved(unresolved)
	, m_address(address)
{
}

String^ InetSocketAddress::getHostName()
{
	return m_hostName;
}

int InetSocketAddress::getPort()
{
	return m_port;
}

String^ InetSocketAddress::getHostString()
{
	return m_hostString;
}

bool InetSocketAddress::isUnresolved()
{
	return m_unresolved;
}

InetAddress^ InetSocketAddress::getAddress()
{
	return m_address;
}
