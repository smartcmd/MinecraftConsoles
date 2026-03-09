#include "Player.h"
#include "NativePlayerCallbacks.h"

using namespace System;
using namespace System::Net;

Player::Player(String^ name)
	: m_name(name), m_health(20.0f), m_food(20), m_fallDistance(0.0f), 
	  m_yRot(0.0f), m_xRot(0.0f), m_sneaking(false), m_sprinting(false),
	  m_x(0.0), m_y(0.0), m_z(0.0), m_dimension(0)
{
}

void Player::SetPlayerData(float health, int food, float fallDistance, float yRot, float xRot,
                           bool sneaking, bool sprinting,
                           double x, double y, double z, int dimension)
{
	m_health = health;
	m_food = food;
	m_fallDistance = fallDistance;
	m_yRot = yRot;
	m_xRot = xRot;
	m_sneaking = sneaking;
	m_sprinting = sprinting;
	m_x = x;
	m_y = y;
	m_z = z;
	m_dimension = dimension;
}

void Player::setFallDistance(float distance)
{
	m_fallDistance = distance;
	NativePlayerCallbacks::SetFallDistance(m_name, distance);
}

void Player::setHealth(float health)
{
	m_health = health;
	NativePlayerCallbacks::SetHealth(m_name, health);
}

void Player::setFood(int food)
{
	m_food = food;
	NativePlayerCallbacks::SetFood(m_name, food);
}

//void Player::setSneaking(bool sneaking)
//{
//	m_sneaking = sneaking;
//	NativePlayerCallbacks::SetSneaking(m_name, sneaking);
//}

void Player::setSprinting(bool sprinting)
{
	m_sprinting = sprinting;
	NativePlayerCallbacks::SetSprinting(m_name, sprinting);
}

void Player::sendMessage(String^ message)
{
	NativePlayerCallbacks::SendMessage(m_name, message);
}

void Player::kickPlayer() // String ^ reason
{
    NativePlayerCallbacks::Kick(m_name, "");
}

void Player::teleport(double x, double y, double z)
{
	m_x = x;
	m_y = y;
	m_z = z;
	NativePlayerCallbacks::TeleportTo(m_name, x, y, z);
}

InetSocketAddress^ Player::getAddress()
{
	PlayerNetworkAddressData data;
	if (!NativePlayerCallbacks::GetPlayerNetworkAddress(m_name, &data))
	{
		return nullptr;
	}

	String^ hostAddress = gcnew String(data.hostAddress);
	String^ hostName = gcnew String(data.hostName);
	String^ hostString = gcnew String(data.hostString);
	String^ canonicalHostName = hostName;

	array<Byte>^ addressBytes = gcnew array<Byte>(0);
	IPAddress^ ip = nullptr;
	if (IPAddress::TryParse(hostAddress, ip))
	{
		addressBytes = ip->GetAddressBytes();
		try
		{
			IPHostEntry^ entry = Dns::GetHostEntry(ip);
			if (entry != nullptr && !String::IsNullOrEmpty(entry->HostName))
			{
				canonicalHostName = entry->HostName;
				if (String::IsNullOrEmpty(hostName))
				{
					hostName = entry->HostName;
				}
			}
		}
		catch (...)
		{
		}
	}

	InetAddress^ address = gcnew InetAddress(addressBytes, hostAddress, hostName, canonicalHostName);
	return gcnew InetSocketAddress(hostName, hostString, data.port, data.unresolved, address);
}
