

#include "stdafx.h"



TLSStorageOrbis* TLSStorageOrbis::m_pInstance = nullptr;

BOOL TLSStorageOrbis::m_activeList[sc_maxSlots];
__thread LPVOID TLSStorageOrbis::m_values[sc_maxSlots];



TLSStorageOrbis::TLSStorageOrbis()
{
	for(int i=0;i<sc_maxSlots; i++)
	{
		m_activeList[i] = false;
		m_values[i] = nullptr;
	}
}

TLSStorageOrbis* TLSStorageOrbis::Instance()
{
	if ( m_pInstance == 0 )			// Is this the first time?
	{
		m_pInstance = new TLSStorageOrbis;		// Create the singleton instance.
	}
	return m_pInstance;
}


int TLSStorageOrbis::Alloc()
{
	for(int i=0; i<sc_maxSlots; i++) 
	{
		if(m_activeList[i] == false)
		{
			m_activeList[i] = true;
			m_values[i] = nullptr;
			return i;
		}
	}
	assert(0);		// we've ran out of slots
}

BOOL TLSStorageOrbis::Free( DWORD _index )
{
	if(m_activeList[_index] == false)
		return false; // not been allocated

	m_activeList[_index] = false;
	m_values[_index] = nullptr;
	return true;
}

BOOL TLSStorageOrbis::SetValue( DWORD _index, LPVOID _val )
{
	if(m_activeList[_index] == false)
		return false;
	m_values[_index] = _val;
	return true;
}

LPVOID TLSStorageOrbis::GetValue( DWORD _index )
{
	if(m_activeList[_index] == false)
		return nullptr;
	return m_values[_index];
}

