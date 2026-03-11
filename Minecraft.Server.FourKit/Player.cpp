#include "Player.h"
#include "NativePlayerCallbacks.h"
#include "World.h"

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

World^ Player::getWorld()
{
	return gcnew World(m_dimension);
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

bool Player::getAllowFlight()
{
	return NativePlayerCallbacks::GetAllowFlight(m_name);
}

void Player::setAllowFlight(bool flight)
{
	NativePlayerCallbacks::SetAllowFlight(m_name, flight);
}

float Player::getExhaustion()
{
	return NativePlayerCallbacks::GetExhaustion(m_name);
}

void Player::setExhaustion(float value)
{
	NativePlayerCallbacks::SetExhaustion(m_name, value);
}

float Player::getSaturation()
{
	return NativePlayerCallbacks::GetSaturation(m_name);
}

void Player::setSaturation(float value)
{
	NativePlayerCallbacks::SetSaturation(m_name, value);
}

void Player::giveExp(int amount)
{
	NativePlayerCallbacks::GiveExp(m_name, amount);
}

void Player::giveExpLevels(int amount)
{
	NativePlayerCallbacks::GiveExpLevels(m_name, amount);
}

int Player::getTotalExperience()
{
	return NativePlayerCallbacks::GetTotalExperience(m_name);
}

bool Player::isFlying()
{
	return NativePlayerCallbacks::IsFlying(m_name);
}

void Player::setFlying(bool value)
{
	NativePlayerCallbacks::SetFlying(m_name, value);
}

void Player::setExp(float exp)
{
	NativePlayerCallbacks::SetExp(m_name, exp);
}

void Player::setLevel(int level)
{
	NativePlayerCallbacks::SetPlayerLevel(m_name, level);
}

void Player::setWalkSpeed(float value)
{
	NativePlayerCallbacks::SetWalkSpeed(m_name, value);
}

float Player::getWalkSpeed()
{
	return NativePlayerCallbacks::GetWalkSpeed(m_name);
}

ItemStack^ Player::getItemInHand()
{
	ItemInHandData data;
	if (!NativePlayerCallbacks::GetItemInHand(m_name, &data) || !data.hasItem)
	{
		return nullptr;
	}

	return gcnew ItemStack(data.itemId, data.count, data.data);
}

void Player::setItemInHand(ItemStack^ item)
{
	if (item == nullptr)
	{
		NativePlayerCallbacks::SetItemInHand(m_name, 0, 0, 0);
	}
	else
	{
		NativePlayerCallbacks::SetItemInHand(m_name, item->getTypeId(), item->getAmount(), item->getData());
	}
}

GameMode Player::getGameMode()
{
	int mode = NativePlayerCallbacks::GetGameMode(m_name);
	switch (mode)
	{
	case 0: return GameMode::SURVIVAL;
	case 1: return GameMode::CREATIVE;
	case 2: return GameMode::ADVENTURE;
	default: return GameMode::SURVIVAL;
	}
}

void Player::setGameMode(GameMode mode)
{
	NativePlayerCallbacks::SetPlayerGameMode(m_name, (int)mode);
}

bool Player::isSleeping()
{
	return NativePlayerCallbacks::IsSleepingPlayer(m_name);
}
