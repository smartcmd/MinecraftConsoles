#pragma once
using namespace std;

#include "stdafx.h"
#include <queue>
#include "System.h"
#include "DataInputStream.h"
#include "DataOutputStream.h"
#include "net.minecraft.network.packet.h"
#include "C4JThread.h"

#include "Socket.h"

// 4J JEV, size of the threads (bytes).
#define READ_STACK_SIZE 0
#define WRITE_STACK_SIZE 0

class ByteArrayOutputStream;

class Connection
{
	friend DWORD WINAPI runRead(LPVOID lpParam);
	friend DWORD WINAPI runWrite(LPVOID lpParam);
	friend DWORD WINAPI runSendAndQuit(LPVOID lpParam);
	friend DWORD WINAPI runClose(LPVOID lpParam);

private:
	static const int SEND_BUFFER_SIZE = 1024 * 5;

public:
	static int readThreads, writeThreads;

private:
	static const int MAX_TICKS_WITHOUT_INPUT = 20 * 60;

public:
	static const int IPTOS_LOWCOST = 0x02;
	static const int IPTOS_RELIABILITY = 0x04;
	static const int IPTOS_THROUGHPUT = 0x08;
	static const int IPTOS_LOWDELAY = 0x10;

private:
	Socket *socket;
	const SocketAddress *address;
	DataInputStream *dis;
	DataOutputStream *bufferedDos; // 4J This is the same type of dos the java game has

	// 4J Added
	DataOutputStream *byteArrayDos; // 4J This dos allows us to write individual packets to the socket
	ByteArrayOutputStream *baos;
	Socket::SocketOutputStream *sos;

	bool running;

	queue<shared_ptr<Packet> > incoming;			// 4J - was using synchronizedList...
	CRITICAL_SECTION incoming_cs;		// ... now has this critical section
	queue<shared_ptr<Packet> > outgoing;			// 4J - was using synchronizedList - but don't think it is required as usage is wrapped in writeLock critical section
	queue<shared_ptr<Packet> > outgoing_slow;		// 4J - was using synchronizedList - but don't think it is required as usage is wrapped in writeLock critical section
	

	PacketListener *packetListener;
	bool quitting;


	C4JThread* readThread;
	C4JThread* writeThread;

	C4JThread::Event* m_hWakeReadThread;
	C4JThread::Event* m_hWakeWriteThread;

	DWORD saqThreadID, closeThreadID;

	bool disconnected;
	DisconnectPacket::eDisconnectReason disconnectReason;
	void **disconnectReasonObjects; // 4J a pointer to an array.

	int noInputTicks;
	int estimatedRemaining;

	int tickCount; // 4J Added

public:
	static int readSizes[256];
	static int writeSizes[256];

	int fakeLag;

private:
	void _init();

	// 4J Jev, these might be better of as private
	CRITICAL_SECTION threadCounterLock;
	CRITICAL_SECTION writeLock;

public:
	// 4J Jev, need to delete the critical section.
	~Connection();
	Connection(Socket *socket, const wstring& id, PacketListener *packetListener); // throws IOException

	void setListener(PacketListener *packetListener);
	void send(shared_ptr<Packet> packet);

public:
	void queueSend(shared_ptr<Packet> packet);

private:
	int slowWriteDelay;

	bool writeTick();

public:
	void flush();

private:
	bool readTick();

private:

	/* 4J JEV, removed try/catch
	void handleException(Exception e)
	{
	e.printStackTrace();
	close("disconnect.genericReason", "Internal exception: " + e.toString());
	}*/

public:
	void close(DisconnectPacket::eDisconnectReason reason, ...);

	void tick();

	SocketAddress *getRemoteAddress();

	void sendAndQuit();

	int countDelayedPackets();

	Socket *getSocket() { return socket; }

private:
	static int runRead(void* lpParam);
	static int runWrite(void* lpParam);
	static int runClose(void* lpParam);
	static int runSendAndQuit(void* lpParam);

};
