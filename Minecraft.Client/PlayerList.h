#pragma once
#include <deque>
#include "..\Minecraft.World\ArrayWithLength.h"

class ServerPlayer;
class PlayerChunkMap;
class MinecraftServer;
class PlayerIO;
class PendingConnection;
class Packet;
class ServerLevel;
class TileEntity;
class ProgressListener;
class GameType;
class LoginPacket;

using namespace std;

class PlayerList
{
private:
	 static const int SEND_PLAYER_INFO_INTERVAL = 20 * 10;	// 4J - brought forward from 1.2.3
//    public static Logger logger = Logger.getLogger("Minecraft");
public:
	vector<shared_ptr<ServerPlayer> > players;

private:
	MinecraftServer *server;
    unsigned int maxPlayers;

	// 4J Added
	vector<PlayerUID> m_bannedXuids;
	deque<BYTE> m_smallIdsToKick;
	CRITICAL_SECTION m_kickPlayersCS;
	deque<BYTE> m_smallIdsToClose;
	CRITICAL_SECTION m_closePlayersCS;
/* 4J - removed
	Set<String> bans = new HashSet<String>();
    Set<String> ipBans = new HashSet<String>();
    Set<String> ops = new HashSet<String>();
    Set<String> whitelist = new HashSet<String>();
    File banFile, ipBanFile, opFile, whiteListFile;
	*/
    PlayerIO *playerIo;
    bool doWhiteList;

	GameType *overrideGameMode;
	bool allowCheatsForAllPlayers;
	int viewDistance;

	int sendAllPlayerInfoIn;

	// 4J Added to maintain which players in which dimensions can receive all packet types
	vector<shared_ptr<ServerPlayer> > receiveAllPlayers[3];
private:
	shared_ptr<ServerPlayer> findAlivePlayerOnSystem(shared_ptr<ServerPlayer> currentPlayer);

public:
	void removePlayerFromReceiving(shared_ptr<ServerPlayer> player, bool usePlayerDimension = true, int dimension = 0);
	void addPlayerToReceiving(shared_ptr<ServerPlayer> player);
	bool canReceiveAllPackets(shared_ptr<ServerPlayer> player);

public:
	PlayerList(MinecraftServer *server);
	~PlayerList();
	void placeNewPlayer(Connection *connection, shared_ptr<ServerPlayer> player, shared_ptr<LoginPacket> packet);
    void setLevel(ServerLevelArray levels);
    void changeDimension(shared_ptr<ServerPlayer> player, ServerLevel *from);
    int getMaxRange();
	bool load(shared_ptr<ServerPlayer> player); // 4J Changed return val to bool to check if new player or loaded player
protected:
	void save(shared_ptr<ServerPlayer> player);
public:
	void validatePlayerSpawnPosition(shared_ptr<ServerPlayer> player); // 4J Added
    void add(shared_ptr<ServerPlayer> player);
    void move(shared_ptr<ServerPlayer> player);
    void remove(shared_ptr<ServerPlayer> player);
    shared_ptr<ServerPlayer> getPlayerForLogin(PendingConnection *pendingConnection, const wstring& userName, PlayerUID xuid, PlayerUID OnlineXuid);
    shared_ptr<ServerPlayer> respawn(shared_ptr<ServerPlayer> serverPlayer, int targetDimension, bool keepAllPlayerData);
    void toggleDimension(shared_ptr<ServerPlayer> player, int targetDimension);
    void tick();
	bool isTrackingTile(int x, int y, int z, int dimension);		// 4J added
	void prioritiseTileChanges(int x, int y, int z, int dimension);	// 4J added
    void broadcastAll(shared_ptr<Packet> packet);
    void broadcastAll(shared_ptr<Packet> packet, int dimension);

    wstring getPlayerNames();

public:
	bool isWhiteListed(const wstring& name);
    bool isOp(const wstring& name);
	bool isOp(shared_ptr<ServerPlayer> player); // 4J Added
    shared_ptr<ServerPlayer> getPlayer(const wstring& name);
	shared_ptr<ServerPlayer> getPlayer(PlayerUID uid);
    void sendMessage(const wstring& name, const wstring& message);
    void broadcast(double x, double y, double z, double range, int dimension, shared_ptr<Packet> packet);
    void broadcast(shared_ptr<Player> except, double x, double y, double z, double range, int dimension, shared_ptr<Packet> packet);
    void broadcastToAllOps(const wstring& message);
    bool sendTo(const wstring& name, shared_ptr<Packet> packet);
	// 4J Added ProgressListener *progressListener param and bDeleteGuestMaps param
    void saveAll(ProgressListener *progressListener, bool bDeleteGuestMaps = false);
    void whiteList(const wstring& playerName);
    void blackList(const wstring& playerName);
//    Set<String> getWhiteList();		/ 4J removed
    void reloadWhitelist();
    void sendLevelInfo(shared_ptr<ServerPlayer> player, ServerLevel *level);
    void sendAllPlayerInfo(shared_ptr<ServerPlayer> player);
	int getPlayerCount();
	int getPlayerCount(ServerLevel *level); // 4J Added
	int getMaxPlayers();
	MinecraftServer *getServer();
	int getViewDistance();
	void setOverrideGameMode(GameType *gameMode);

private:
	void updatePlayerGameMode(shared_ptr<ServerPlayer> newPlayer, shared_ptr<ServerPlayer> oldPlayer, Level *level);

public:
	void setAllowCheatsForAllPlayers(bool allowCommands);

	// 4J Added
	void kickPlayerByShortId(BYTE networkSmallId);
	void closePlayerConnectionBySmallId(BYTE networkSmallId);
	bool isXuidBanned(PlayerUID xuid);
	// AP added for Vita so the range can be increased once the level starts
	void setViewDistance(int newViewDistance);
};
