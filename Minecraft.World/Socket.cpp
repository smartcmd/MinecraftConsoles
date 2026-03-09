#include "stdafx.h"
#include "InputOutputStream.h"
#include "SocketAddress.h"
#include "Socket.h"
#include "ThreadName.h"
#include "..\Minecraft.Client\ServerConnection.h"
#include <algorithm>
#include "..\Minecraft.Client\PS3\PS3Extras\ShutdownManager.h"

// This current socket implementation is for the creation of a single local link. 2 sockets can be created, one for either end of this local
// link, the end (0 or 1) is passed as a parameter to the ctor.

CRITICAL_SECTION Socket::s_hostQueueLock[2]; 
std::queue<byte> Socket::s_hostQueue[2];
Socket::SocketOutputStreamLocal *Socket::s_hostOutStream[2];
Socket::SocketInputStreamLocal *Socket::s_hostInStream[2];
ServerConnection *Socket::s_serverConnection = nullptr;

void Socket::Initialise(ServerConnection *serverConnection)
{
	s_serverConnection = serverConnection;

	// Only initialise everything else once - just setting up static data, one time xrnm things, thread for ticking sockets
	static bool init = false;
	if( init )
	{
		for( int i = 0; i < 2; i++ )
		{
			if(TryEnterCriticalSection(&s_hostQueueLock[i]))
			{			
				// Clear the queue
				std::queue<byte> empty;
				std::swap( s_hostQueue[i], empty );
				LeaveCriticalSection(&s_hostQueueLock[i]);
			}
			s_hostOutStream[i]->m_streamOpen = true;
			s_hostInStream[i]->m_streamOpen = true;
		}
		return;
	}
	init = true;

	for( int i = 0; i < 2; i++ )
	{
		InitializeCriticalSection(&Socket::s_hostQueueLock[i]);
		s_hostOutStream[i] = new SocketOutputStreamLocal(i);
		s_hostInStream[i] = new SocketInputStreamLocal(i);
	}
}

Socket::Socket(bool response)
{
	m_hostServerConnection = true;
	m_hostLocal = true;
	if( response )
	{
		m_end = SOCKET_SERVER_END;
	}
	else
	{
		m_end = SOCKET_CLIENT_END;
		Socket *socket = new Socket(1);
		s_serverConnection->NewIncomingSocket(socket);
	}

	for( int i = 0; i < 2; i++ )
	{		
		m_endClosed[i] = false;
	}
	m_socketClosedEvent = nullptr;
	createdOk = true;
	networkPlayerSmallId = g_NetworkManager.GetHostPlayer()->GetSmallId();
}

Socket::Socket(INetworkPlayer *player, bool response /* = false*/, bool hostLocal /*= false*/)
{
	m_hostServerConnection = false;
	m_hostLocal = hostLocal;

	for( int i = 0; i < 2; i++ )
	{
		InitializeCriticalSection(&m_queueLockNetwork[i]);		
		m_inputStream[i] = nullptr;
		m_outputStream[i] = nullptr;		
		m_endClosed[i] = false;
	}
	
	if(!response || hostLocal)
	{
		m_inputStream[0] = new SocketInputStreamNetwork(this,0);
		m_outputStream[0] = new SocketOutputStreamNetwork(this,0);
		m_end = SOCKET_CLIENT_END;
	}
	if(response || hostLocal)
	{
		m_inputStream[1] = new SocketInputStreamNetwork(this,1);
		m_outputStream[1] = new SocketOutputStreamNetwork(this,1);
		m_end = SOCKET_SERVER_END;
	}
	m_socketClosedEvent = new C4JThread::Event;
	//printf("New socket made %s\n", player->GetGamertag() );
	networkPlayerSmallId = player->GetSmallId();
	createdOk = true;
}

SocketAddress *Socket::getRemoteSocketAddress()
{
	return nullptr;
}

INetworkPlayer *Socket::getPlayer()
{
	return g_NetworkManager.GetPlayerBySmallId(networkPlayerSmallId);
}

void Socket::setPlayer(INetworkPlayer *player)
{
	if(player!=nullptr)
	{
		networkPlayerSmallId = player->GetSmallId();
	}
	else
	{
		networkPlayerSmallId = 0;
	}
}

void Socket::pushDataToQueue(const BYTE * pbData, DWORD dwDataSize, bool fromHost /*= true*/)
{
	int queueIdx = SOCKET_CLIENT_END;
	if(!fromHost)
		queueIdx = SOCKET_SERVER_END;

	if( queueIdx != m_end && !m_hostLocal )
	{
		app.DebugPrintf("SOCKET: Error pushing data to queue. End is %d but queue idx id %d\n", m_end, queueIdx);
		return;
	}

	//app.DebugPrintf("SOCKET PUSH: %u bytes to queue[%d] firstByte=0x%02X smallId=%d\n",
	//	dwDataSize, queueIdx, dwDataSize > 0 ? pbData[0] : 0, networkPlayerSmallId);

	EnterCriticalSection(&m_queueLockNetwork[queueIdx]);
    if (m_queueNetwork[queueIdx].size() + dwDataSize > 2 * 1024 * 1024)
    {
        LeaveCriticalSection(&m_queueLockNetwork[queueIdx]);
        return;
    }
	for( unsigned int i = 0; i < dwDataSize; i++ )
	{
		m_queueNetwork[queueIdx].push(*pbData++);
	}
	LeaveCriticalSection(&m_queueLockNetwork[queueIdx]);
}

void Socket::addIncomingSocket(Socket *socket)
{
	if( s_serverConnection != nullptr )
	{
		s_serverConnection->NewIncomingSocket(socket);
	}
}

InputStream *Socket::getInputStream(bool isServerConnection)
{
	if( !m_hostServerConnection )
	{
		if( m_hostLocal )
		{
			if( isServerConnection )
			{
				return m_inputStream[SOCKET_SERVER_END];
			}
			else
			{
				return m_inputStream[SOCKET_CLIENT_END];
			}
		}
		else
		{
			return m_inputStream[m_end];
		}
	}
	else
	{
		return s_hostInStream[m_end];
	}
}

void Socket::setSoTimeout(int a )
{
}

void Socket::setTrafficClass( int a )
{
}

Socket::SocketOutputStream *Socket::getOutputStream(bool isServerConnection)
{
	if( !m_hostServerConnection )
	{
		if( m_hostLocal )
		{
			if( isServerConnection )
			{
				return m_outputStream[SOCKET_SERVER_END];
			}
			else
			{
				return m_outputStream[SOCKET_CLIENT_END];
			}
		}
		else
		{
			return m_outputStream[m_end];
		}
	}
	else
	{
		return s_hostOutStream[ 1 - m_end ];
	}
}

bool Socket::close(bool isServerConnection)
{
	bool allClosed = false;
	if( m_hostLocal )
	{
		if( isServerConnection )
		{
			m_endClosed[SOCKET_SERVER_END] = true;
			if(m_endClosed[SOCKET_CLIENT_END])
			{
				allClosed = true;
			}
		}
		else
		{
			m_endClosed[SOCKET_CLIENT_END] = true;
			if(m_endClosed[SOCKET_SERVER_END])
			{
				allClosed = true;
			}
		}
	}
	else
	{
		allClosed = true;
		m_endClosed[m_end] = true;
	}
	if( allClosed && m_socketClosedEvent != nullptr )
	{
		m_socketClosedEvent->Set();
	}
	if(allClosed) createdOk = false;
	return allClosed;
}

/////////////////////////////////// Socket for input, on local connection ////////////////////

Socket::SocketInputStreamLocal::SocketInputStreamLocal(int queueIdx)
{
	m_streamOpen = true;
	m_queueIdx = queueIdx;
}

// Try and get an input byte, blocking until one is available
int Socket::SocketInputStreamLocal::read()
{
	while(m_streamOpen && ShutdownManager::ShouldRun(ShutdownManager::eConnectionReadThreads))
	{
		if(TryEnterCriticalSection(&s_hostQueueLock[m_queueIdx]))
		{
			if( s_hostQueue[m_queueIdx].size() )
			{
				byte retval = s_hostQueue[m_queueIdx].front();
				s_hostQueue[m_queueIdx].pop();
				LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
				return retval;
			}
			LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
		}
		Sleep(1);
	}
	return -1;
}

// Try and get an input array of bytes, blocking until enough bytes are available
int Socket::SocketInputStreamLocal::read(byteArray b)
{
	return read(b, 0, b.length);
}

// Try and get an input range of bytes, blocking until enough bytes are available
int Socket::SocketInputStreamLocal::read(byteArray b, unsigned int offset, unsigned int length)
{
	while(m_streamOpen)
	{
		if(TryEnterCriticalSection(&s_hostQueueLock[m_queueIdx]))
		{
			if( s_hostQueue[m_queueIdx].size() >= length )
			{
				for( unsigned int i = 0; i < length; i++ )
				{
					b[i+offset] = s_hostQueue[m_queueIdx].front();
					s_hostQueue[m_queueIdx].pop();
				}
				LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
				return length;
			}
			LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
		}
		Sleep(1);
	}
	return -1;
}

void Socket::SocketInputStreamLocal::close()
{
	m_streamOpen = false;
	EnterCriticalSection(&s_hostQueueLock[m_queueIdx]);
	std::queue<byte> empty;
	std::swap(s_hostQueue[m_queueIdx], empty);
	LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
}

/////////////////////////////////// Socket for output, on local connection ////////////////////

Socket::SocketOutputStreamLocal::SocketOutputStreamLocal(int queueIdx)
{
	m_streamOpen = true;
	m_queueIdx = queueIdx;
}

void Socket::SocketOutputStreamLocal::write(unsigned int b)
{
	if( m_streamOpen != true )
	{
		return;
	}
	EnterCriticalSection(&s_hostQueueLock[m_queueIdx]);
	s_hostQueue[m_queueIdx].push(static_cast<byte>(b));
	LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
}

void Socket::SocketOutputStreamLocal::write(byteArray b)
{
	write(b, 0, b.length);
}

void Socket::SocketOutputStreamLocal::write(byteArray b, unsigned int offset, unsigned int length)
{
	if( m_streamOpen != true )
	{
		return;
	}
	MemSect(12);
	EnterCriticalSection(&s_hostQueueLock[m_queueIdx]);
	for( unsigned int i = 0; i < length; i++ )
	{
		s_hostQueue[m_queueIdx].push(b[offset+i]);
	}
	LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
	MemSect(0);
}

void Socket::SocketOutputStreamLocal::close()
{
	m_streamOpen = false;
	EnterCriticalSection(&s_hostQueueLock[m_queueIdx]);
	std::queue<byte> empty;
	std::swap(s_hostQueue[m_queueIdx], empty);
	LeaveCriticalSection(&s_hostQueueLock[m_queueIdx]);
}

/////////////////////////////////// Socket for input, on network connection ////////////////////

Socket::SocketInputStreamNetwork::SocketInputStreamNetwork(Socket *socket, int queueIdx)
{
	m_streamOpen = true;
	m_queueIdx = queueIdx;
	m_socket = socket;
}

// Try and get an input byte, blocking until one is available
int Socket::SocketInputStreamNetwork::read()
{
	while(m_streamOpen && ShutdownManager::ShouldRun(ShutdownManager::eConnectionReadThreads))
	{
		if(TryEnterCriticalSection(&m_socket->m_queueLockNetwork[m_queueIdx]))
		{
			if( m_socket->m_queueNetwork[m_queueIdx].size() )
			{
				byte retval = m_socket->m_queueNetwork[m_queueIdx].front();
				m_socket->m_queueNetwork[m_queueIdx].pop();
				LeaveCriticalSection(&m_socket->m_queueLockNetwork[m_queueIdx]);
				return retval;
			}
			LeaveCriticalSection(&m_socket->m_queueLockNetwork[m_queueIdx]);
		}
		Sleep(1);
	}
	return -1;
}

// Try and get an input array of bytes, blocking until enough bytes are available
int Socket::SocketInputStreamNetwork::read(byteArray b)
{
	return read(b, 0, b.length);
}

// Try and get an input range of bytes, blocking until enough bytes are available
int Socket::SocketInputStreamNetwork::read(byteArray b, unsigned int offset, unsigned int length)
{
	while(m_streamOpen)
	{
		if(TryEnterCriticalSection(&m_socket->m_queueLockNetwork[m_queueIdx]))
		{
			if( m_socket->m_queueNetwork[m_queueIdx].size() >= length )
			{
				for( unsigned int i = 0; i < length; i++ )
				{
					b[i+offset] = m_socket->m_queueNetwork[m_queueIdx].front();
					m_socket->m_queueNetwork[m_queueIdx].pop();
				}
				LeaveCriticalSection(&m_socket->m_queueLockNetwork[m_queueIdx]);
				return length;
			}
			LeaveCriticalSection(&m_socket->m_queueLockNetwork[m_queueIdx]);
		}
		Sleep(1);
	}
	return -1;
}

void Socket::SocketInputStreamNetwork::close()
{
	m_streamOpen = false;
}

/////////////////////////////////// Socket for output, on network connection ////////////////////

Socket::SocketOutputStreamNetwork::SocketOutputStreamNetwork(Socket *socket, int queueIdx)
{
	m_queueIdx = queueIdx;
	m_socket = socket;
	m_streamOpen = true;
}

void Socket::SocketOutputStreamNetwork::write(unsigned int b)
{
	if( m_streamOpen != true ) return;
	byteArray barray;
	byte bb;
	bb = static_cast<byte>(b);
	barray.data = &bb;
	barray.length = 1;
	write(barray, 0, 1);

}

void Socket::SocketOutputStreamNetwork::write(byteArray b)
{
	write(b, 0, b.length);
}

void Socket::SocketOutputStreamNetwork::write(byteArray b, unsigned int offset, unsigned int length)
{	
	writeWithFlags(b, offset, length, 0);
}

void Socket::SocketOutputStreamNetwork::writeWithFlags(byteArray b, unsigned int offset, unsigned int length, int flags)
{
	if( m_streamOpen != true ) return;
	if( length == 0 ) return;

	// If this is a local connection, don't bother going through QNet as it just delivers it straight anyway
	if( m_socket->m_hostLocal )
	{
		// We want to write to the queue for the other end of this socket stream
		int queueIdx = m_queueIdx;
		if(queueIdx == SOCKET_CLIENT_END)
			queueIdx = SOCKET_SERVER_END;
		else
			queueIdx = SOCKET_CLIENT_END;

		EnterCriticalSection(&m_socket->m_queueLockNetwork[queueIdx]);
		for( unsigned int i = 0; i < length; i++ )
		{
			m_socket->m_queueNetwork[queueIdx].push(b[offset+i]);
		}
		LeaveCriticalSection(&m_socket->m_queueLockNetwork[queueIdx]);
	}
	else
	{
		XRNM_SEND_BUFFER buffer;
		buffer.pbyData = &b[offset];
		buffer.dwDataSize = length;

		INetworkPlayer *hostPlayer = g_NetworkManager.GetHostPlayer();
		if(hostPlayer == nullptr)
		{
			app.DebugPrintf("Trying to write to network, but the hostPlayer is nullptr\n");
			return;
		}
		INetworkPlayer *socketPlayer = m_socket->getPlayer();
		if(socketPlayer == nullptr)
		{
			app.DebugPrintf("Trying to write to network, but the socketPlayer is nullptr\n");
			return;
		}

#ifdef _XBOX
		bool lowPriority = ( ( flags & QNET_SENDDATA_LOW_PRIORITY )		== QNET_SENDDATA_LOW_PRIORITY );
		bool requireAck = lowPriority;
#else
		bool lowPriority = false;
		bool requireAck =  ( ( flags & NON_QNET_SENDDATA_ACK_REQUIRED ) == NON_QNET_SENDDATA_ACK_REQUIRED );
#endif

		if( m_queueIdx == SOCKET_SERVER_END )
		{
			//printf( "Sent %u bytes of data from \"%ls\" to \"%ls\"\n",
			//buffer.dwDataSize,
			//hostPlayer->GetGamertag(),
			//m_socket->networkPlayer->GetGamertag());

			hostPlayer->SendData(socketPlayer, buffer.pbyData, buffer.dwDataSize, lowPriority, requireAck);

	// 		DWORD queueSize = hostPlayer->GetSendQueueSize( nullptr, QNET_GETSENDQUEUESIZE_BYTES  );
	// 		if( queueSize > 24000 )
	// 		{
	// 			//printf("Queue size is: %d, forcing doWork()\n",queueSize);
	// 			g_NetworkManager.DoWork();
	// 		}
		}
		else
		{
			//printf( "Sent %u bytes of data from \"%ls\" to \"%ls\"\n",
			//buffer.dwDataSize,
			//m_socket->networkPlayer->GetGamertag(),
			//hostPlayer->GetGamertag());

			socketPlayer->SendData(hostPlayer, buffer.pbyData, buffer.dwDataSize, lowPriority, requireAck);
		}
	}
}

void Socket::SocketOutputStreamNetwork::close()
{
	m_streamOpen = false;
}