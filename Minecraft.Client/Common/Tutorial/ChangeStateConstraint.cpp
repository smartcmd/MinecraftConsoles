#include "stdafx.h"

#include "Tutorial.h"
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "../../../Minecraft.World/net.minecraft.world.level.h"
#include "ChangeStateConstraint.h"
#include "../../../Minecraft.World/AABB.h"
#include "../../ClientConnection.h"
#include "../../../Minecraft.World/net.minecraft.network.packet.h"

ChangeStateConstraint::ChangeStateConstraint( Tutorial *tutorial, eTutorial_State targetState, eTutorial_State sourceStates[], DWORD sourceStatesCount,
	double x0, double y0, double z0, double x1, double y1, double z1, bool contains /*= true*/, bool changeGameMode /*= false*/, GameType *targetGameMode /*= 0*/ )
	: TutorialConstraint( -1 )
{
	movementArea = AABB::newPermanent(x0, y0, z0, x1, y1, z1);

	this->contains = contains;

	m_changeGameMode = changeGameMode;
	m_targetGameMode = targetGameMode;
	m_changedFromGameMode = 0;

	m_tutorial = tutorial;
	m_targetState = targetState;
	m_sourceStatesCount = sourceStatesCount;
	
	m_bHasChanged = false;
	m_changedFromState = e_Tutorial_State_None;

	m_bComplete = false;

	m_sourceStates = new eTutorial_State [m_sourceStatesCount];
	for(unsigned int i=0;i<m_sourceStatesCount;i++)
	{
		m_sourceStates[i]=sourceStates[i];
	}
}

ChangeStateConstraint::~ChangeStateConstraint()
{
	delete movementArea;
	if(m_sourceStatesCount>0) delete [] m_sourceStates;
}

void ChangeStateConstraint::tick(int iPad)
{
	if(m_bComplete) return;

	if(m_tutorial->isStateCompleted(m_targetState))
	{
		Minecraft *minecraft = Minecraft::GetInstance();
		if(m_changeGameMode)
		{
			unsigned int playerPrivs = minecraft->localplayers[iPad]->getAllPlayerGamePrivileges();
			Player::setPlayerGamePrivilege(playerPrivs,Player::ePlayerGamePrivilege_CreativeMode,m_changedFromGameMode == GameType::CREATIVE);

			unsigned int originalPrivileges = minecraft->localplayers[iPad]->getAllPlayerGamePrivileges();
			if(originalPrivileges != playerPrivs)
			{
				// Send update settings packet to server
				Minecraft *pMinecraft = Minecraft::GetInstance();				
				shared_ptr<MultiplayerLocalPlayer> player = minecraft->localplayers[iPad];
				if(player != NULL && player->connection && player->connection->getNetworkPlayer() != NULL)
				{
					player->connection->send( shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket( player->connection->getNetworkPlayer()->GetSmallId(), -1, playerPrivs) ) );
				}
			}
		}
		m_bComplete = true;
		return;
	}

	bool inASourceState = false;
	Minecraft *minecraft = Minecraft::GetInstance();
	for(DWORD i = 0; i < m_sourceStatesCount; ++i)
	{
		if(m_sourceStates[i] == m_tutorial->getCurrentState())
		{
			inASourceState = true;
			break;
		}
	}
	if( !m_bHasChanged && inASourceState && movementArea->contains( minecraft->localplayers[iPad]->getPos(1) ) == contains )
	{
		m_bHasChanged = true;
		m_changedFromState = m_tutorial->getCurrentState();
		m_tutorial->changeTutorialState(m_targetState);

		if(m_changeGameMode)
		{
			if(minecraft->localgameModes[iPad] != NULL)
			{
				m_changedFromGameMode = minecraft->localplayers[iPad]->abilities.instabuild ? GameType::CREATIVE : GameType::SURVIVAL;

				unsigned int playerPrivs = minecraft->localplayers[iPad]->getAllPlayerGamePrivileges();
				Player::setPlayerGamePrivilege(playerPrivs,Player::ePlayerGamePrivilege_CreativeMode,m_targetGameMode == GameType::CREATIVE);

				unsigned int originalPrivileges = minecraft->localplayers[iPad]->getAllPlayerGamePrivileges();
				if(originalPrivileges != playerPrivs)
				{
					// Send update settings packet to server
					Minecraft *pMinecraft = Minecraft::GetInstance();				
					shared_ptr<MultiplayerLocalPlayer> player = minecraft->localplayers[iPad];
					if(player != NULL && player->connection && player->connection->getNetworkPlayer() != NULL)
					{
						player->connection->send( shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket( player->connection->getNetworkPlayer()->GetSmallId(), -1, playerPrivs) ) );
					}
				}
			}
		}
	}
	else if( m_bHasChanged && movementArea->contains( minecraft->localplayers[iPad]->getPos(1) ) != contains )
	{
		m_bHasChanged = false;
		m_tutorial->changeTutorialState(m_changedFromState);

		if(m_changeGameMode)
		{
			unsigned int playerPrivs = minecraft->localplayers[iPad]->getAllPlayerGamePrivileges();
			Player::setPlayerGamePrivilege(playerPrivs,Player::ePlayerGamePrivilege_CreativeMode,m_changedFromGameMode == GameType::CREATIVE);

			unsigned int originalPrivileges = minecraft->localplayers[iPad]->getAllPlayerGamePrivileges();
			if(originalPrivileges != playerPrivs)
			{
				// Send update settings packet to server
				Minecraft *pMinecraft = Minecraft::GetInstance();				
				shared_ptr<MultiplayerLocalPlayer> player = minecraft->localplayers[iPad];
				if(player != NULL && player->connection && player->connection->getNetworkPlayer() != NULL)
				{
					player->connection->send( shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket( player->connection->getNetworkPlayer()->GetSmallId(), -1, playerPrivs) ) );
				}
			}
		}
	}
}