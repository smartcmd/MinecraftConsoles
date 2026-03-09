#include "InetAddress.h"

using namespace System;
using namespace System::Net;
using namespace System::Net::NetworkInformation;

InetAddress::InetAddress(array<Byte>^ address, String^ hostAddress, String^ hostName, String^ canonicalHostName)
	: m_address(address)
	, m_hostAddress(String::IsNullOrEmpty(hostAddress) ? String::Empty : hostAddress)
	, m_hostName(String::IsNullOrEmpty(hostName) ? String::Empty : hostName)
	, m_canonicalHostName(String::IsNullOrEmpty(canonicalHostName) ? String::Empty : canonicalHostName)
{
}

array<Byte>^ InetAddress::getAddress()
{
	return m_address;
}

String^ InetAddress::getHostAddress()
{
	return m_hostAddress;
}

String^ InetAddress::getHostName()
{
	return m_hostName;
}

String^ InetAddress::getCanonicalHostName()
{
	return m_canonicalHostName;
}

bool InetAddress::isAnyLocalAddress()
{
	IPAddress^ ip = nullptr;
	if (!IPAddress::TryParse(m_hostAddress, ip)) return false;
	return ip->Equals(IPAddress::Any) || ip->Equals(IPAddress::IPv6Any);
}

bool InetAddress::isLinkLocalAddress()
{
	IPAddress^ ip = nullptr;
	if (!IPAddress::TryParse(m_hostAddress, ip)) return false;
	if (ip->AddressFamily == System::Net::Sockets::AddressFamily::InterNetworkV6)
	{
		return ip->IsIPv6LinkLocal;
	}
	array<Byte>^ b = ip->GetAddressBytes();
	return b->Length == 4 && b[0] == 169 && b[1] == 254;
}

bool InetAddress::isLoopbackAddress()
{
	IPAddress^ ip = nullptr;
	if (!IPAddress::TryParse(m_hostAddress, ip)) return false;
	return IPAddress::IsLoopback(ip);
}

bool InetAddress::isMulticastAddress()
{
	IPAddress^ ip = nullptr;
	if (!IPAddress::TryParse(m_hostAddress, ip)) return false;
	array<Byte>^ b = ip->GetAddressBytes();
	if (b->Length == 4)
	{
		return b[0] >= 224 && b[0] <= 239;
	}
	return b->Length > 0 && b[0] == 0xFF;
}

bool InetAddress::isSiteLocalAddress()
{
	IPAddress^ ip = nullptr;
	if (!IPAddress::TryParse(m_hostAddress, ip)) return false;
	if (ip->AddressFamily == System::Net::Sockets::AddressFamily::InterNetworkV6)
	{
		return ip->IsIPv6SiteLocal;
	}
	array<Byte>^ b = ip->GetAddressBytes();
	if (b->Length != 4) return false;
	if (b[0] == 10) return true;
	if (b[0] == 172 && b[1] >= 16 && b[1] <= 31) return true;
	if (b[0] == 192 && b[1] == 168) return true;
	return false;
}

bool InetAddress::isReachable(int timeout)
{
	if (String::IsNullOrEmpty(m_hostAddress)) return false;
	try
	{
		Ping^ ping = gcnew Ping();
		PingReply^ reply = ping->Send(m_hostAddress, timeout);
		return reply != nullptr && reply->Status == IPStatus::Success;
	}
	catch (...)
	{
		return false;
	}
}
