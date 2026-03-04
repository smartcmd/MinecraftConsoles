


#include "stdafx.h"
#include "Orbis_PlayerUID.h"
#include "..\Minecraft.World\StringHelpers.h"

#include <libnetctl.h>


// taken from Boost, there is no TR1 equivalent
template <class T>
inline void hash_combine(std::size_t& seed, T const& v)
{
	hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

std::size_t PlayerUID::Hash::operator()(const PlayerUID& k) const
{
	//  now only hashing against the local ID, as this is now unique across the network too.
	std::size_t seed = 0;
	hash_combine(seed, k.m_userID);
 	for(int i=0; i<SCE_NET_ETHER_ADDR_LEN; i++)
 		hash_combine(seed, k.m_macAddress[i]);

	return seed;
}

PlayerUID::PlayerUID()
{
	memset(this,0,sizeof(PlayerUID));
} 


PlayerUID::PlayerUID(SceUserServiceUserId userID, SceNpOnlineId& onlineID, bool bSignedInPSN, int quadrant)
{
	m_userID = userID;
	m_quadrant = quadrant;
	setCurrentMacAddress();
	setOnlineID(onlineID, bSignedInPSN);
} 

PlayerUID::PlayerUID(wstring fromString)
{
	// create for the format 	P_5e7ff8372ea9_00000004_Mark_4J

	if(fromString[0] == L'P')
		m_quadrant = 0;	// primary player, so make this zero
	else
		m_quadrant = 1; //'N' might not be quadrant 1, but doesn't matter here

	// parse the mac address next
	for(int i=0;i<6;i++)
	{
		wstring macDigit = fromString.substr(2+(i*2),2);
		m_macAddress[i] = _fromHEXString<int>(macDigit);
	}

	int userIdLen = 0;

	// parse the userID
	wstring userIDDigits = L"";
	
	while(fromString.at(15 + userIdLen) != L'_')
	{
		userIDDigits.push_back(fromString.at(15 + userIdLen));
		++userIdLen;
	}
	fromString.substr(15,8);
	m_userID = _fromString<int>(userIDDigits);

	// finally, the onlineID, if there is one
	wstring onlineID = fromString.substr(15 + userIdLen + 1);
	if(onlineID.size() > 0)
	{
		wcstombs(m_onlineID, onlineID.c_str(), 16);
		m_bSignedIntoPSN = true;
	}
	else
	{
		m_onlineID[0] = 0;
		m_bSignedIntoPSN = false;
	}
}


bool PlayerUID::operator==(const PlayerUID& rhs) const
{
	// comparing online IDs	
	if(isSignedIntoPSN() && rhs.isSignedIntoPSN())
	{
		return (strcmp(m_onlineID, rhs.m_onlineID) == 0);
	}
	// comparing offline IDs	
	if(m_userID != rhs.m_userID)
		return false;
	for(int i=0; i<SCE_NET_ETHER_ADDR_LEN;i++)
		if(m_macAddress[i] != rhs.m_macAddress[i])return false;
	return true;
}

bool PlayerUID::operator!=(const PlayerUID& rhs)
{
	return !(*this==rhs);
}

void PlayerUID::setCurrentMacAddress()
{
	// get the mac address of this machine
	SceNetEtherAddr etherAddr;
	int err = sceNetGetMacAddress(&etherAddr, 0);
	assert(err == 0);

	for(int i=0;i<SCE_NET_ETHER_ADDR_LEN; i++)
		m_macAddress[i] = etherAddr.data[i];
}

std::wstring g_testStringW;

std::wstring PlayerUID::macAddressStr() const
{
	wchar_t macAddr[16];
	const uint8_t* m = m_macAddress;
	swprintf(macAddr, 16, L"%02x%02x%02x%02x%02x%02x", m[0],m[1],m[2],m[3],m[4],m[5]);
	g_testStringW = std::wstring(macAddr);
	return g_testStringW;
}

std::wstring PlayerUID::userIDStr() const
{
	char finalString[16];
	wchar_t finalStringW[16];
	sprintf(finalString, "%08d", m_userID);
	mbstowcs(finalStringW, finalString, 64);
	std::wstring retVal = finalStringW;
	return retVal;
}

std::wstring PlayerUID::toString() const
{
	char macAddr[16];
	char finalString[64];
	wchar_t finalStringW[64];

	const uint8_t* m = m_macAddress;
	sprintf(macAddr, "%02x%02x%02x%02x%02x%02x", m[0],m[1],m[2],m[3],m[4],m[5]);
	sprintf(finalString, "%s_%s_%08d_%s",	isPrimaryUser() ? "P" : "N", 
		macAddr, m_userID, 
		isSignedIntoPSN() ? m_onlineID : "");
	mbstowcs(finalStringW, finalString, 64);
	return std::wstring(finalStringW);
}
void PlayerUID::setOnlineID(SceNpOnlineId& id, bool bSignedIntoPSN)
{
	memcpy(m_onlineID, id.data, SCE_NP_ONLINEID_MAX_LENGTH);
	term = id.term;
	m_bSignedIntoPSN = bSignedIntoPSN;
}
void PlayerUID::setUserID(unsigned int id) { m_userID = id; }

bool PlayerUID::isPrimaryUser() const /* only true if we're on the local machine and signed into the first quadrant */
{
	if(m_quadrant != 0)
		return false;

	// get the mac address of this machine
	SceNetEtherAddr etherAddr;
	int err = sceNetGetMacAddress(&etherAddr, 0);
	assert(err == 0);
	bool macIsLocal = true;
	for(int i=0;i<SCE_NET_ETHER_ADDR_LEN; i++)
	{
		if(m_macAddress[i] != etherAddr.data[i])
			macIsLocal = false;
	}
	return macIsLocal;		// only true if we're on the local machine and signed into the first quadrant
}





GameSessionUID::GameSessionUID()
{
	memset(this,0,sizeof(GameSessionUID));
} 
GameSessionUID::GameSessionUID(int nullVal)
{
	assert(nullVal == 0);
	memset(this,0,sizeof(GameSessionUID));
}  

bool GameSessionUID::operator==(const GameSessionUID& rhs) const
{
	// comparing online IDs	
	if( getQuadrant() != rhs.getQuadrant() )
	{
		return false;
	}
	return (strcmp(m_onlineID, rhs.m_onlineID) == 0);
}
bool GameSessionUID::operator!=(const GameSessionUID& rhs)
{
	return !(*this==rhs);
}


GameSessionUID& GameSessionUID::operator=(const PlayerUID& rhs)
{
	memcpy(m_onlineID, rhs.getOnlineID(), SCE_NP_ONLINEID_MAX_LENGTH);
	term = 0;
	m_quadrant = rhs.getQuadrant();
	m_bSignedIntoPSN = rhs.isSignedIntoPSN();
	return *this;
}

