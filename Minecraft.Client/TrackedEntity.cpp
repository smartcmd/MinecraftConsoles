#include "stdafx.h"
#include "TrackedEntity.h"
#include "ServerPlayer.h"
#include "PlayerConnection.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.entity.monster.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\Minecraft.World\net.minecraft.world.entity.global.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\net.minecraft.network.packet.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.saveddata.h"
#include "..\Minecraft.World\net.minecraft.world.entity.ai.attributes.h"
#include "MinecraftServer.h"
#include "ServerLevel.h"
#include "PlayerList.h"
#include "EntityTracker.h"
#include "PlayerChunkMap.h"
#include <qnet.h>

#include <memory>

TrackedEntity::TrackedEntity(shared_ptr<Entity> e, int range, int updateInterval, bool trackDelta)
{
	// 4J added initialisers
	xap = yap = zap = 0;
	tickCount = 0;
	xpu = ypu = zpu = 0;
	updatedPlayerVisibility = false;
	teleportDelay = 0;
	moved = false;
	wasRiding = false;

	this->e = e;
	this->range = range;
	this->updateInterval = updateInterval;
	this->trackDelta = trackDelta;

	xp = Mth::floor(e->x * 32);
	yp = Mth::floor(e->y * 32);
	zp = Mth::floor(e->z * 32);

	yRotp = Mth::floor(e->yRot * 256 / 360);
	xRotp = Mth::floor(e->xRot * 256 / 360);
	yHeadRotp = Mth::floor(e->getYHeadRot() * 256 / 360);
}

int c0a = 0, c0b = 0, c1a = 0, c1b = 0, c1c = 0, c2a = 0, c2b = 0;

void TrackedEntity::tick(EntityTracker *tracker, vector<shared_ptr<Player> > *players)
{
	moved = false;
	if (!updatedPlayerVisibility || e->distanceToSqr(xpu, ypu, zpu) > 4 * 4)
	{
		xpu = e->x;
		ypu = e->y;
		zpu = e->z;
		updatedPlayerVisibility = true;
		moved = true;
		updatePlayers(tracker, players);
	}

	if (lastRidingEntity != e->riding || (e->riding != nullptr && tickCount % (SharedConstants::TICKS_PER_SECOND * 3) == 0))
	{
		lastRidingEntity = e->riding;
		broadcast(std::make_shared<SetEntityLinkPacket>(SetEntityLinkPacket::RIDING, e, e->riding));
	}

	// Moving forward  special case for item frames
	if (e->GetType()== eTYPE_ITEM_FRAME && tickCount % 10 == 0)
	{
		shared_ptr<ItemFrame> frame = dynamic_pointer_cast<ItemFrame> (e);
		shared_ptr<ItemInstance> item = frame->getItem();

		if (item != nullptr && item->getItem()->id == Item::map_Id && !e->removed)
		{
			shared_ptr<MapItemSavedData> data = Item::map->getSavedData(item, e->level);
			for (auto& it : *players)
			{
				shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(it);
				data->tickCarriedBy(player, item);

				if (!player->removed && player->connection && player->connection->countDelayedPackets() <= 5)
				{
					shared_ptr<Packet> packet = Item::map->getUpdatePacket(item, e->level, player);
					if (packet != nullptr) player->connection->send(packet);
				}
			}
		}

		shared_ptr<SynchedEntityData> entityData = e->getEntityData();
		if (entityData->isDirty())
		{
			broadcastAndSend(std::make_shared<SetEntityDataPacket>(e->entityId, entityData, false));
		}
	}
	else if (tickCount % updateInterval == 0 || e->hasImpulse || e->getEntityData()->isDirty())
	{
		// 4J: Moved this as it's shared
		int yRotn = Mth::floor(e->yRot * 256 / 360);
		int xRotn = Mth::floor(e->xRot * 256 / 360);

		// 4J: Changed rotation to be generally sent as a delta as well as position
		int yRota = yRotn - yRotp;
		int xRota = xRotn - xRotp;

		if(e->riding == nullptr)
		{
			teleportDelay++;

			int xn = Mth::floor(e->x * 32.0);
			int yn = Mth::floor(e->y * 32.0);
			int zn = Mth::floor(e->z * 32.0);

			int xa = xn - xp;
			int ya = yn - yp;
			int za = zn - zp;

			shared_ptr<Packet> packet = nullptr;

			// 4J - this pos flag used to be set based on abs(xn) etc. but that just seems wrong
			bool pos = abs(xa) >= TOLERANCE_LEVEL || abs(ya) >= TOLERANCE_LEVEL || abs(za) >= TOLERANCE_LEVEL || (tickCount % (SharedConstants::TICKS_PER_SECOND * 3) == 0);

			// Keep rotation deltas in +/- 180 degree range
			while( yRota > 127 ) yRota -= 256;
			while( yRota < -128 ) yRota += 256;
			while( xRota > 127 ) xRota -= 256;
			while( xRota < -128 ) xRota += 256;

			bool rot = abs(yRota) >= TOLERANCE_LEVEL || abs(xRota) >= TOLERANCE_LEVEL;

			// 4J: Modified the following check. It was originally added by Mojang to address
			// certain unspecified issues with entity position. Turns out the issue effects a
			// variety of different entities so we've left it in and just added the new exceptions
			// (so far just players)

			// 4J: Original comment follows
			// TODO: Figure out how to fix this properly
			// skip first tick since position is sent in addEntity packet
			// FallingTile depends on this because it removes its source block in the first tick()

			if (tickCount > 0 || e->instanceof(eTYPE_ARROW) || e->instanceof(eTYPE_PLAYER)) // 4J: Modifed, see above
			{
				if (xa < -128 || xa >= 128 || ya < -128 || ya >= 128 || za < -128 || za >= 128 || wasRiding
					// 4J Stu - I fixed the initialisation of teleportDelay in the ctor, but we managed this far without out
						// and would prefer not to have all the extra traffix so ignore it
							// 4J Stu - Fix for #9579 - GAMEPLAY: Boats with a player in them slowly sink under the water over time, and with no player in them they float into the sky.
								|| (e->GetType() == eTYPE_BOAT && teleportDelay > 20 * 20)
								)
				{
					teleportDelay = 0;
					packet = std::make_shared<TeleportEntityPacket>(e->entityId, xn, yn, zn, static_cast<byte>(yRotn), static_cast<byte>(xRotn));
					//			printf("%d: New teleport rot %d\n",e->entityId,yRotn);
					yRotp = yRotn;
					xRotp = xRotn;
				}
				else
				{
					if (pos && rot)
					{
						// 4J If the movement is small enough, and there's no xrot, then use the new smaller packets
						if( ( xa >= -16 ) && ( xa <= 15 ) &&
							( za >= -16 ) && ( za <= 15 ) &&
							( ya >= -32 ) && ( ya <= 31 ) &&
							( xRota == 0 ))
						{
							// Clamp rotations that are too big
							if( yRota < -16 )
							{
								yRota = -16;
								yRotn = yRotp + yRota;
							}
							else if( yRota > 15 )
							{
								yRota = 15;
								yRotn = yRotp + yRota;
							}
							// 5 bits each for x & z, and 6 for y
							packet = std::make_shared<MoveEntityPacketSmall::PosRot>(e->entityId, static_cast<char>(xa), static_cast<char>(ya), static_cast<char>(za), static_cast<char>(yRota), 0);
							c0a++;
						}
						else
						{
							packet = std::make_shared<MoveEntityPacket::PosRot>(e->entityId, static_cast<char>(xa), static_cast<char>(ya), static_cast<char>(za), static_cast<char>(yRota), static_cast<char>(xRota));
							//					printf("%d: New posrot %d + %d = %d\n",e->entityId,yRotp,yRota,yRotn);
							c0b++;
						}
					}
					else if (pos)
					{
						// 4J If the movement is small enough, then use the new smaller packets
						if( ( xa >= -8 ) && ( xa <= 7 ) &&
							( za >= -8 ) && ( za <= 7 ) &&
							( ya >= -16 ) && ( ya <= 15 ) )
						{
							// 4 bits each for x & z, and 5 for y
							packet = std::make_shared<MoveEntityPacketSmall::Pos>(e->entityId, static_cast<char>(xa), static_cast<char>(ya), static_cast<char>(za));
							c1a++;
						}

						else if( ( xa >= -16 ) && ( xa <= 15 ) &&
							( za >= -16 ) && ( za <= 15 ) &&
							( ya >= -32 ) && ( ya <= 31 ) )
						{
							// use the packet with small packet with rotation if we can - 5 bits each for x & z, and 6 for y - still a byte less than the alternative
							packet = std::make_shared<MoveEntityPacketSmall::PosRot>(e->entityId, static_cast<char>(xa), static_cast<char>(ya), static_cast<char>(za), 0, 0);
							c1b++;
						}
						else
						{
							packet = std::make_shared<MoveEntityPacket::Pos>(e->entityId, static_cast<char>(xa), static_cast<char>(ya), static_cast<char>(za));
							c1c++;
						}
					}
					else if (rot)
					{
						// 4J If there's no x rotation, then use the new smaller packet type
						if( xRota == 0 )
						{
							// Clamp rotations that are too big
							if( yRota < -16 )
							{
								yRota = -16;
								yRotn = yRotp + yRota;
							}
							else if( yRota > 15 )
							{
								yRota = 15;
								yRotn = yRotp + yRota;
							}
							packet = std::make_shared<MoveEntityPacketSmall::Rot>(e->entityId, static_cast<char>(yRota), 0);
							c2a++;
						}
						else
						{
							//					printf("%d: New rot %d + %d = %d\n",e->entityId,yRotp,yRota,yRotn);
							packet = std::make_shared<MoveEntityPacket::Rot>(e->entityId, static_cast<char>(yRota), static_cast<char>(xRota));
							c2b++;
						}
					}
				}
			}

			if (trackDelta)
			{
				double xad = e->xd - xap;
				double yad = e->yd - yap;
				double zad = e->zd - zap;

				double max = 0.02;

				double diff = xad * xad + yad * yad + zad * zad;

				if (diff > max * max || (diff > 0 && e->xd == 0 && e->yd == 0 && e->zd == 0))
				{
					xap = e->xd;
					yap = e->yd;
					zap = e->zd;
					broadcast(std::make_shared<SetEntityMotionPacket>(e->entityId, xap, yap, zap));
				}

			}

			if (packet != nullptr)
			{
				broadcast(packet);
			}

			sendDirtyEntityData();

			if (pos)
			{
				xp = xn;
				yp = yn;
				zp = zn;
			}
			if (rot)
			{
				yRotp = yRotn;
				xRotp = xRotn;
			}

			wasRiding = false;
		}
		else
		{
			bool rot = abs(yRotn - yRotp) >= TOLERANCE_LEVEL || abs(xRotn - xRotp) >= TOLERANCE_LEVEL;
			if (rot)
			{
				// 4J: Changed this to use deltas
				broadcast(std::make_shared<MoveEntityPacket::Rot>(e->entityId, static_cast<byte>(yRota), static_cast<byte>(xRota)));
				yRotp = yRotn;
				xRotp = xRotn;
			}

			xp = Mth::floor(e->x * 32.0);
			yp = Mth::floor(e->y * 32.0);
			zp = Mth::floor(e->z * 32.0);

			sendDirtyEntityData();

			wasRiding = true;
		}

		int yHeadRot = Mth::floor(e->getYHeadRot() * 256 / 360);
		if (abs(yHeadRot - yHeadRotp) >= TOLERANCE_LEVEL)
		{
			broadcast(std::make_shared<RotateHeadPacket>(e->entityId, static_cast<byte>(yHeadRot)));
			yHeadRotp = yHeadRot;
		}

		e->hasImpulse = false;
	}

	tickCount++;

	if (e->hurtMarked)
	{
		// broadcast(new AnimatePacket(e, AnimatePacket.HURT));
		broadcastAndSend(std::make_shared<SetEntityMotionPacket>(e));
		e->hurtMarked = false;
	}

}

void TrackedEntity::sendDirtyEntityData()
{
	shared_ptr<SynchedEntityData> entityData = e->getEntityData();
	if (entityData->isDirty())
	{
		broadcastAndSend(std::make_shared<SetEntityDataPacket>(e->entityId, entityData, false));
	}

	if ( e->instanceof(eTYPE_LIVINGENTITY) )
	{
		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(e);
		ServersideAttributeMap *attributeMap = static_cast<ServersideAttributeMap *>(living->getAttributes());
		unordered_set<AttributeInstance *> *attributes = attributeMap->getDirtyAttributes();

		if (!attributes->empty())
		{
			broadcastAndSend(std::make_shared<UpdateAttributesPacket>(e->entityId, attributes));
		}

		attributes->clear();
	}
}

void TrackedEntity::broadcast(shared_ptr<Packet> packet)
{
	if( Packet::canSendToAnyClient( packet ) )
	{
		// 4J-PB - due to the knockback on a player being hit, we need to send to all players, but limit the network traffic here to players that have not already had it sent to their system
		vector< shared_ptr<ServerPlayer> > sentTo;

		// 4J - don't send to a player we've already sent this data to that shares the same machine.
		// EntityMotionPacket used to limit themselves to sending once to each machine
		// by only sending to the primary player on each machine. This was causing trouble for split screen
		// as only the primary player would get a knockback velocity. Now these packets can be sent to any
		// player, but we try to restrict the network impact this has by not resending to the one machine

		for( auto& player : seenBy)
		{
			bool dontSend = false;
			if( sentTo.size() )
			{
				INetworkPlayer *thisPlayer =player->connection->getNetworkPlayer();
				if( thisPlayer == nullptr )
				{
					dontSend = true;
				}
				else
				{
					for(auto& player2 : sentTo)
					{
						INetworkPlayer *otherPlayer = player2->connection->getNetworkPlayer();
						if( otherPlayer != nullptr && thisPlayer->IsSameSystem(otherPlayer) )
						{
							dontSend = true;
						}
					}
				}
			}
			if( dontSend )
			{
				continue;
			}


			player->connection->send(packet);
			sentTo.push_back(player);
		}
	}
	else
	{
		// This packet hasn't got canSendToAnyClient set, so just send to everyone here, and it

		for(auto& it : seenBy)
		{
			it->connection->send(packet);
		}
	}
}

void TrackedEntity::broadcastAndSend(shared_ptr<Packet> packet)
{
	vector< shared_ptr<ServerPlayer> > sentTo;
	broadcast(packet);
	shared_ptr<ServerPlayer> sp = e->instanceof(eTYPE_SERVERPLAYER) ? dynamic_pointer_cast<ServerPlayer>(e) : nullptr;
	if (sp != nullptr && sp->connection)
	{
		sp->connection->send(packet);
	}
}

void TrackedEntity::broadcastRemoved()
{
	for(const auto& it : seenBy)
	{
		it->entitiesToRemove.push_back(e->entityId);
	}
}

void TrackedEntity::removePlayer(shared_ptr<ServerPlayer> sp)
{
    auto it = seenBy.find(sp);
    if( it != seenBy.end() )
	{
		sp->entitiesToRemove.push_back(e->entityId);
		seenBy.erase( it );
	}
}

// 4J-JEV: Added for code reuse.
TrackedEntity::eVisibility TrackedEntity::isVisible(EntityTracker *tracker, shared_ptr<ServerPlayer> sp, bool forRider)
{
	// 4J Stu - We call update players when the entity has moved more than a certain amount at the start of it's tick
	// Before this call we set xpu, ypu and zpu to the entities new position, but xp,yp and zp are the old position until later in the tick.
	// Therefore we should use the new position for visibility checks
	double xd = sp->x - xpu; //xp / 32;
	double zd = sp->z - zpu; //zp / 32;

	// 4J Stu - Fix for loading a player who is currently riding something (e.g. a horse)
	if(e->forcedLoading)
	{
		xd = sp->x - xp / 32;
		zd = sp->z - zp / 32;
	}

	int playersRange = range;
	if( playersRange > TRACKED_ENTITY_MINIMUM_VIEW_DISTANCE  )
	{
		playersRange -= sp->getPlayerViewDistanceModifier();
	}

	bool bVisible = xd >= -playersRange && xd <= playersRange && zd >= -playersRange && zd <= playersRange;
	bool canBeSeenBy = canBySeenBy(sp);

	// 4J - added. Try and find other players who are in the same dimension as this one and on the same machine, and extend our visibility
	// so things are consider visible to this player if they are near the other one. This is because we only send entity tracking info to
	// players who canReceiveAllPackets().
	if(!bVisible)
	{
		MinecraftServer *server = MinecraftServer::getInstance();
		INetworkPlayer *thisPlayer = sp->connection->getNetworkPlayer();
		if( thisPlayer )
		{
			for( unsigned int i = 0; i < server->getPlayers()->players.size(); i++ )
			{
				// Consider extra players, but not if they are the entity we are tracking, or the player we've been passed as input, or in another dimension
				shared_ptr<ServerPlayer> ep = server->getPlayers()->players[i];
				if( ep == sp ) continue;
				if( ep == e ) continue;
				if( ep->dimension != sp->dimension ) continue;

				INetworkPlayer * otherPlayer = ep->connection->getNetworkPlayer();
				if( otherPlayer != nullptr && thisPlayer->IsSameSystem(otherPlayer) )
				{
					// 4J Stu - We call update players when the entity has moved more than a certain amount at the start of it's tick
					// Before this call we set xpu, ypu and zpu to the entities new position, but xp,yp and zp are the old position until later in the tick.
					// Therefore we should use the new position for visibility checks
					double xd = ep->x - xpu; //xp / 32;
					double zd = ep->z - zpu; //zp / 32;
					bVisible |= ( xd >= -playersRange && xd <= playersRange && zd >= -playersRange && zd <= playersRange );
					canBeSeenBy |= canBySeenBy(ep);
				}
			}
		}
	}

	// 4J Stu - We need to ensure that we send the mount before the rider, so check that the player has been added to the seenBy list
	if(forRider)
	{
		canBeSeenBy = canBeSeenBy && (seenBy.find(sp) != seenBy.end());
	}

	// 4J-JEV: ADDED! An entities mount has to be visible before the entity visible,
	// this is to ensure that the mount is already in the client's game when the rider is added.
	if (canBeSeenBy && bVisible && e->riding != nullptr)
	{
		return tracker->getTracker(e->riding)->isVisible(tracker, sp, true);
	}
	else if (canBeSeenBy && bVisible)	return eVisibility_SeenAndVisible;
	else if (bVisible)					return eVisibility_IsVisible;
	else								return eVisibility_NotVisible;
}

void TrackedEntity::updatePlayer(EntityTracker *tracker, shared_ptr<ServerPlayer> sp)
{
	if (sp == e) return;

	eVisibility visibility = this->isVisible(tracker, sp);

	if (	visibility == eVisibility_SeenAndVisible
		&&	(seenBy.find(sp) == seenBy.end() || e->forcedLoading))
	{
		seenBy.insert(sp);
		shared_ptr<Packet> packet = getAddEntityPacket();
		sp->connection->send(packet);

		xap = e->xd;
		yap = e->yd;
		zap = e->zd;

		if ( e->instanceof(eTYPE_PLAYER) )
		{
			shared_ptr<Player> plr = dynamic_pointer_cast<Player>(e);
			app.DebugPrintf( "TrackedEntity:: Player '%ls' is now visible to player '%ls', %s.\n",
				plr->name.c_str(), sp->name.c_str(),
				(e->riding==nullptr?"not riding minecart":"in minecart")
				);
		}

		bool isAddMobPacket = dynamic_pointer_cast<AddMobPacket>(packet) != nullptr;

		// 4J Stu brought forward to fix when Item Frames
		if (!e->getEntityData()->isEmpty() && !isAddMobPacket)
		{
			sp->connection->send(std::make_shared<SetEntityDataPacket>(e->entityId, e->getEntityData(), true));
		}

		if ( e->instanceof(eTYPE_LIVINGENTITY) )
		{
			shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(e);
			ServersideAttributeMap *attributeMap = static_cast<ServersideAttributeMap *>(living->getAttributes());
			unordered_set<AttributeInstance *> *attributes = attributeMap->getSyncableAttributes();

			if (!attributes->empty())
			{
				sp->connection->send(std::make_shared<UpdateAttributesPacket>(e->entityId, attributes));
			}
			delete attributes;
		}

		if (trackDelta && !isAddMobPacket)
		{
			sp->connection->send(std::make_shared<SetEntityMotionPacket>(e->entityId, e->xd, e->yd, e->zd));
		}

		if (e->riding != nullptr)
		{
			sp->connection->send(std::make_shared<SetEntityLinkPacket>(SetEntityLinkPacket::RIDING, e, e->riding));
		}
		if ( e->instanceof(eTYPE_MOB) && dynamic_pointer_cast<Mob>(e)->getLeashHolder() != nullptr)
		{
			sp->connection->send(std::make_shared<SetEntityLinkPacket>(SetEntityLinkPacket::LEASH, e, dynamic_pointer_cast<Mob>(e)->getLeashHolder()));
		}

		if ( e->instanceof(eTYPE_LIVINGENTITY) )
		{
			for (int i = 0; i < 5; i++)
			{
				shared_ptr<ItemInstance> item = dynamic_pointer_cast<LivingEntity>(e)->getCarried(i);
				if(item != nullptr) sp->connection->send(std::make_shared<SetEquippedItemPacket>(e->entityId, i, item));
			}
		}

		if ( e->instanceof(eTYPE_PLAYER) )
		{
			shared_ptr<Player> spe = dynamic_pointer_cast<Player>(e);
			if (spe->isSleeping())
			{
				sp->connection->send(std::make_shared<EntityActionAtPositionPacket>(e, EntityActionAtPositionPacket::START_SLEEP, Mth::floor(e->x), Mth::floor(e->y), Mth::floor(e->z)));
			}
		}

		if ( e->instanceof(eTYPE_LIVINGENTITY) )
		{
			shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(e);
			vector<MobEffectInstance *> *activeEffects = mob->getActiveEffects();
			for(auto& effect : *activeEffects)
			{
				sp->connection->send(std::make_shared<UpdateMobEffectPacket>( e->entityId, effect ) );
			}
			delete activeEffects;
		}
	}
	else if (visibility == eVisibility_NotVisible)
	{
        auto it = seenBy.find(sp);
        if (it != seenBy.end())
		{
			seenBy.erase(it);
			sp->entitiesToRemove.push_back(e->entityId);
		}
	}

}

bool TrackedEntity::canBySeenBy(shared_ptr<ServerPlayer> player)
{
	// 4J - for some reason this isn't currently working, and is causing players to not appear until we are really close to them. Not sure
	// what the conflict is between the java & our version, but removing for now as it is causing issues and we shouldn't *really* need it
	// TODO - investigate further

	return true;
	//	return player->getLevel()->getChunkMap()->isPlayerIn(player, e->xChunk, e->zChunk);
}

void TrackedEntity::updatePlayers(EntityTracker *tracker, vector<shared_ptr<Player> > *players)
{
	for (unsigned int i = 0; i < players->size(); i++)
	{
		updatePlayer(tracker, dynamic_pointer_cast<ServerPlayer>( players->at(i) ) );
	}
}

shared_ptr<Packet> TrackedEntity::getAddEntityPacket()
{
	if (e->removed)
	{
		app.DebugPrintf("Fetching addPacket for removed entity - %ls\n", e->getAName().c_str());
	}

	// 4J-PB - replacing with a switch, rather than tons of ifs
	if (dynamic_pointer_cast<Creature>(e) != nullptr)
	{
		yHeadRotp = Mth::floor(e->getYHeadRot() * 256 / 360);
		return std::make_shared<AddMobPacket>(dynamic_pointer_cast<Mob>(e), yRotp, xRotp, xp, yp, zp, yHeadRotp);
	}

	if (e->instanceof(eTYPE_ITEMENTITY))
	{
		shared_ptr<AddEntityPacket> packet = std::make_shared<AddEntityPacket>(e, AddEntityPacket::ITEM, 1, yRotp, xRotp, xp, yp, zp);
		return packet;
	}
	else if (e->instanceof(eTYPE_SERVERPLAYER))
	{
		shared_ptr<ServerPlayer> player = dynamic_pointer_cast<ServerPlayer>(e);

		PlayerUID xuid = INVALID_XUID;
		PlayerUID OnlineXuid = INVALID_XUID;
		if( player != nullptr )
		{
			xuid = player->getXuid();
			OnlineXuid = player->getOnlineXuid();
		}
		// 4J Added yHeadRotp param to fix #102563 - TU12: Content: Gameplay: When one of the Players is idle for a few minutes his head turns 180 degrees.
		return std::make_shared<AddPlayerPacket>(player, xuid, OnlineXuid, xp, yp, zp, yRotp, xRotp, yHeadRotp);
	}
	else if (e->instanceof(eTYPE_MINECART))
	{
		shared_ptr<Minecart> minecart = dynamic_pointer_cast<Minecart>(e);
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::MINECART, minecart->getType(), yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_BOAT))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::BOAT, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_ENDERDRAGON))
	{
		yHeadRotp = Mth::floor(e->getYHeadRot() * 256 / 360);
		return std::make_shared<AddMobPacket>(dynamic_pointer_cast<LivingEntity>(e), yRotp, xRotp, xp, yp, zp, yHeadRotp);
	}
	else if (e->instanceof(eTYPE_FISHINGHOOK))
	{
		shared_ptr<Entity> owner = dynamic_pointer_cast<FishingHook>(e)->owner;
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::FISH_HOOK, owner != nullptr ? owner->entityId : e->entityId, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_ARROW))
	{
		shared_ptr<Entity> owner = (dynamic_pointer_cast<Arrow>(e))->owner;
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::ARROW, owner != nullptr ? owner->entityId : e->entityId, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_SNOWBALL))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::SNOWBALL, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_THROWNPOTION))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::THROWN_POTION, ((dynamic_pointer_cast<ThrownPotion>(e))->getPotionValue()), yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_THROWNEXPBOTTLE))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::THROWN_EXPBOTTLE, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_THROWNENDERPEARL))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::THROWN_ENDERPEARL, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_EYEOFENDERSIGNAL))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::EYEOFENDERSIGNAL, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_FIREWORKS_ROCKET))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::FIREWORKS, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_FIREBALL))
	{
		eINSTANCEOF classType = e->GetType();
		int type = AddEntityPacket::FIREBALL;
		if (classType == eTYPE_SMALL_FIREBALL)
		{
			type = AddEntityPacket::SMALL_FIREBALL;
		}
		else if (classType == eTYPE_DRAGON_FIREBALL)
		{
			type = AddEntityPacket::DRAGON_FIRE_BALL;
		}
		else if (classType == eTYPE_WITHER_SKULL)
		{
			type = AddEntityPacket::WITHER_SKULL;
		}

		shared_ptr<Fireball> fb = dynamic_pointer_cast<Fireball>(e);
		shared_ptr<AddEntityPacket> aep = nullptr;
		if (fb->owner != nullptr)
		{
			aep = std::make_shared<AddEntityPacket>(e, type, fb->owner->entityId, yRotp, xRotp, xp, yp, zp);
		}
		else
		{
			aep = std::make_shared<AddEntityPacket>(e, type, 0, yRotp, xRotp, xp, yp, zp);
		}
		aep->xa = static_cast<int>(fb->xPower * 8000);
		aep->ya = static_cast<int>(fb->yPower * 8000);
		aep->za = static_cast<int>(fb->zPower * 8000);
		return aep;
	}
	else if (e->instanceof(eTYPE_THROWNEGG))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::EGG, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_PRIMEDTNT))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::PRIMED_TNT, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_ENDER_CRYSTAL))
	{
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::ENDER_CRYSTAL, yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_FALLINGTILE))
	{
		shared_ptr<FallingTile> ft = dynamic_pointer_cast<FallingTile>(e);
		return std::make_shared<AddEntityPacket>(e, AddEntityPacket::FALLING, ft->tile | (ft->data << 16), yRotp, xRotp, xp, yp, zp);
	}
	else if (e->instanceof(eTYPE_PAINTING))
	{
		return std::make_shared<AddPaintingPacket>(dynamic_pointer_cast<Painting>(e));
	}
	else if (e->instanceof(eTYPE_ITEM_FRAME))
	{
		shared_ptr<ItemFrame> frame = dynamic_pointer_cast<ItemFrame>(e);

		{

			int ix= (int)frame->xTile;
			int iy= (int)frame->yTile;
			int iz= (int)frame->zTile;
			app.DebugPrintf("eTYPE_ITEM_FRAME xyz %d,%d,%d\n",ix,iy,iz);
		}

		shared_ptr<AddEntityPacket> packet = std::make_shared<AddEntityPacket>(e, AddEntityPacket::ITEM_FRAME, frame->dir, yRotp, xRotp, xp, yp, zp);
		packet->x = Mth::floor(frame->xTile * 32.0f);
		packet->y = Mth::floor(frame->yTile * 32.0f);
		packet->z = Mth::floor(frame->zTile * 32.0f);
		return packet;
	}
	else if (e->instanceof(eTYPE_LEASHFENCEKNOT))
	{
		shared_ptr<LeashFenceKnotEntity> knot = dynamic_pointer_cast<LeashFenceKnotEntity>(e);
		shared_ptr<AddEntityPacket> packet = std::make_shared<AddEntityPacket>(e, AddEntityPacket::LEASH_KNOT, yRotp, xRotp, xp, yp, zp);
		packet->x = Mth::floor(static_cast<float>(knot->xTile) * 32);
		packet->y = Mth::floor(static_cast<float>(knot->yTile) * 32);
		packet->z = Mth::floor(static_cast<float>(knot->zTile) * 32);
		return packet;
	}
	else if (e->instanceof(eTYPE_EXPERIENCEORB))
	{
		return std::make_shared<AddExperienceOrbPacket>(dynamic_pointer_cast<ExperienceOrb>(e));
	}
	else
	{
		assert(false);
	}

	return nullptr;
}

void TrackedEntity::clear(shared_ptr<ServerPlayer> sp)
{
    auto it = seenBy.find(sp);
    if (it != seenBy.end())
	{
		seenBy.erase(it);
		sp->entitiesToRemove.push_back(e->entityId);
	}
}
