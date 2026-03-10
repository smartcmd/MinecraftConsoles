#include "stdafx.h"
#include "IntCache.h"

DWORD IntCache::tlsIdx = TlsAlloc();

void IntCache::CreateNewThreadStorage()
{
	ThreadStorage *tls = new ThreadStorage();
	TlsSetValue(tlsIdx, (void *)tls);
	tls->maxSize = TINY_CUTOFF;
}

IntCache::ThreadStorage::~ThreadStorage()
{
	for(unsigned int i = 0; i < tcache.size(); i++ )
	{
		delete [] tcache[i].data;
	}
	for(unsigned int i = 0; i < tallocated.size(); i++ )
	{
		delete [] tallocated[i].data;
	}
	for(unsigned int i = 0; i < cache.size(); i++ )
	{
		delete [] cache[i].data;
	}
	for(unsigned int i = 0; i < allocated.size(); i++ )
	{
		delete [] allocated[i].data;
	}
	for( size_t i = 0; i < toosmall.size(); i++ )
	{
		delete [] toosmall[i].data;
	}
}

void IntCache::ReleaseThreadStorage()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));


	delete tls;
}

intArray IntCache::allocate(int size)
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));

	if (size <= TINY_CUTOFF)
	{
		if (tls->tcache.empty())
		{
			intArray result = intArray(static_cast<unsigned int>(TINY_CUTOFF), true);
			tls->tallocated.push_back(result);
			return result;
		}
		else
		{
			intArray result = tls->tcache.back();
			tls->tcache.pop_back();
			tls->tallocated.push_back(result);
			return result;
		}
	}

	if (size > tls->maxSize)
	{
//		app.DebugPrintf("IntCache: New max size: %d\n" , size);
		tls->maxSize = size;

		// 4J - added - all the vectors in cache & allocated are smaller than maxSize so should be discarded. However, we
		// can't delete them until the next releaseAll so copy into another vector until then
		tls->toosmall.insert(tls->toosmall.end(),tls->cache.begin(),tls->cache.end());
		tls->toosmall.insert(tls->toosmall.end(),tls->allocated.begin(),tls->allocated.end());

		tls->cache.clear();
		tls->allocated.clear();

		intArray result = intArray(static_cast<unsigned int>(tls->maxSize), true);
		tls->allocated.push_back(result);
		return result;
	}
	else
	{
		if (tls->cache.empty())
		{
			intArray result = intArray(static_cast<unsigned int>(tls->maxSize), true);
			tls->allocated.push_back(result);
			return result;
		}
		else
		{
			intArray result = tls->cache.back();
			tls->cache.pop_back();
			tls->allocated.push_back(result);
			return result;
		}
	}
}

void IntCache::releaseAll()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));

	// 4J - added - we can now remove the vectors that were deemed as too small (see comment in IntCache::allocate)
	for( size_t i = 0; i < tls->toosmall.size(); i++ )
	{
		delete [] tls->toosmall[i].data;
	}
	tls->toosmall.clear();

	if (!tls->cache.empty())
	{
		delete [] tls->cache.back().data;
		tls->cache.pop_back();
	}
	if (!tls->tcache.empty())
	{
		delete [] tls->tcache.back().data;
		tls->tcache.pop_back();
	}

	tls->cache.insert(tls->cache.end(),tls->allocated.begin(),tls->allocated.end());
	tls->tcache.insert(tls->tcache.end(),tls->tallocated.begin(),tls->tallocated.end());

	tls->allocated.clear();
	tls->tallocated.clear();
}

// 4J added so that we can fully reset between levels
void IntCache::Reset()
{
	ThreadStorage *tls = static_cast<ThreadStorage *>(TlsGetValue(tlsIdx));
	tls->maxSize = TINY_CUTOFF;
	for( size_t i = 0; i < tls->allocated.size(); i++ )
	{
		delete [] tls->allocated[i].data;
	}
	tls->allocated.clear();

	for( size_t i = 0; i < tls->cache.size(); i++ )
	{
		delete [] tls->cache[i].data;
	}
	tls->cache.clear();

	for( size_t i = 0; i < tls->tallocated.size(); i++ )
	{
		delete [] tls->tallocated[i].data;
	}
	tls->tallocated.clear();

	for( size_t i = 0; i < tls->tcache.size(); i++ )
	{
		delete [] tls->tcache[i].data;
	}
	tls->tcache.clear();

	for( int i = 0; i < tls->toosmall.size(); i++ )
	{
		delete [] tls->toosmall[i].data;
	}
	tls->toosmall.clear();
}