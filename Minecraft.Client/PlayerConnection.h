#include "ConsoleInputSource.h"
#include "..\Minecraft.World\PacketListener.h"
#include "..\Minecraft.World\JavaIntHash.h"

class MinecraftServer;
class Connection;
class ServerPlayer;
class INetworkPlayer;

using namespace std;

class PlayerConnection : public PacketListener, public ConsoleInputSource
{
//    public static Logger logger = Logger.getLogger("Minecraft");

public:
	Connection *connection;
    bool done;
	CRITICAL_SECTION done_cs;

	// 4J Stu - Added this so that we can manage UGC privileges
	PlayerUID m_offlineXUID, m_onlineXUID;
	bool m_friendsOnlyUGC;

private:
	MinecraftServer *server;
    shared_ptr<ServerPlayer> player;
    int tickCount;
    int aboveGroundTickCount;

    bool didTick;
	int lastKeepAliveId;
	int64_t lastKeepAliveTime;
	static Random random;
	int64_t lastKeepAliveTick;
	int chatSpamTickCount;
	int dropSpamTickCount;

	bool m_bHasClientTickedOnce;

public:
	PlayerConnection(MinecraftServer *server, Connection *connection, shared_ptr<ServerPlayer> player);
	~PlayerConnection();
    void tick();
    void disconnect(DisconnectPacket::eDisconnectReason reason);

private:
	double xLastOk, yLastOk, zLastOk;
    bool synched;

public:
	virtual void handlePlayerInput(shared_ptr<PlayerInputPacket> packet);
    virtual void handleMovePlayer(shared_ptr<MovePlayerPacket> packet);
    void teleport(double x, double y, double z, float yRot, float xRot, bool sendPacket = true); // 4J Added sendPacket param
    virtual void handlePlayerAction(shared_ptr<PlayerActionPacket> packet);
    virtual void handleUseItem(shared_ptr<UseItemPacket> packet);
    virtual void onDisconnect(DisconnectPacket::eDisconnectReason reason, void *reasonObjects);
    virtual void onUnhandledPacket(shared_ptr<Packet> packet);
    void send(shared_ptr<Packet> packet);
	void queueSend(shared_ptr<Packet> packet); // 4J Added
    virtual void handleSetCarriedItem(shared_ptr<SetCarriedItemPacket> packet);
    virtual void handleChat(shared_ptr<ChatPacket> packet);
private:
	void handleCommand(const wstring& message);
public:
	virtual void handleAnimate(shared_ptr<AnimatePacket> packet);
    virtual void handlePlayerCommand(shared_ptr<PlayerCommandPacket> packet);
    virtual void handleDisconnect(shared_ptr<DisconnectPacket> packet);
    int countDelayedPackets();
    virtual void info(const wstring& string);
    virtual void warn(const wstring& string);
    virtual wstring getConsoleName();
    virtual void handleInteract(shared_ptr<InteractPacket> packet);
	bool canHandleAsyncPackets();
	virtual void handleClientCommand(shared_ptr<ClientCommandPacket> packet);
    virtual void handleRespawn(shared_ptr<RespawnPacket> packet);
    virtual void handleContainerClose(shared_ptr<ContainerClosePacket> packet);

private:
	unordered_map<int, short, IntKeyHash, IntKeyEq> expectedAcks;

public:
	// 4J Stu - Handlers only valid in debug mode
#ifndef _CONTENT_PACKAGE
    virtual void handleContainerSetSlot(shared_ptr<ContainerSetSlotPacket> packet);
#endif
	virtual void handleContainerClick(shared_ptr<ContainerClickPacket> packet);
	virtual void handleContainerButtonClick(shared_ptr<ContainerButtonClickPacket> packet);
	virtual void handleSetCreativeModeSlot(shared_ptr<SetCreativeModeSlotPacket> packet);
    virtual void handleContainerAck(shared_ptr<ContainerAckPacket> packet);
    virtual void handleSignUpdate(shared_ptr<SignUpdatePacket> packet);
	virtual void handleKeepAlive(shared_ptr<KeepAlivePacket> packet);
	virtual void handlePlayerInfo(shared_ptr<PlayerInfoPacket> packet); // 4J Added
    virtual bool isServerPacketListener();
	virtual void handlePlayerAbilities(shared_ptr<PlayerAbilitiesPacket> playerAbilitiesPacket);
	virtual void handleCustomPayload(shared_ptr<CustomPayloadPacket> customPayloadPacket);
	virtual bool isDisconnected();

	// 4J Added
	virtual void handleCraftItem(shared_ptr<CraftItemPacket> packet);
	virtual void handleTradeItem(shared_ptr<TradeItemPacket> packet);
	virtual void handleDebugOptions(shared_ptr<DebugOptionsPacket> packet);
	virtual void handleTexture(shared_ptr<TexturePacket> packet);
	virtual void handleTextureAndGeometry(shared_ptr<TextureAndGeometryPacket> packet);
	virtual void handleTextureChange(shared_ptr<TextureChangePacket> packet);
	virtual void handleTextureAndGeometryChange(shared_ptr<TextureAndGeometryChangePacket> packet);
	virtual void handleServerSettingsChanged(shared_ptr<ServerSettingsChangedPacket> packet);
	virtual void handleKickPlayer(shared_ptr<KickPlayerPacket> packet);
	virtual void handleGameCommand(shared_ptr<GameCommandPacket> packet);

	INetworkPlayer *getNetworkPlayer();
	bool isLocal();
	bool isGuest();

	// 4J Added as we need to set this from outside sometimes
	void setPlayer(shared_ptr<ServerPlayer> player) { this->player = player; }
	shared_ptr<ServerPlayer> getPlayer() { return player; }

	// 4J Added to signal a disconnect from another thread
	void closeOnTick() { m_bCloseOnTick = true; }

	// 4J Added so that we can send on textures that get received after this connection requested them
	void handleTextureReceived(const wstring &textureName);
	void handleTextureAndGeometryReceived(const wstring &textureName);

	void setShowOnMaps(bool bVal);

	void setWasKicked() { m_bWasKicked = true; }
	bool getWasKicked() { return m_bWasKicked; }

	// 4J Added
	bool hasClientTickedOnce() { return m_bHasClientTickedOnce; }

private:
	bool m_bCloseOnTick;
	vector<wstring> m_texturesRequested;

	bool m_bWasKicked;
};