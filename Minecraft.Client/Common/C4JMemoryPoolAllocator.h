

#pragma once
#include "../Minecraft.Client/Common/C4JMemoryPool.h"

// Custom allocator, takes a C4JMemoryPool class, which can be one of a number of pool implementations.

template <class T>
class C4JPoolAllocator
{
public:
    typedef T          value_type;
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
 
    typedef T*         pointer;
    typedef const T*   const_pointer;
 
    typedef T&         reference;
    typedef const T&   const_reference;
 
	//! A struct to construct an allocator for a different type.
	template<typename U> 
	struct rebind { typedef C4JPoolAllocator<U> other; };


	C4JMemoryPool*	m_pPool;
	bool			m_selfAllocated;

	C4JPoolAllocator( C4JMemoryPool* pool = new C4JMemoryPoolFixed(32, 4096 )) : m_pPool( pool ), m_selfAllocated(true) 
	{
		printf("allocated mempool\n");
	}

	template<typename U>
	C4JPoolAllocator(C4JPoolAllocator<U> const& obj) : m_pPool( obj.m_pPool ), m_selfAllocated(false) // copy constructor
    {
		printf("C4JPoolAllocator constructed from 0x%08x\n", &obj);
		assert(obj.m_pPool);
    }
private:

public:
  
    ~C4JPoolAllocator()
    {
		if(m_selfAllocated)
			delete m_pPool;
    }
 
    pointer address( reference r ) const				{ return &r; }
    const_pointer address( const_reference r ) const    { return &r; }
 
    pointer allocate( size_type n, const void* /*hint*/=0 )
    {
		assert(m_pPool);
        return (pointer)m_pPool->Alloc(n * sizeof(T));
    }
 
    void deallocate( pointer p, size_type /*n*/ )
    {
		assert(m_pPool);
		m_pPool->Free(p);
    }
 
    void construct( pointer p, const T& val )
    {
        new (p) T(val);
    }
 
    void destroy( pointer p )
    {
        p->~T();
    }
 
    size_type max_size() const
    {
        return ULONG_MAX / sizeof(T);
    }
 
};
 
 
template <class T>
bool
operator==( const C4JPoolAllocator<T>& left, const C4JPoolAllocator<T>& right )
{
	if (left.m_pPool == right.m_pPool)
    {
        return true;
    }
	return false;
}
 
template <class T>
bool
operator!=( const C4JPoolAllocator<T>& left, const C4JPoolAllocator<T>& right)
{
     if (left.m_pPool != right.m_pPool)
     {
         return true;
     }
    return false;
}

 







