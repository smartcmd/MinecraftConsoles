#include "stdafx.h"
//#include "..\Minecraft.World\JavaMath.h"
#include "MultiplayerLocalPlayer.h"
#include "ClientConnection.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.network.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\AABB.h"
#include "..\Minecraft.World\net.minecraft.stats.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.effect.h"
#include "..\Minecraft.World\LevelData.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"




MultiplayerLocalPlayer::MultiplayerLocalPlayer(Minecraft *minecraft, Level *level, User *user, ClientConnection *connection) : LocalPlayer(minecraft, level, user, level->dimension->id)
{
	// 4J - added initialisers
    flashOnSetHealth = false;
	xLast = yLast1 = yLast2 = zLast = 0;
    yRotLast = xRotLast = 0;
	lastOnGround = false;
    lastSneaked = false;
	lastIdle = false;
	lastSprinting = false;
    positionReminder = 0;

    this->connection = connection;
}

bool MultiplayerLocalPlayer::hurt(DamageSource *source, int dmg)
{
	return false;
}

void MultiplayerLocalPlayer::heal(int heal)
{
}

void MultiplayerLocalPlayer::tick()
{
	// 4J Added
	// 4J-PB - changing this to a game host option ot hide gamertags
	//bool bIsisPrimaryHost=g_NetworkManager.IsHost() && (ProfileManager.GetPrimaryPad()==m_iPad);

	/*if((app.GetGameSettings(m_iPad,eGameSetting_PlayerVisibleInMap)!=0) != m_bShownOnMaps)
	{
		m_bShownOnMaps = (app.GetGameSettings(m_iPad,eGameSetting_PlayerVisibleInMap)!=0);
		if (m_bShownOnMaps) connection->send( shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::SHOW_ON_MAPS) ) );
        else connection->send( shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::HIDE_ON_MAPS) ) );
	}*/

	if (!level->hasChunkAt(Mth::floor(x), 0, Mth::floor(z))) return;

	double tempX = x, tempY = y, tempZ = z;
    LocalPlayer::tick();
	
	//if( !minecraft->localgameModes[m_iPad]->isTutorial() || minecraft->localgameModes[m_iPad]->getTutorial()->canMoveToPosition(tempX, tempY, tempZ, x, y, z) )
	if(minecraft->localgameModes[m_iPad]->getTutorial()->canMoveToPosition(tempX, tempY, tempZ, x, y, z))
	{		
		sendPosition();
	}
	else
	{
		//app.Debugprintf("Cannot move to position (%f, %f, %f), falling back to (%f, %f, %f)\n", x, y, z, tempX, y, tempZ);
		this->setPos(tempX, y, tempZ);
	}
}

void MultiplayerLocalPlayer::sendPosition()
{
	bool sprinting = isSprinting();
	if (sprinting != lastSprinting)
	{
		if (sprinting) connection->send(shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::START_SPRINTING)));
		else connection->send(shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::STOP_SPRINTING)));

		lastSprinting = sprinting;
	}

    bool sneaking = isSneaking();
    if (sneaking != lastSneaked)
	{
        if (sneaking) connection->send( shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::START_SNEAKING) ) );
        else connection->send( shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::STOP_SNEAKING) ) );

        lastSneaked = sneaking;
    }

	bool idle = isIdle();
	if (idle != lastIdle)
	{
		if (idle) connection->send( shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::START_IDLEANIM) ) );
		else connection->send( shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::STOP_IDLEANIM) ) );

		lastIdle = idle;
	}

    double xdd = x - xLast;
    double ydd1 = bb->y0 - yLast1;
    double zdd = z - zLast;

    double rydd = yRot - yRotLast;
    double rxdd = xRot - xRotLast;

	bool move = (xdd * xdd + ydd1 * ydd1 + zdd * zdd) > 0.03 * 0.03 || positionReminder >= POSITION_REMINDER_INTERVAL;
    bool rot = rydd != 0 || rxdd != 0;
    if (riding != NULL)
	{
		connection->send( shared_ptr<MovePlayerPacket>( new MovePlayerPacket::PosRot(xd, -999, -999, zd, yRot, xRot, onGround, abilities.flying) ) );
        move = false;
    }
	else
	{
        if (move && rot)
		{
            connection->send( shared_ptr<MovePlayerPacket>( new MovePlayerPacket::PosRot(x, bb->y0, y, z, yRot, xRot, onGround, abilities.flying) ) );
        }
		else if (move)
		{
            connection->send( shared_ptr<MovePlayerPacket>( new MovePlayerPacket::Pos(x, bb->y0, y, z, onGround, abilities.flying) ) );
        }
		else if (rot)
		{
            connection->send( shared_ptr<MovePlayerPacket>( new MovePlayerPacket::Rot(yRot, xRot, onGround, abilities.flying) ) );
        }
		else
		{
		  connection->send( shared_ptr<MovePlayerPacket>( new MovePlayerPacket(onGround, abilities.flying) ) );
        }
    }

	positionReminder++;
    lastOnGround = onGround;

    if (move)
	{
        xLast = x;
        yLast1 = bb->y0;
        yLast2 = y;
        zLast = z;
		positionReminder = 0;
    }
    if (rot)
	{
        yRotLast = yRot;
        xRotLast = xRot;
    }

}

shared_ptr<ItemEntity> MultiplayerLocalPlayer::drop()
{
	connection->send( shared_ptr<PlayerActionPacket>( new PlayerActionPacket(PlayerActionPacket::DROP_ITEM, 0, 0, 0, 0) ) );
	return nullptr;
}

void MultiplayerLocalPlayer::reallyDrop(shared_ptr<ItemEntity> itemEntity)
{
}

void MultiplayerLocalPlayer::chat(const wstring& message)
{
	connection->send( shared_ptr<ChatPacket>( new ChatPacket(message) ) );
}

void MultiplayerLocalPlayer::swing()
{
	LocalPlayer::swing();
	connection->send( shared_ptr<AnimatePacket>( new AnimatePacket(shared_from_this(), AnimatePacket::SWING) ) );

}

void MultiplayerLocalPlayer::respawn()
{
	connection->send( shared_ptr<ClientCommandPacket>( new ClientCommandPacket(ClientCommandPacket::PERFORM_RESPAWN)));
}


void MultiplayerLocalPlayer::actuallyHurt(DamageSource *source, int dmg)
{
	setHealth(getHealth() - dmg);
}

// 4J Added override to capture event for tutorial messages
void MultiplayerLocalPlayer::completeUsingItem()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(useItem != NULL && pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		Tutorial *tutorial = gameMode->getTutorial();
		tutorial->completeUsingItem(useItem);
	}
	Player::completeUsingItem();
}

void MultiplayerLocalPlayer::onEffectAdded(MobEffectInstance *effect)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		Tutorial *tutorial = gameMode->getTutorial();
		tutorial->onEffectChanged(MobEffect::effects[effect->getId()]);
	}
	Player::onEffectAdded(effect);
}


void MultiplayerLocalPlayer::onEffectUpdated(MobEffectInstance *effect)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		Tutorial *tutorial = gameMode->getTutorial();
		tutorial->onEffectChanged(MobEffect::effects[effect->getId()]);
	}
	Player::onEffectUpdated(effect);
}


void MultiplayerLocalPlayer::onEffectRemoved(MobEffectInstance *effect)
{
	Minecraft *pMinecraft = Minecraft::GetInstance();
	if(pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		Tutorial *tutorial = gameMode->getTutorial();
		tutorial->onEffectChanged(MobEffect::effects[effect->getId()],true);
	}
	Player::onEffectRemoved(effect);
}

void MultiplayerLocalPlayer::closeContainer()
{
    connection->send( shared_ptr<ContainerClosePacket>( new ContainerClosePacket(containerMenu->containerId) ) );
    inventory->setCarried(nullptr);
    LocalPlayer::closeContainer();
}

void MultiplayerLocalPlayer::hurtTo(int newHealth, ETelemetryChallenges damageSource)
{
	if (flashOnSetHealth)
	{
		LocalPlayer::hurtTo(newHealth, damageSource);
	}
	else
	{
		setHealth(newHealth);
		flashOnSetHealth = true;
	}
}

void MultiplayerLocalPlayer::awardStat(Stat *stat, byteArray param)
{
    if (stat == NULL)
	{
		delete [] param.data;
        return;
    }

    if (stat->awardLocallyOnly)
	{
		LocalPlayer::awardStat(stat, param);
	}
	else
	{
		delete [] param.data;
        return;
    }
}

void MultiplayerLocalPlayer::awardStatFromServer(Stat *stat, byteArray param)
{
	if ( stat != NULL && !stat->awardLocallyOnly )
	{
		LocalPlayer::awardStat(stat, param);
	}
	else delete [] param.data;
}

void MultiplayerLocalPlayer::onUpdateAbilities()
{
	connection->send(shared_ptr<PlayerAbilitiesPacket>(new PlayerAbilitiesPacket(&abilities)));
}

bool MultiplayerLocalPlayer::isLocalPlayer()
{
	return true;
}

void MultiplayerLocalPlayer::ride(shared_ptr<Entity> e)
{
	bool wasRiding = riding != NULL;
	LocalPlayer::ride(e);
	bool isRiding = riding != NULL;

	if( isRiding )
	{
		ETelemetryChallenges eventType = eTelemetryChallenges_Unknown;
		if( this->riding != NULL )
		{
			switch(riding->GetType())
			{
			case eTYPE_BOAT:
				eventType = eTelemetryInGame_Ride_Boat;
				break;
			case eTYPE_MINECART:
				eventType = eTelemetryInGame_Ride_Minecart;
				break;
			case eTYPE_PIG:
				eventType = eTelemetryInGame_Ride_Pig;
				break;
			};
		}
		TelemetryManager->RecordEnemyKilledOrOvercome(GetXboxPad(), 0, y, 0, 0, 0, 0, eventType);
	}

	updateRichPresence();

	Minecraft *pMinecraft = Minecraft::GetInstance();
	
	if( pMinecraft->localgameModes[m_iPad] != NULL )
	{
		TutorialMode *gameMode = (TutorialMode *)pMinecraft->localgameModes[m_iPad];
		if(wasRiding && !isRiding)
		{
			gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Gameplay);
		}
		else if (!wasRiding && isRiding)
		{
			if(dynamic_pointer_cast<Minecart>(e) != NULL)
				gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Riding_Minecart);
			else if(dynamic_pointer_cast<Boat>(e) != NULL)
				gameMode->getTutorial()->changeTutorialState(e_Tutorial_State_Riding_Boat);
		}
	}
}

void MultiplayerLocalPlayer::StopSleeping()
{
	connection->send( shared_ptr<PlayerCommandPacket>( new PlayerCommandPacket(shared_from_this(), PlayerCommandPacket::STOP_SLEEPING) ) );
}

// 4J Added
void MultiplayerLocalPlayer::setAndBroadcastCustomSkin(DWORD skinId)
{
	DWORD oldSkinIndex = getCustomSkin();
	LocalPlayer::setCustomSkin(skinId);
#ifndef _CONTENT_PACKAGE
	wprintf(L"Skin for local player %ls has changed to %ls (%d)\n", name.c_str(), customTextureUrl.c_str(), getPlayerDefaultSkin() );
#endif
	if(getCustomSkin() != oldSkinIndex) connection->send( shared_ptr<TextureAndGeometryChangePacket>( new TextureAndGeometryChangePacket( shared_from_this(), app.GetPlayerSkinName(GetXboxPad()) ) ) );
}

void MultiplayerLocalPlayer::setAndBroadcastCustomCape(DWORD capeId)
{
	DWORD oldCapeIndex = getCustomCape();
	LocalPlayer::setCustomCape(capeId);
#ifndef _CONTENT_PACKAGE
	wprintf(L"Cape for local player %ls has changed to %ls\n", name.c_str(), customTextureUrl2.c_str());
#endif
	if(getCustomCape() != oldCapeIndex) connection->send( shared_ptr<TextureChangePacket>( new TextureChangePacket( shared_from_this(), TextureChangePacket::e_TextureChange_Cape, app.GetPlayerCapeName(GetXboxPad()) ) ) );
}
