#pragma once
using namespace std;

class Packet;

class AddEntityPacket;
class AddGlobalEntityPacket;
class AddMobPacket;
class AddPaintingPacket;
class AddPlayerPacket;
class AnimatePacket;
class AwardStatPacket;
class BlockRegionUpdatePacket;
class ChatPacket;
class ChunkTilesUpdatePacket;
class ChunkVisibilityPacket;
class ComplexItemDataPacket;
class ContainerAckPacket;
class ContainerClickPacket;
class ContainerClosePacket;
class ContainerOpenPacket;
class ContainerSetContentPacket;
class ContainerSetDataPacket;
class ContainerSetSlotPacket;
#include "DisconnectPacket.h"
class EntityActionAtPositionPacket;
class EntityEventPacket;
class ExplodePacket;
class GameEventPacket;
class InteractPacket;
class KeepAlivePacket;
class LevelEventPacket;
class LoginPacket;
class MoveEntityPacket;
class MoveEntityPacketSmall;
class MovePlayerPacket;
class PlayerActionPacket;
class PlayerCommandPacket;
class PlayerInputPacket;
class PreLoginPacket;
class RemoveEntitiesPacket;
class RespawnPacket;
class SetCarriedItemPacket;
class SetEntityDataPacket;
class SetEntityMotionPacket;
class SetEquippedItemPacket;
class SetHealthPacket;
class SetEntityLinkPacket;
class SetSpawnPositionPacket;
class SetTimePacket;
class SignUpdatePacket;
class TakeItemEntityPacket;
class TeleportEntityPacket;
class TileEventPacket;
class TileUpdatePacket;
class UseItemPacket;

// 1.8.2
class GetInfoPacket;
class UpdateMobEffectPacket;
class RemoveMobEffectPacket;
class PlayerInfoPacket;
class SetExperiencePacket;
class SetCreativeModeSlotPacket;
class AddExperienceOrbPacket;

// 1.0.1
class ContainerButtonClickPacket;
class TileEntityDataPacket;

// 1.1
class CustomPayloadPacket;

// 1.2.3
class RotateHeadPacket;

// 1.3.2
class ClientProtocolPacket;
class ServerAuthDataPacket;
class SharedKeyPacket;
class PlayerAbilitiesPacket;
class ChatAutoCompletePacket;
class ClientInformationPacket;
class LevelSoundPacket;
class TileDestructionPacket;
class ClientCommandPacket;
class LevelChunksPacket;

// 1.6.4
class SetObjectivePacket;
class SetScorePacket;
class SetDisplayObjectivePacket;
class SetPlayerTeamPacket;
class LevelParticlesPacket;
class UpdateAttributesPacket;
class TileEditorOpenPacket;

// 4J Added
class CraftItemPacket;
class TradeItemPacket;
class DebugOptionsPacket;
class ServerSettingsChangedPacket;
class TexturePacket;
class TextureAndGeometryPacket;
class ChunkVisibilityAreaPacket;
class UpdateProgressPacket;
class TextureChangePacket;
class TextureAndGeometryChangePacket;
class UpdateGameRuleProgressPacket;
class KickPlayerPacket;
class AdditionalModelPartsPacket;
class XZPacket;
class GameCommandPacket;

class PacketListener
{
public:
	virtual bool isServerPacketListener() = 0;
	virtual void handleBlockRegionUpdate(shared_ptr<BlockRegionUpdatePacket> packet);
	virtual void onUnhandledPacket(shared_ptr<Packet> packet);
	virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason, void *reasonObjects);
	virtual void handleDisconnect(shared_ptr<DisconnectPacket> packet);
	virtual void handleLogin(shared_ptr<LoginPacket> packet);
	virtual void handleMovePlayer(shared_ptr<MovePlayerPacket> packet);
	virtual void handleChunkTilesUpdate(shared_ptr<ChunkTilesUpdatePacket> packet);
	virtual void handlePlayerAction(shared_ptr<PlayerActionPacket> packet);
	virtual void handleTileUpdate(shared_ptr<TileUpdatePacket> packet);
	virtual void handleChunkVisibility(shared_ptr<ChunkVisibilityPacket> packet);
	virtual void handleAddPlayer(shared_ptr<AddPlayerPacket> packet);
	virtual void handleMoveEntity(shared_ptr<MoveEntityPacket> packet);
	virtual void handleMoveEntitySmall(shared_ptr<MoveEntityPacketSmall> packet);
	virtual void handleTeleportEntity(shared_ptr<TeleportEntityPacket> packet);
	virtual void handleUseItem(shared_ptr<UseItemPacket> packet);
	virtual void handleSetCarriedItem(shared_ptr<SetCarriedItemPacket> packet);
	virtual void handleRemoveEntity(shared_ptr<RemoveEntitiesPacket> packet);
	virtual void handleTakeItemEntity(shared_ptr<TakeItemEntityPacket> packet);
	virtual void handleChat(shared_ptr<ChatPacket> packet);
	virtual void handleAddEntity(shared_ptr<AddEntityPacket> packet);
	virtual void handleAnimate(shared_ptr<AnimatePacket> packet);
	virtual void handlePlayerCommand(shared_ptr<PlayerCommandPacket> packet);
	virtual void handlePreLogin(shared_ptr<PreLoginPacket> packet);
	virtual void handleAddMob(shared_ptr<AddMobPacket> packet);
	virtual void handleSetTime(shared_ptr<SetTimePacket> packet);
	virtual void handleSetSpawn(shared_ptr<SetSpawnPositionPacket> packet);
	virtual void handleSetEntityMotion(shared_ptr<SetEntityMotionPacket> packet);
	virtual void handleSetEntityData(shared_ptr<SetEntityDataPacket> packet);
	virtual void handleEntityLinkPacket(shared_ptr<SetEntityLinkPacket> packet);
	virtual void handleInteract(shared_ptr<InteractPacket> packet);
	virtual void handleEntityEvent(shared_ptr<EntityEventPacket> packet);
	virtual void handleSetHealth(shared_ptr<SetHealthPacket> packet);
	virtual void handleRespawn(shared_ptr<RespawnPacket> packet);
	virtual void handleExplosion(shared_ptr<ExplodePacket> packet);
	virtual void handleContainerOpen(shared_ptr<ContainerOpenPacket> packet);
	virtual void handleContainerClose(shared_ptr<ContainerClosePacket> packet);
	virtual void handleContainerClick(shared_ptr<ContainerClickPacket> packet);
	virtual void handleContainerSetSlot(shared_ptr<ContainerSetSlotPacket> packet);
	virtual void handleContainerContent(shared_ptr<ContainerSetContentPacket> packet);
	virtual void handleSignUpdate(shared_ptr<SignUpdatePacket> packet);
	virtual void handleContainerSetData(shared_ptr<ContainerSetDataPacket> packet);
	virtual void handleSetEquippedItem(shared_ptr<SetEquippedItemPacket> packet);
	virtual void handleContainerAck(shared_ptr<ContainerAckPacket> packet);
	virtual void handleAddPainting(shared_ptr<AddPaintingPacket> packet);
	virtual void handleTileEvent(shared_ptr<TileEventPacket> packet);
	virtual void handleAwardStat(shared_ptr<AwardStatPacket> packet);
	virtual void handleEntityActionAtPosition(shared_ptr<EntityActionAtPositionPacket> packet);
	virtual void handlePlayerInput(shared_ptr<PlayerInputPacket> packet);
	virtual void handleGameEvent(shared_ptr<GameEventPacket> packet);
	virtual void handleAddGlobalEntity(shared_ptr<AddGlobalEntityPacket> packet);
	virtual void handleComplexItemData(shared_ptr<ComplexItemDataPacket> packet);
	virtual void handleLevelEvent(shared_ptr<LevelEventPacket> packet);

	//  1.8.2
	virtual void handleGetInfo(shared_ptr<GetInfoPacket> packet);
	virtual void handleUpdateMobEffect(shared_ptr<UpdateMobEffectPacket> packet);
	virtual void handleRemoveMobEffect(shared_ptr<RemoveMobEffectPacket> packet);
	virtual void handlePlayerInfo(shared_ptr<PlayerInfoPacket> packet);
	virtual void handleKeepAlive(shared_ptr<KeepAlivePacket> packet);
	virtual void handleSetExperience(shared_ptr<SetExperiencePacket> packet);
	virtual void handleSetCreativeModeSlot(shared_ptr<SetCreativeModeSlotPacket> packet);
	virtual void handleAddExperienceOrb(shared_ptr<AddExperienceOrbPacket> packet);

	// 1.0.1
	virtual void handleContainerButtonClick(shared_ptr<ContainerButtonClickPacket> packet);
	virtual void handleTileEntityData(shared_ptr<TileEntityDataPacket> tileEntityDataPacket);

	// 1.1s
	virtual void handleCustomPayload(shared_ptr<CustomPayloadPacket> customPayloadPacket);

	// 1.2.3
	virtual void handleRotateMob(shared_ptr<RotateHeadPacket> rotateMobPacket);

	// 1.3.2
	virtual void handleClientProtocolPacket(shared_ptr<ClientProtocolPacket> packet);
	virtual void handleServerAuthData(shared_ptr<ServerAuthDataPacket> packet);
	//virtual void handleSharedKey(shared_ptr<SharedKeyPacket> packet);
	virtual void handlePlayerAbilities(shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket);
	virtual void handleChatAutoComplete(shared_ptr<ChatAutoCompletePacket> packet);
	virtual void handleClientInformation(shared_ptr<ClientInformationPacket> packet);
	virtual void handleSoundEvent(shared_ptr<LevelSoundPacket> packet);
	virtual void handleTileDestruction(shared_ptr<TileDestructionPacket> packet);
	virtual void handleClientCommand(shared_ptr<ClientCommandPacket> packet);
	//virtual void handleLevelChunks(shared_ptr<LevelChunksPacket> packet);
	virtual bool canHandleAsyncPackets();

	// 1.6.4
	virtual void handleAddObjective(shared_ptr<SetObjectivePacket> packet);
	virtual void handleSetScore(shared_ptr<SetScorePacket> packet);
	virtual void handleSetDisplayObjective(shared_ptr<SetDisplayObjectivePacket> packet);
	virtual void handleSetPlayerTeamPacket(shared_ptr<SetPlayerTeamPacket> packet);
	virtual void handleParticleEvent(shared_ptr<LevelParticlesPacket> packet);
	virtual void handleUpdateAttributes(shared_ptr<UpdateAttributesPacket> packet);
	virtual void handleTileEditorOpen(shared_ptr<TileEditorOpenPacket> tileEditorOpenPacket);
	virtual bool isDisconnected();

	// 4J Added
	virtual void handleCraftItem(shared_ptr<CraftItemPacket> packet);
	virtual void handleTradeItem(shared_ptr<TradeItemPacket> packet);
	virtual void handleDebugOptions(shared_ptr<DebugOptionsPacket> packet);
	virtual void handleServerSettingsChanged(shared_ptr<ServerSettingsChangedPacket> packet);
	virtual void handleTexture(shared_ptr<TexturePacket> packet);	
	virtual void handleTextureAndGeometry(shared_ptr<TextureAndGeometryPacket> packet);	
	virtual void handleChunkVisibilityArea(shared_ptr<ChunkVisibilityAreaPacket> packet);
	virtual void handleUpdateProgress(shared_ptr<UpdateProgressPacket> packet);
	virtual void handleTextureChange(shared_ptr<TextureChangePacket> packet);
	virtual void handleTextureAndGeometryChange(shared_ptr<TextureAndGeometryChangePacket> packet);
	virtual void handleUpdateGameRuleProgressPacket(shared_ptr<UpdateGameRuleProgressPacket> packet);
	virtual void handleKickPlayer(shared_ptr<KickPlayerPacket> packet);
	virtual void handleXZ(shared_ptr<XZPacket> packet);
	virtual void handleGameCommand(shared_ptr<GameCommandPacket> packet);
};
