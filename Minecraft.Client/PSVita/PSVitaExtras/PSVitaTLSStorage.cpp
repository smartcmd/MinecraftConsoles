#include "stdafx.h"
#include "PSVitaTLSStorage.h"

#if 1
static PSVitaTLSStorage Singleton;

#define MAX_THREADS 64

typedef struct
{
	int ThreadID;
	LPVOID Value;
} TLSInfo;

typedef struct
{
	TLSInfo *Array;
	int Size;
} TLSArray;

static std::vector<TLSArray *> PSVitaTLSStorage_ActiveInfos;

void PSVitaTLSStorage::Init()
{
}

PSVitaTLSStorage *PSVitaTLSStorage::Instance()
{
	return &Singleton;
}

DWORD PSVitaTLSStorage::Alloc()
{
	TLSArray* Array = new TLSArray;
	Array->Array = new TLSInfo[MAX_THREADS];
	Array->Size = 0;
	for( int i = 0;i < MAX_THREADS;i += 1 )
	{
		Array->Array[i].ThreadID = -1;
	}

	// add to the active infos
	PSVitaTLSStorage_ActiveInfos.push_back(Array);
	return (DWORD) Array;
}

BOOL PSVitaTLSStorage::Free(DWORD dwTlsIndex)
{
	// remove from the active infos
	std::vector<TLSArray *>::iterator iter = std::find(PSVitaTLSStorage_ActiveInfos.begin(), PSVitaTLSStorage_ActiveInfos.end(), (TLSArray *) dwTlsIndex);
	PSVitaTLSStorage_ActiveInfos.erase(iter);

	delete []((TLSInfo*)dwTlsIndex);
	return 0;
}

void PSVitaTLSStorage::RemoveThread(int threadID)
{
	// remove this thread from all the active Infos
	std::vector<TLSArray *>::iterator iter = PSVitaTLSStorage_ActiveInfos.begin();
	for( ; iter != PSVitaTLSStorage_ActiveInfos.end(); ++iter )
	{
		TLSArray* Array = ((TLSArray*)*iter);

		for( int i = 0;i < Array->Size;i += 1 )
		{
			if( Array->Array[i].ThreadID == threadID )
			{
				// shift all the other entries down
				for( ;i < MAX_THREADS - 1;i += 1 )
				{
					Array->Array[i].ThreadID = Array->Array[i+1].ThreadID;
					Array->Array[i].Value = Array->Array[i+1].Value;
					if( Array->Array[i].ThreadID != -1 )
					{
						Array->Size = i + 1;
					}
				}
				// mark the top one as unused
				Array->Array[i].ThreadID = -1;
				break;
			}
		}
	}
}

LPVOID PSVitaTLSStorage::GetValue(DWORD dwTlsIndex)
{
	if( !dwTlsIndex )
	{
		return 0;
	}

	TLSArray* Array = ((TLSArray*)dwTlsIndex);
	SceUID threadID = sceKernelGetThreadId();
	for( int i = 0;i < Array->Size;i += 1 )
	{
		if( Array->Array[i].ThreadID == threadID )
		{
			return Array->Array[i].Value;
		}
	}

	//assert(0);
	return 0;
}

BOOL PSVitaTLSStorage::SetValue(DWORD dwTlsIndex, LPVOID lpTlsValue)
{
	if( !dwTlsIndex )
	{
		return 0;
	}

	TLSArray* Array = ((TLSArray*)dwTlsIndex);
	SceUID threadID = sceKernelGetThreadId();
	for( int i = 0;i < Array->Size;i += 1 )
	{
		if( Array->Array[i].ThreadID == threadID || Array->Array[i].ThreadID == -1 )
		{
			Array->Array[i].ThreadID = threadID;
			Array->Array[i].Value = lpTlsValue;
			return 0;
		}
	}
	if( Array->Size < MAX_THREADS )
	{
		Array->Array[Array->Size].ThreadID = threadID;
		Array->Array[Array->Size].Value = lpTlsValue;
		Array->Size++;
		return 0;
	}

	assert(0);
	return 0;
}
#else


PSVitaTLSStorage* m_pInstance = nullptr;

#define sc_maxSlots 64
BOOL m_activeList[sc_maxSlots];
//__thread void* m_values[64];

// AP - Oh my. It seems __thread doesn't like cpp files. I couldn't get it to be thread sensitive as it would always return a shared value.
// For now I've stuck m_values and accessor functions into user_malloc.c. Cheap cheap hack.
extern "C"
{
void user_setValue( unsigned int _index, void* _val );
void* user_getValue( unsigned int _index );
}


void PSVitaTLSStorage::Init()
{
	for(int i=0;i<sc_maxSlots; i++)
	{
		m_activeList[i] = false;
//		m_values[i] = nullptr;
	}
}

PSVitaTLSStorage* PSVitaTLSStorage::Instance()
{
	if ( m_pInstance == 0 )			// Is this the first time?
	{
		m_pInstance = new PSVitaTLSStorage;		// Create the singleton instance.
		m_pInstance->Init();
	}
	return m_pInstance;
}


DWORD PSVitaTLSStorage::Alloc()
{
	for(int i=0; i<sc_maxSlots; i++) 
	{
		if(m_activeList[i] == false)
		{
			m_activeList[i] = true;
//			m_values[i] = nullptr;
			return i;
		}
	}
	assert(0);		// we've ran out of slots
	return -1;
}

BOOL PSVitaTLSStorage::Free( DWORD _index )
{
	if(m_activeList[_index] == false)
		return false; // not been allocated

	m_activeList[_index] = false;
//	m_values[_index] = nullptr;
	return true;
}

BOOL PSVitaTLSStorage::SetValue( DWORD _index, LPVOID _val )
{
	if(m_activeList[_index] == false)
		return false;
	user_setValue(_index, _val);
//	m_values[_index] = _val;
	return true;
}

LPVOID PSVitaTLSStorage::GetValue( DWORD _index )
{
	if(m_activeList[_index] == false)
		return nullptr;
	return user_getValue(_index);
//	return m_values[_index];
}

void PSVitaTLSStorage::RemoveThread(int threadID)
{
	for(int i=0; i<sc_maxSlots; i++) 
	{
		//m_values[i] = nullptr;
		user_setValue(i, nullptr);
	}
}

// AP - Oh my. It seems __thread doesn't like cpp files. I couldn't get it to be thread sensitive as it would always return a shared value.
// For now I've stuck m_values and accessor functions into user_malloc.c. Cheap cheap hack.
__thread void* m_values[64];
void user_setValue( unsigned int _index, void* _val )
{
	m_values[_index] = _val;
}
void* user_getValue( unsigned int _index )
{
	return m_values[_index];
}
#endif