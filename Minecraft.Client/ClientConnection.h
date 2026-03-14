#pragma once
#include <unordered_set>
#include "../Minecraft.World/net.minecraft.network.h"
class Minecraft;
class MultiPlayerLevel;
class SavedDataStorage;
class Socket;
class MultiplayerLocalPlayer;

class ClientConnection : public PacketListener
{
private:
	enum eClientConnectionConnectingState
	{
		eCCPreLoginSent = 0,
		eCCPreLoginReceived,
		eCCLoginSent,
		eCCLoginReceived,
		eCCConnected
	};

private:
	bool done;
    Connection *connection;
public:
	wstring message;
	bool createdOk;	// 4J added
private:
	Minecraft *minecraft;
    MultiPlayerLevel *level;
    bool started;

	// 4J Stu - I don't think we are interested in the PlayerInfo data, so I'm not going to use it at the moment
	//Map<String, PlayerInfo> playerInfoMap = new HashMap<String, PlayerInfo>();
public:
	//List<PlayerInfo> playerInfos = new ArrayList<PlayerInfo>();

	int maxPlayers;

public:
	bool isStarted() { return started; } // 4J Added
	bool isClosed() { return done; } // 4J Added
	Socket *getSocket() { return connection->getSocket(); } // 4J Added

private:
	DWORD m_userIndex; // 4J Added
	bool isPrimaryConnection() const;

	std::unordered_set<int> m_trackedEntityIds;
	std::unordered_set<int64_t> m_visibleChunks;

	static int64_t chunkKey(int x, int z) { return ((int64_t)x << 32) | ((int64_t)z & 0xFFFFFFFF); }

	ClientConnection* findPrimaryConnection() const;
	bool shouldProcessForEntity(int entityId) const;
	bool shouldProcessForPosition(int blockX, int blockZ) const;
	bool anyOtherConnectionHasChunk(int x, int z) const;

public:
	bool isTrackingEntity(int entityId) const { return m_trackedEntityIds.count(entityId) > 0; }

public:
	SavedDataStorage *savedDataStorage;
    ClientConnection(Minecraft *minecraft, const wstring& ip, int port);
	ClientConnection(Minecraft *minecraft, Socket *socket, int iUserIndex = -1);
	~ClientConnection();
    void tick();
	INetworkPlayer *getNetworkPlayer();
    virtual void handleLogin(shared_ptr<LoginPacket> packet);
    virtual void handleAddEntity(shared_ptr<AddEntityPacket> packet);
	virtual void handleAddExperienceOrb(shared_ptr<AddExperienceOrbPacket> packet);
    virtual void handleAddGlobalEntity(shared_ptr<AddGlobalEntityPacket> packet);
    virtual void handleAddPainting(shared_ptr<AddPaintingPacket> packet);
    virtual void handleSetEntityMotion(shared_ptr<SetEntityMotionPacket> packet);
    virtual void handleSetEntityData(shared_ptr<SetEntityDataPacket> packet);
    virtual void handleAddPlayer(shared_ptr<AddPlayerPacket> packet);
    virtual void handleTeleportEntity(shared_ptr<TeleportEntityPacket> packet);
	virtual void handleSetCarriedItem(shared_ptr<SetCarriedItemPacket> packet);
    virtual void handleMoveEntity(shared_ptr<MoveEntityPacket> packet);
	virtual void handleRotateMob(shared_ptr<RotateHeadPacket> packet);
	virtual void handleMoveEntitySmall(shared_ptr<MoveEntityPacketSmall> packet);
    virtual void handleRemoveEntity(shared_ptr<RemoveEntitiesPacket> packet);
	virtual void handleMovePlayer(shared_ptr<MovePlayerPacket> packet);

    Random *random;
	
	// 4J Added
	virtual void handleChunkVisibilityArea(shared_ptr<ChunkVisibilityAreaPacket> packet);

    virtual void handleChunkVisibility(shared_ptr<ChunkVisibilityPacket> packet);
    virtual void handleChunkTilesUpdate(shared_ptr<ChunkTilesUpdatePacket> packet);
    virtual void handleBlockRegionUpdate(shared_ptr<BlockRegionUpdatePacket> packet);
    virtual void handleTileUpdate(shared_ptr<TileUpdatePacket> packet);
    virtual void handleDisconnect(shared_ptr<DisconnectPacket> packet);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason, void *reasonObjects);
    void sendAndDisconnect(shared_ptr<Packet> packet);
    void send(shared_ptr<Packet> packet);
    virtual void handleTakeItemEntity(shared_ptr<TakeItemEntityPacket> packet);
    virtual void handleChat(shared_ptr<ChatPacket> packet);
    virtual void handleAnimate(shared_ptr<AnimatePacket> packet);
    virtual void handleEntityActionAtPosition(shared_ptr<EntityActionAtPositionPacket> packet);
    virtual void handlePreLogin(shared_ptr<PreLoginPacket> packet);
    void close();
    virtual void handleAddMob(shared_ptr<AddMobPacket> packet);
    virtual void handleSetTime(shared_ptr<SetTimePacket> packet);
    virtual void handleSetSpawn(shared_ptr<SetSpawnPositionPacket> packet);
    virtual void handleEntityLinkPacket(shared_ptr<SetEntityLinkPacket> packet);
    virtual void handleEntityEvent(shared_ptr<EntityEventPacket> packet);
private:
	shared_ptr<Entity> getEntity(int entityId);
	wstring GetDisplayNameByGamertag(wstring gamertag);
public:
    virtual void handleSetHealth(shared_ptr<SetHealthPacket> packet);
	virtual void handleSetExperience(shared_ptr<SetExperiencePacket> packet);
    virtual void handleRespawn(shared_ptr<RespawnPacket> packet);
    virtual void handleExplosion(shared_ptr<ExplodePacket> packet);
    virtual void handleContainerOpen(shared_ptr<ContainerOpenPacket> packet);
    virtual void handleContainerSetSlot(shared_ptr<ContainerSetSlotPacket> packet);
    virtual void handleContainerAck(shared_ptr<ContainerAckPacket> packet);
    virtual void handleContainerContent(shared_ptr<ContainerSetContentPacket> packet);
	virtual void handleTileEditorOpen(shared_ptr<TileEditorOpenPacket> packet);
    virtual void handleSignUpdate(shared_ptr<SignUpdatePacket> packet);
	virtual void handleTileEntityData(shared_ptr<TileEntityDataPacket> packet);
    virtual void handleContainerSetData(shared_ptr<ContainerSetDataPacket> packet);
    virtual void handleSetEquippedItem(shared_ptr<SetEquippedItemPacket> packet);
    virtual void handleContainerClose(shared_ptr<ContainerClosePacket> packet);
    virtual void handleTileEvent(shared_ptr<TileEventPacket> packet);
	virtual void handleTileDestruction(shared_ptr<TileDestructionPacket> packet);
	virtual bool canHandleAsyncPackets();
    virtual void handleGameEvent(shared_ptr<GameEventPacket> gameEventPacket);
    virtual void handleComplexItemData(shared_ptr<ComplexItemDataPacket> packet);
    virtual void handleLevelEvent(shared_ptr<LevelEventPacket> packet);
    virtual void handleAwardStat(shared_ptr<AwardStatPacket> packet);
	virtual void handleUpdateMobEffect(shared_ptr<UpdateMobEffectPacket> packet);
	virtual void handleRemoveMobEffect(shared_ptr<RemoveMobEffectPacket> packet);
	virtual bool isServerPacketListener();
	virtual void handlePlayerInfo(shared_ptr<PlayerInfoPacket> packet);
	virtual void handleKeepAlive(shared_ptr<KeepAlivePacket> packet);
	virtual void handlePlayerAbilities(shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket);
	virtual void handleSoundEvent(shared_ptr<LevelSoundPacket> packet);
	virtual void handleCustomPayload(shared_ptr<CustomPayloadPacket> customPayloadPacket);
	virtual Connection *getConnection();

	// 4J Added
	virtual void handleServerSettingsChanged(shared_ptr<ServerSettingsChangedPacket> packet);
	virtual void handleTexture(shared_ptr<TexturePacket> packet);
	virtual void handleTextureAndGeometry(shared_ptr<TextureAndGeometryPacket> packet);
	virtual void handleUpdateProgress(shared_ptr<UpdateProgressPacket> packet);

	// 4J Added
	static int HostDisconnectReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	static int ExitGameAndSaveReturned(void *pParam,int iPad,C4JStorage::EMessageResult result);
	virtual void handleTextureChange(shared_ptr<TextureChangePacket> packet);
	virtual void handleTextureAndGeometryChange(shared_ptr<TextureAndGeometryChangePacket> packet);
	virtual void handleUpdateGameRuleProgressPacket(shared_ptr<UpdateGameRuleProgressPacket> packet);
	virtual void handleXZ(shared_ptr<XZPacket> packet);

	void displayPrivilegeChanges(shared_ptr<MultiplayerLocalPlayer> player, unsigned int oldPrivileges);

	virtual void handleAddObjective(shared_ptr<SetObjectivePacket> packet);
    virtual void handleSetScore(shared_ptr<SetScorePacket> packet);
    virtual void handleSetDisplayObjective(shared_ptr<SetDisplayObjectivePacket> packet);
    virtual void handleSetPlayerTeamPacket(shared_ptr<SetPlayerTeamPacket> packet);
    virtual void handleParticleEvent(shared_ptr<LevelParticlesPacket> packet);
	virtual void handleUpdateAttributes(shared_ptr<UpdateAttributesPacket> packet);

private:
	// 4J: Entity link packet deferred
	class DeferredEntityLinkPacket
	{
	public:
		DWORD m_recievedTick;
		shared_ptr<SetEntityLinkPacket> m_packet;

		DeferredEntityLinkPacket(shared_ptr<SetEntityLinkPacket> packet);
	};

	vector<DeferredEntityLinkPacket> deferredEntityLinkPackets;
	static const int MAX_ENTITY_LINK_DEFERRAL_INTERVAL = 1000;

	void checkDeferredEntityLinkPackets(int newEntityId);
};