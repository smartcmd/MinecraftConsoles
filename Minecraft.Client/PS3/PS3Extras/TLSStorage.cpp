

#include "stdafx.h"



TLSStoragePS3* TLSStoragePS3::m_pInstance = nullptr;

BOOL TLSStoragePS3::m_activeList[sc_maxSlots];
__thread LPVOID TLSStoragePS3::m_values[sc_maxSlots];



TLSStoragePS3::TLSStoragePS3()
{
	for(int i=0;i<sc_maxSlots; i++)
	{
		m_activeList[i] = false;
		m_values[i] = nullptr;
	}
}

TLSStoragePS3* TLSStoragePS3::Instance()
{
	if ( m_pInstance == 0 )			// Is this the first time?
	{
		m_pInstance = new TLSStoragePS3;		// Create the singleton instance.
	}
	return m_pInstance;
}


int TLSStoragePS3::Alloc()
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
	return -1;
}

BOOL TLSStoragePS3::Free( DWORD _index )
{
	if(m_activeList[_index] == false)
		return false; // not been allocated

	m_activeList[_index] = false;
	m_values[_index] = nullptr;
	return true;
}

BOOL TLSStoragePS3::SetValue( DWORD _index, LPVOID _val )
{
	if(m_activeList[_index] == false)
		return false;
	m_values[_index] = _val;
	return true;
}

LPVOID TLSStoragePS3::GetValue( DWORD _index )
{
	if(m_activeList[_index] == false)
		return nullptr;
	return m_values[_index];
}

