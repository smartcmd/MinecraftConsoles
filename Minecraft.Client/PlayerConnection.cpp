#include "stdafx.h"
#include "PlayerConnection.h"
#include "ServerPlayer.h"
#include "ServerLevel.h"
#include "ServerPlayerGameMode.h"
#include "PlayerList.h"
#include "MinecraftServer.h"
#include "..\Minecraft.World\net.minecraft.commands.h"
#include "..\Minecraft.World\net.minecraft.network.h"
#include "..\Minecraft.World\net.minecraft.world.entity.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.item.trading.h"
#include "..\Minecraft.World\net.minecraft.world.inventory.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.entity.h"
#include "..\Minecraft.World\net.minecraft.world.level.saveddata.h"
#include "..\Minecraft.World\net.minecraft.world.entity.animal.h"
#include "..\Minecraft.World\net.minecraft.network.h"
#include "..\Minecraft.World\net.minecraft.world.food.h"
#include "..\Minecraft.World\AABB.h"
#include "..\Minecraft.World\Pos.h"
#include "..\Minecraft.World\SharedConstants.h"
#include "..\Minecraft.World\Socket.h"
#include "..\Minecraft.World\Achievements.h"
#include "..\Minecraft.World\net.minecraft.h"
#include "EntityTracker.h"
#include "ServerConnection.h"
#include "..\Minecraft.World\GenericStats.h"
#include "..\Minecraft.World\JavaMath.h"

// 4J Added
#include "..\Minecraft.World\net.minecraft.world.item.crafting.h"
#include "Options.h"

Random PlayerConnection::random;

PlayerConnection::PlayerConnection(MinecraftServer *server, Connection *connection, shared_ptr<ServerPlayer> player)
{
	// 4J - added initialisers
	done = false;
	tickCount = 0;
	aboveGroundTickCount = 0;
	xLastOk = yLastOk = zLastOk = 0;
	synched = true;
	didTick = false;
	lastKeepAliveId = 0;
	lastKeepAliveTime = 0;
	lastKeepAliveTick = 0;
	chatSpamTickCount = 0;
	dropSpamTickCount = 0;

	this->server = server;
	this->connection = connection;
	connection->setListener(this);
	this->player = player;
	//	player->connection = this;		// 4J - moved out as we can't assign in a ctor
	InitializeCriticalSection(&done_cs);

	m_bCloseOnTick = false;
	m_bWasKicked = false;

	m_friendsOnlyUGC = false;
	m_offlineXUID = INVALID_XUID;
	m_onlineXUID = INVALID_XUID;
	m_bHasClientTickedOnce = false;

	setShowOnMaps(app.GetGameHostOption(eGameHostOption_Gamertags)!=0?true:false);
}

PlayerConnection::~PlayerConnection()
{
	delete connection;
	DeleteCriticalSection(&done_cs);
}

void PlayerConnection::tick()
{
	if( done ) return;

	if( m_bCloseOnTick )
	{
		disconnect( DisconnectPacket::eDisconnect_Closed );
		return;
	}

	didTick = false;
	tickCount++;
	connection->tick();
	if(done) return;

	if ((tickCount - lastKeepAliveTick) > 20 * 1)
	{
		lastKeepAliveTick = tickCount;
		lastKeepAliveTime = System::nanoTime() / 1000000;
		lastKeepAliveId = random.nextInt();
		send( shared_ptr<KeepAlivePacket>( new KeepAlivePacket(lastKeepAliveId) ) );
	}

	if (chatSpamTickCount > 0)
	{
		chatSpamTickCount--;
	}
	if (dropSpamTickCount > 0)
	{
		dropSpamTickCount--;
	}
}

void PlayerConnection::disconnect(DisconnectPacket::eDisconnectReason reason)
{
	EnterCriticalSection(&done_cs);
	if( done )
	{
		LeaveCriticalSection(&done_cs);
		return;
	}

	app.DebugPrintf("PlayerConnection disconect reason: %d\n", reason );
	player->disconnect();

	// 4J Stu - Need to remove the player from the receiving list before their socket is NULLed so that we can find another player on their system
	server->getPlayers()->removePlayerFromReceiving( player );
	send( shared_ptr<DisconnectPacket>( new DisconnectPacket(reason) ));
	connection->sendAndQuit();
	// 4J-PB - removed, since it needs to be localised in the language the client is in
	//server->players->broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(L"�e" + player->name + L" left the game.") ) );
	if(getWasKicked())
	{
		server->getPlayers()->broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(player->name, ChatPacket::e_ChatPlayerKickedFromGame) ) );
	}
	else
	{
		server->getPlayers()->broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(player->name, ChatPacket::e_ChatPlayerLeftGame) ) );
	}

	server->getPlayers()->remove(player);
	done = true;
	LeaveCriticalSection(&done_cs);
}

void PlayerConnection::handlePlayerInput(shared_ptr<PlayerInputPacket> packet)
{
	player->setPlayerInput(packet->getXxa(), packet->getYya(), packet->isJumping(), packet->isSneaking());
}

void PlayerConnection::handleMovePlayer(shared_ptr<MovePlayerPacket> packet)
{
	ServerLevel *level = server->getLevel(player->dimension);

	didTick = true;
	if(synched) m_bHasClientTickedOnce = true;

	if (player->wonGame) return;

	if (!synched)
	{
		double yDiff = packet->y - yLastOk;
		if (packet->x == xLastOk && yDiff * yDiff < 0.01 && packet->z == zLastOk)
		{
			synched = true;
		}
	}

	if (synched)
	{
		if (player->riding != NULL)
		{

			float yRotT = player->yRot;
			float xRotT = player->xRot;
			player->riding->positionRider();
			double xt = player->x;
			double yt = player->y;
			double zt = player->z;

			if (packet->hasRot)
			{
				yRotT = packet->yRot;
				xRotT = packet->xRot;
			}

			player->onGround = packet->onGround;

			player->doTick(false);
			player->ySlideOffset = 0;
			player->absMoveTo(xt, yt, zt, yRotT, xRotT);
			if (player->riding != NULL) player->riding->positionRider();
			server->getPlayers()->move(player);

			// player may have been kicked off the mount during the tick, so
			// only copy valid coordinates if the player still is "synched"
			if (synched) {
				xLastOk = player->x;
				yLastOk = player->y;
				zLastOk = player->z;
			}
			((Level *)level)->tick(player);

			return;
		}

		if (player->isSleeping())
		{
			player->doTick(false);
			player->absMoveTo(xLastOk, yLastOk, zLastOk, player->yRot, player->xRot);
			((Level *)level)->tick(player);
			return;
		}

		double startY = player->y;
		xLastOk = player->x;
		yLastOk = player->y;
		zLastOk = player->z;


		double xt = player->x;
		double yt = player->y;
		double zt = player->z;

		float yRotT = player->yRot;
		float xRotT = player->xRot;

		if (packet->hasPos && packet->y == -999 && packet->yView == -999)
		{
			packet->hasPos = false;
		}

		if (packet->hasPos)
		{
			xt = packet->x;
			yt = packet->y;
			zt = packet->z;
			double yd = packet->yView - packet->y;
			if (!player->isSleeping() && (yd > 1.65 || yd < 0.1))
			{
				disconnect(DisconnectPacket::eDisconnect_IllegalStance);
				//                logger.warning(player->name + " had an illegal stance: " + yd);
				return;
			}
			if (abs(packet->x) > 32000000 || abs(packet->z) > 32000000)
			{
				disconnect(DisconnectPacket::eDisconnect_IllegalPosition);
				return;
			}
		}
		if (packet->hasRot)
		{
			yRotT = packet->yRot;
			xRotT = packet->xRot;
		}

		// 4J Stu Added to stop server player y pos being different than client when flying
		if(player->abilities.mayfly || player->isAllowedToFly() )
		{
			player->abilities.flying = packet->isFlying;
		}
		else player->abilities.flying = false;

		player->doTick(false);
		player->ySlideOffset = 0;
		player->absMoveTo(xLastOk, yLastOk, zLastOk, yRotT, xRotT);

		if (!synched) return;

		double xDist = xt - player->x;
		double yDist = yt - player->y;
		double zDist = zt - player->z;

		double dist = xDist * xDist + yDist * yDist + zDist * zDist;

		// 4J-PB - removing this one for now
		/*if (dist > 100.0f)
		{
		//            logger.warning(player->name + " moved too quickly!");
		disconnect(DisconnectPacket::eDisconnect_MovedTooQuickly);
		//                System.out.println("Moved too quickly at " + xt + ", " + yt + ", " + zt);
		//                teleport(player->x, player->y, player->z, player->yRot, player->xRot);
		return;
		}
		*/

		float r = 1 / 16.0f;
		bool oldOk = level->getCubes(player, player->bb->copy()->shrink(r, r, r))->empty();

		if (player->onGround && !packet->onGround && yDist > 0)
		{
			// assume the player made a jump
			player->causeFoodExhaustion(FoodConstants::EXHAUSTION_JUMP);
		}

		player->move(xDist, yDist, zDist);

		// 4J Stu - It is possible that we are no longer synched (eg By moving into an End Portal), so we should stop any further movement based on this packet
		// Fix for #87764 - Code: Gameplay: Host cannot move and experiences End World Chunks flickering, while in Splitscreen Mode
		// and Fix for #87788 - Code: Gameplay: Client cannot move and experiences End World Chunks flickering, while in Splitscreen Mode
		if (!synched) return;

		player->onGround = packet->onGround;
		// Since server players don't call travel we check food exhaustion
		// here
		player->checkMovementStatistiscs(xDist, yDist, zDist);

		double oyDist = yDist;

		xDist = xt - player->x;
		yDist = yt - player->y;

		// 4J-PB - line below will always be true!
		if (yDist > -0.5 || yDist < 0.5)
		{
			yDist = 0;
		}
		zDist = zt - player->z;
		dist = xDist * xDist + yDist * yDist + zDist * zDist;
		bool fail = false;
		if (dist > 0.25 * 0.25 && !player->isSleeping() && !player->gameMode->isCreative() && !player->isAllowedToFly())
		{
			fail = true;
			//            logger.warning(player->name + " moved wrongly!");
			//            System.out.println("Got position " + xt + ", " + yt + ", " + zt);
			//            System.out.println("Expected " + player->x + ", " + player->y + ", " + player->z);
#ifndef _CONTENT_PACKAGE
			wprintf(L"%ls moved wrongly!\n",player->name.c_str());
			app.DebugPrintf("Got position %f, %f, %f\n", xt,yt,zt);
			app.DebugPrintf("Expected %f, %f, %f\n", player->x, player->y, player->z);
#endif
		}
		player->absMoveTo(xt, yt, zt, yRotT, xRotT);

		bool newOk = level->getCubes(player, player->bb->copy()->shrink(r, r, r))->empty();
		if (oldOk && (fail || !newOk) && !player->isSleeping())
		{
			teleport(xLastOk, yLastOk, zLastOk, yRotT, xRotT);
			return;
		}
		AABB *testBox = player->bb->copy()->grow(r, r, r)->expand(0, -0.55, 0);
		// && server.level.getCubes(player, testBox).size() == 0
		if (!server->isFlightAllowed() && !player->gameMode->isCreative() && !level->containsAnyBlocks(testBox) && !player->isAllowedToFly() )
		{
			if (oyDist >= (-0.5f / 16.0f))
			{
				aboveGroundTickCount++;
				if (aboveGroundTickCount > 80)
				{
					//                    logger.warning(player->name + " was kicked for floating too long!");
#ifndef _CONTENT_PACKAGE
					wprintf(L"%ls was kicked for floating too long!\n", player->name.c_str());
#endif
					disconnect(DisconnectPacket::eDisconnect_NoFlying);
					return;
				}
			}
		}
		else
		{
			aboveGroundTickCount = 0;
		}

		player->onGround = packet->onGround;
		server->getPlayers()->move(player);
		player->doCheckFallDamage(player->y - startY, packet->onGround);
	}
	else if ((tickCount % SharedConstants::TICKS_PER_SECOND) == 0)
	{
		teleport(xLastOk, yLastOk, zLastOk, player->yRot, player->xRot);
	}
}

void PlayerConnection::teleport(double x, double y, double z, float yRot, float xRot, bool sendPacket /*= true*/)
{
	synched = false;
	xLastOk = x;
	yLastOk = y;
	zLastOk = z;
	player->absMoveTo(x, y, z, yRot, xRot);
	// 4J - note that 1.62 is added to the height here as the client connection that receives this will presume it represents y + heightOffset at that end
	// This is different to the way that height is sent back to the server, where it represents the bottom of the player bounding volume
	if(sendPacket) player->connection->send( shared_ptr<MovePlayerPacket>( new MovePlayerPacket::PosRot(x, y + 1.62f, y, z, yRot, xRot, false, false) ) );
}

void PlayerConnection::handlePlayerAction(shared_ptr<PlayerActionPacket> packet)
{
	ServerLevel *level = server->getLevel(player->dimension);
	player->resetLastActionTime();

	if (packet->action == PlayerActionPacket::DROP_ITEM)
	{
		player->drop(false);
		return;
	}
	else if (packet->action == PlayerActionPacket::DROP_ALL_ITEMS)
	{
		player->drop(true);
		return;
	}
	else if (packet->action == PlayerActionPacket::RELEASE_USE_ITEM)
	{
		player->releaseUsingItem();
		return;
	}

	bool shouldVerifyLocation = false;
	if (packet->action == PlayerActionPacket::START_DESTROY_BLOCK) shouldVerifyLocation = true;
	if (packet->action == PlayerActionPacket::ABORT_DESTROY_BLOCK) shouldVerifyLocation = true;
	if (packet->action == PlayerActionPacket::STOP_DESTROY_BLOCK) shouldVerifyLocation = true;

	int x = packet->x;
	int y = packet->y;
	int z = packet->z;
	if (shouldVerifyLocation)
	{
		double xDist = player->x - (x + 0.5);
		// there is a mismatch between the player's camera and the player's
		// position, so add 1.5 blocks
		double yDist = player->y - (y + 0.5) + 1.5;
		double zDist = player->z - (z + 0.5);
		double dist = xDist * xDist + yDist * yDist + zDist * zDist;
		if (dist > 6 * 6)
		{
			return;
		}
		if (y >= server->getMaxBuildHeight())
		{
			return;
		}
	}

	if (packet->action == PlayerActionPacket::START_DESTROY_BLOCK)
	{
		if (true) player->gameMode->startDestroyBlock(x, y, z, packet->face);									// 4J - condition was !server->isUnderSpawnProtection(level, x, y, z, player) (from Java 1.6.4) but putting back to old behaviour
		else player->connection->send( shared_ptr<TileUpdatePacket>( new TileUpdatePacket(x, y, z, level) ) );

	}
	else if (packet->action == PlayerActionPacket::STOP_DESTROY_BLOCK)
	{
		player->gameMode->stopDestroyBlock(x, y, z);
		server->getPlayers()->prioritiseTileChanges(x, y, z, level->dimension->id);	// 4J added - make sure that the update packets for this get prioritised over other general world updates
		if (level->getTile(x, y, z) != 0) player->connection->send( shared_ptr<TileUpdatePacket>( new TileUpdatePacket(x, y, z, level) ) );
	}
	else if (packet->action == PlayerActionPacket::ABORT_DESTROY_BLOCK)
	{
		player->gameMode->abortDestroyBlock(x, y, z);
		if (level->getTile(x, y, z) != 0) player->connection->send(shared_ptr<TileUpdatePacket>( new TileUpdatePacket(x, y, z, level)));
	}
}

void PlayerConnection::handleUseItem(shared_ptr<UseItemPacket> packet)
{
	ServerLevel *level = server->getLevel(player->dimension);
	shared_ptr<ItemInstance> item = player->inventory->getSelected();
	bool informClient = false;
	int x = packet->getX();
	int y = packet->getY();
	int z = packet->getZ();
	int face = packet->getFace();
	player->resetLastActionTime();

	// 4J Stu - We don't have ops, so just use the levels setting
	bool canEditSpawn = level->canEditSpawn; // = level->dimension->id != 0 || server->players->isOp(player->name);
	if (packet->getFace() == 255)
	{
		if (item == NULL) return;
		player->gameMode->useItem(player, level, item);
	}
	else if ((packet->getY() < server->getMaxBuildHeight() - 1) || (packet->getFace() != Facing::UP && packet->getY() < server->getMaxBuildHeight()))
	{
		if (synched && player->distanceToSqr(x + 0.5, y + 0.5, z + 0.5) < 8 * 8)
		{
			if (true)		// 4J - condition was !server->isUnderSpawnProtection(level, x, y, z, player) (from java 1.6.4) but putting back to old behaviour
			{
				player->gameMode->useItemOn(player, level, item, x, y, z, face, packet->getClickX(), packet->getClickY(), packet->getClickZ());
			}
		}

		informClient = true;
	}
	else
	{
		//player->connection->send(shared_ptr<ChatPacket>(new ChatPacket("\u00A77Height limit for building is " + server->maxBuildHeight)));
		informClient = true;
	}

	if (informClient)
	{

		player->connection->send( shared_ptr<TileUpdatePacket>( new TileUpdatePacket(x, y, z, level) ) );

		if (face == 0) y--;
		if (face == 1) y++;
		if (face == 2) z--;
		if (face == 3) z++;
		if (face == 4) x--;
		if (face == 5) x++;

		// 4J - Fixes an issue where pistons briefly disappear when retracting. The pistons themselves shouldn't have their change from being pistonBase_Id to  pistonMovingPiece_Id
		// directly sent to the client, as this will happen on the client as a result of it actioning (via a tile event) the retraction of the piston locally. However, by putting a switch
		// beside a piston and then performing an action on the side of it facing a piston, the following line of code will send a TileUpdatePacket containing the change to pistonMovingPiece_Id
		// to the client, and this packet is received before the piston retract action happens - when the piston retract then occurs, it doesn't work properly because the piston tile
		// isn't what it is expecting.
		if( level->getTile(x,y,z) != Tile::pistonMovingPiece_Id )
		{
			player->connection->send( shared_ptr<TileUpdatePacket>( new TileUpdatePacket(x, y, z, level) ) );
		}

	}

	item = player->inventory->getSelected();

	bool forceClientUpdate = false;
	if(item != NULL && packet->getItem() == NULL)
	{
		forceClientUpdate = true;
	}
	if (item != NULL && item->count == 0)
	{
		player->inventory->items[player->inventory->selected] = nullptr;
		item = nullptr;
	}

	if (item == NULL || item->getUseDuration() == 0)
	{
		player->ignoreSlotUpdateHack = true;
		player->inventory->items[player->inventory->selected] = ItemInstance::clone(player->inventory->items[player->inventory->selected]);
		Slot *s = player->containerMenu->getSlotFor(player->inventory, player->inventory->selected);
		player->containerMenu->broadcastChanges();
		player->ignoreSlotUpdateHack = false;

		if (forceClientUpdate || !ItemInstance::matches(player->inventory->getSelected(), packet->getItem()))
		{
			send( shared_ptr<ContainerSetSlotPacket>( new ContainerSetSlotPacket(player->containerMenu->containerId, s->index, player->inventory->getSelected()) ) );
		}
	}
}

void PlayerConnection::onDisconnect(DisconnectPacket::eDisconnectReason reason, void *reasonObjects)
{
	EnterCriticalSection(&done_cs);
	if( done ) return;
	//    logger.info(player.name + " lost connection: " + reason);
	// 4J-PB - removed, since it needs to be localised in the language the client is in
	//server->players->broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(L"�e" + player->name + L" left the game.") ) );
	if(getWasKicked())
	{
		server->getPlayers()->broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(player->name, ChatPacket::e_ChatPlayerKickedFromGame) ) );
	}
	else
	{
		server->getPlayers()->broadcastAll( shared_ptr<ChatPacket>( new ChatPacket(player->name, ChatPacket::e_ChatPlayerLeftGame) ) );
	}
	server->getPlayers()->remove(player);
	done = true;
	LeaveCriticalSection(&done_cs);
}

void PlayerConnection::onUnhandledPacket(shared_ptr<Packet> packet)
{
	//    logger.warning(getClass() + " wasn't prepared to deal with a " + packet.getClass());
	disconnect(DisconnectPacket::eDisconnect_UnexpectedPacket);
}

void PlayerConnection::send(shared_ptr<Packet> packet)
{
	if( connection->getSocket() != NULL )
	{
		if( !server->getPlayers()->canReceiveAllPackets( player ) )
		{
			// Check if we are allowed to send this packet type
			if( !Packet::canSendToAnyClient(packet) )
			{
				//wprintf(L"Not the systems primary player, so not sending them a packet : %ls / %d\n", player->name.c_str(), packet->getId() );
				return;
			}
		}
		connection->send(packet);
	}
}

// 4J Added
void PlayerConnection::queueSend(shared_ptr<Packet> packet)
{
	if( connection->getSocket() != NULL )
	{
		if( !server->getPlayers()->canReceiveAllPackets( player ) )
		{
			// Check if we are allowed to send this packet type
			if( !Packet::canSendToAnyClient(packet) )
			{
				//wprintf(L"Not the systems primary player, so not queueing them a packet : %ls\n", connection->getSocket()->getPlayer()->GetGamertag() );
				return;
			}
		}
		connection->queueSend(packet);
	}
}

void PlayerConnection::handleSetCarriedItem(shared_ptr<SetCarriedItemPacket> packet)
{
	if (packet->slot < 0 || packet->slot >= Inventory::getSelectionSize())
	{
		//        logger.warning(player.name + " tried to set an invalid carried item");
		return;
	}
	player->inventory->selected = packet->slot;
	player->resetLastActionTime();
}

void PlayerConnection::handleChat(shared_ptr<ChatPacket> packet)
{
	// 4J - TODO
#if 0
	wstring message = packet->message;
	if (message.length() > SharedConstants::maxChatLength)
	{
		disconnect(L"Chat message too long");
		return;
	}
	message = message.trim();
	for (int i = 0; i < message.length(); i++)
	{
		if (SharedConstants.acceptableLetters.indexOf(message.charAt(i)) < 0 && (int) message.charAt(i) < 32)
		{
			disconnect(L"Illegal characters in chat");
			return;
		}
	}

	if (message.startsWith("/"))
	{
		handleCommand(message);
	} else {
		message = "<" + player.name + "> " + message;
		logger.info(message);
		server.players.broadcastAll(new ChatPacket(message));
	}
	chatSpamTickCount += SharedConstants::TICKS_PER_SECOND;
	if (chatSpamTickCount > SharedConstants::TICKS_PER_SECOND * 10)
	{
		disconnect("disconnect.spam");
	}
#endif
}

void PlayerConnection::handleCommand(const wstring& message)
{
	// 4J - TODO
#if 0
	server.getCommandDispatcher().performCommand(player, message);
#endif
}

void PlayerConnection::handleAnimate(shared_ptr<AnimatePacket> packet)
{
	player->resetLastActionTime();
	if (packet->action == AnimatePacket::SWING)
	{
		player->swing();
	}
}

void PlayerConnection::handlePlayerCommand(shared_ptr<PlayerCommandPacket> packet)
{
	player->resetLastActionTime();
	if (packet->action == PlayerCommandPacket::START_SNEAKING)
	{
		player->setSneaking(true);
	}
	else if (packet->action == PlayerCommandPacket::STOP_SNEAKING)
	{
		player->setSneaking(false);
	}
	else if (packet->action == PlayerCommandPacket::START_SPRINTING)
	{
		player->setSprinting(true);
	}
	else if (packet->action == PlayerCommandPacket::STOP_SPRINTING)
	{
		player->setSprinting(false);
	}
	else if (packet->action == PlayerCommandPacket::STOP_SLEEPING)
	{
		player->stopSleepInBed(false, true, true);
		synched = false;
	}
	else if (packet->action == PlayerCommandPacket::RIDING_JUMP)
	{
		// currently only supported by horses...
		if ( (player->riding != NULL) && player->riding->GetType() == eTYPE_HORSE)
		{
			dynamic_pointer_cast<EntityHorse>(player->riding)->onPlayerJump(packet->data);
		}
	}
	else if (packet->action == PlayerCommandPacket::OPEN_INVENTORY)
	{
		// also only supported by horses...
		if ( (player->riding != NULL) && player->riding->instanceof(eTYPE_HORSE) )
		{
			dynamic_pointer_cast<EntityHorse>(player->riding)->openInventory(player);
		}
	}
	else if (packet->action == PlayerCommandPacket::START_IDLEANIM)
	{
		player->setIsIdle(true);
	}
	else if (packet->action == PlayerCommandPacket::STOP_IDLEANIM)
	{
		player->setIsIdle(false);
	}
}

void PlayerConnection::setShowOnMaps(bool bVal)
{
	player->setShowOnMaps(bVal);
}

void PlayerConnection::handleDisconnect(shared_ptr<DisconnectPacket> packet)
{
	// 4J Stu - Need to remove the player from the receiving list before their socket is NULLed so that we can find another player on their system
	server->getPlayers()->removePlayerFromReceiving( player );
	connection->close(DisconnectPacket::eDisconnect_Quitting);
}

int PlayerConnection::countDelayedPackets()
{
	return connection->countDelayedPackets();
}

void PlayerConnection::info(const wstring& string)
{
	// 4J-PB - removed, since it needs to be localised in the language the client is in
	//send( shared_ptr<ChatPacket>( new ChatPacket(L"�7" + string) ) );
}

void PlayerConnection::warn(const wstring& string)
{
	// 4J-PB - removed, since it needs to be localised in the language the client is in
	//send( shared_ptr<ChatPacket>( new ChatPacket(L"�9" + string) ) );
}

wstring PlayerConnection::getConsoleName()
{
	return player->getName();
}

void PlayerConnection::handleInteract(shared_ptr<InteractPacket> packet)
{
	ServerLevel *level = server->getLevel(player->dimension);
	shared_ptr<Entity> target = level->getEntity(packet->target);
	player->resetLastActionTime();

	// Fix for #8218 - Gameplay: Attacking zombies from a different level often results in no hits being registered
	// 4J Stu - If the client says that we hit something, then agree with it. The canSee can fail here as it checks
	// a ray from head->head, but we may actually be looking at a different part of the entity that can be seen
	// even though the ray is blocked.
	if (target != NULL) // && player->canSee(target) && player->distanceToSqr(target) < 6 * 6)
	{
		//boole canSee = player->canSee(target);
		//double maxDist = 6 * 6;
		//if (!canSee)
		//{
		//	maxDist = 3 * 3;
		//}

		//if (player->distanceToSqr(target) < maxDist)
		//{
		if (packet->action == InteractPacket::INTERACT)
		{
			player->interact(target);
		}
		else if (packet->action == InteractPacket::ATTACK)
		{
			if ((target->GetType() == eTYPE_ITEMENTITY) || (target->GetType() == eTYPE_EXPERIENCEORB) || (target->GetType() == eTYPE_ARROW) || target == player)
			{
				//disconnect("Attempting to attack an invalid entity");
				//server.warn("Player " + player.getName() + " tried to attack an invalid entity");
				return;
			}
			player->attack(target);
		}
		//}
	}

}

bool PlayerConnection::canHandleAsyncPackets()
{
	return true;
}

void PlayerConnection::handleTexture(shared_ptr<TexturePacket> packet)
{
	// Both PlayerConnection and ClientConnection should handle this mostly the same way

	if(packet->dwBytes==0)
	{
		// Request for texture
#ifndef _CONTENT_PACKAGE
		wprintf(L"Server received request for custom texture %ls\n",packet->textureName.c_str());
#endif
		PBYTE pbData=NULL;
		DWORD dwBytes=0;
		app.GetMemFileDetails(packet->textureName,&pbData,&dwBytes);

		if(dwBytes!=0)
		{
			send( shared_ptr<TexturePacket>( new TexturePacket(packet->textureName,pbData,dwBytes) ) );
		}
		else
		{
			m_texturesRequested.push_back( packet->textureName );
		}
	}
	else
	{
		// Response with texture data
#ifndef _CONTENT_PACKAGE
		wprintf(L"Server received custom texture %ls\n",packet->textureName.c_str());
#endif
		app.AddMemoryTextureFile(packet->textureName,packet->pbData,packet->dwBytes);
		server->connection->handleTextureReceived(packet->textureName);
	}
}

void PlayerConnection::handleTextureAndGeometry(shared_ptr<TextureAndGeometryPacket> packet)
{
	// Both PlayerConnection and ClientConnection should handle this mostly the same way

	if(packet->dwTextureBytes==0)
	{
		// Request for texture and geometry
#ifndef _CONTENT_PACKAGE
		wprintf(L"Server received request for custom texture %ls\n",packet->textureName.c_str());
#endif
		PBYTE pbData=NULL;
		DWORD dwTextureBytes=0;
		app.GetMemFileDetails(packet->textureName,&pbData,&dwTextureBytes);
		DLCSkinFile *pDLCSkinFile = app.m_dlcManager.getSkinFile(packet->textureName);

		if(dwTextureBytes!=0)
		{

			if(pDLCSkinFile)
			{
				if(pDLCSkinFile->getAdditionalBoxesCount()!=0)
				{
					send( shared_ptr<TextureAndGeometryPacket>( new TextureAndGeometryPacket(packet->textureName,pbData,dwTextureBytes,pDLCSkinFile) ) );
				}
				else
				{
					send( shared_ptr<TextureAndGeometryPacket>( new TextureAndGeometryPacket(packet->textureName,pbData,dwTextureBytes) ) );
				}
			}
			else
			{
				// we don't have the dlc skin, so retrieve the data from the app store
				vector<SKIN_BOX *> *pvSkinBoxes = app.GetAdditionalSkinBoxes(packet->dwSkinID);
				unsigned int uiAnimOverrideBitmask= app.GetAnimOverrideBitmask(packet->dwSkinID);

				send( shared_ptr<TextureAndGeometryPacket>( new TextureAndGeometryPacket(packet->textureName,pbData,dwTextureBytes,pvSkinBoxes,uiAnimOverrideBitmask) ) );
			}
		}
		else
		{
			m_texturesRequested.push_back( packet->textureName );
		}
	}
	else
	{
		// Response with texture and geometry data
#ifndef _CONTENT_PACKAGE
		wprintf(L"Server received custom texture %ls and geometry\n",packet->textureName.c_str());
#endif
		app.AddMemoryTextureFile(packet->textureName,packet->pbData,packet->dwTextureBytes);

		// add the geometry to the app list
		if(packet->dwBoxC!=0)
		{
#ifndef _CONTENT_PACKAGE
			wprintf(L"Adding skin boxes for skin id %X, box count %d\n",packet->dwSkinID,packet->dwBoxC);
#endif
			app.SetAdditionalSkinBoxes(packet->dwSkinID,packet->BoxDataA,packet->dwBoxC);
		}
		// Add the anim override
		app.SetAnimOverrideBitmask(packet->dwSkinID,packet->uiAnimOverrideBitmask);

		player->setCustomSkin(packet->dwSkinID);

		server->connection->handleTextureAndGeometryReceived(packet->textureName);
	}
}

void PlayerConnection::handleTextureReceived(const wstring &textureName)
{
	// This sends the server received texture out to any other players waiting for the data
    auto it = find(m_texturesRequested.begin(), m_texturesRequested.end(), textureName);
    if( it != m_texturesRequested.end() )
	{
		PBYTE pbData=NULL;
		DWORD dwBytes=0;
		app.GetMemFileDetails(textureName,&pbData,&dwBytes);

		if(dwBytes!=0)
		{
			send( shared_ptr<TexturePacket>( new TexturePacket(textureName,pbData,dwBytes) ) );
			m_texturesRequested.erase(it);
		}
	}
}

void PlayerConnection::handleTextureAndGeometryReceived(const wstring &textureName)
{
	// This sends the server received texture out to any other players waiting for the data
    auto it = find(m_texturesRequested.begin(), m_texturesRequested.end(), textureName);
    if( it != m_texturesRequested.end() )
	{
		PBYTE pbData=NULL;
		DWORD dwTextureBytes=0;
		app.GetMemFileDetails(textureName,&pbData,&dwTextureBytes);
		DLCSkinFile *pDLCSkinFile=app.m_dlcManager.getSkinFile(textureName);

		if(dwTextureBytes!=0)
		{
			if(pDLCSkinFile && (pDLCSkinFile->getAdditionalBoxesCount()!=0))
			{
				send( shared_ptr<TextureAndGeometryPacket>( new TextureAndGeometryPacket(textureName,pbData,dwTextureBytes,pDLCSkinFile) ) );
			}
			else
			{
				// get the data from the app
				DWORD dwSkinID = app.getSkinIdFromPath(textureName);
				vector<SKIN_BOX *> *pvSkinBoxes = app.GetAdditionalSkinBoxes(dwSkinID);
				unsigned int uiAnimOverrideBitmask= app.GetAnimOverrideBitmask(dwSkinID);

				send( shared_ptr<TextureAndGeometryPacket>( new TextureAndGeometryPacket(textureName,pbData,dwTextureBytes, pvSkinBoxes, uiAnimOverrideBitmask) ) );
			}
			m_texturesRequested.erase(it);
		}
	}
}

void PlayerConnection::handleTextureChange(shared_ptr<TextureChangePacket> packet)
{
	switch(packet->action)
	{
	case TextureChangePacket::e_TextureChange_Skin:
		player->setCustomSkin( app.getSkinIdFromPath( packet->path ) );
#ifndef _CONTENT_PACKAGE
		wprintf(L"Skin for server player %ls has changed to %ls (%d)\n", player->name.c_str(), player->customTextureUrl.c_str(), player->getPlayerDefaultSkin() );
#endif
		break;
	case TextureChangePacket::e_TextureChange_Cape:
		player->setCustomCape( Player::getCapeIdFromPath( packet->path ) );
		//player->customTextureUrl2 = packet->path;
#ifndef _CONTENT_PACKAGE
		wprintf(L"Cape for server player %ls has changed to %ls\n", player->name.c_str(), player->customTextureUrl2.c_str() );
#endif
		break;
	}
	if(!packet->path.empty() && packet->path.substr(0,3).compare(L"def") != 0 && !app.IsFileInMemoryTextures(packet->path))
	{
		if(	server->connection->addPendingTextureRequest(packet->path))
		{
#ifndef _CONTENT_PACKAGE
			wprintf(L"Sending texture packet to get custom skin %ls from player %ls\n",packet->path.c_str(), player->name.c_str());
#endif
			send(shared_ptr<TexturePacket>( new TexturePacket(packet->path,NULL,0) ) );
		}
	}
	else if(!packet->path.empty() && app.IsFileInMemoryTextures(packet->path))
	{
		// Update the ref count on the memory texture data
		app.AddMemoryTextureFile(packet->path,NULL,0);
	}
	server->getPlayers()->broadcastAll( shared_ptr<TextureChangePacket>( new TextureChangePacket(player,packet->action,packet->path) ), player->dimension );
}

void PlayerConnection::handleTextureAndGeometryChange(shared_ptr<TextureAndGeometryChangePacket> packet)
{

	player->setCustomSkin( app.getSkinIdFromPath( packet->path ) );
#ifndef _CONTENT_PACKAGE
	wprintf(L"PlayerConnection::handleTextureAndGeometryChange - Skin for server player %ls has changed to %ls (%d)\n", player->name.c_str(), player->customTextureUrl.c_str(), player->getPlayerDefaultSkin() );
#endif


	if(!packet->path.empty() && packet->path.substr(0,3).compare(L"def") != 0 && !app.IsFileInMemoryTextures(packet->path))
	{
		if(	server->connection->addPendingTextureRequest(packet->path))
		{
#ifndef _CONTENT_PACKAGE
			wprintf(L"Sending texture packet to get custom skin %ls from player %ls\n",packet->path.c_str(), player->name.c_str());
#endif
			send(shared_ptr<TextureAndGeometryPacket>( new TextureAndGeometryPacket(packet->path,NULL,0) ) );
		}
	}
	else if(!packet->path.empty() && app.IsFileInMemoryTextures(packet->path))
	{
		// Update the ref count on the memory texture data
		app.AddMemoryTextureFile(packet->path,NULL,0);

		player->setCustomSkin(packet->dwSkinID);

		// If we already have the texture, then we already have the model parts too
		//app.SetAdditionalSkinBoxes(packet->dwSkinID,)
		//DebugBreak();
	}
	server->getPlayers()->broadcastAll( shared_ptr<TextureAndGeometryChangePacket>( new TextureAndGeometryChangePacket(player,packet->path) ), player->dimension );
}

void PlayerConnection::handleServerSettingsChanged(shared_ptr<ServerSettingsChangedPacket> packet)
{
	if(packet->action==ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS)
	{
		// Need to check that this player has permission to change each individual setting?

		INetworkPlayer *networkPlayer = getNetworkPlayer();
		if( (networkPlayer != NULL && networkPlayer->IsHost()) || player->isModerator())
		{
			app.SetGameHostOption(eGameHostOption_FireSpreads, app.GetGameHostOption(packet->data,eGameHostOption_FireSpreads));
			app.SetGameHostOption(eGameHostOption_TNT, app.GetGameHostOption(packet->data,eGameHostOption_TNT));
			app.SetGameHostOption(eGameHostOption_MobGriefing, app.GetGameHostOption(packet->data, eGameHostOption_MobGriefing));
			app.SetGameHostOption(eGameHostOption_KeepInventory, app.GetGameHostOption(packet->data, eGameHostOption_KeepInventory));
			app.SetGameHostOption(eGameHostOption_DoMobSpawning, app.GetGameHostOption(packet->data, eGameHostOption_DoMobSpawning));
			app.SetGameHostOption(eGameHostOption_DoMobLoot, app.GetGameHostOption(packet->data, eGameHostOption_DoMobLoot));
			app.SetGameHostOption(eGameHostOption_DoTileDrops, app.GetGameHostOption(packet->data, eGameHostOption_DoTileDrops));
			app.SetGameHostOption(eGameHostOption_DoDaylightCycle, app.GetGameHostOption(packet->data, eGameHostOption_DoDaylightCycle));
			app.SetGameHostOption(eGameHostOption_NaturalRegeneration, app.GetGameHostOption(packet->data, eGameHostOption_NaturalRegeneration));

			server->getPlayers()->broadcastAll( shared_ptr<ServerSettingsChangedPacket>( new ServerSettingsChangedPacket( ServerSettingsChangedPacket::HOST_IN_GAME_SETTINGS,app.GetGameHostOption(eGameHostOption_All) ) ) );

			// Update the QoS data
			g_NetworkManager.UpdateAndSetGameSessionData();
		}
	}
}

void PlayerConnection::handleKickPlayer(shared_ptr<KickPlayerPacket> packet)
{
	INetworkPlayer *networkPlayer = getNetworkPlayer();
	if( (networkPlayer != NULL && networkPlayer->IsHost()) || player->isModerator())
	{
		server->getPlayers()->kickPlayerByShortId(packet->m_networkSmallId);
	}
}

void PlayerConnection::handleGameCommand(shared_ptr<GameCommandPacket> packet)
{
	MinecraftServer::getInstance()->getCommandDispatcher()->performCommand(player, packet->command, packet->data);
}

void PlayerConnection::handleClientCommand(shared_ptr<ClientCommandPacket> packet)
{
	player->resetLastActionTime();
	if (packet->action == ClientCommandPacket::PERFORM_RESPAWN)
	{
		if (player->wonGame)
		{
			player = server->getPlayers()->respawn(player, player->m_enteredEndExitPortal?0:player->dimension, true);
		}
		//else if (player.getLevel().getLevelData().isHardcore())
		//{
		//	if (server.isSingleplayer() && player.name.equals(server.getSingleplayerName()))
		//	{
		//		player.connection.disconnect("You have died. Game over, man, it's game over!");
		//		server.selfDestruct();
		//	}
		//	else
		//	{
		//		BanEntry ban = new BanEntry(player.name);
		//		ban.setReason("Death in Hardcore");

		//		server.getPlayers().getBans().add(ban);
		//		player.connection.disconnect("You have died. Game over, man, it's game over!");
		//	}
		//}
		else
		{
			if (player->getHealth() > 0) return;
			player = server->getPlayers()->respawn(player, 0, false);
		}
	}
}

void PlayerConnection::handleRespawn(shared_ptr<RespawnPacket> packet)
{
}

void PlayerConnection::handleContainerClose(shared_ptr<ContainerClosePacket> packet)
{
	player->doCloseContainer();
}

#ifndef _CONTENT_PACKAGE
void PlayerConnection::handleContainerSetSlot(shared_ptr<ContainerSetSlotPacket> packet)
{
	if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_CARRIED )
	{
		player->inventory->setCarried(packet->item);
	}
	else
	{
		if (packet->containerId == AbstractContainerMenu::CONTAINER_ID_INVENTORY && packet->slot >= 36 && packet->slot < 36 + 9)
		{
			shared_ptr<ItemInstance> lastItem = player->inventoryMenu->getSlot(packet->slot)->getItem();
			if (packet->item != NULL)
			{
				if (lastItem == NULL || lastItem->count < packet->item->count)
				{
					packet->item->popTime = Inventory::POP_TIME_DURATION;
				}
			}
			player->inventoryMenu->setItem(packet->slot, packet->item);
			player->ignoreSlotUpdateHack = true;
			player->containerMenu->broadcastChanges();
			player->broadcastCarriedItem();
			player->ignoreSlotUpdateHack = false;
		}
		else if (packet->containerId == player->containerMenu->containerId)
		{
			player->containerMenu->setItem(packet->slot, packet->item);
			player->ignoreSlotUpdateHack = true;
			player->containerMenu->broadcastChanges();
			player->broadcastCarriedItem();
			player->ignoreSlotUpdateHack = false;
		}
	}
}
#endif

void PlayerConnection::handleContainerClick(shared_ptr<ContainerClickPacket> packet)
{
	player->resetLastActionTime();
	if (player->containerMenu->containerId == packet->containerId && player->containerMenu->isSynched(player))
	{
		shared_ptr<ItemInstance> clicked = player->containerMenu->clicked(packet->slotNum, packet->buttonNum, packet->clickType, player);

		if (ItemInstance::matches(packet->item, clicked))
		{
			// Yep, you sure did click what you claimed to click!
			player->connection->send( shared_ptr<ContainerAckPacket>( new ContainerAckPacket(packet->containerId, packet->uid, true) ) );
			player->ignoreSlotUpdateHack = true;
			player->containerMenu->broadcastChanges();
			player->broadcastCarriedItem();
			player->ignoreSlotUpdateHack = false;
		}
		else
		{
			// No, you clicked the wrong thing!
			expectedAcks[player->containerMenu->containerId] = packet->uid;
			player->connection->send( shared_ptr<ContainerAckPacket>( new ContainerAckPacket(packet->containerId, packet->uid, false) ) );
			player->containerMenu->setSynched(player, false);

			vector<shared_ptr<ItemInstance> > items;
			for (unsigned int i = 0; i < player->containerMenu->slots.size(); i++)
			{
				items.push_back(player->containerMenu->slots.at(i)->getItem());
			}
			player->refreshContainer(player->containerMenu, &items);

			//                player.containerMenu.broadcastChanges();
		}
	}

}

void PlayerConnection::handleContainerButtonClick(shared_ptr<ContainerButtonClickPacket> packet)
{
	player->resetLastActionTime();
	if (player->containerMenu->containerId == packet->containerId && player->containerMenu->isSynched(player))
	{
		player->containerMenu->clickMenuButton(player, packet->buttonId);
		player->containerMenu->broadcastChanges();
	}
}

void PlayerConnection::handleSetCreativeModeSlot(shared_ptr<SetCreativeModeSlotPacket> packet)
{
	if (player->gameMode->isCreative())
	{
		bool drop = packet->slotNum < 0;
		shared_ptr<ItemInstance> item = packet->item;

		if(item != NULL && item->id == Item::map_Id)
		{
			int mapScale = 3;
#ifdef _LARGE_WORLDS
			int scale = MapItemSavedData::MAP_SIZE * 2 * (1 << mapScale);
			int centreXC = (int) (Math::round(player->x / scale) * scale);
			int centreZC = (int) (Math::round(player->z / scale) * scale);
#else
			// 4J-PB - for Xbox maps, we'll centre them on the origin of the world, since we can fit the whole world in our map
			int centreXC = 0;
			int centreZC = 0;
#endif
			item->setAuxValue( player->level->getAuxValueForMap(player->getXuid(), player->dimension, centreXC, centreZC, mapScale) );

			shared_ptr<MapItemSavedData> data = MapItem::getSavedData(item->getAuxValue(), player->level);
			// 4J Stu - We only have one map per player per dimension, so don't reset the one that they have
			// when a new one is created
			wchar_t buf[64];
			swprintf(buf,64,L"map_%d", item->getAuxValue());
			std::wstring id = wstring(buf);
			if( data == NULL )
			{
				data = shared_ptr<MapItemSavedData>( new MapItemSavedData(id) );
			}
			player->level->setSavedData(id, (shared_ptr<SavedData> ) data);

			data->scale = mapScale;
			// 4J-PB - for Xbox maps, we'll centre them on the origin of the world, since we can fit the whole world in our map
			data->x = centreXC;
			data->z = centreZC;
			data->dimension = (byte) player->level->dimension->id;
			data->setDirty();
		}

		bool validSlot = (packet->slotNum >= InventoryMenu::CRAFT_SLOT_START && packet->slotNum < (InventoryMenu::USE_ROW_SLOT_START + Inventory::getSelectionSize()));
		bool validItem = item == NULL || (item->id < Item::items.length && item->id >= 0 && Item::items[item->id] != NULL);
		bool validData = item == NULL || (item->getAuxValue() >= 0 && item->count > 0 && item->count <= 64);

		if (validSlot && validItem && validData)
		{
			if (item == NULL)
			{
				player->inventoryMenu->setItem(packet->slotNum, nullptr);
			}
			else
			{
				player->inventoryMenu->setItem(packet->slotNum, item );
			}
			player->inventoryMenu->setSynched(player, true);
			//                player.slotChanged(player.inventoryMenu, packet.slotNum, player.inventoryMenu.getSlot(packet.slotNum).getItem());
		}
		else if (drop && validItem && validData)
		{
			if (dropSpamTickCount < SharedConstants::TICKS_PER_SECOND * 10)
			{
				dropSpamTickCount += SharedConstants::TICKS_PER_SECOND;
				// drop item
				shared_ptr<ItemEntity> dropped = player->drop(item);
				if (dropped != NULL)
				{
					dropped->setShortLifeTime();
				}
			}
		}

		if( item != NULL && item->id == Item::map_Id )
		{
			// 4J Stu - Maps need to have their aux value update, so the client should always be assumed to be wrong
			// This is how the Java works, as the client also incorrectly predicts the auxvalue of the mapItem
			vector<shared_ptr<ItemInstance> > items;
			for (unsigned int i = 0; i < player->inventoryMenu->slots.size(); i++)
			{
				items.push_back(player->inventoryMenu->slots.at(i)->getItem());
			}
			player->refreshContainer(player->inventoryMenu, &items);
		}
	}
}

void PlayerConnection::handleContainerAck(shared_ptr<ContainerAckPacket> packet)
{
    auto it = expectedAcks.find(player->containerMenu->containerId);

    if (it != expectedAcks.end() && packet->uid == it->second && player->containerMenu->containerId == packet->containerId && !player->containerMenu->isSynched(player))
	{
		player->containerMenu->setSynched(player, true);
	}
}

void PlayerConnection::handleSignUpdate(shared_ptr<SignUpdatePacket> packet)
{
	player->resetLastActionTime();
	app.DebugPrintf("PlayerConnection::handleSignUpdate\n");

	ServerLevel *level = server->getLevel(player->dimension);
	if (level->hasChunkAt(packet->x, packet->y, packet->z))
	{
		shared_ptr<TileEntity> te = level->getTileEntity(packet->x, packet->y, packet->z);

		if (dynamic_pointer_cast<SignTileEntity>(te) != NULL)
		{
			shared_ptr<SignTileEntity> ste = dynamic_pointer_cast<SignTileEntity>(te);
			if (!ste->isEditable() || ste->getPlayerWhoMayEdit() != player)
			{
				server->warn(L"Player " + player->getName() + L" just tried to change non-editable sign");
				return;
			}
		}

		// 4J-JEV: Changed to allow characters to display as a [].
		if (dynamic_pointer_cast<SignTileEntity>(te) != NULL)
		{
			int x = packet->x;
			int y = packet->y;
			int z = packet->z;
			shared_ptr<SignTileEntity> ste = dynamic_pointer_cast<SignTileEntity>(te);
			for (int i = 0; i < 4; i++)
			{
				wstring lineText = packet->lines[i].substr(0,15);
				ste->SetMessage( i, lineText );
			}
			ste->SetVerified(false);
			ste->setChanged();
			level->sendTileUpdated(x, y, z);
		}
	}

}

void PlayerConnection::handleKeepAlive(shared_ptr<KeepAlivePacket> packet)
{
	if (packet->id == lastKeepAliveId)
	{
		int time = (int) (System::nanoTime() / 1000000 - lastKeepAliveTime);
		player->latency = (player->latency * 3 + time) / 4;
	}
}

void PlayerConnection::handlePlayerInfo(shared_ptr<PlayerInfoPacket> packet)
{
	// Need to check that this player has permission to change each individual setting?

	INetworkPlayer *networkPlayer = getNetworkPlayer();
	if( (networkPlayer != NULL && networkPlayer->IsHost()) || player->isModerator() )
	{
		shared_ptr<ServerPlayer> serverPlayer;
		// Find the player being edited
		for(auto& checkingPlayer : server->getPlayers()->players)
		{
			if(checkingPlayer->connection->getNetworkPlayer() != NULL && checkingPlayer->connection->getNetworkPlayer()->GetSmallId() == packet->m_networkSmallId)
			{
				serverPlayer = checkingPlayer;
				break;
			}
		}

		if(serverPlayer != NULL)
		{
			unsigned int origPrivs = serverPlayer->getAllPlayerGamePrivileges();

			bool trustPlayers = app.GetGameHostOption(eGameHostOption_TrustPlayers) != 0;
			bool cheats = app.GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
			if(serverPlayer == player)
			{
				GameType *gameType = Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) ? GameType::CREATIVE : GameType::SURVIVAL;
				gameType = LevelSettings::validateGameType(gameType->getId());
				if (serverPlayer->gameMode->getGameModeForPlayer() != gameType)
				{
#ifndef _CONTENT_PACKAGE
					wprintf(L"Setting %ls to game mode %d\n", serverPlayer->name.c_str(), gameType->getId());
#endif
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CreativeMode,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CreativeMode) );
					serverPlayer->gameMode->setGameModeForPlayer(gameType);
					serverPlayer->connection->send( shared_ptr<GameEventPacket>( new GameEventPacket(GameEventPacket::CHANGE_GAME_MODE, gameType->getId()) ));
				}
				else
				{
#ifndef _CONTENT_PACKAGE
					wprintf(L"%ls already has game mode %d\n", serverPlayer->name.c_str(), gameType);
#endif
				}
				if(cheats)
				{
					// Editing self
					bool canBeInvisible = Player::getPlayerGamePrivilege(origPrivs, Player::ePlayerGamePrivilege_CanToggleInvisible) != 0;
					if(canBeInvisible)serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invisible,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_Invisible) );
					if(canBeInvisible)serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Invulnerable,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_Invulnerable) );

					bool inCreativeMode = Player::getPlayerGamePrivilege(origPrivs,Player::ePlayerGamePrivilege_CreativeMode) != 0;
					if(!inCreativeMode)
					{
						bool canFly = Player::getPlayerGamePrivilege(origPrivs,Player::ePlayerGamePrivilege_CanToggleFly);
						bool canChangeHunger = Player::getPlayerGamePrivilege(origPrivs,Player::ePlayerGamePrivilege_CanToggleClassicHunger);

						if(canFly)serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanFly,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CanFly) );
						if(canChangeHunger)serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_ClassicHunger,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_ClassicHunger) );
					}
				}
			}
			else
			{
				// Editing someone else
				if(!trustPlayers && !serverPlayer->connection->getNetworkPlayer()->IsHost())
				{
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotMine,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CannotMine) );
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotBuild,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CannotBuild) );
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotAttackPlayers,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CannotAttackPlayers) );
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CannotAttackAnimals,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CannotAttackAnimals) );
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CanUseDoorsAndSwitches) );
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanUseContainers,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CanUseContainers) );
				}

				if(networkPlayer->IsHost())
				{
					if(cheats)
					{
						serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanToggleInvisible,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleInvisible) );
						serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanToggleFly,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleFly) );
						serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanToggleClassicHunger,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CanToggleClassicHunger) );
						serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_CanTeleport,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_CanTeleport) );
					}
					serverPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_Op,Player::getPlayerGamePrivilege(packet->m_playerPrivileges,Player::ePlayerGamePrivilege_Op) );
				}
			}

			server->getPlayers()->broadcastAll( shared_ptr<PlayerInfoPacket>( new PlayerInfoPacket( serverPlayer ) ) );
		}
	}
}

bool PlayerConnection::isServerPacketListener()
{
	return true;
}

void PlayerConnection::handlePlayerAbilities(shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket)
{
	player->abilities.flying = playerAbilitiesPacket->isFlying() && player->abilities.mayfly;
}

//void handleChatAutoComplete(ChatAutoCompletePacket packet) {
//	StringBuilder result = new StringBuilder();

//	for (String candidate : server.getAutoCompletions(player, packet.getMessage())) {
//		if (result.length() > 0) result.append("\0");

//		result.append(candidate);
//	}

//	player.connection.send(new ChatAutoCompletePacket(result.toString()));
//}

//void handleClientInformation(shared_ptr<ClientInformationPacket> packet)
//{
//	player->updateOptions(packet);
//}

void PlayerConnection::handleCustomPayload(shared_ptr<CustomPayloadPacket> customPayloadPacket)
{
#if 0
	if (CustomPayloadPacket.CUSTOM_BOOK_PACKET.equals(customPayloadPacket.identifier))
	{
		ByteArrayInputStream bais(customPayloadPacket->data);
		DataInputStream input(&bais);
		shared_ptr<ItemInstance> sentItem = Packet::readItem(input);

		if (!WritingBookItem.makeSureTagIsValid(sentItem.getTag()))
		{
			throw new IOException("Invalid book tag!");
		}

		// make sure the sent item is the currently carried item
		ItemInstance carried = player.inventory.getSelected();
		if (sentItem != null && sentItem.id == Item.writingBook.id && sentItem.id == carried.id)
		{
			carried.addTagElement(WrittenBookItem.TAG_PAGES, sentItem.getTag().getList(WrittenBookItem.TAG_PAGES));
		}
	}
	else if (CustomPayloadPacket.CUSTOM_BOOK_SIGN_PACKET.equals(customPayloadPacket.identifier))
	{
		DataInputStream input = new DataInputStream(new ByteArrayInputStream(customPayloadPacket.data));
		ItemInstance sentItem = Packet.readItem(input);

		if (!WrittenBookItem.makeSureTagIsValid(sentItem.getTag()))
		{
			throw new IOException("Invalid book tag!");
		}

		// make sure the sent item is the currently carried item
		ItemInstance carried = player.inventory.getSelected();
		if (sentItem != null && sentItem.id == Item.writtenBook.id && carried.id == Item.writingBook.id)
		{
			carried.addTagElement(WrittenBookItem.TAG_AUTHOR, new StringTag(WrittenBookItem.TAG_AUTHOR, player.getName()));
			carried.addTagElement(WrittenBookItem.TAG_TITLE, new StringTag(WrittenBookItem.TAG_TITLE, sentItem.getTag().getString(WrittenBookItem.TAG_TITLE)));
			carried.addTagElement(WrittenBookItem.TAG_PAGES, sentItem.getTag().getList(WrittenBookItem.TAG_PAGES));
			carried.id = Item.writtenBook.id;
		}
	}
	else
#endif
		if (CustomPayloadPacket::TRADER_SELECTION_PACKET.compare(customPayloadPacket->identifier) == 0)
		{
			ByteArrayInputStream bais(customPayloadPacket->data);
			DataInputStream input(&bais);
			int selection = input.readInt();

			AbstractContainerMenu *menu = player->containerMenu;
			if (dynamic_cast<MerchantMenu *>(menu))
			{
				((MerchantMenu *) menu)->setSelectionHint(selection);
			}
		}
		else if (CustomPayloadPacket::SET_ADVENTURE_COMMAND_PACKET.compare(customPayloadPacket->identifier) == 0)
		{
			if (!server->isCommandBlockEnabled())
			{
				app.DebugPrintf("Command blocks not enabled");
				//player->sendMessage(ChatMessageComponent.forTranslation("advMode.notEnabled"));
			}
			else if (player->hasPermission(eGameCommand_Effect) && player->abilities.instabuild)
			{
				ByteArrayInputStream bais(customPayloadPacket->data);
				DataInputStream input(&bais);
				int x = input.readInt();
				int y = input.readInt();
				int z = input.readInt();
				wstring command = Packet::readUtf(&input, 256);

				shared_ptr<TileEntity> tileEntity = player->level->getTileEntity(x, y, z);
				shared_ptr<CommandBlockEntity> cbe = dynamic_pointer_cast<CommandBlockEntity>(tileEntity);
				if (tileEntity != NULL && cbe != NULL)
				{
					cbe->setCommand(command);
					player->level->sendTileUpdated(x, y, z);
					//player->sendMessage(ChatMessageComponent.forTranslation("advMode.setCommand.success", command));
				}
			}
			else
			{
				//player.sendMessage(ChatMessageComponent.forTranslation("advMode.notAllowed"));
			}
		}
		else if (CustomPayloadPacket::SET_BEACON_PACKET.compare(customPayloadPacket->identifier) == 0)
		{
			if ( dynamic_cast<BeaconMenu *>( player->containerMenu) != NULL)
			{
				ByteArrayInputStream bais(customPayloadPacket->data);
				DataInputStream input(&bais);
				int primary = input.readInt();
				int secondary = input.readInt();

				BeaconMenu *beaconMenu = (BeaconMenu *) player->containerMenu;
				Slot *slot = beaconMenu->getSlot(0);
				if (slot->hasItem())
				{
					slot->remove(1);
					shared_ptr<BeaconTileEntity> beacon = beaconMenu->getBeacon();
					beacon->setPrimaryPower(primary);
					beacon->setSecondaryPower(secondary);
					beacon->setChanged();
				}
			}
		}
		else if (CustomPayloadPacket::SET_ITEM_NAME_PACKET.compare(customPayloadPacket->identifier) == 0)
		{
			AnvilMenu *menu = dynamic_cast<AnvilMenu *>( player->containerMenu);
			if (menu)
			{
				if (customPayloadPacket->data.data == NULL || customPayloadPacket->data.length < 1)
				{
					menu->setItemName(L"");
				}
				else
				{
					ByteArrayInputStream bais(customPayloadPacket->data);
					DataInputStream dis(&bais);
					wstring name = dis.readUTF();
					if (name.length() <= 30)
					{
						menu->setItemName(name);
					}
				}
			}
		}
}

bool PlayerConnection::isDisconnected()
{
	return done;
}

// 4J Added

void PlayerConnection::handleDebugOptions(shared_ptr<DebugOptionsPacket> packet)
{
	//Player player = dynamic_pointer_cast<Player>( player->shared_from_this() );
	player->SetDebugOptions(packet->m_uiVal);
}

void PlayerConnection::handleCraftItem(shared_ptr<CraftItemPacket> packet)
{
	int iRecipe = packet->recipe;

	if(iRecipe == -1)
		return;

	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
	shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);

	if(app.DebugSettingsOn() && (player->GetDebugOptions()&(1L<<eDebugSetting_CraftAnything)))
	{
		pTempItemInst->onCraftedBy(player->level, dynamic_pointer_cast<Player>( player->shared_from_this() ), pTempItemInst->count );
		if(player->inventory->add(pTempItemInst)==false )
		{
			// no room in inventory, so throw it down
			player->drop(pTempItemInst);
		}
	}
	else if (pTempItemInst->id == Item::fireworksCharge_Id || pTempItemInst->id == Item::fireworks_Id)
	{
		CraftingMenu *menu = (CraftingMenu *)player->containerMenu;
		player->openFireworks(menu->getX(), menu->getY(), menu->getZ() );
	}
	else
	{


		// TODO 4J Stu - Assume at the moment that the client can work this out for us...
		//if(pRecipeIngredientsRequired[iRecipe].bCanMake)
		//{
		pTempItemInst->onCraftedBy(player->level, dynamic_pointer_cast<Player>( player->shared_from_this() ), pTempItemInst->count );

		// and remove those resources from your inventory
		for(int i=0;i<pRecipeIngredientsRequired[iRecipe].iIngC;i++)
		{
			for(int j=0;j<pRecipeIngredientsRequired[iRecipe].iIngValA[i];j++)
			{
				shared_ptr<ItemInstance> ingItemInst = nullptr;
				// do we need to remove a specific aux value?
				if(pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]!=Recipes::ANY_AUX_VALUE)
				{
					ingItemInst = player->inventory->getResourceItem( pRecipeIngredientsRequired[iRecipe].iIngIDA[i],pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i] );
					player->inventory->removeResource(pRecipeIngredientsRequired[iRecipe].iIngIDA[i],pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]);
				}
				else
				{
					ingItemInst = player->inventory->getResourceItem( pRecipeIngredientsRequired[iRecipe].iIngIDA[i] );
					player->inventory->removeResource(pRecipeIngredientsRequired[iRecipe].iIngIDA[i]);
				}

				// 4J Stu - Fix for #13097 - Bug: Milk Buckets are removed when crafting Cake
				if (ingItemInst != NULL)
				{
					if (ingItemInst->getItem()->hasCraftingRemainingItem())
					{
						// replace item with remaining result
						player->inventory->add( shared_ptr<ItemInstance>( new ItemInstance(ingItemInst->getItem()->getCraftingRemainingItem()) ) );
					}

				}
			}
		}

		// 4J Stu - Fix for #13119 - We should add the item after we remove the ingredients
		if(player->inventory->add(pTempItemInst)==false )
		{
			// no room in inventory, so throw it down
			player->drop(pTempItemInst);
		}

		if( pTempItemInst->id == Item::map_Id )
		{
			// 4J Stu - Maps need to have their aux value update, so the client should always be assumed to be wrong
			// This is how the Java works, as the client also incorrectly predicts the auxvalue of the mapItem
			vector<shared_ptr<ItemInstance> > items;
			for (unsigned int i = 0; i < player->containerMenu->slots.size(); i++)
			{
				items.push_back(player->containerMenu->slots.at(i)->getItem());
			}
			player->refreshContainer(player->containerMenu, &items);
		}
		else
		{
			// Do same hack as PlayerConnection::handleContainerClick does - do our broadcast of changes just now, but with a hack so it just thinks it has sent
			// things but hasn't really. This will stop the client getting a message back confirming the current inventory items, which might then arrive
			// after another local change has been made on the client and be stale.
			player->ignoreSlotUpdateHack = true;
			player->containerMenu->broadcastChanges();
			player->broadcastCarriedItem();
			player->ignoreSlotUpdateHack = false;
		}
	}

	// handle achievements
	switch(pTempItemInst->id )
	{
	case Tile::workBench_Id:		player->awardStat(GenericStats::buildWorkbench(),		GenericStats::param_buildWorkbench());		break;
	case Item::pickAxe_wood_Id:		player->awardStat(GenericStats::buildPickaxe(),			GenericStats::param_buildPickaxe());		break;
	case Tile::furnace_Id:			player->awardStat(GenericStats::buildFurnace(),			GenericStats::param_buildFurnace());		break;
	case Item::hoe_wood_Id:			player->awardStat(GenericStats::buildHoe(),				GenericStats::param_buildHoe());			break;
	case Item::bread_Id:			player->awardStat(GenericStats::makeBread(),			GenericStats::param_makeBread());			break;
	case Item::cake_Id:				player->awardStat(GenericStats::bakeCake(),				GenericStats::param_bakeCake());			break;
	case Item::pickAxe_stone_Id:	player->awardStat(GenericStats::buildBetterPickaxe(),	GenericStats::param_buildBetterPickaxe());	break;
	case Item::sword_wood_Id:		player->awardStat(GenericStats::buildSword(),			GenericStats::param_buildSword());			break;
	case Tile::dispenser_Id:		player->awardStat(GenericStats::dispenseWithThis(),		GenericStats::param_dispenseWithThis());	break;
	case Tile::enchantTable_Id:		player->awardStat(GenericStats::enchantments(),			GenericStats::param_enchantments());		break;
	case Tile::bookshelf_Id:		player->awardStat(GenericStats::bookcase(),				GenericStats::param_bookcase());			break;
	}
	//}
	// ELSE The server thinks the client was wrong...
}


void PlayerConnection::handleTradeItem(shared_ptr<TradeItemPacket> packet)
{
	if (player->containerMenu->containerId == packet->containerId)
	{
		MerchantMenu *menu = (MerchantMenu *)player->containerMenu;

		MerchantRecipeList *offers = menu->getMerchant()->getOffers(player);

		if(offers)
		{
			int selectedShopItem = packet->offer;
			if( selectedShopItem < offers->size() )
			{
				MerchantRecipe *activeRecipe = offers->at(selectedShopItem);
				if(!activeRecipe->isDeprecated())
				{
					// Do we have the ingredients?
					shared_ptr<ItemInstance> buyAItem = activeRecipe->getBuyAItem();
					shared_ptr<ItemInstance> buyBItem = activeRecipe->getBuyBItem();

					int buyAMatches = player->inventory->countMatches(buyAItem);
					int buyBMatches = player->inventory->countMatches(buyBItem);
					if( (buyAItem != NULL && buyAMatches >= buyAItem->count) && (buyBItem == NULL || buyBMatches >= buyBItem->count) )
					{
						menu->getMerchant()->notifyTrade(activeRecipe);

						// Remove the items we are purchasing with
						player->inventory->removeResources(buyAItem);
						player->inventory->removeResources(buyBItem);

						// Add the item we have purchased
						shared_ptr<ItemInstance> result = activeRecipe->getSellItem()->copy();

						// 4J JEV - Award itemsBought stat.
						player->awardStat(
							GenericStats::itemsBought(result->getItem()->id),
							GenericStats::param_itemsBought(
							result->getItem()->id,
							result->getAuxValue(),
							result->GetCount()
							)
							);

						if (!player->inventory->add(result))
						{
							player->drop(result);
						}
					}
				}
			}
		}
	}
}

INetworkPlayer *PlayerConnection::getNetworkPlayer()
{
	if( connection != NULL && connection->getSocket() != NULL) return connection->getSocket()->getPlayer();
	else return NULL;
}

bool PlayerConnection::isLocal()
{
	if( connection->getSocket() == NULL )
	{
		return false;
	}
	else
	{
		bool isLocal = connection->getSocket()->isLocal();
		return connection->getSocket()->isLocal();
	}
}

bool PlayerConnection::isGuest()
{
	if( connection->getSocket() == NULL )
	{
		return false;
	}
	else
	{
		INetworkPlayer *networkPlayer = connection->getSocket()->getPlayer();
		bool isGuest = false;
		if(networkPlayer != NULL)
		{
			isGuest = networkPlayer->IsGuest() == TRUE;
		}
		return isGuest;
	}
}
