#include "stdafx.h"
#include "net.minecraft.network.packet.h"
#include "PacketListener.h"

void PacketListener::handleBlockRegionUpdate(shared_ptr<BlockRegionUpdatePacket> packet) 
{
}

void PacketListener::onUnhandledPacket(shared_ptr<Packet> packet)
{
}

void PacketListener::onDisconnect(DisconnectPacket::eDisconnectReason reason, void *reasonObjects) 
{
}

void PacketListener::handleDisconnect(shared_ptr<DisconnectPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleLogin(shared_ptr<LoginPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleMovePlayer(shared_ptr<MovePlayerPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleChunkTilesUpdate(shared_ptr<ChunkTilesUpdatePacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handlePlayerAction(shared_ptr<PlayerActionPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTileUpdate(shared_ptr<TileUpdatePacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleChunkVisibility(shared_ptr<ChunkVisibilityPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleAddPlayer(shared_ptr<AddPlayerPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleMoveEntity(shared_ptr<MoveEntityPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleMoveEntitySmall(shared_ptr<MoveEntityPacketSmall> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTeleportEntity(shared_ptr<TeleportEntityPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleUseItem(shared_ptr<UseItemPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSetCarriedItem(shared_ptr<SetCarriedItemPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleRemoveEntity(shared_ptr<RemoveEntitiesPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTakeItemEntity(shared_ptr<TakeItemEntityPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleChat(shared_ptr<ChatPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleAddEntity(shared_ptr<AddEntityPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleAnimate(shared_ptr<AnimatePacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handlePlayerCommand(shared_ptr<PlayerCommandPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handlePreLogin(shared_ptr<PreLoginPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleAddMob(shared_ptr<AddMobPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSetTime(shared_ptr<SetTimePacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSetSpawn(shared_ptr<SetSpawnPositionPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSetEntityMotion(shared_ptr<SetEntityMotionPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSetEntityData(shared_ptr<SetEntityDataPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleEntityLinkPacket(shared_ptr<SetEntityLinkPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleInteract(shared_ptr<InteractPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleEntityEvent(shared_ptr<EntityEventPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSetHealth(shared_ptr<SetHealthPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleRespawn(shared_ptr<RespawnPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTexture(shared_ptr<TexturePacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTextureAndGeometry(shared_ptr<TextureAndGeometryPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleExplosion(shared_ptr<ExplodePacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleContainerOpen(shared_ptr<ContainerOpenPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleContainerClose(shared_ptr<ContainerClosePacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleContainerClick(shared_ptr<ContainerClickPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleContainerSetSlot(shared_ptr<ContainerSetSlotPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleContainerContent(shared_ptr<ContainerSetContentPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSignUpdate(shared_ptr<SignUpdatePacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleContainerSetData(shared_ptr<ContainerSetDataPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleSetEquippedItem(shared_ptr<SetEquippedItemPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleContainerAck(shared_ptr<ContainerAckPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleAddPainting(shared_ptr<AddPaintingPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTileEvent(shared_ptr<TileEventPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleAwardStat(shared_ptr<AwardStatPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleEntityActionAtPosition(shared_ptr<EntityActionAtPositionPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handlePlayerInput(shared_ptr<PlayerInputPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleGameEvent(shared_ptr<GameEventPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleAddGlobalEntity(shared_ptr<AddGlobalEntityPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleComplexItemData(shared_ptr<ComplexItemDataPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleLevelEvent(shared_ptr<LevelEventPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

// 1.8.2
void PacketListener::handleGetInfo(shared_ptr<GetInfoPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleUpdateMobEffect(shared_ptr<UpdateMobEffectPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleRemoveMobEffect(shared_ptr<RemoveMobEffectPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handlePlayerInfo(shared_ptr<PlayerInfoPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleKeepAlive(shared_ptr<KeepAlivePacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleSetExperience(shared_ptr<SetExperiencePacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleSetCreativeModeSlot(shared_ptr<SetCreativeModeSlotPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleAddExperienceOrb(shared_ptr<AddExperienceOrbPacket> packet)
{
	onUnhandledPacket(packet);
}

// 1.0.1
void PacketListener::handleContainerButtonClick(shared_ptr<ContainerButtonClickPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleTileEntityData(shared_ptr<TileEntityDataPacket> tileEntityDataPacket)
{
	onUnhandledPacket(tileEntityDataPacket);
}

// 1.1
void PacketListener::handleCustomPayload(shared_ptr<CustomPayloadPacket> customPayloadPacket)
{
	onUnhandledPacket(customPayloadPacket);
}

// 1.2.3
void PacketListener::handleRotateMob(shared_ptr<RotateHeadPacket> rotateMobPacket)
{
	onUnhandledPacket(rotateMobPacket);
}

// 1.3.2
void PacketListener::handleClientProtocolPacket(shared_ptr<ClientProtocolPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleServerAuthData(shared_ptr<ServerAuthDataPacket> packet)
{
	onUnhandledPacket(packet);
}

//void PacketListener::handleSharedKey(shared_ptr<SharedKeyPacket> packet)
//{
//	onUnhandledPacket(packet);
//}

void PacketListener::handlePlayerAbilities(shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket)
{
	onUnhandledPacket(playerAbilitiesPacket);
}

void PacketListener::handleChatAutoComplete(shared_ptr<ChatAutoCompletePacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleClientInformation(shared_ptr<ClientInformationPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleSoundEvent(shared_ptr<LevelSoundPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleTileDestruction(shared_ptr<TileDestructionPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleClientCommand(shared_ptr<ClientCommandPacket> packet)
{
}

//void PacketListener::handleLevelChunks(shared_ptr<LevelChunksPacket> packet)
//{
//	onUnhandledPacket(packet);
//}

bool PacketListener::canHandleAsyncPackets()
{
	return false;
}



// 1.6.4
void PacketListener::handleAddObjective(shared_ptr<SetObjectivePacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleSetScore(shared_ptr<SetScorePacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleSetDisplayObjective(shared_ptr<SetDisplayObjectivePacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleSetPlayerTeamPacket(shared_ptr<SetPlayerTeamPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleParticleEvent(shared_ptr<LevelParticlesPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleUpdateAttributes(shared_ptr<UpdateAttributesPacket> packet)
{
	onUnhandledPacket(packet);
}

void PacketListener::handleTileEditorOpen(shared_ptr<TileEditorOpenPacket> tileEditorOpenPacket)
{
}

bool PacketListener::isDisconnected()
{
	return false;
}

// 4J Added

void PacketListener::handleCraftItem(shared_ptr<CraftItemPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTradeItem(shared_ptr<TradeItemPacket> packet) 
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleDebugOptions(shared_ptr<DebugOptionsPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleServerSettingsChanged(shared_ptr<ServerSettingsChangedPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleChunkVisibilityArea(shared_ptr<ChunkVisibilityAreaPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleUpdateProgress(shared_ptr<UpdateProgressPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTextureChange(shared_ptr<TextureChangePacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleTextureAndGeometryChange(shared_ptr<TextureAndGeometryChangePacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleUpdateGameRuleProgressPacket(shared_ptr<UpdateGameRuleProgressPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleKickPlayer(shared_ptr<KickPlayerPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleXZ(shared_ptr<XZPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}

void PacketListener::handleGameCommand(shared_ptr<GameCommandPacket> packet)
{
	onUnhandledPacket( (shared_ptr<Packet> ) packet);
}
