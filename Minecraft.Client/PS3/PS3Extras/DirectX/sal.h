/***
*sal.h - markers for documenting the semantics of APIs
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       sal.h provides a set of annotations to describe how a function uses its
*       parameters - the assumptions it makes about them, and the guarantees it makes
*       upon finishing.
*
*       [Public]
*
****/

#pragma once
/*==========================================================================

   The macros are defined in 3 layers:

   _In_\_Out_ Layer:
   ----------------
   This layer provides the highest abstraction and its macros should be used
   in most cases. Its macros start with _In_, _Out_ or _Inout_. For the
   typical case they provide the most concise annotations.

   _Pre_\_Post_ Layer:
   ------------------
   The macros of this layer only should be used when there is no suitable macro
   in the _In_\_Out_ layer. Its macros start with _Pre_, _Post_, _Ret_,
   _Deref_pre_ _Deref_post_ and _Deref_ret_. This layer provides the most
   flexibility for annotations.

   Implementation Abstraction Layer:
   --------------------------------
   Macros from this layer should never be used directly. The layer only exists
   to hide the implementation of the annotation macros.


   Annotation Syntax:
   |--------------|----------|----------------|-----------------------------|
   |   Usage      | Nullness | ZeroTerminated |  Extent                     |
   |--------------|----------|----------------|-----------------------------|
   | _In_         | <>       | <>             | <>                          |
   | _Out_        | opt_     | z_             | [byte]cap_[c_|x_]( size )   |
   | _Inout_      |          |                | [byte]count_[c_|x_]( size ) |
   | _Deref_out_  |          |                | ptrdiff_cap_( ptr )         |
   |--------------|          |                | ptrdiff_count_( ptr )       |
   | _Ret_        |          |                |                             |
   | _Deref_ret_  |          |                |                             |
   |--------------|          |                |                             |
   | _Pre_        |          |                |                             |
   | _Post_       |          |                |                             |
   | _Deref_pre_  |          |                |                             |
   | _Deref_post_ |          |                |                             |
   |--------------|----------|----------------|-----------------------------|

   Usage:
   -----
   _In_, _Out_, _Inout_, _Pre_, _Post_, _Deref_pre_, _Deref_post_ are for
   formal parameters.
   _Ret_, _Deref_ret_ must be used for return values.

   Nullness:
   --------
   If the pointer can be NULL the annotation contains _opt. If the macro
   does not contain '_opt' the pointer may not be NULL.

   String Type:
   -----------
   _z: NullTerminated string
   for _In_ parameters the buffer must have the specified stringtype before the call
   for _Out_ parameters the buffer must have the specified stringtype after the call
   for _Inout_ parameters both conditions apply

   Extent Syntax:
   |------|---------------|---------------|
   | Unit | Writ\Readable | Argument Type |
   |------|---------------|---------------|
   |  <>  | cap_          | <>            |
   | byte | count_        | c_            |
   |      |               | x_            |
   |------|---------------|---------------|

   'cap' (capacity) describes the writable size of the buffer and is typically used
   with _Out_. The default unit is elements. Use 'bytecap' if the size is given in bytes
   'count' describes the readable size of the buffer and is typically used with _In_.
   The default unit is elements. Use 'bytecount' if the size is given in bytes.
   
   Argument syntax for cap_, bytecap_, count_, bytecount_:
   (<parameter>|return)[+n]  e.g. cch, return, cb+2
   
   If the buffer size is a constant expression use the c_ postfix.
   E.g. cap_c_(20), count_c_(MAX_PATH), bytecount_c_(16)

   If the buffer size is given by a limiting pointer use the ptrdiff_ versions
   of the macros.

   If the buffer size is neither a parameter nor a constant expression use the x_
   postfix. e.g. bytecount_x_(num*size) x_ annotations accept any arbitrary string.
   No analysis can be done for x_ annotations but they at least tell the tool that
   the buffer has some sort of extent description. x_ annotations might be supported
   by future compiler versions.

============================================================================*/

#define __ATTR_SAL

#ifdef _PREFAST_
// choose attribute or __declspec implementation
#ifndef _USE_DECLSPECS_FOR_SAL
#define _USE_DECLSPECS_FOR_SAL 0
#endif

#if _USE_DECLSPECS_FOR_SAL
#undef _USE_ATTRIBUTES_FOR_SAL
#define _USE_ATTRIBUTES_FOR_SAL 0
#elif !defined(_USE_ATTRIBUTES_FOR_SAL)
#if _MSC_VER >= 1500 
#define _USE_ATTRIBUTES_FOR_SAL 1
#else
#define _USE_ATTRIBUTES_FOR_SAL 0
#endif // if _MSC_VER >= 1400
#endif // if _USE_DECLSPECS_FOR_SAL


#if !_USE_DECLSPECS_FOR_SAL
#if !_USE_ATTRIBUTES_FOR_SAL
#if _MSC_VER >= 1500 
#undef _USE_ATTRIBUTES_FOR_SAL
#define _USE_ATTRIBUTES_FOR_SAL 1
#else
#undef _USE_DECLSPECS_FOR_SAL
#define _USE_DECLSPECS_FOR_SAL  1
#endif  // _MSC_VER >= 1400
#endif  // !_USE_ATTRIBUTES_FOR_SAL
#endif  // !_USE_DECLSPECS_FOR_SAL

#endif // #ifdef _PREFAST_

// Disable expansion of SAL macros in non-Prefast mode to 
// improve compiler throughput.
#ifndef _USE_DECLSPECS_FOR_SAL
#define _USE_DECLSPECS_FOR_SAL 0
#endif
#ifndef _USE_ATTRIBUTES_FOR_SAL
#define _USE_ATTRIBUTES_FOR_SAL 0
#endif

// safeguard for MIDL and RC builds
#if _USE_DECLSPECS_FOR_SAL && ( defined( MIDL_PASS ) || defined(__midl) || defined(RC_INVOKED) || !defined(_PREFAST_) ) 
#undef _USE_DECLSPECS_FOR_SAL
#define _USE_DECLSPECS_FOR_SAL 0
#endif
#if _USE_ATTRIBUTES_FOR_SAL && ( !defined(_MSC_EXTENSIONS) || defined( MIDL_PASS ) || defined(__midl) || defined(RC_INVOKED) ) 
#undef _USE_ATTRIBUTES_FOR_SAL
#define _USE_ATTRIBUTES_FOR_SAL 0
#endif

#if defined(_MSC_EXTENSIONS) && !defined( MIDL_PASS ) && !defined(__midl) && !defined(RC_INVOKED)
#include "codeanalysis/sourceannotations.h"
#endif

//============================================================================
//   _In_\_Out_ Layer:
//============================================================================

// 'in' parameters --------------------------

// input pointer parameter
// e.g. void SetPoint( _In_ const POINT* pPT );
#define _In_                           _Pre1_impl_(_$notnull) _Deref_pre2_impl_(_$valid, _$readaccess)
#define _In_opt_                       _Pre_opt_valid_ _Deref_pre_readonly_

// nullterminated 'in' parameters.
// e.g. void CopyStr( _In_z_ const char* szFrom, _Out_z_cap_(cchTo) char* szTo, size_t cchTo );
#define _In_z_                         _Pre_z_      _Deref_pre_readonly_
#define _In_opt_z_                     _Pre_opt_z_  _Deref_pre_readonly_

// 'input' buffers with given size

// e.g. void SetCharRange( _In_count_(cch) const char* rgch, size_t cch )
// valid buffer extent described by another parameter
#define _In_count_(size)              _Pre_count_(size)         _Deref_pre_readonly_
#define _In_opt_count_(size)          _Pre_opt_count_(size)     _Deref_pre_readonly_
#define _In_bytecount_(size)          _Pre_bytecount_(size)     _Deref_pre_readonly_
#define _In_opt_bytecount_(size)      _Pre_opt_bytecount_(size) _Deref_pre_readonly_

// valid buffer extent described by a constant extression
#define _In_count_c_(size)            _Pre_count_c_(size)         _Deref_pre_readonly_
#define _In_opt_count_c_(size)        _Pre_opt_count_c_(size)     _Deref_pre_readonly_
#define _In_bytecount_c_(size)        _Pre_bytecount_c_(size)     _Deref_pre_readonly_
#define _In_opt_bytecount_c_(size)    _Pre_opt_bytecount_c_(size) _Deref_pre_readonly_

// nullterminated  'input' buffers with given size

// e.g. void SetCharRange( _In_count_(cch) const char* rgch, size_t cch )
// nullterminated valid buffer extent described by another parameter
#define _In_z_count_(size)              _Pre_z_ _Pre_count_(size)         _Deref_pre_readonly_
#define _In_opt_z_count_(size)          _Pre_opt_z_ _Pre_opt_count_(size)     _Deref_pre_readonly_
#define _In_z_bytecount_(size)          _Pre_z_ _Pre_bytecount_(size)     _Deref_pre_readonly_
#define _In_opt_z_bytecount_(size)      _Pre_opt_z_ _Pre_opt_bytecount_(size) _Deref_pre_readonly_

// nullterminated valid buffer extent described by a constant extression
#define _In_z_count_c_(size)            _Pre_z_ _Pre_count_c_(size)         _Deref_pre_readonly_
#define _In_opt_z_count_c_(size)        _Pre_opt_z_ _Pre_opt_count_c_(size)     _Deref_pre_readonly_
#define _In_z_bytecount_c_(size)        _Pre_z_ _Pre_bytecount_c_(size)     _Deref_pre_readonly_
#define _In_opt_z_bytecount_c_(size)    _Pre_opt_z_ _Pre_opt_bytecount_c_(size) _Deref_pre_readonly_

// buffer capacity is described by another pointer
// e.g. void Foo( _In_ptrdiff_count_(pchMax) const char* pch, const char* pchMax ) { while pch < pchMax ) pch++; }
#define _In_ptrdiff_count_(size)      _Pre_ptrdiff_count_(size)     _Deref_pre_readonly_
#define _In_opt_ptrdiff_count_(size)  _Pre_opt_ptrdiff_count_(size) _Deref_pre_readonly_

// 'x' version for complex expressions that are not supported by the current compiler version
// e.g. void Set3ColMatrix( _In_count_x_(3*cRows) const Elem* matrix, int cRows );
#define _In_count_x_(size)            _Pre_count_x_(size)         _Deref_pre_readonly_
#define _In_opt_count_x_(size)        _Pre_opt_count_x_(size)     _Deref_pre_readonly_
#define _In_bytecount_x_(size)        _Pre_bytecount_x_(size)     _Deref_pre_readonly_
#define _In_opt_bytecount_x_(size)    _Pre_opt_bytecount_x_(size) _Deref_pre_readonly_

// 'out' parameters --------------------------

// output pointer parameter
// e.g. void GetPoint( _Out_ POINT* pPT );
#define _Out_                            _Pre_cap_c_(1)            _Pre_invalid_
#define _Out_opt_                        _Pre_opt_cap_c_(1)        _Pre_invalid_

// 'out' with buffer size
// e.g. void GetIndeces( _Out_cap_(cIndeces) int* rgIndeces, size_t cIndices );
// buffer capacity is described by another parameter
#define _Out_cap_(size)                  _Pre_cap_(size)           _Pre_invalid_
#define _Out_opt_cap_(size)              _Pre_opt_cap_(size)       _Pre_invalid_
#define _Out_bytecap_(size)              _Pre_bytecap_(size)       _Pre_invalid_
#define _Out_opt_bytecap_(size)          _Pre_opt_bytecap_(size)   _Pre_invalid_

// buffer capacity is described by a constant expression
#define _Out_cap_c_(size)                _Pre_cap_c_(size)         _Pre_invalid_
#define _Out_opt_cap_c_(size)            _Pre_opt_cap_c_(size)     _Pre_invalid_
#define _Out_bytecap_c_(size)            _Pre_bytecap_c_(size)     _Pre_invalid_
#define _Out_opt_bytecap_c_(size)        _Pre_opt_bytecap_c_(size) _Pre_invalid_

// buffer capacity is described by another parameter multiplied by a constant expression
#define _Out_cap_m_(mult,size)           _Pre_cap_m_(mult,size)     _Pre_invalid_
#define _Out_opt_cap_m_(mult,size)       _Pre_opt_cap_m_(mult,size) _Pre_invalid_
#define _Out_z_cap_m_(mult,size)         _Pre_cap_m_(mult,size)     _Pre_invalid_ _Post_z_
#define _Out_opt_z_cap_m_(mult,size)     _Pre_opt_cap_m_(mult,size) _Pre_invalid_ _Post_z_

// buffer capacity is described by another pointer
// e.g. void Foo( _Out_ptrdiff_cap_(pchMax) char* pch, const char* pchMax ) { while pch < pchMax ) pch++; }
#define _Out_ptrdiff_cap_(size)          _Pre_ptrdiff_cap_(size)     _Pre_invalid_
#define _Out_opt_ptrdiff_cap_(size)      _Pre_opt_ptrdiff_cap_(size) _Pre_invalid_

// buffer capacity is described by a complex expression
#define _Out_cap_x_(size)                _Pre_cap_x_(size)         _Pre_invalid_
#define _Out_opt_cap_x_(size)            _Pre_opt_cap_x_(size)     _Pre_invalid_
#define _Out_bytecap_x_(size)            _Pre_bytecap_x_(size)     _Pre_invalid_
#define _Out_opt_bytecap_x_(size)        _Pre_opt_bytecap_x_(size) _Pre_invalid_

// a zero terminated string is filled into a buffer of given capacity
// e.g. void CopyStr( _In_z_ const char* szFrom, _Out_z_cap_(cchTo) char* szTo, size_t cchTo );
// buffer capacity is described by another parameter
#define _Out_z_cap_(size)                _Pre_cap_(size)           _Pre_invalid_ _Post_z_
#define _Out_opt_z_cap_(size)            _Pre_opt_cap_(size)       _Pre_invalid_ _Post_z_
#define _Out_z_bytecap_(size)            _Pre_bytecap_(size)       _Pre_invalid_ _Post_z_
#define _Out_opt_z_bytecap_(size)        _Pre_opt_bytecap_(size)   _Pre_invalid_ _Post_z_

// buffer capacity is described by a constant expression
#define _Out_z_cap_c_(size)              _Pre_cap_c_(size)         _Pre_invalid_ _Post_z_
#define _Out_opt_z_cap_c_(size)          _Pre_opt_cap_c_(size)     _Pre_invalid_ _Post_z_
#define _Out_z_bytecap_c_(size)          _Pre_bytecap_c_(size)     _Pre_invalid_ _Post_z_
#define _Out_opt_z_bytecap_c_(size)      _Pre_opt_bytecap_c_(size) _Pre_invalid_ _Post_z_

// buffer capacity is described by a complex expression
#define _Out_z_cap_x_(size)              _Pre_cap_x_(size)         _Pre_invalid_ _Post_z_
#define _Out_opt_z_cap_x_(size)          _Pre_opt_cap_x_(size)     _Pre_invalid_ _Post_z_
#define _Out_z_bytecap_x_(size)          _Pre_bytecap_x_(size)     _Pre_invalid_ _Post_z_
#define _Out_opt_z_bytecap_x_(size)      _Pre_opt_bytecap_x_(size) _Pre_invalid_ _Post_z_

// a zero terminated string is filled into a buffer of given capacity
// e.g. size_t CopyCharRange( _In_count_(cchFrom) const char* rgchFrom, size_t cchFrom, _Out_cap_post_count_(cchTo,return)) char* rgchTo, size_t cchTo );
#define _Out_cap_post_count_(cap,count)               _Pre_cap_(cap)         _Pre_invalid_ _Post_count_(count)
#define _Out_opt_cap_post_count_(cap,count)           _Pre_opt_cap_(cap)     _Pre_invalid_ _Post_count_(count)
#define _Out_bytecap_post_bytecount_(cap,count)       _Pre_bytecap_(cap)     _Pre_invalid_ _Post_bytecount_(count)
#define _Out_opt_bytecap_post_bytecount_(cap,count)   _Pre_opt_bytecap_(cap) _Pre_invalid_ _Post_bytecount_(count)

// a zero terminated string is filled into a buffer of given capacity
// e.g. size_t CopyStr( _In_z_ const char* szFrom, _Out_z_cap_post_count_(cchTo,return+1) char* szTo, size_t cchTo );
#define _Out_z_cap_post_count_(cap,count)              _Pre_cap_(cap)         _Pre_invalid_ _Post_z_count_(count)
#define _Out_opt_z_cap_post_count_(cap,count)          _Pre_opt_cap_(cap)     _Pre_invalid_ _Post_z_count_(count)
#define _Out_z_bytecap_post_bytecount_(cap,count)      _Pre_bytecap_(cap)     _Pre_invalid_ _Post_z_bytecount_(count)
#define _Out_opt_z_bytecap_post_bytecount_(cap,count)  _Pre_opt_bytecap_(cap) _Pre_invalid_ _Post_z_bytecount_(count)

// only use with dereferenced arguments e.g. '*pcch' 
#define _Out_capcount_(capcount)            _Pre_cap_(capcount)         _Pre_invalid_ _Post_count_(capcount)
#define _Out_opt_capcount_(capcount)        _Pre_opt_cap_(capcount)     _Pre_invalid_ _Post_count_(capcount)
#define _Out_bytecapcount_(capcount)        _Pre_bytecap_(capcount)     _Pre_invalid_ _Post_bytecount_(capcount)
#define _Out_opt_bytecapcount_(capcount)    _Pre_opt_bytecap_(capcount) _Pre_invalid_ _Post_bytecount_(capcount)

#define _Out_capcount_x_(capcount)          _Pre_cap_x_(capcount)         _Pre_invalid_ _Post_count_x_(capcount)
#define _Out_opt_capcount_x_(capcount)      _Pre_opt_cap_x_(capcount)     _Pre_invalid_ _Post_count_x_(capcount)
#define _Out_bytecapcount_x_(capcount)      _Pre_bytecap_x_(capcount)     _Pre_invalid_ _Post_bytecount_x_(capcount)
#define _Out_opt_bytecapcount_x_(capcount)  _Pre_opt_bytecap_x_(capcount) _Pre_invalid_ _Post_bytecount_x_(capcount)

// e.g. GetString( _Out_z_capcount_(*pLen+1) char* sz, size_t* pLen );
#define _Out_z_capcount_(capcount)          _Pre_cap_(capcount)         _Pre_invalid_ _Post_z_count_(capcount)
#define _Out_opt_z_capcount_(capcount)      _Pre_opt_cap_(capcount)     _Pre_invalid_ _Post_z_count_(capcount)
#define _Out_z_bytecapcount_(capcount)      _Pre_bytecap_(capcount)     _Pre_invalid_ _Post_z_bytecount_(capcount)
#define _Out_opt_z_bytecapcount_(capcount)  _Pre_opt_bytecap_(capcount) _Pre_invalid_ _Post_z_bytecount_(capcount)

// inout parameters ----------------------------

// inout pointer parameter
// e.g. void ModifyPoint( _Inout_ POINT* pPT );
#define _Inout_                          _Prepost_valid_
#define _Inout_opt_                      _Prepost_opt_valid_

// string buffers
// e.g. void toupper( _Inout_z_ char* sz );
#define _Inout_z_                        _Prepost_z_
#define _Inout_opt_z_                    _Prepost_opt_z_

// 'inout' buffers with initialized elements before and after the call
// e.g. void ModifyIndices( _Inout_count_(cIndices) int* rgIndeces, size_t cIndices );
#define _Inout_count_(size)              _Prepost_count_(size)
#define _Inout_opt_count_(size)          _Prepost_opt_count_(size)
#define _Inout_bytecount_(size)          _Prepost_bytecount_(size)
#define _Inout_opt_bytecount_(size)      _Prepost_opt_bytecount_(size)

#define _Inout_count_c_(size)            _Prepost_count_c_(size)
#define _Inout_opt_count_c_(size)        _Prepost_opt_count_c_(size)
#define _Inout_bytecount_c_(size)        _Prepost_bytecount_c_(size)
#define _Inout_opt_bytecount_c_(size)    _Prepost_opt_bytecount_c_(size)

// nullterminated 'inout' buffers with initialized elements before and after the call
// e.g. void ModifyIndices( _Inout_count_(cIndices) int* rgIndeces, size_t cIndices );
#define _Inout_z_count_(size)              _Prepost_z_ _Prepost_count_(size)
#define _Inout_opt_z_count_(size)          _Prepost_z_ _Prepost_opt_count_(size)
#define _Inout_z_bytecount_(size)          _Prepost_z_ _Prepost_bytecount_(size)
#define _Inout_opt_z_bytecount_(size)      _Prepost_z_ _Prepost_opt_bytecount_(size)

#define _Inout_z_count_c_(size)            _Prepost_z_ _Prepost_count_c_(size)
#define _Inout_opt_z_count_c_(size)        _Prepost_z_ _Prepost_opt_count_c_(size)
#define _Inout_z_bytecount_c_(size)        _Prepost_z_ _Prepost_bytecount_c_(size)
#define _Inout_opt_z_bytecount_c_(size)    _Prepost_z_ _Prepost_opt_bytecount_c_(size)

#define _Inout_ptrdiff_count_(size)      _Pre_ptrdiff_count_(size)
#define _Inout_opt_ptrdiff_count_(size)  _Pre_opt_ptrdiff_count_(size)

#define _Inout_count_x_(size)            _Prepost_count_x_(size)
#define _Inout_opt_count_x_(size)        _Prepost_opt_count_x_(size)
#define _Inout_bytecount_x_(size)        _Prepost_bytecount_x_(size)
#define _Inout_opt_bytecount_x_(size)    _Prepost_opt_bytecount_x_(size)

// e.g. void AppendToLPSTR( _In_ LPCSTR szFrom, _Inout_cap_(cchTo) LPSTR* szTo, size_t cchTo );
#define _Inout_cap_(size)                _Pre_valid_cap_(size)           _Post_valid_
#define _Inout_opt_cap_(size)            _Pre_opt_valid_cap_(size)       _Post_valid_
#define _Inout_bytecap_(size)            _Pre_valid_bytecap_(size)       _Post_valid_
#define _Inout_opt_bytecap_(size)        _Pre_opt_valid_bytecap_(size)   _Post_valid_

#define _Inout_cap_c_(size)              _Pre_valid_cap_c_(size)         _Post_valid_
#define _Inout_opt_cap_c_(size)          _Pre_opt_valid_cap_c_(size)     _Post_valid_
#define _Inout_bytecap_c_(size)          _Pre_valid_bytecap_c_(size)     _Post_valid_
#define _Inout_opt_bytecap_c_(size)      _Pre_opt_valid_bytecap_c_(size) _Post_valid_

#define _Inout_cap_x_(size)              _Pre_valid_cap_x_(size)         _Post_valid_
#define _Inout_opt_cap_x_(size)          _Pre_opt_valid_cap_x_(size)     _Post_valid_
#define _Inout_bytecap_x_(size)          _Pre_valid_bytecap_x_(size)     _Post_valid_
#define _Inout_opt_bytecap_x_(size)      _Pre_opt_valid_bytecap_x_(size) _Post_valid_

// inout string buffers with writable size
// e.g. void AppendStr( _In_z_ const char* szFrom, _Inout_z_cap_(cchTo) char* szTo, size_t cchTo );
#define _Inout_z_cap_(size)                 _Pre_z_cap_(size)            _Post_z_
#define _Inout_opt_z_cap_(size)             _Pre_opt_z_cap_(size)        _Post_z_
#define _Inout_z_bytecap_(size)             _Pre_z_bytecap_(size)        _Post_z_
#define _Inout_opt_z_bytecap_(size)         _Pre_opt_z_bytecap_(size)    _Post_z_

#define _Inout_z_cap_c_(size)               _Pre_z_cap_c_(size)          _Post_z_
#define _Inout_opt_z_cap_c_(size)           _Pre_opt_z_cap_c_(size)      _Post_z_
#define _Inout_z_bytecap_c_(size)           _Pre_z_bytecap_c_(size)      _Post_z_
#define _Inout_opt_z_bytecap_c_(size)       _Pre_opt_z_bytecap_c_(size)  _Post_z_

#define _Inout_z_cap_x_(size)               _Pre_z_cap_x_(size)          _Post_z_
#define _Inout_opt_z_cap_x_(size)           _Pre_opt_z_cap_x_(size)      _Post_z_
#define _Inout_z_bytecap_x_(size)           _Pre_z_bytecap_x_(size)      _Post_z_
#define _Inout_opt_z_bytecap_x_(size)       _Pre_opt_z_bytecap_x_(size)  _Post_z_

// return values -------------------------------

// returning pointers to valid objects
#define _Ret_                  _Ret_valid_
#define _Ret_opt_              _Ret_opt_valid_

// More _Ret_ annotations are defined below

// Pointer to pointers -------------------------

// e.g.  HRESULT HrCreatePoint( _Deref_out_opt_ POINT** ppPT );
#define _Deref_out_            _Out_ _Deref_pre_invalid_ _Deref_post_valid_
#define _Deref_out_opt_        _Out_ _Deref_pre_invalid_ _Deref_post_opt_valid_
#define _Deref_opt_out_        _Out_opt_ _Deref_pre_invalid_ _Deref_post_valid_
#define _Deref_opt_out_opt_    _Out_opt_ _Deref_pre_invalid_ _Deref_post_opt_valid_

// e.g.  void CloneString( _In_z_ const wchar_t* wzFrom, _Deref_out_z_ wchar_t** pWzTo );
#define _Deref_out_z_          _Out_ _Deref_pre_invalid_ _Deref_post_z_
#define _Deref_out_opt_z_      _Out_ _Deref_pre_invalid_ _Deref_post_opt_z_
#define _Deref_opt_out_z_      _Out_opt_ _Deref_pre_invalid_ _Deref_post_z_
#define _Deref_opt_out_opt_z_  _Out_opt_ _Deref_pre_invalid_ _Deref_post_opt_z_

// More _Deref_ annotations are defined below

// Other annotations

// Check the return value of a function e.g. _Check_return_ ErrorCode Foo();
#define _Check_return_          _Check_return_impl_

// e.g. MyPrintF( _Printf_format_string_ const wchar_t* wzFormat, ... );
#define _Printf_format_string_ _Printf_format_string_impl_
#define _Scanf_format_string_  _Scanf_format_string_impl_
#define _Scanf_s_format_string_ _Scanf_s_format_string_impl_
#define _FormatMessage_format_string_

// <expr> indicates whether post conditions apply
#define _Success_(expr)     _Success_impl_(expr)

// annotations to express 'boundedness' of integral value parameter
#define _In_bound_          _In_bound_impl_
#define _Out_bound_         _Out_bound_impl_
#define _Ret_bound_         _Ret_bound_impl_
#define _Deref_in_bound_    _Deref_in_bound_impl_
#define _Deref_out_bound_   _Deref_out_bound_impl_
#define _Deref_inout_bound_ _Deref_in_bound_ _Deref_out_bound_
#define _Deref_ret_bound_   _Deref_ret_bound_impl_

// annotations to express upper and lower bounds of integral value parameter
#define _In_range_(lb,ub)          _In_range_impl_(lb,ub)
#define _Out_range_(lb,ub)         _Out_range_impl_(lb,ub)
#define _Ret_range_(lb,ub)         _Ret_range_impl_(lb,ub)
#define _Deref_in_range_(lb,ub)    _Deref_in_range_impl_(lb,ub)
#define _Deref_out_range_(lb,ub)   _Deref_out_range_impl_(lb,ub)
#define _Deref_ret_range_(lb,ub)   _Deref_ret_range_impl_(lb,ub)

//============================================================================
//   _Pre_\_Post_ Layer:
//============================================================================

//
// _Pre_ annotation ---
//
// describing conditions that must be met before the call of the function

// e.g. int strlen( _Pre_z_ const char* sz );
// buffer is a zero terminated string
#define _Pre_z_                          _Pre2_impl_(_$notnull,  _$zterm) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_z_                      _Pre2_impl_(_$maybenull,_$zterm) _Deref_pre1_impl_(_$valid)

// e.g. void FreeMemory( _Pre_bytecap_(cb) _Post_ptr_invalid_ void* pv, size_t cb );
// buffer capacity described by another parameter
#define _Pre_cap_(size)                  _Pre2_impl_(_$notnull,  _$cap(size))
#define _Pre_opt_cap_(size)              _Pre2_impl_(_$maybenull,_$cap(size))
#define _Pre_bytecap_(size)              _Pre2_impl_(_$notnull,  _$bytecap(size))
#define _Pre_opt_bytecap_(size)          _Pre2_impl_(_$maybenull,_$bytecap(size))

// buffer capacity described by a constant expression
#define _Pre_cap_c_(size)                _Pre2_impl_(_$notnull,  _$cap_c(size))
#define _Pre_opt_cap_c_(size)            _Pre2_impl_(_$maybenull,_$cap_c(size))
#define _Pre_bytecap_c_(size)            _Pre2_impl_(_$notnull,  _$bytecap_c(size))
#define _Pre_opt_bytecap_c_(size)        _Pre2_impl_(_$maybenull,_$bytecap_c(size))

// buffer capacity is described by another parameter multiplied by a constant expression
#define _Pre_cap_m_(mult,size)           _Pre2_impl_(_$notnull,  _$mult(mult,size))
#define _Pre_opt_cap_m_(mult,size)       _Pre2_impl_(_$maybenull,_$mult(mult,size))

// buffer capacity described by size of other buffer, only used by dangerous legacy APIs
// e.g. int strcpy(_Pre_cap_for_(src) char* dst, const char* src);
#define _Pre_cap_for_(param)             _Pre2_impl_(_$notnull,  _$cap_for(param))
#define _Pre_opt_cap_for_(param)         _Pre2_impl_(_$maybenull,_$cap_for(param))

// buffer capacity described by a complex condition
#define _Pre_cap_x_(size)                _Pre2_impl_(_$notnull,  _$cap_x(size))
#define _Pre_opt_cap_x_(size)            _Pre2_impl_(_$maybenull,_$cap_x(size))
#define _Pre_bytecap_x_(size)            _Pre2_impl_(_$notnull,  _$bytecap_x(size))
#define _Pre_opt_bytecap_x_(size)        _Pre2_impl_(_$maybenull,_$bytecap_x(size))

// buffer capacity described by the difference to another pointer parameter
#define _Pre_ptrdiff_cap_(ptr)           _Pre2_impl_(_$notnull,  _$cap_x(__ptrdiff(ptr)))
#define _Pre_opt_ptrdiff_cap_(ptr)       _Pre2_impl_(_$maybenull,_$cap_x(__ptrdiff(ptr)))

// e.g. void AppendStr( _Pre_z_ const char* szFrom, _Pre_z_cap_(cchTo) _Post_z_ char* szTo, size_t cchTo );
#define _Pre_z_cap_(size)                _Pre3_impl_(_$notnull,  _$zterm,_$cap(size))       _Deref_pre1_impl_(_$valid)
#define _Pre_opt_z_cap_(size)            _Pre3_impl_(_$maybenull,_$zterm,_$cap(size))       _Deref_pre1_impl_(_$valid)
#define _Pre_z_bytecap_(size)            _Pre3_impl_(_$notnull,  _$zterm,_$bytecap(size))   _Deref_pre1_impl_(_$valid)
#define _Pre_opt_z_bytecap_(size)        _Pre3_impl_(_$maybenull,_$zterm,_$bytecap(size))   _Deref_pre1_impl_(_$valid)

#define _Pre_z_cap_c_(size)              _Pre3_impl_(_$notnull,  _$zterm,_$cap_c(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_opt_z_cap_c_(size)          _Pre3_impl_(_$maybenull,_$zterm,_$cap_c(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_z_bytecap_c_(size)          _Pre3_impl_(_$notnull,  _$zterm,_$bytecap_c(size)) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_z_bytecap_c_(size)      _Pre3_impl_(_$maybenull,_$zterm,_$bytecap_c(size)) _Deref_pre1_impl_(_$valid)

#define _Pre_z_cap_x_(size)              _Pre3_impl_(_$notnull,  _$zterm,_$cap_x(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_opt_z_cap_x_(size)          _Pre3_impl_(_$maybenull,_$zterm,_$cap_x(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_z_bytecap_x_(size)          _Pre3_impl_(_$notnull,  _$zterm,_$bytecap_x(size)) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_z_bytecap_x_(size)      _Pre3_impl_(_$maybenull,_$zterm,_$bytecap_x(size)) _Deref_pre1_impl_(_$valid)

// known capacity and valid but unknown readable extent
#define _Pre_valid_cap_(size)            _Pre2_impl_(_$notnull,  _$cap(size))       _Deref_pre1_impl_(_$valid)
#define _Pre_opt_valid_cap_(size)        _Pre2_impl_(_$maybenull,_$cap(size))       _Deref_pre1_impl_(_$valid)
#define _Pre_valid_bytecap_(size)        _Pre2_impl_(_$notnull,  _$bytecap(size))   _Deref_pre1_impl_(_$valid)
#define _Pre_opt_valid_bytecap_(size)    _Pre2_impl_(_$maybenull,_$bytecap(size))   _Deref_pre1_impl_(_$valid)

#define _Pre_valid_cap_c_(size)          _Pre2_impl_(_$notnull,  _$cap_c(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_opt_valid_cap_c_(size)      _Pre2_impl_(_$maybenull,_$cap_c(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_valid_bytecap_c_(size)      _Pre2_impl_(_$notnull,  _$bytecap_c(size)) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_valid_bytecap_c_(size)  _Pre2_impl_(_$maybenull,_$bytecap_c(size)) _Deref_pre1_impl_(_$valid)

#define _Pre_valid_cap_x_(size)          _Pre2_impl_(_$notnull,  _$cap_x(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_opt_valid_cap_x_(size)      _Pre2_impl_(_$maybenull,_$cap_x(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_valid_bytecap_x_(size)      _Pre2_impl_(_$notnull,  _$bytecap_x(size)) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_valid_bytecap_x_(size)  _Pre2_impl_(_$maybenull,_$bytecap_x(size)) _Deref_pre1_impl_(_$valid)

// e.g. void AppendCharRange( _Pre_count_(cchFrom) const char* rgFrom, size_t cchFrom, _Out_z_cap_(cchTo) char* szTo, size_t cchTo );
// Valid buffer extent described by another parameter
#define _Pre_count_(size)                _Pre2_impl_(_$notnull,  _$count(size))       _Deref_pre1_impl_(_$valid)
#define _Pre_opt_count_(size)            _Pre2_impl_(_$maybenull,_$count(size))       _Deref_pre1_impl_(_$valid)
#define _Pre_bytecount_(size)            _Pre2_impl_(_$notnull,  _$bytecount(size))   _Deref_pre1_impl_(_$valid)
#define _Pre_opt_bytecount_(size)        _Pre2_impl_(_$maybenull,_$bytecount(size))   _Deref_pre1_impl_(_$valid)

// Valid buffer extent described by a constant expression
#define _Pre_count_c_(size)              _Pre2_impl_(_$notnull,  _$count_c(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_opt_count_c_(size)          _Pre2_impl_(_$maybenull,_$count_c(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_bytecount_c_(size)          _Pre2_impl_(_$notnull,  _$bytecount_c(size)) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_bytecount_c_(size)      _Pre2_impl_(_$maybenull,_$bytecount_c(size)) _Deref_pre1_impl_(_$valid)

// Valid buffer extent described by a complex expression
#define _Pre_count_x_(size)              _Pre2_impl_(_$notnull,  _$count_x(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_opt_count_x_(size)          _Pre2_impl_(_$maybenull,_$count_x(size))     _Deref_pre1_impl_(_$valid)
#define _Pre_bytecount_x_(size)          _Pre2_impl_(_$notnull,  _$bytecount_x(size)) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_bytecount_x_(size)      _Pre2_impl_(_$maybenull,_$bytecount_x(size)) _Deref_pre1_impl_(_$valid)

// Valid buffer extent described by the difference to another pointer parameter
#define _Pre_ptrdiff_count_(ptr)         _Pre2_impl_(_$notnull,  _$count_x(__ptrdiff(ptr))) _Deref_pre1_impl_(_$valid)
#define _Pre_opt_ptrdiff_count_(ptr)     _Pre2_impl_(_$maybenull,_$count_x(__ptrdiff(ptr))) _Deref_pre1_impl_(_$valid)

// valid size unknown or indicated by type (e.g.:LPSTR)
#define _Pre_valid_                      _Pre1_impl_(_$notnull)   _Deref_pre1_impl_(_$valid)
#define _Pre_opt_valid_                  _Pre1_impl_(_$maybenull) _Deref_pre1_impl_(_$valid)

#define _Pre_invalid_                    _Deref_pre1_impl_(_$notvalid)

// used with allocated but not yet initialized objects
#define _Pre_notnull_                    _Pre1_impl_(_$notnull)
#define _Pre_maybenull_                  _Pre1_impl_(_$maybenull)
#define _Pre_null_                       _Pre1_impl_(_$null)

// restrict access rights
#define _Pre_readonly_                   _Pre1_impl_(_$readaccess)
#define _Pre_writeonly_                  _Pre1_impl_(_$writeaccess)
//
// _Post_ annotations ---
//
// describing conditions that hold after the function call

// void CopyStr( _In_z_ const char* szFrom, _Pre_cap_(cch) _Post_z_ char* szFrom, size_t cchFrom );
// buffer will be a zero-terminated string after the call
#define _Post_z_                        _Post1_impl_(_$zterm) _Deref_post1_impl_(_$valid)

// char * strncpy(_Out_cap_(_Count) _Post_maybez_ char * _Dest, _In_z_ const char * _Source, _In_ size_t _Count)
// buffer maybe zero-terminated after the call
#define _Post_maybez_                   _Post1_impl_(_$maybezterm)

// e.g. SIZE_T HeapSize( _In_ HANDLE hHeap, DWORD dwFlags, _Pre_notnull_ _Post_bytecap_(return) LPCVOID lpMem );
#define _Post_cap_(size)                _Post1_impl_(_$cap(size))
#define _Post_bytecap_(size)            _Post1_impl_(_$bytecap(size))

// e.g. int strlen( _In_z_ _Post_count_(return+1) const char* sz );
#define _Post_count_(size)              _Post1_impl_(_$count(size))       _Deref_post1_impl_(_$valid)
#define _Post_bytecount_(size)          _Post1_impl_(_$bytecount(size))   _Deref_post1_impl_(_$valid)
#define _Post_count_c_(size)            _Post1_impl_(_$count_c(size))     _Deref_post1_impl_(_$valid)
#define _Post_bytecount_c_(size)        _Post1_impl_(_$bytecount_c(size)) _Deref_post1_impl_(_$valid)
#define _Post_count_x_(size)            _Post1_impl_(_$count_x(size))     _Deref_post1_impl_(_$valid)
#define _Post_bytecount_x_(size)        _Post1_impl_(_$bytecount_x(size)) _Deref_post1_impl_(_$valid)

// e.g. size_t CopyStr( _In_z_ const char* szFrom, _Pre_cap_(cch) _Post_z_count_(return+1) char* szFrom, size_t cchFrom );
#define _Post_z_count_(size)            _Post2_impl_(_$zterm,_$count(size))       _Deref_post1_impl_(_$valid)
#define _Post_z_bytecount_(size)        _Post2_impl_(_$zterm,_$bytecount(size))   _Deref_post1_impl_(_$valid)
#define _Post_z_count_c_(size)          _Post2_impl_(_$zterm,_$count_c(size))     _Deref_post1_impl_(_$valid)
#define _Post_z_bytecount_c_(size)      _Post2_impl_(_$zterm,_$bytecount_c(size)) _Deref_post1_impl_(_$valid)
#define _Post_z_count_x_(size)          _Post2_impl_(_$zterm,_$count_x(size))     _Deref_post1_impl_(_$valid)
#define _Post_z_bytecount_x_(size)      _Post2_impl_(_$zterm,_$bytecount_x(size)) _Deref_post1_impl_(_$valid)

// e.g. void free( _Post_ptr_invalid_ void* pv );
#define _Post_ptr_invalid_              _Post1_impl_(_$notvalid)

// e.g. HRESULT InitStruct( _Post_valid_ Struct* pobj );
#define _Post_valid_                    _Deref_post1_impl_(_$valid)
#define _Post_invalid_                  _Deref_post1_impl_(_$notvalid)

// e.g. void ThrowExceptionIfNull( _Post_notnull_ const void* pv );
#define _Post_notnull_                  _Post1_impl_(_$notnull)

//
// _Ret_ annotations
//
// describing conditions that hold for return values after the call

// e.g. _Ret_z_ CString::operator const wchar_t*() const throw();
#define _Ret_z_                          _Ret2_impl_(_$notnull,  _$zterm) _Deref_ret1_impl_(_$valid)
#define _Ret_opt_z_                      _Ret2_impl_(_$maybenull,_$zterm) _Deref_ret1_impl_(_$valid)

// e.g. _Ret_opt_bytecap_(cb) void* AllocateMemory( size_t cb );
// Buffer capacity is described by another parameter
#define _Ret_cap_(size)                  _Ret2_impl_(_$notnull,  _$cap(size))
#define _Ret_opt_cap_(size)              _Ret2_impl_(_$maybenull,_$cap(size))
#define _Ret_bytecap_(size)              _Ret2_impl_(_$notnull,  _$bytecap(size))
#define _Ret_opt_bytecap_(size)          _Ret2_impl_(_$maybenull,_$bytecap(size))

// Buffer capacity is described by a constant expression
#define _Ret_cap_c_(size)                _Ret2_impl_(_$notnull,  _$cap_c(size))
#define _Ret_opt_cap_c_(size)            _Ret2_impl_(_$maybenull,_$cap_c(size))
#define _Ret_bytecap_c_(size)            _Ret2_impl_(_$notnull,  _$bytecap_c(size))
#define _Ret_opt_bytecap_c_(size)        _Ret2_impl_(_$maybenull,_$bytecap_c(size))

// Buffer capacity is described by a complex condition
#define _Ret_cap_x_(size)                _Ret2_impl_(_$notnull,  _$cap_x(size))
#define _Ret_opt_cap_x_(size)            _Ret2_impl_(_$maybenull,_$cap_x(size))
#define _Ret_bytecap_x_(size)            _Ret2_impl_(_$notnull,  _$bytecap_x(size))
#define _Ret_opt_bytecap_x_(size)        _Ret2_impl_(_$maybenull,_$bytecap_x(size))

// return value is nullterminated and capacity is given by another parameter
#define _Ret_z_cap_(size)                _Ret3_impl_(_$notnull,  _$zterm,_$cap(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_opt_z_cap_(size)            _Ret3_impl_(_$maybenull,_$zterm,_$cap(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_z_bytecap_(size)            _Ret3_impl_(_$notnull,  _$zterm,_$bytecap(size)) _Deref_ret1_impl_(_$valid)
#define _Ret_opt_z_bytecap_(size)        _Ret3_impl_(_$maybenull,_$zterm,_$bytecap(size)) _Deref_ret1_impl_(_$valid)

// e.g. _Ret_opt_bytecount_(cb) void* AllocateZeroInitializedMemory( size_t cb );
// Valid Buffer extent is described by another parameter
#define _Ret_count_(size)                _Ret2_impl_(_$notnull,  _$count(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_opt_count_(size)            _Ret2_impl_(_$maybenull,_$count(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_bytecount_(size)            _Ret2_impl_(_$notnull,  _$bytecount(size)) _Deref_ret1_impl_(_$valid)
#define _Ret_opt_bytecount_(size)        _Ret2_impl_(_$maybenull,_$bytecount(size)) _Deref_ret1_impl_(_$valid)

// Valid Buffer extent is described by a constant expression
#define _Ret_count_c_(size)              _Ret2_impl_(_$notnull,  _$count_c(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_opt_count_c_(size)          _Ret2_impl_(_$maybenull,_$count_c(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_bytecount_c_(size)          _Ret2_impl_(_$notnull,  _$bytecount_c(size)) _Deref_ret1_impl_(_$valid)
#define _Ret_opt_bytecount_c_(size)      _Ret2_impl_(_$maybenull,_$bytecount_c(size)) _Deref_ret1_impl_(_$valid)

// Valid Buffer extent is described by a complex expression
#define _Ret_count_x_(size)              _Ret2_impl_(_$notnull,  _$count_x(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_opt_count_x_(size)          _Ret2_impl_(_$maybenull,_$count_x(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_bytecount_x_(size)          _Ret2_impl_(_$notnull,  _$bytecount_x(size)) _Deref_ret1_impl_(_$valid)
#define _Ret_opt_bytecount_x_(size)      _Ret2_impl_(_$maybenull,_$bytecount_x(size)) _Deref_ret1_impl_(_$valid)

// return value is nullterminated and length is given by another parameter
#define _Ret_z_count_(size)              _Ret3_impl_(_$notnull,  _$zterm,_$count(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_opt_z_count_(size)          _Ret3_impl_(_$maybenull,_$zterm,_$count(size))     _Deref_ret1_impl_(_$valid)
#define _Ret_z_bytecount_(size)          _Ret3_impl_(_$notnull,  _$zterm,_$bytecount(size)) _Deref_ret1_impl_(_$valid)
#define _Ret_opt_z_bytecount_(size)      _Ret3_impl_(_$maybenull,_$zterm,_$bytecount(size)) _Deref_ret1_impl_(_$valid)

// e.g. _Ret_opt_valid_ LPSTR void* CloneSTR( _Pre_valid_ LPSTR src );
#define _Ret_valid_                      _Ret1_impl_(_$notnull)   _Deref_ret1_impl_(_$valid)
#define _Ret_opt_valid_                  _Ret1_impl_(_$maybenull) _Deref_ret1_impl_(_$valid)

// used with allocated but not yet initialized objects
#define _Ret_notnull_                    _Ret1_impl_(_$notnull)
#define _Ret_maybenull_                  _Ret1_impl_(_$maybenull)
#define _Ret_null_                       _Ret1_impl_(_$null)

//
// _Deref_pre_ ---
//
// describing conditions for array elements of dereferenced pointer parameters that must be met before the call

// e.g. void SaveStringArray( _In_count_(cStrings) _Deref_pre_z_ const wchar_t* const rgpwch[] );
#define _Deref_pre_z_                          _Deref_pre2_impl_(_$notnull,  _$zterm) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_z_                      _Deref_pre2_impl_(_$maybenull,_$zterm) _Deref2_pre1_impl_(_$valid)

// e.g. void FillInArrayOfStr32( _In_count_(cStrings) _Deref_pre_cap_c_(32) _Deref_post_z_ wchar_t* const rgpwch[] );
// buffer capacity is described by another parameter
#define _Deref_pre_cap_(size)                  _Deref_pre2_impl_(_$notnull,  _$cap(size))
#define _Deref_pre_opt_cap_(size)              _Deref_pre2_impl_(_$maybenull,_$cap(size))
#define _Deref_pre_bytecap_(size)              _Deref_pre2_impl_(_$notnull,  _$bytecap(size))
#define _Deref_pre_opt_bytecap_(size)          _Deref_pre2_impl_(_$maybenull,_$bytecap(size))

// buffer capacity is described by a constant expression
#define _Deref_pre_cap_c_(size)                _Deref_pre2_impl_(_$notnull,  _$cap_c(size))
#define _Deref_pre_opt_cap_c_(size)            _Deref_pre2_impl_(_$maybenull,_$cap_c(size))
#define _Deref_pre_bytecap_c_(size)            _Deref_pre2_impl_(_$notnull,  _$bytecap_c(size))
#define _Deref_pre_opt_bytecap_c_(size)        _Deref_pre2_impl_(_$maybenull,_$bytecap_c(size))

// buffer capacity is described by a complex condition
#define _Deref_pre_cap_x_(size)                _Deref_pre2_impl_(_$notnull,  _$cap_x(size))
#define _Deref_pre_opt_cap_x_(size)            _Deref_pre2_impl_(_$maybenull,_$cap_x(size))
#define _Deref_pre_bytecap_x_(size)            _Deref_pre2_impl_(_$notnull,  _$bytecap_x(size))
#define _Deref_pre_opt_bytecap_x_(size)        _Deref_pre2_impl_(_$maybenull,_$bytecap_x(size))

// convenience macros for nullterminated buffers with given capacity
#define _Deref_pre_z_cap_(size)                _Deref_pre3_impl_(_$notnull,  _$zterm,_$cap(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_z_cap_(size)            _Deref_pre3_impl_(_$maybenull,_$zterm,_$cap(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_z_bytecap_(size)            _Deref_pre3_impl_(_$notnull,  _$zterm,_$bytecap(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_z_bytecap_(size)        _Deref_pre3_impl_(_$maybenull,_$zterm,_$bytecap(size)) _Deref2_pre1_impl_(_$valid)

#define _Deref_pre_z_cap_c_(size)              _Deref_pre3_impl_(_$notnull,  _$zterm,_$cap_c(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_z_cap_c_(size)          _Deref_pre3_impl_(_$maybenull,_$zterm,_$cap_c(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_z_bytecap_c_(size)          _Deref_pre3_impl_(_$notnull,  _$zterm,_$bytecap_c(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_z_bytecap_c_(size)      _Deref_pre3_impl_(_$maybenull,_$zterm,_$bytecap_c(size)) _Deref2_pre1_impl_(_$valid)

#define _Deref_pre_z_cap_x_(size)              _Deref_pre3_impl_(_$notnull,  _$zterm,_$cap_x(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_z_cap_x_(size)          _Deref_pre3_impl_(_$maybenull,_$zterm,_$cap_x(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_z_bytecap_x_(size)          _Deref_pre3_impl_(_$notnull,  _$zterm,_$bytecap_x(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_z_bytecap_x_(size)      _Deref_pre3_impl_(_$maybenull,_$zterm,_$bytecap_x(size)) _Deref2_pre1_impl_(_$valid)

// known capacity and valid but unknown readable extent
#define _Deref_pre_valid_cap_(size)            _Deref_pre2_impl_(_$notnull,  _$cap(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_valid_cap_(size)        _Deref_pre2_impl_(_$maybenull,_$cap(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_valid_bytecap_(size)        _Deref_pre2_impl_(_$notnull,  _$bytecap(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_valid_bytecap_(size)    _Deref_pre2_impl_(_$maybenull,_$bytecap(size)) _Deref2_pre1_impl_(_$valid)

#define _Deref_pre_valid_cap_c_(size)          _Deref_pre2_impl_(_$notnull,  _$cap_c(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_valid_cap_c_(size)      _Deref_pre2_impl_(_$maybenull,_$cap_c(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_valid_bytecap_c_(size)      _Deref_pre2_impl_(_$notnull,  _$bytecap_c(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_valid_bytecap_c_(size)  _Deref_pre2_impl_(_$maybenull,_$bytecap_c(size)) _Deref2_pre1_impl_(_$valid)

#define _Deref_pre_valid_cap_x_(size)          _Deref_pre2_impl_(_$notnull,  _$cap_x(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_valid_cap_x_(size)      _Deref_pre2_impl_(_$maybenull,_$cap_x(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_valid_bytecap_x_(size)      _Deref_pre2_impl_(_$notnull,  _$bytecap_x(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_valid_bytecap_x_(size)  _Deref_pre2_impl_(_$maybenull,_$bytecap_x(size)) _Deref2_pre1_impl_(_$valid)

// e.g. void SaveMatrix( _In_count_(n) _Deref_pre_count_(n) const Elem** matrix, size_t n ); 
// valid buffer extent is described by another parameter
#define _Deref_pre_count_(size)                _Deref_pre2_impl_(_$notnull,  _$count(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_count_(size)            _Deref_pre2_impl_(_$maybenull,_$count(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_bytecount_(size)            _Deref_pre2_impl_(_$notnull,  _$bytecount(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_bytecount_(size)        _Deref_pre2_impl_(_$maybenull,_$bytecount(size)) _Deref2_pre1_impl_(_$valid)

// valid buffer extent is described by a constant expression
#define _Deref_pre_count_c_(size)              _Deref_pre2_impl_(_$notnull,  _$count_c(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_count_c_(size)          _Deref_pre2_impl_(_$maybenull,_$count_c(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_bytecount_c_(size)          _Deref_pre2_impl_(_$notnull,  _$bytecount_c(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_bytecount_c_(size)      _Deref_pre2_impl_(_$maybenull,_$bytecount_c(size)) _Deref2_pre1_impl_(_$valid)

// valid buffer extent is described by a complex expression
#define _Deref_pre_count_x_(size)              _Deref_pre2_impl_(_$notnull,  _$count_x(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_count_x_(size)          _Deref_pre2_impl_(_$maybenull,_$count_x(size))     _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_bytecount_x_(size)          _Deref_pre2_impl_(_$notnull,  _$bytecount_x(size)) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_bytecount_x_(size)      _Deref_pre2_impl_(_$maybenull,_$bytecount_x(size)) _Deref2_pre1_impl_(_$valid)

// e.g. void PrintStringArray( _In_count_(cElems) _Deref_pre_valid_ LPCSTR rgStr[], size_t cElems );
#define _Deref_pre_valid_                      _Deref_pre1_impl_(_$notnull)   _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_opt_valid_                  _Deref_pre1_impl_(_$maybenull) _Deref2_pre1_impl_(_$valid)
#define _Deref_pre_invalid_                    _Deref2_pre1_impl_(_$notvalid)

#define _Deref_pre_notnull_                    _Deref_pre1_impl_(_$notnull)
#define _Deref_pre_maybenull_                  _Deref_pre1_impl_(_$maybenull)
#define _Deref_pre_null_                       _Deref_pre1_impl_(_$null)

// restrict access rights
#define _Deref_pre_readonly_                   _Deref_pre1_impl_(_$readaccess)
#define _Deref_pre_writeonly_                  _Deref_pre1_impl_(_$writeaccess)

//
// _Deref_post_ ---
//
// describing conditions for array elements or dereferenced pointer parameters that hold after the call

// e.g. void CloneString( _In_z_ const Wchar_t* wzIn _Out_ _Deref_post_z_ wchar_t** pWzOut );
#define _Deref_post_z_                          _Deref_post2_impl_(_$notnull,  _$zterm) _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_z_                      _Deref_post2_impl_(_$maybenull,_$zterm) _Deref2_post1_impl_(_$valid)

// e.g. HRESULT HrAllocateMemory( size_t cb, _Out_ _Deref_post_bytecap_(cb) void** ppv );
// buffer capacity is described by another parameter
#define _Deref_post_cap_(size)                  _Deref_post2_impl_(_$notnull,  _$cap(size))
#define _Deref_post_opt_cap_(size)              _Deref_post2_impl_(_$maybenull,_$cap(size))
#define _Deref_post_bytecap_(size)              _Deref_post2_impl_(_$notnull,  _$bytecap(size))
#define _Deref_post_opt_bytecap_(size)          _Deref_post2_impl_(_$maybenull,_$bytecap(size))

// buffer capacity is described by a constant expression
#define _Deref_post_cap_c_(size)                _Deref_post2_impl_(_$notnull,  _$cap_z(size))
#define _Deref_post_opt_cap_c_(size)            _Deref_post2_impl_(_$maybenull,_$cap_z(size))
#define _Deref_post_bytecap_c_(size)            _Deref_post2_impl_(_$notnull,  _$bytecap_z(size))
#define _Deref_post_opt_bytecap_c_(size)        _Deref_post2_impl_(_$maybenull,_$bytecap_z(size))

// buffer capacity is described by a complex expression
#define _Deref_post_cap_x_(size)                _Deref_post2_impl_(_$notnull,  _$cap_x(size))
#define _Deref_post_opt_cap_x_(size)            _Deref_post2_impl_(_$maybenull,_$cap_x(size))
#define _Deref_post_bytecap_x_(size)            _Deref_post2_impl_(_$notnull,  _$bytecap_x(size))
#define _Deref_post_opt_bytecap_x_(size)        _Deref_post2_impl_(_$maybenull,_$bytecap_x(size))

// convenience macros for nullterminated buffers with given capacity
#define _Deref_post_z_cap_(size)                _Deref_post3_impl_(_$notnull,  _$zterm,_$cap(size))       _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_z_cap_(size)            _Deref_post3_impl_(_$maybenull,_$zterm,_$cap(size))       _Deref2_post1_impl_(_$valid)
#define _Deref_post_z_bytecap_(size)            _Deref_post3_impl_(_$notnull,  _$zterm,_$bytecap(size))   _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_z_bytecap_(size)        _Deref_post3_impl_(_$maybenull,_$zterm,_$bytecap(size))   _Deref2_post1_impl_(_$valid)

#define _Deref_post_z_cap_c_(size)              _Deref_post3_impl_(_$notnull,  _$zterm,_$cap_c(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_z_cap_c_(size)          _Deref_post3_impl_(_$maybenull,_$zterm,_$cap_c(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_z_bytecap_c_(size)          _Deref_post3_impl_(_$notnull,  _$zterm,_$bytecap_c(size)) _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_z_bytecap_c_(size)      _Deref_post3_impl_(_$maybenull,_$zterm,_$bytecap_c(size)) _Deref2_post1_impl_(_$valid)

#define _Deref_post_z_cap_x_(size)              _Deref_post3_impl_(_$notnull,  _$zterm,_$cap_x(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_z_cap_x_(size)          _Deref_post3_impl_(_$maybenull,_$zterm,_$cap_x(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_z_bytecap_x_(size)          _Deref_post3_impl_(_$notnull,  _$zterm,_$bytecap_x(size)) _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_z_bytecap_x_(size)      _Deref_post3_impl_(_$maybenull,_$zterm,_$bytecap_x(size)) _Deref2_post1_impl_(_$valid)

// known capacity and valid but unknown readable extent
#define _Deref_post_valid_cap_(size)            _Deref_post2_impl_(_$notnull,  _$cap(size))       _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_valid_cap_(size)        _Deref_post2_impl_(_$maybenull,_$cap(size))       _Deref2_post1_impl_(_$valid)
#define _Deref_post_valid_bytecap_(size)        _Deref_post2_impl_(_$notnull,  _$bytecap(size))   _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_valid_bytecap_(size)    _Deref_post2_impl_(_$maybenull,_$bytecap(size))   _Deref2_post1_impl_(_$valid)
                                                
#define _Deref_post_valid_cap_c_(size)          _Deref_post2_impl_(_$notnull,  _$cap_c(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_valid_cap_c_(size)      _Deref_post2_impl_(_$maybenull,_$cap_c(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_valid_bytecap_c_(size)      _Deref_post2_impl_(_$notnull,  _$bytecap_c(size)) _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_valid_bytecap_c_(size)  _Deref_post2_impl_(_$maybenull,_$bytecap_c(size)) _Deref2_post1_impl_(_$valid)
                                                
#define _Deref_post_valid_cap_x_(size)          _Deref_post2_impl_(_$notnull,  _$cap_x(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_valid_cap_x_(size)      _Deref_post2_impl_(_$maybenull,_$cap_x(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_valid_bytecap_x_(size)      _Deref_post2_impl_(_$notnull,  _$bytecap_x(size)) _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_valid_bytecap_x_(size)  _Deref_post2_impl_(_$maybenull,_$bytecap_x(size)) _Deref2_post1_impl_(_$valid)

// e.g. HRESULT HrAllocateZeroInitializedMemory( size_t cb, _Out_ _Deref_post_bytecount_(cb) void** ppv );
// valid buffer extent is described by another parameter
#define _Deref_post_count_(size)                _Deref_post2_impl_(_$notnull,  _$count(size))       _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_count_(size)            _Deref_post2_impl_(_$maybenull,_$count(size))       _Deref2_post1_impl_(_$valid)
#define _Deref_post_bytecount_(size)            _Deref_post2_impl_(_$notnull,  _$bytecount(size))   _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_bytecount_(size)        _Deref_post2_impl_(_$maybenull,_$bytecount(size))   _Deref2_post1_impl_(_$valid)

// buffer capacity is described by a constant expression
#define _Deref_post_count_c_(size)              _Deref_post2_impl_(_$notnull,  _$count_c(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_count_c_(size)          _Deref_post2_impl_(_$maybenull,_$count_c(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_bytecount_c_(size)          _Deref_post2_impl_(_$notnull,  _$bytecount_c(size)) _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_bytecount_c_(size)      _Deref_post2_impl_(_$maybenull,_$bytecount_c(size)) _Deref2_post1_impl_(_$valid)

// buffer capacity is described by a complex expression
#define _Deref_post_count_x_(size)              _Deref_post2_impl_(_$notnull,  _$count_x(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_count_x_(size)          _Deref_post2_impl_(_$maybenull,_$count_x(size))     _Deref2_post1_impl_(_$valid)
#define _Deref_post_bytecount_x_(size)          _Deref_post2_impl_(_$notnull,  _$bytecount_x(size)) _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_bytecount_x_(size)      _Deref_post2_impl_(_$maybenull,_$bytecount_x(size)) _Deref2_post1_impl_(_$valid)

// e.g. void GetStrings( _Out_count_(cElems) _Deref_post_valid_ LPSTR const rgStr[], size_t cElems );
#define _Deref_post_valid_                      _Deref_post1_impl_(_$notnull)   _Deref2_post1_impl_(_$valid)
#define _Deref_post_opt_valid_                  _Deref_post1_impl_(_$maybenull) _Deref2_post1_impl_(_$valid)

#define _Deref_post_notnull_                    _Deref_post1_impl_(_$notnull)
#define _Deref_post_maybenull_                  _Deref_post1_impl_(_$maybenull)
#define _Deref_post_null_                       _Deref_post1_impl_(_$null)

//
// _Deref_ret_ ---
//

#define _Deref_ret_z_                           _Deref_ret2_impl_(_$notnull,  _$zterm)
#define _Deref_ret_opt_z_                       _Deref_ret2_impl_(_$maybenull,_$zterm)

//
// special _Deref_ ---
//
#define _Deref2_pre_readonly_                   _Deref2_pre1_impl_(_$readaccess)

// Convenience macros for more concise annotations

//
// _Pre_post ---
//
// describing conditions that hold before and after the function call

#define _Prepost_z_                      _Pre_z_      _Post_z_
#define _Prepost_opt_z_                  _Pre_opt_z_  _Post_z_

#define _Prepost_count_(size)           _Pre_count_(size)           _Post_count_(size)
#define _Prepost_opt_count_(size)       _Pre_opt_count_(size)       _Post_count_(size)
#define _Prepost_bytecount_(size)       _Pre_bytecount_(size)       _Post_bytecount_(size)
#define _Prepost_opt_bytecount_(size)   _Pre_opt_bytecount_(size)   _Post_bytecount_(size)
#define _Prepost_count_c_(size)         _Pre_count_c_(size)         _Post_count_c_(size)
#define _Prepost_opt_count_c_(size)     _Pre_opt_count_c_(size)     _Post_count_c_(size)
#define _Prepost_bytecount_c_(size)     _Pre_bytecount_c_(size)     _Post_bytecount_c_(size)
#define _Prepost_opt_bytecount_c_(size) _Pre_opt_bytecount_c_(size) _Post_bytecount_c_(size)
#define _Prepost_count_x_(size)         _Pre_count_x_(size)         _Post_count_x_(size)
#define _Prepost_opt_count_x_(size)     _Pre_opt_count_x_(size)     _Post_count_x_(size)
#define _Prepost_bytecount_x_(size)     _Pre_bytecount_x_(size)     _Post_bytecount_x_(size)
#define _Prepost_opt_bytecount_x_(size) _Pre_opt_bytecount_x_(size) _Post_bytecount_x_(size)

#define _Prepost_valid_                  _Pre_valid_     _Post_valid_
#define _Prepost_opt_valid_              _Pre_opt_valid_ _Post_valid_

//
// _Deref_<both> ---
//
// short version for _Deref_pre_<ann> _Deref_post_<ann>
// describing conditions for array elements or dereferenced pointer parameters that hold before and after the call

#define _Deref_prepost_z_                        _Deref_pre_z_      _Deref_post_z_
#define _Deref_prepost_opt_z_                    _Deref_pre_opt_z_  _Deref_post_opt_z_

#define _Deref_prepost_cap_(size)                _Deref_pre_cap_(size)                _Deref_post_cap_(size)
#define _Deref_prepost_opt_cap_(size)            _Deref_pre_opt_cap_(size)            _Deref_post_opt_cap_(size)
#define _Deref_prepost_bytecap_(size)            _Deref_pre_bytecap_(size)            _Deref_post_bytecap_(size)
#define _Deref_prepost_opt_bytecap_(size)        _Deref_pre_opt_bytecap_(size)        _Deref_post_opt_bytecap_(size)

#define _Deref_prepost_cap_x_(size)              _Deref_pre_cap_x_(size)              _Deref_post_cap_x_(size)             
#define _Deref_prepost_opt_cap_x_(size)          _Deref_pre_opt_cap_x_(size)          _Deref_post_opt_cap_x_(size)         
#define _Deref_prepost_bytecap_x_(size)          _Deref_pre_bytecap_x_(size)          _Deref_post_bytecap_x_(size)             
#define _Deref_prepost_opt_bytecap_x_(size)      _Deref_pre_opt_bytecap_x_(size)      _Deref_post_opt_bytecap_x_(size)         

#define _Deref_prepost_z_cap_(size)              _Deref_pre_z_cap_(size)              _Deref_post_z_cap_(size)             
#define _Deref_prepost_opt_z_cap_(size)          _Deref_pre_opt_z_cap_(size)          _Deref_post_opt_z_cap_(size)         
#define _Deref_prepost_z_bytecap_(size)          _Deref_pre_z_bytecap_(size)          _Deref_post_z_bytecap_(size)         
#define _Deref_prepost_opt_z_bytecap_(size)      _Deref_pre_opt_z_bytecap_(size)      _Deref_post_opt_z_bytecap_(size)     

#define _Deref_prepost_valid_cap_(size)          _Deref_pre_valid_cap_(size)          _Deref_post_valid_cap_(size)             
#define _Deref_prepost_opt_valid_cap_(size)      _Deref_pre_opt_valid_cap_(size)      _Deref_post_opt_valid_cap_(size)         
#define _Deref_prepost_valid_bytecap_(size)      _Deref_pre_valid_bytecap_(size)      _Deref_post_valid_bytecap_(size)         
#define _Deref_prepost_opt_valid_bytecap_(size)  _Deref_pre_opt_valid_bytecap_(size)  _Deref_post_opt_valid_bytecap_(size)     

#define _Deref_prepost_valid_cap_x_(size)          _Deref_pre_valid_cap_x_(size)          _Deref_post_valid_cap_x_(size)             
#define _Deref_prepost_opt_valid_cap_x_(size)      _Deref_pre_opt_valid_cap_x_(size)      _Deref_post_opt_valid_cap_x_(size)         
#define _Deref_prepost_valid_bytecap_x_(size)      _Deref_pre_valid_bytecap_x_(size)      _Deref_post_valid_bytecap_x_(size)         
#define _Deref_prepost_opt_valid_bytecap_x_(size)  _Deref_pre_opt_valid_bytecap_x_(size)  _Deref_post_opt_valid_bytecap_x_(size)     

#define _Deref_prepost_count_(size)            _Deref_pre_count_(size)            _Deref_post_count_(size)
#define _Deref_prepost_opt_count_(size)        _Deref_pre_opt_count_(size)        _Deref_post_opt_count_(size)
#define _Deref_prepost_bytecount_(size)        _Deref_pre_bytecount_(size)        _Deref_post_bytecount_(size)
#define _Deref_prepost_opt_bytecount_(size)    _Deref_pre_opt_bytecount_(size)    _Deref_post_opt_bytecount_(size)

#define _Deref_prepost_count_x_(size)          _Deref_pre_count_x_(size)          _Deref_post_count_x_(size)
#define _Deref_prepost_opt_count_x_(size)      _Deref_pre_opt_count_x_(size)      _Deref_post_opt_count_x_(size)
#define _Deref_prepost_bytecount_x_(size)      _Deref_pre_bytecount_x_(size)      _Deref_post_bytecount_x_(size)
#define _Deref_prepost_opt_bytecount_x_(size)  _Deref_pre_opt_bytecount_x_(size)  _Deref_post_opt_bytecount_x_(size)

#define _Deref_prepost_valid_                   _Deref_pre_valid_     _Deref_post_valid_
#define _Deref_prepost_opt_valid_               _Deref_pre_opt_valid_ _Deref_post_opt_valid_

//
// _Deref_<miscellaneous>
//
// used with references to arrays

#define _Deref_out_z_cap_c_(size) _Deref_pre_cap_c_(size) _Deref_pre_invalid_ _Deref_post_z_
#define _Deref_inout_z_cap_c_(size) _Deref_pre_z_cap_c_(size) _Deref_post_z_
#define _Deref_out_z_bytecap_c_(size) _Deref_pre_bytecap_c_(size) _Deref_pre_invalid_ _Deref_post_z_
#define _Deref_inout_z_bytecap_c_(size) _Deref_pre_z_bytecap_c_(size) _Deref_post_z_
#define _Deref_inout_z_ _Deref_prepost_z_

//============================================================================
//   Implementation Layer:
//============================================================================

#if _USE_ATTRIBUTES_FOR_SAL

#define _Check_return_impl_ [returnvalue:SA_Post(MustCheck=SA_Yes)]

#define _Success_impl_(expr) [SA_Success(Condition=#expr)]

#define _Printf_format_string_impl_   [SA_FormatString(Style="printf")]
#define _Scanf_format_string_impl_    [SA_FormatString(Style="scanf")]
#define _Scanf_s_format_string_impl_  [SA_FormatString(Style="scanf_s")]

#define _In_bound_impl_           [SA_PreBound(Deref=0)]
#define _Out_bound_impl_          [SA_PostBound(Deref=0)]
#define _Ret_bound_impl_          [returnvalue:SA_PostBound(Deref=0)]
#define _Deref_in_bound_impl_     [SA_PreBound(Deref=1)]
#define _Deref_out_bound_impl_    [SA_PostBound(Deref=1)]
#define _Deref_ret_bound_impl_    [returnvalue:SA_PostBound(Deref=1)]

#define _In_range_impl_(min,max)        [SA_PreRange(MinVal=#min,MaxVal=#max)]
#define _Out_range_impl_(min,max)       [SA_PostRange(MinVal=#min,MaxVal=#max)]
#define _Ret_range_impl_(min,max)       [returnvalue:SA_PostRange(MinVal=#min,MaxVal=#max)]
#define _Deref_in_range_impl_(min,max)  [SA_PreRange(Deref=1,MinVal=#min,MaxVal=#max)]
#define _Deref_out_range_impl_(min,max) [SA_PostRange(Deref=1,MinVal=#min,MaxVal=#max)]
#define _Deref_ret_range_impl_(min,max) [returnvalue:SA_PostRange(Deref=1,MinVal=#min,MaxVal=#max)]

#define _$valid       Valid=SA_Yes
#define _$maybevalid  Valid=SA_Maybe
#define _$notvalid    Valid=SA_No

#define _$null        Null=SA_Yes
#define _$maybenull   Null=SA_Maybe
#define _$notnull     Null=SA_No

#define _$zterm       NullTerminated=SA_Yes
#define _$maybezterm  NullTerminated=SA_Maybe
#define _$notzterm    NullTerminated=SA_No

#define _$readaccess  Access=SA_Read
#define _$writeaccess Access=SA_Write

#define _$cap(size)      WritableElements=#size
#define _$cap_c(size)    WritableElementsConst=size
#define _$cap_for(param) WritableElementsLength=#param
#define _$cap_x(size)    WritableElements="\n@"#size

#define _$bytecap(size)   WritableBytes=#size
#define _$bytecap_c(size) WritableBytesConst=size
#define _$bytecap_x(size) WritableBytes="\n@"#size

#define _$mult(mult,size) ElementSizeConst=mult,_$cap(size)

#define _$count(size)   ValidElements=#size
#define _$count_c(size) ValidElementsConst=size
#define _$count_x(size) ValidElements="\n@"#size

#define _$bytecount(size)   ValidBytes=#size
#define _$bytecount_c(size) ValidBytesConst=size
#define _$bytecount_x(size) ValidBytes="\n@"#size

#define _Pre1_impl_(p1)                    [SA_Pre(p1)]
#define _Pre2_impl_(p1,p2)                 [SA_Pre(p1,p2)]
#define _Pre3_impl_(p1,p2,p3)              [SA_Pre(p1,p2,p3)]

#define _Post1_impl_(p1)                   [SA_Post(p1)]
#define _Post2_impl_(p1,p2)                [SA_Post(p1,p2)]
#define _Post3_impl_(p1,p2,p3)             [SA_Post(p1,p2,p3)]

#define _Ret1_impl_(p1)                    [returnvalue:SA_Post(p1)]
#define _Ret2_impl_(p1,p2)                 [returnvalue:SA_Post(p1,p2)]
#define _Ret3_impl_(p1,p2,p3)              [returnvalue:SA_Post(p1,p2,p3)]

#define _Deref_pre1_impl_(p1)              [SA_Pre(Deref=1,p1)]
#define _Deref_pre2_impl_(p1,p2)           [SA_Pre(Deref=1,p1,p2)]
#define _Deref_pre3_impl_(p1,p2,p3)        [SA_Pre(Deref=1,p1,p2,p3)]

#define _Deref_post1_impl_(p1)             [SA_Post(Deref=1,p1)]
#define _Deref_post2_impl_(p1,p2)          [SA_Post(Deref=1,p1,p2)]
#define _Deref_post3_impl_(p1,p2,p3)       [SA_Post(Deref=1,p1,p2,p3)]

#define _Deref_ret1_impl_(p1)              [returnvalue:SA_Post(Deref=1,p1)]
#define _Deref_ret2_impl_(p1,p2)           [returnvalue:SA_Post(Deref=1,p1,p2)]
#define _Deref_ret3_impl_(p1,p2,p3)        [returnvalue:SA_Post(Deref=1,p1,p2,p3)]

#define _Deref2_pre1_impl_(p1)             [SA_Pre(Deref=2,p1)]
#define _Deref2_post1_impl_(p1)            [SA_Post(Deref=2,p1)]
#define _Deref2_ret1_impl_(p1)             [returnvalue:SA_Post(Deref=2,p1)]

#elif _USE_DECLSPECS_FOR_SAL

#define _$SPECSTRIZE( x ) #x

#define _Check_return_impl_ __declspec("SAL_checkReturn")

#define _Success_impl_(expr) __declspec("SAL_success("_$SPECSTRIZE(expr)")")

#define _Printf_format_string_impl_
#define _Scanf_format_string_impl_
#define _Scanf_s_format_string_impl_

#define _In_bound_impl_           _$pre _$bound
#define _Out_bound_impl_          _$post _$bound
#define _Ret_bound_impl_          _$post _$bound
#define _Deref_in_bound_impl_     _$derefpre _$bound
#define _Deref_out_bound_impl_    _$derefpost _$bound
#define _Deref_ret_bound_impl_    _$derefpost bound

#define _In_range_impl_(min,max)        _$pre _$range(min,max)
#define _Out_range_impl_(min,max)       _$post _$range(min,max)
#define _Ret_range_impl_(min,max)       _$post _$range(min,max)
#define _Deref_in_range_impl_(min,max)  _$derefpre _$range(min,max)
#define _Deref_out_range_impl_(min,max) _$derefpost _$range(min,max)
#define _Deref_ret_range_impl_(min,max) _$derefpost _$range(min,max)

#define _$valid             __declspec("SAL_valid")
#define _$maybevalid        __declspec("SAL_maybevalid")
#define _$notvalid          __declspec("SAL_notvalid")

#define _$null              __declspec("SAL_null")
#define _$maybenull         __declspec("SAL_maybenull")
#define _$notnull           __declspec("SAL_notnull")

#define _$zterm             __declspec("SAL_readableTo(sentinel(0))")
#define _$maybezterm
#define _$notzterm

#define _$readaccess        __declspec("SAL_readonly")
#define _$writeaccess       __declspec("SAL_notreadonly")

#define _$cap(size)         __declspec("SAL_writableTo(elementCount("_$SPECSTRIZE(size)"))")
#define _$cap_c(size)       __declspec("SAL_writableTo(elementCount("_$SPECSTRIZE(size)"))")
#define _$cap_for(param)    __declspec("SAL_writableTo(needsCountFor("_$SPECSTRIZE(param)"))")
#define _$cap_x(size)       __declspec("SAL_writableTo(inexpressibleCount('"_$SPECSTRIZE(size)"'))")

#define _$bytecap(size)     __declspec("SAL_writableTo(byteCount("_$SPECSTRIZE(size)"))")
#define _$bytecap_c(size)   __declspec("SAL_writableTo(byteCount("_$SPECSTRIZE(size)"))")
#define _$bytecap_x(size)   __declspec("SAL_writableTo(inexpressibleCount('"_$SPECSTRIZE(size)"'))")

#define _$mult(mult,size)   __declspec("SAL_writableTo(inexpressibleCount("_$SPECSTRIZE(mult)"*"_$SPECSTRIZE(size)"))")

#define _$count(size)       __declspec("SAL_readableTo(elementCount("_$SPECSTRIZE(size)"))")
#define _$count_c(size)     __declspec("SAL_readableTo(elementCount("_$SPECSTRIZE(size)"))")
#define _$count_x(size)     __declspec("SAL_readableTo(inexpressibleCount('"_$SPECSTRIZE(size)"'))")

#define _$bytecount(size)   __declspec("SAL_readableTo(byteCount("_$SPECSTRIZE(size)"))")
#define _$bytecount_c(size) __declspec("SAL_readableTo(byteCount("_$SPECSTRIZE(size)"))")
#define _$bytecount_x(size) __declspec("SAL_readableTo(inexpressibleCount('"_$SPECSTRIZE(size)"'))")

#define _$pre        __declspec("SAL_pre")
#define _$post       __declspec("SAL_post")
#define _$deref_pre  __declspec("SAL_pre")  __declspec("SAL_deref")
#define _$deref_post __declspec("SAL_post") __declspec("SAL_deref")

#define _$bound          __declspec("SAL_bound")
#define _$range(min,max) __declspec("SAL_range("_$SPECSTRIZE(min)","_$SPECSTRIZE(max)")")

#define _Pre1_impl_(p1)                    _$pre p1
#define _Pre2_impl_(p1,p2)                 _$pre p1 _$pre p2
#define _Pre3_impl_(p1,p2,p3)              _$pre p1 _$pre p2 _$pre p3

#define _Post1_impl_(p1)                   _$post p1
#define _Post2_impl_(p1,p2)                _$post p1 _$post p2
#define _Post3_impl_(p1,p2,p3)             _$post p1 _$post p2 _$post p3

#define _Ret1_impl_(p1)                    _$post p1
#define _Ret2_impl_(p1,p2)                 _$post p1 _$post p2
#define _Ret3_impl_(p1,p2,p3)              _$post p1 _$post p2 _$post p3

#define _Deref_pre1_impl_(p1)              _$deref_pre p1
#define _Deref_pre2_impl_(p1,p2)           _$deref_pre p1 _$deref_pre p2
#define _Deref_pre3_impl_(p1,p2,p3)        _$deref_pre p1 _$deref_pre p2 _$deref_pre p3

#define _Deref_post1_impl_(p1)             _$deref_post p1
#define _Deref_post2_impl_(p1,p2)          _$deref_post p1 _$deref_post p2
#define _Deref_post3_impl_(p1,p2,p3)       _$deref_post p1 _$deref_post p2 _$deref_post p3

#define _Deref_ret1_impl_(p1)              _$deref_post p1
#define _Deref_ret2_impl_(p1,p2)           _$deref_post p1 _$deref_post p2
#define _Deref_ret3_impl_(p1,p2,p3)        _$deref_post p1 _$deref_post p2 _$deref_post p3

#define _Deref2_pre1_impl_(p1)             _$deref_pre __declspec("SAL_deref") p1
#define _Deref2_post1_impl_(p1)            _$deref_post __declspec("SAL_deref") p1
#define _Deref2_ret1_impl_(p1)             _$deref_post __declspec("SAL_deref") p1

#elif defined(_MSC_EXTENSIONS) && !defined( MIDL_PASS ) && !defined(__midl) && !defined(RC_INVOKED) && defined(_PFT_VER) && _MSC_VER >= 1400 

// minimum attribute expansion for foreground build

#pragma push_macro( "SA" )
#pragma push_macro( "REPEATABLE" )

#ifdef __cplusplus
#define SA( id ) id
#define REPEATABLE [repeatable]
#else  // !__cplusplus
#define SA( id ) SA_##id
#define REPEATABLE
#endif  // !__cplusplus

REPEATABLE
[source_annotation_attribute( SA( Parameter ) )]
struct _$P
{
#ifdef __cplusplus
	_$P();
#endif
   int _$d;
};
typedef struct _$P _$P;

REPEATABLE
[source_annotation_attribute( SA( ReturnValue ) )]
struct _$R
{
#ifdef __cplusplus
	_$R();
#endif
   int _$d;
};
typedef struct _$R _$R;

[source_annotation_attribute( SA( Method ) )]
struct _$M
{
#ifdef __cplusplus
	_$M();
#endif
   int _$d;
};
typedef struct _$M _$M;

#pragma pop_macro( "REPEATABLE" )
#pragma pop_macro( "SA" )

#define _Check_return_impl_ [returnvalue:_$R(_$d=0)]

#define _Success_impl_(expr) [_$M(_$d=0)]

#define _Printf_format_string_impl_   [_$P(_$d=0)]
#define _Scanf_format_string_impl_    [_$P(_$d=0)]
#define _Scanf_s_format_string_impl_  [_$P(_$d=0)]

#define _In_bound_impl_           [_$P(_$d=0)]
#define _Out_bound_impl_          [_$P(_$d=0)]
#define _Ret_bound_impl_          [returnvalue:_$R(_$d=0)]
#define _Deref_in_bound_impl_     [_$P(_$d=0)]
#define _Deref_out_bound_impl_    [_$P(_$d=0)]
#define _Deref_ret_bound_impl_    [returnvalue:_$R(_$d=0)]

#define _In_range_impl_(min,max)        [_$P(_$d=0)]
#define _Out_range_impl_(min,max)       [_$P(_$d=0)]
#define _Ret_range_impl_(min,max)       [returnvalue:_$R(_$d=0)]
#define _Deref_in_range_impl_(min,max)  [_$P(_$d=0)]
#define _Deref_out_range_impl_(min,max) [_$P(_$d=0)]
#define _Deref_ret_range_impl_(min,max) [returnvalue:_$R(_$d=0)]

#define _Pre1_impl_(p1)          [_$P(_$d=0)]
#define _Pre2_impl_(p1,p2)       [_$P(_$d=0)]
#define _Pre3_impl_(p1,p2,p3)    [_$P(_$d=0)]

#define _Post1_impl_(p1)         [_$P(_$d=0)]
#define _Post2_impl_(p1,p2)      [_$P(_$d=0)]
#define _Post3_impl_(p1,p2,p3)   [_$P(_$d=0)]

#define _Ret1_impl_(p1)          [returnvalue:_$R(_$d=0)]
#define _Ret2_impl_(p1,p2)       [returnvalue:_$R(_$d=0)]
#define _Ret3_impl_(p1,p2,p3)    [returnvalue:_$R(_$d=0)]

#define _Deref_pre1_impl_(p1)        [_$P(_$d=0)]
#define _Deref_pre2_impl_(p1,p2)     [_$P(_$d=0)]
#define _Deref_pre3_impl_(p1,p2,p3)  [_$P(_$d=0)]

#define _Deref_post1_impl_(p1)       [_$P(_$d=0)]
#define _Deref_post2_impl_(p1,p2)    [_$P(_$d=0)]
#define _Deref_post3_impl_(p1,p2,p3) [_$P(_$d=0)]

#define _Deref_ret1_impl_(p1)        [returnvalue:_$R(_$d=0)]
#define _Deref_ret2_impl_(p1,p2)     [returnvalue:_$R(_$d=0)]
#define _Deref_ret3_impl_(p1,p2,p3)  [returnvalue:_$R(_$d=0)]

#define _Deref2_pre1_impl_(p1)       //[_$P(_$d=0)]
#define _Deref2_post1_impl_(p1)      //[_$P(_$d=0)]
#define _Deref2_ret1_impl_(p1)       //[_$P(_$d=0)]

#else

#define _Check_return_impl_

#define _Success_impl_(expr)

#define _Printf_format_string_impl_
#define _Scanf_format_string_impl_
#define _Scanf_s_format_string_impl_

#define _In_bound_impl_
#define _Out_bound_impl_
#define _Ret_bound_impl_
#define _Deref_in_bound_impl_
#define _Deref_out_bound_impl_
#define _Deref_ret_bound_impl_

#define _In_range_impl_(min,max)
#define _Out_range_impl_(min,max)
#define _Ret_range_impl_(min,max)
#define _Deref_in_range_impl_(min,max)
#define _Deref_out_range_impl_(min,max)
#define _Deref_ret_range_impl_(min,max)

#define _Pre1_impl_(p1)
#define _Pre2_impl_(p1,p2)
#define _Pre3_impl_(p1,p2,p3)

#define _Post1_impl_(p1)       
#define _Post2_impl_(p1,p2)
#define _Post3_impl_(p1,p2,p3)

#define _Ret1_impl_(p1)      
#define _Ret2_impl_(p1,p2)
#define _Ret3_impl_(p1,p2,p3)

#define _Deref_pre1_impl_(p1)       
#define _Deref_pre2_impl_(p1,p2)
#define _Deref_pre3_impl_(p1,p2,p3)

#define _Deref_post1_impl_(p1)
#define _Deref_post2_impl_(p1,p2)
#define _Deref_post3_impl_(p1,p2,p3)

#define _Deref_ret1_impl_(p1)
#define _Deref_ret2_impl_(p1,p2)
#define _Deref_ret3_impl_(p1,p2,p3)

#define _Deref2_pre1_impl_(p1)
#define _Deref2_post1_impl_(p1)
#define _Deref2_ret1_impl_(p1)

#endif

// This section contains the deprecated annotations

/* 
 -------------------------------------------------------------------------------
 Introduction

 sal.h provides a set of annotations to describe how a function uses its
 parameters - the assumptions it makes about them, and the guarantees it makes
 upon finishing.

 Annotations may be placed before either a function parameter's type or its return
 type, and describe the function's behavior regarding the parameter or return value.
 There are two classes of annotations: buffer annotations and advanced annotations.
 Buffer annotations describe how functions use their pointer parameters, and
 advanced annotations either describe complex/unusual buffer behavior, or provide
 additional information about a parameter that is not otherwise expressible.

 -------------------------------------------------------------------------------
 Buffer Annotations

 The most important annotations in sal.h provide a consistent way to annotate
 buffer parameters or return values for a function. Each of these annotations describes
 a single buffer (which could be a string, a fixed-length or variable-length array,
 or just a pointer) that the function interacts with: where it is, how large it is,
 how much is initialized, and what the function does with it.

 The appropriate macro for a given buffer can be constructed using the table below.
 Just pick the appropriate values from each category, and combine them together
 with a leading underscore. Some combinations of values do not make sense as buffer
 annotations. Only meaningful annotations can be added to your code; for a list of
 these, see the buffer annotation definitions section.

 Only a single buffer annotation should be used for each parameter.

 |------------|------------|---------|--------|----------|----------|---------------|
 |   Level    |   Usage    |  Size   | Output | NullTerm | Optional |  Parameters   |
 |------------|------------|---------|--------|----------|----------|---------------|
 | <>         | <>         | <>      | <>     | _z       | <>       | <>            |
 | _deref     | _in        | _ecount | _full  | _nz      | _opt     | (size)        |
 | _deref_opt | _out       | _bcount | _part  |          |          | (size,length) |
 |            | _inout     |         |        |          |          |               |
 |            |            |         |        |          |          |               |
 |------------|------------|---------|--------|----------|----------|---------------|

 Level: Describes the buffer pointer's level of indirection from the parameter or
          return value 'p'.

 <>         : p is the buffer pointer.
 _deref     : *p is the buffer pointer. p must not be NULL.
 _deref_opt : *p may be the buffer pointer. p may be NULL, in which case the rest of
                the annotation is ignored.

 Usage: Describes how the function uses the buffer.

 <>     : The buffer is not accessed. If used on the return value or with _deref, the
            function will provide the buffer, and it will be uninitialized at exit.
            Otherwise, the caller must provide the buffer. This should only be used
            for alloc and free functions.
 _in    : The function will only read from the buffer. The caller must provide the
            buffer and initialize it. Cannot be used with _deref.
 _out   : The function will only write to the buffer. If used on the return value or
            with _deref, the function will provide the buffer and initialize it.
            Otherwise, the caller must provide the buffer, and the function will
            initialize it.
 _inout : The function may freely read from and write to the buffer. The caller must
            provide the buffer and initialize it. If used with _deref, the buffer may
            be reallocated by the function.

 Size: Describes the total size of the buffer. This may be less than the space actually
         allocated for the buffer, in which case it describes the accessible amount.

 <>      : No buffer size is given. If the type specifies the buffer size (such as
             with LPSTR and LPWSTR), that amount is used. Otherwise, the buffer is one
             element long. Must be used with _in, _out, or _inout.
 _ecount : The buffer size is an explicit element count.
 _bcount : The buffer size is an explicit byte count.

 Output: Describes how much of the buffer will be initialized by the function. For
           _inout buffers, this also describes how much is initialized at entry. Omit this
           category for _in buffers; they must be fully initialized by the caller.

 <>    : The type specifies how much is initialized. For instance, a function initializing
           an LPWSTR must NULL-terminate the string.
 _full : The function initializes the entire buffer.
 _part : The function initializes part of the buffer, and explicitly indicates how much.

 NullTerm: States if the present of a '\0' marks the end of valid elements in the buffer.
 _z    : A '\0' indicated the end of the buffer
 _nz	 : The buffer may not be null terminated and a '\0' does not indicate the end of the
          buffer.
 Optional: Describes if the buffer itself is optional.

 <>   : The pointer to the buffer must not be NULL.
 _opt : The pointer to the buffer might be NULL. It will be checked before being dereferenced.

 Parameters: Gives explicit counts for the size and length of the buffer.

 <>            : There is no explicit count. Use when neither _ecount nor _bcount is used.
 (size)        : Only the buffer's total size is given. Use with _ecount or _bcount but not _part.
 (size,length) : The buffer's total size and initialized length are given. Use with _ecount_part
                   and _bcount_part.

 -------------------------------------------------------------------------------
 Buffer Annotation Examples

 LWSTDAPI_(BOOL) StrToIntExA(
     LPCSTR pszString,                    -- No annotation required, const implies __in.
     DWORD dwFlags,
     __out int *piRet                     -- A pointer whose dereference will be filled in.
 );

 void MyPaintingFunction(
     __in HWND hwndControl,               -- An initialized read-only parameter.
     __in_opt HDC hdcOptional,            -- An initialized read-only parameter that might be NULL.
     __inout IPropertyStore *ppsStore     -- An initialized parameter that may be freely used
                                          --   and modified.
 );

 LWSTDAPI_(BOOL) PathCompactPathExA(
     __out_ecount(cchMax) LPSTR pszOut,   -- A string buffer with cch elements that will
                                          --   be NULL terminated on exit.
     LPCSTR pszSrc,                       -- No annotation required, const implies __in.
     UINT cchMax,
     DWORD dwFlags
 );

 HRESULT SHLocalAllocBytes(
     size_t cb,
     __deref_bcount(cb) T **ppv           -- A pointer whose dereference will be set to an
                                          --   uninitialized buffer with cb bytes.
 );

 __inout_bcount_full(cb) : A buffer with cb elements that is fully initialized at
     entry and exit, and may be written to by this function.

 __out_ecount_part(count, *countOut) : A buffer with count elements that will be
     partially initialized by this function. The function indicates how much it
     initialized by setting *countOut.

 -------------------------------------------------------------------------------
 Advanced Annotations

 Advanced annotations describe behavior that is not expressible with the regular
 buffer macros. These may be used either to annotate buffer parameters that involve
 complex or conditional behavior, or to enrich existing annotations with additional
 information.

 __success(expr) f :
     <expr> indicates whether function f succeeded or not. If <expr> is true at exit,
     all the function's guarantees (as given by other annotations) must hold. If <expr>
     is false at exit, the caller should not expect any of the function's guarantees
     to hold. If not used, the function must always satisfy its guarantees. Added
     automatically to functions that indicate success in standard ways, such as by
     returning an HRESULT.

 __nullterminated p :
     Pointer p is a buffer that may be read or written up to and including the first
     NULL character or pointer. May be used on typedefs, which marks valid (properly
     initialized) instances of that type as being NULL-terminated.

 __nullnullterminated p :
     Pointer p is a buffer that may be read or written up to and including the first
     sequence of two NULL characters or pointers. May be used on typedefs, which marks
     valid instances of that type as being double-NULL terminated.

 __reserved v :
     Value v must be 0/NULL, reserved for future use.

 __checkReturn v :
     Return value v must not be ignored by callers of this function.

 __typefix(ctype) v :
     Value v should be treated as an instance of ctype, rather than its declared type.

 __override f :
     Specify C#-style 'override' behaviour for overriding virtual methods.

 __callback f :
     Function f can be used as a function pointer.

 __format_string p :
     Pointer p is a string that contains % markers in the style of printf.

 __blocksOn(resource) f :
     Function f blocks on the resource 'resource'.

 __fallthrough :
     Annotates switch statement labels where fall-through is desired, to distinguish
     from forgotten break statements.

 -------------------------------------------------------------------------------
 Advanced Annotation Examples

 __success(return == TRUE) LWSTDAPI_(BOOL) 
 PathCanonicalizeA(__out_ecount(MAX_PATH) LPSTR pszBuf, LPCSTR pszPath) :
    pszBuf is only guaranteed to be NULL-terminated when TRUE is returned.

 typedef __nullterminated WCHAR* LPWSTR : Initialized LPWSTRs are NULL-terminated strings.

 __out_ecount(cch) __typefix(LPWSTR) void *psz : psz is a buffer parameter which will be
     a NULL-terminated WCHAR string at exit, and which initially contains cch WCHARs.

 -------------------------------------------------------------------------------
*/

#define __specstrings

#ifdef  __cplusplus
#ifndef __nothrow
# define __nothrow __declspec(nothrow)
#endif
extern "C" {
#else
#ifndef __nothrow
# define __nothrow
#endif
#endif  /* #ifdef __cplusplus */


/*
 -------------------------------------------------------------------------------
 Helper Macro Definitions

 These express behavior common to many of the high-level annotations.
 DO NOT USE THESE IN YOUR CODE.
 -------------------------------------------------------------------------------
*/

/*
The helper annotations are only understood by the compiler version used by various
defect detection tools. When the regular compiler is running, they are defined into
nothing, and do not affect the compiled code.
*/

#if !defined(__midl) && defined(_PREFAST_) 

    /*
     In the primitive __declspec("SAL_*") annotations "SAL" stands for Standard
     Annotation Language.  These __declspec("SAL_*") annotations are the
     primitives the compiler understands and all high-level SpecString MACROs
     will decompose into these primivates.
    */

    #define SPECSTRINGIZE( x ) #x

    /*
     __null p
     __notnull p
     __maybenull p
    
     Annotates a pointer p. States that pointer p is null. Commonly used
     in the negated form __notnull or the possibly null form __maybenull.
    */

    #define __null                  __declspec("SAL_null")
    #define __notnull               __declspec("SAL_notnull")
    #define __maybenull             __declspec("SAL_maybenull")

    /*
     __readonly l
     __notreadonly l
     __mabyereadonly l
    
     Annotates a location l. States that location l is not modified after
     this point.  If the annotation is placed on the precondition state of
     a function, the restriction only applies until the postcondition state
     of the function.  __maybereadonly states that the annotated location
     may be modified, whereas __notreadonly states that a location must be
     modified.
    */

    #define __readonly              __declspec("SAL_readonly")
    #define __notreadonly           __declspec("SAL_notreadonly")
    #define __maybereadonly         __declspec("SAL_maybereadonly")

    /*
     __valid v
     __notvalid v
     __maybevalid v
    
     Annotates any value v. States that the value satisfies all properties of
     valid values of its type. For example, for a string buffer, valid means
     that the buffer pointer is either NULL or points to a NULL-terminated string.
    */

    #define __valid                 __declspec("SAL_valid")
    #define __notvalid              __declspec("SAL_notvalid")
    #define __maybevalid            __declspec("SAL_maybevalid")

    /*
     __readableTo(extent) p
    
     Annotates a buffer pointer p.  If the buffer can be read, extent describes
     how much of the buffer is readable. For a reader of the buffer, this is
     an explicit permission to read up to that amount, rather than a restriction to
     read only up to it.
    */

    #define __readableTo(extent)    __declspec("SAL_readableTo("SPECSTRINGIZE(extent)")")

    /*
    
     __elem_readableTo(size)
    
     Annotates a buffer pointer p as being readable to size elements.
    */

    #define __elem_readableTo(size)   __declspec("SAL_readableTo(elementCount("SPECSTRINGIZE(size)"))")
    
    /*
     __byte_readableTo(size)
    
     Annotates a buffer pointer p as being readable to size bytes.
    */
    #define __byte_readableTo(size)   __declspec("SAL_readableTo(byteCount("SPECSTRINGIZE(size)"))")
    
    /*
     __writableTo(extent) p
    
     Annotates a buffer pointer p. If the buffer can be modified, extent
     describes how much of the buffer is writable (usually the allocation
     size). For a writer of the buffer, this is an explicit permission to
     write up to that amount, rather than a restriction to write only up to it.
    */
    #define __writableTo(size)   __declspec("SAL_writableTo("SPECSTRINGIZE(size)")")

    /*
     __elem_writableTo(size)
    
     Annotates a buffer pointer p as being writable to size elements.
    */
    #define __elem_writableTo(size)   __declspec("SAL_writableTo(elementCount("SPECSTRINGIZE(size)"))")
    
    /*
     __byte_writableTo(size)
    
     Annotates a buffer pointer p as being writable to size bytes.
    */
    #define __byte_writableTo(size)   __declspec("SAL_writableTo(byteCount("SPECSTRINGIZE(size)"))")

    /*
     __deref p
    
     Annotates a pointer p. The next annotation applies one dereference down
     in the type. If readableTo(p, size) then the next annotation applies to
     all elements *(p+i) for which i satisfies the size. If p is a pointer
     to a struct, the next annotation applies to all fields of the struct.
    */
    #define __deref                 __declspec("SAL_deref")
    
    /*
     __pre __next_annotation
    
     The next annotation applies in the precondition state
    */
    #define __pre                   __declspec("SAL_pre")
    
    /*
     __post __next_annotation
    
     The next annotation applies in the postcondition state
    */
    #define __post                  __declspec("SAL_post")
    
    /*
     __precond(<expr>)
    
     When <expr> is true, the next annotation applies in the precondition state
     (currently not enabled)
    */
    #define __precond(expr)         __pre

    /*
     __postcond(<expr>)
    
     When <expr> is true, the next annotation applies in the postcondition state
     (currently not enabled)
    */
    #define __postcond(expr)        __post

    /*
     __exceptthat
    
     Given a set of annotations Q containing __exceptthat maybeP, the effect of
     the except clause is to erase any P or notP annotations (explicit or
     implied) within Q at the same level of dereferencing that the except
     clause appears, and to replace it with maybeP.
    
      Example 1: __valid __exceptthat __maybenull on a pointer p means that the
                 pointer may be null, and is otherwise valid, thus overriding
                 the implicit notnull annotation implied by __valid on
                 pointers.
    
      Example 2: __valid __deref __exceptthat __maybenull on an int **p means
                 that p is not null (implied by valid), but the elements
                 pointed to by p could be null, and are otherwise valid. 
    */
    #define __exceptthat                __declspec("SAL_except")
    #define __execeptthat               __exceptthat
 
    /*
     _refparam
    
     Added to all out parameter macros to indicate that they are all reference
     parameters.
    */
    #define __refparam                  __deref __notreadonly

    /*
     __inner_*
    
     Helper macros that directly correspond to certain high-level annotations.
    
    */

    /*
     Macros to classify the entrypoints and indicate their category.
    
     Pre-defined control point categories include: RPC, LPC, DeviceDriver, UserToKernel, ISAPI, COM.
    
    */
    #define __inner_control_entrypoint(category) __declspec("SAL_entrypoint(controlEntry, "SPECSTRINGIZE(category)")")

    /*
     Pre-defined data entry point categories include: Registry, File, Network.
    */
    #define __inner_data_entrypoint(category)    __declspec("SAL_entrypoint(dataEntry, "SPECSTRINGIZE(category)")")

    #define __inner_success(expr)               __declspec("SAL_success("SPECSTRINGIZE(expr)")")
    #define __inner_checkReturn                 __declspec("SAL_checkReturn")
    #define __inner_typefix(ctype)              __declspec("SAL_typefix("SPECSTRINGIZE(ctype)")")
    #define __inner_override                    __declspec("__override")
    #define __inner_callback                    __declspec("__callback")
    #define __inner_blocksOn(resource)          __declspec("SAL_blocksOn("SPECSTRINGIZE(resource)")")
    #define __inner_fallthrough_dec             __inline __nothrow void __FallThrough() {}
    #define __inner_fallthrough                 __FallThrough();

#else
    #define __null
    #define __notnull
    #define __maybenull
    #define __readonly
    #define __notreadonly
    #define __maybereadonly
    #define __valid
    #define __notvalid
    #define __maybevalid
    #define __readableTo(extent)
    #define __elem_readableTo(size)
    #define __byte_readableTo(size)
    #define __writableTo(size)
    #define __elem_writableTo(size)
    #define __byte_writableTo(size)
    #define __deref
    #define __pre
    #define __post
    #define __precond(expr)
    #define __postcond(expr)
    #define __exceptthat
    #define __execeptthat
    #define __inner_success(expr)
    #define __inner_checkReturn
    #define __inner_typefix(ctype)
    #define __inner_override
    #define __inner_callback
    #define __inner_blocksOn(resource)
    #define __inner_fallthrough_dec
    #define __inner_fallthrough
    #define __refparam
    #define __inner_control_entrypoint(category)
    #define __inner_data_entrypoint(category)
#endif /* #if !defined(__midl) && defined(_PREFAST_) */

/* 
-------------------------------------------------------------------------------
Buffer Annotation Definitions

Any of these may be used to directly annotate functions, but only one should
be used for each parameter. To determine which annotation to use for a given
buffer, use the table in the buffer annotations section.
-------------------------------------------------------------------------------
*/

#define __ecount(size)                                          __notnull __elem_writableTo(size)
#define __bcount(size)                                          __notnull __byte_writableTo(size)
#define __in                                                    __pre __valid __pre __deref __readonly
#define __in_ecount(size)                                       __in __pre __elem_readableTo(size)
#define __in_bcount(size)                                       __in __pre __byte_readableTo(size)
#define __in_z                                                  __in __pre __nullterminated
#define __in_ecount_z(size)                                     __in_ecount(size) __pre __nullterminated
#define __in_bcount_z(size)                                     __in_bcount(size) __pre __nullterminated
#define __in_nz                                                 __in
#define __in_ecount_nz(size)                                    __in_ecount(size)
#define __in_bcount_nz(size)                                    __in_bcount(size)
#define __out                                                   __ecount(1) __post __valid __refparam
#define __out_ecount(size)                                      __ecount(size) __post __valid __refparam
#define __out_bcount(size)                                      __bcount(size) __post __valid __refparam
#define __out_ecount_part(size,length)                          __out_ecount(size) __post __elem_readableTo(length)
#define __out_bcount_part(size,length)                          __out_bcount(size) __post __byte_readableTo(length)
#define __out_ecount_full(size)                                 __out_ecount_part(size,size)
#define __out_bcount_full(size)                                 __out_bcount_part(size,size)
#define __out_z                                                 __post __valid __refparam __post __nullterminated
#define __out_z_opt                                             __post __valid __refparam __post __nullterminated __exceptthat __maybenull
#define __out_ecount_z(size)                                    __ecount(size) __post __valid __refparam __post __nullterminated
#define __out_bcount_z(size)                                    __bcount(size) __post __valid __refparam __post __nullterminated
#define __out_ecount_part_z(size,length)                        __out_ecount_part(size,length) __post __nullterminated
#define __out_bcount_part_z(size,length)                        __out_bcount_part(size,length) __post __nullterminated
#define __out_ecount_full_z(size)                               __out_ecount_full(size) __post __nullterminated
#define __out_bcount_full_z(size)                               __out_bcount_full(size) __post __nullterminated
#define __out_nz                                                __post __valid __refparam __post
#define __out_nz_opt                                            __post __valid __refparam __post __exceptthat __maybenull
#define __out_ecount_nz(size)                                   __ecount(size) __post __valid __refparam
#define __out_bcount_nz(size)                                   __bcount(size) __post __valid __refparam
#define __inout                                                 __pre __valid __post __valid __refparam
#define __inout_ecount(size)                                    __out_ecount(size) __pre __valid
#define __inout_bcount(size)                                    __out_bcount(size) __pre __valid
#define __inout_ecount_part(size,length)                        __out_ecount_part(size,length) __pre __valid __pre __elem_readableTo(length)
#define __inout_bcount_part(size,length)                        __out_bcount_part(size,length) __pre __valid __pre __byte_readableTo(length)
#define __inout_ecount_full(size)                               __inout_ecount_part(size,size)
#define __inout_bcount_full(size)                               __inout_bcount_part(size,size)
#define __inout_z                                               __inout __pre __nullterminated __post __nullterminated
#define __inout_ecount_z(size)                                  __inout_ecount(size) __pre __nullterminated __post __nullterminated
#define __inout_bcount_z(size)                                  __inout_bcount(size) __pre __nullterminated __post __nullterminated
#define __inout_nz                                              __inout
#define __inout_ecount_nz(size)                                 __inout_ecount(size) 
#define __inout_bcount_nz(size)                                 __inout_bcount(size) 
#define __ecount_opt(size)                                      __ecount(size)                              __exceptthat __maybenull
#define __bcount_opt(size)                                      __bcount(size)                              __exceptthat __maybenull
#define __in_opt                                                __in                                        __exceptthat __maybenull
#define __in_ecount_opt(size)                                   __in_ecount(size)                           __exceptthat __maybenull
#define __in_bcount_opt(size)                                   __in_bcount(size)                           __exceptthat __maybenull
#define __in_z_opt                                              __in_opt __pre __nullterminated 
#define __in_ecount_z_opt(size)                                 __in_ecount_opt(size) __pre __nullterminated 
#define __in_bcount_z_opt(size)                                 __in_bcount_opt(size) __pre __nullterminated
#define __in_nz_opt                                             __in_opt                                     
#define __in_ecount_nz_opt(size)                                __in_ecount_opt(size)                         
#define __in_bcount_nz_opt(size)                                __in_bcount_opt(size)                         
#define __out_opt                                               __out                                       __exceptthat __maybenull
#define __out_ecount_opt(size)                                  __out_ecount(size)                          __exceptthat __maybenull
#define __out_bcount_opt(size)                                  __out_bcount(size)                          __exceptthat __maybenull
#define __out_ecount_part_opt(size,length)                      __out_ecount_part(size,length)              __exceptthat __maybenull
#define __out_bcount_part_opt(size,length)                      __out_bcount_part(size,length)              __exceptthat __maybenull
#define __out_ecount_full_opt(size)                             __out_ecount_full(size)                     __exceptthat __maybenull
#define __out_bcount_full_opt(size)                             __out_bcount_full(size)                     __exceptthat __maybenull
#define __out_ecount_z_opt(size)                                __out_ecount_opt(size) __post __nullterminated
#define __out_bcount_z_opt(size)                                __out_bcount_opt(size) __post __nullterminated
#define __out_ecount_part_z_opt(size,length)                    __out_ecount_part_opt(size,length) __post __nullterminated
#define __out_bcount_part_z_opt(size,length)                    __out_bcount_part_opt(size,length) __post __nullterminated
#define __out_ecount_full_z_opt(size)                           __out_ecount_full_opt(size) __post __nullterminated
#define __out_bcount_full_z_opt(size)                           __out_bcount_full_opt(size) __post __nullterminated
#define __out_ecount_nz_opt(size)                               __out_ecount_opt(size) __post __nullterminated                       
#define __out_bcount_nz_opt(size)                               __out_bcount_opt(size) __post __nullterminated                        
#define __inout_opt                                             __inout                                     __exceptthat __maybenull
#define __inout_ecount_opt(size)                                __inout_ecount(size)                        __exceptthat __maybenull
#define __inout_bcount_opt(size)                                __inout_bcount(size)                        __exceptthat __maybenull
#define __inout_ecount_part_opt(size,length)                    __inout_ecount_part(size,length)            __exceptthat __maybenull
#define __inout_bcount_part_opt(size,length)                    __inout_bcount_part(size,length)            __exceptthat __maybenull
#define __inout_ecount_full_opt(size)                           __inout_ecount_full(size)                   __exceptthat __maybenull
#define __inout_bcount_full_opt(size)                           __inout_bcount_full(size)                   __exceptthat __maybenull
#define __inout_z_opt                                           __inout_opt __pre __nullterminated __post __nullterminated
#define __inout_ecount_z_opt(size)                              __inout_ecount_opt(size) __pre __nullterminated __post __nullterminated
#define __inout_ecount_z_opt(size)                              __inout_ecount_opt(size) __pre __nullterminated __post __nullterminated
#define __inout_bcount_z_opt(size)                              __inout_bcount_opt(size) 
#define __inout_nz_opt                                          __inout_opt
#define __inout_ecount_nz_opt(size)                             __inout_ecount_opt(size)
#define __inout_bcount_nz_opt(size)                             __inout_bcount_opt(size)
#define __deref_ecount(size)                                    __ecount(1) __post __elem_readableTo(1) __post __deref __notnull __post __deref __elem_writableTo(size)
#define __deref_bcount(size)                                    __ecount(1) __post __elem_readableTo(1) __post __deref __notnull __post __deref __byte_writableTo(size)
#define __deref_out                                             __deref_ecount(1) __post __deref __valid __refparam
#define __deref_out_ecount(size)                                __deref_ecount(size) __post __deref __valid __refparam
#define __deref_out_bcount(size)                                __deref_bcount(size) __post __deref __valid __refparam
#define __deref_out_ecount_part(size,length)                    __deref_out_ecount(size) __post __deref __elem_readableTo(length)
#define __deref_out_bcount_part(size,length)                    __deref_out_bcount(size) __post __deref __byte_readableTo(length)
#define __deref_out_ecount_full(size)                           __deref_out_ecount_part(size,size)
#define __deref_out_bcount_full(size)                           __deref_out_bcount_part(size,size)
#define __deref_out_z                                           __post __deref __valid __refparam __post __deref __nullterminated
#define __deref_out_ecount_z(size)                              __deref_out_ecount(size) __post __deref __nullterminated  
#define __deref_out_bcount_z(size)                              __deref_out_ecount(size) __post __deref __nullterminated  
#define __deref_out_nz                                          __deref_out
#define __deref_out_ecount_nz(size)                             __deref_out_ecount(size)   
#define __deref_out_bcount_nz(size)                             __deref_out_ecount(size)   
#define __deref_inout                                           __notnull __elem_readableTo(1) __pre __deref __valid __post __deref __valid __refparam
#define __deref_inout_z                                         __deref_inout __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_inout_ecount(size)                              __deref_inout __pre __deref __elem_writableTo(size) __post __deref __elem_writableTo(size)
#define __deref_inout_bcount(size)                              __deref_inout __pre __deref __byte_writableTo(size) __post __deref __byte_writableTo(size)
#define __deref_inout_ecount_part(size,length)                  __deref_inout_ecount(size) __pre __deref __elem_readableTo(length) __post __deref __elem_readableTo(length)
#define __deref_inout_bcount_part(size,length)                  __deref_inout_bcount(size) __pre __deref __byte_readableTo(length) __post __deref __byte_readableTo(length)
#define __deref_inout_ecount_full(size)                         __deref_inout_ecount_part(size,size)
#define __deref_inout_bcount_full(size)                         __deref_inout_bcount_part(size,size)
#define __deref_inout_z                                         __deref_inout __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_inout_ecount_z(size)                            __deref_inout_ecount(size) __pre __deref __nullterminated __post __deref __nullterminated   
#define __deref_inout_bcount_z(size)                            __deref_inout_bcount(size) __pre __deref __nullterminated __post __deref __nullterminated  
#define __deref_inout_nz                                        __deref_inout
#define __deref_inout_ecount_nz(size)                           __deref_inout_ecount(size)   
#define __deref_inout_bcount_nz(size)                           __deref_inout_ecount(size)   
#define __deref_ecount_opt(size)                                __deref_ecount(size)                        __post __deref __exceptthat __maybenull
#define __deref_bcount_opt(size)                                __deref_bcount(size)                        __post __deref __exceptthat __maybenull
#define __deref_out_opt                                         __deref_out                                 __post __deref __exceptthat __maybenull
#define __deref_out_ecount_opt(size)                            __deref_out_ecount(size)                    __post __deref __exceptthat __maybenull
#define __deref_out_bcount_opt(size)                            __deref_out_bcount(size)                    __post __deref __exceptthat __maybenull
#define __deref_out_ecount_part_opt(size,length)                __deref_out_ecount_part(size,length)        __post __deref __exceptthat __maybenull
#define __deref_out_bcount_part_opt(size,length)                __deref_out_bcount_part(size,length)        __post __deref __exceptthat __maybenull
#define __deref_out_ecount_full_opt(size)                       __deref_out_ecount_full(size)               __post __deref __exceptthat __maybenull
#define __deref_out_bcount_full_opt(size)                       __deref_out_bcount_full(size)               __post __deref __exceptthat __maybenull
#define __deref_out_z_opt                                       __post __deref __valid __refparam __execeptthat __maybenull __post __deref __nullterminated
#define __deref_out_ecount_z_opt(size)                          __deref_out_ecount_opt(size) __post __deref __nullterminated
#define __deref_out_bcount_z_opt(size)                          __deref_out_bcount_opt(size) __post __deref __nullterminated
#define __deref_out_nz_opt                                      __deref_out_opt
#define __deref_out_ecount_nz_opt(size)                         __deref_out_ecount_opt(size)
#define __deref_out_bcount_nz_opt(size)                         __deref_out_bcount_opt(size)
#define __deref_inout_opt                                       __deref_inout                               __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull
#define __deref_inout_ecount_opt(size)                          __deref_inout_ecount(size)                  __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull
#define __deref_inout_bcount_opt(size)                          __deref_inout_bcount(size)                  __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull
#define __deref_inout_ecount_part_opt(size,length)              __deref_inout_ecount_part(size,length)      __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull
#define __deref_inout_bcount_part_opt(size,length)              __deref_inout_bcount_part(size,length)      __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull
#define __deref_inout_ecount_full_opt(size)                     __deref_inout_ecount_full(size)             __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull
#define __deref_inout_bcount_full_opt(size)                     __deref_inout_bcount_full(size)             __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull
#define __deref_inout_z_opt                                     __deref_inout_opt __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_inout_ecount_z_opt(size)                        __deref_inout_ecount_opt(size) __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_inout_bcount_z_opt(size)                        __deref_inout_bcount_opt(size) __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_inout_nz_opt                                    __deref_inout_opt 
#define __deref_inout_ecount_nz_opt(size)                       __deref_inout_ecount_opt(size)
#define __deref_inout_bcount_nz_opt(size)                       __deref_inout_bcount_opt(size)
#define __deref_opt_ecount(size)                                __deref_ecount(size)                        __exceptthat __maybenull
#define __deref_opt_bcount(size)                                __deref_bcount(size)                        __exceptthat __maybenull
#define __deref_opt_out                                         __deref_out                                 __exceptthat __maybenull
#define __deref_opt_out_z                                       __deref_opt_out __post __deref __nullterminated
#define __deref_opt_out_ecount(size)                            __deref_out_ecount(size)                    __exceptthat __maybenull
#define __deref_opt_out_bcount(size)                            __deref_out_bcount(size)                    __exceptthat __maybenull
#define __deref_opt_out_ecount_part(size,length)                __deref_out_ecount_part(size,length)        __exceptthat __maybenull
#define __deref_opt_out_bcount_part(size,length)                __deref_out_bcount_part(size,length)        __exceptthat __maybenull
#define __deref_opt_out_ecount_full(size)                       __deref_out_ecount_full(size)               __exceptthat __maybenull
#define __deref_opt_out_bcount_full(size)                       __deref_out_bcount_full(size)               __exceptthat __maybenull
#define __deref_opt_inout                                       __deref_inout                               __exceptthat __maybenull
#define __deref_opt_inout_ecount(size)                          __deref_inout_ecount(size)                  __exceptthat __maybenull
#define __deref_opt_inout_bcount(size)                          __deref_inout_bcount(size)                  __exceptthat __maybenull
#define __deref_opt_inout_ecount_part(size,length)              __deref_inout_ecount_part(size,length)      __exceptthat __maybenull
#define __deref_opt_inout_bcount_part(size,length)              __deref_inout_bcount_part(size,length)      __exceptthat __maybenull
#define __deref_opt_inout_ecount_full(size)                     __deref_inout_ecount_full(size)             __exceptthat __maybenull
#define __deref_opt_inout_bcount_full(size)                     __deref_inout_bcount_full(size)             __exceptthat __maybenull
#define __deref_opt_inout_z                                     __deref_opt_inout __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_opt_inout_ecount_z(size)                        __deref_opt_inout_ecount(size) __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_opt_inout_bcount_z(size)                        __deref_opt_inout_bcount(size) __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_opt_inout_nz                                    __deref_opt_inout
#define __deref_opt_inout_ecount_nz(size)                       __deref_opt_inout_ecount(size)
#define __deref_opt_inout_bcount_nz(size)                       __deref_opt_inout_bcount(size)
#define __deref_opt_ecount_opt(size)                            __deref_ecount_opt(size)                    __exceptthat __maybenull
#define __deref_opt_bcount_opt(size)                            __deref_bcount_opt(size)                    __exceptthat __maybenull
#define __deref_opt_out_opt                                     __deref_out_opt                             __exceptthat __maybenull
#define __deref_opt_out_ecount_opt(size)                        __deref_out_ecount_opt(size)                __exceptthat __maybenull
#define __deref_opt_out_bcount_opt(size)                        __deref_out_bcount_opt(size)                __exceptthat __maybenull
#define __deref_opt_out_ecount_part_opt(size,length)            __deref_out_ecount_part_opt(size,length)    __exceptthat __maybenull
#define __deref_opt_out_bcount_part_opt(size,length)            __deref_out_bcount_part_opt(size,length)    __exceptthat __maybenull
#define __deref_opt_out_ecount_full_opt(size)                   __deref_out_ecount_full_opt(size)           __exceptthat __maybenull
#define __deref_opt_out_bcount_full_opt(size)                   __deref_out_bcount_full_opt(size)           __exceptthat __maybenull
#define __deref_opt_out_z_opt                                   __post __deref __valid __refparam __exceptthat __maybenull __pre __deref __exceptthat __maybenull __post __deref __exceptthat __maybenull __post __deref __nullterminated
#define __deref_opt_out_ecount_z_opt(size)                      __deref_opt_out_ecount_opt(size) __post __deref __nullterminated
#define __deref_opt_out_bcount_z_opt(size)                      __deref_opt_out_bcount_opt(size) __post __deref __nullterminated
#define __deref_opt_out_nz_opt                                  __deref_opt_out_opt
#define __deref_opt_out_ecount_nz_opt(size)                     __deref_opt_out_ecount_opt(size)    
#define __deref_opt_out_bcount_nz_opt(size)                     __deref_opt_out_bcount_opt(size)    
#define __deref_opt_inout_opt                                   __deref_inout_opt                           __exceptthat __maybenull
#define __deref_opt_inout_ecount_opt(size)                      __deref_inout_ecount_opt(size)              __exceptthat __maybenull
#define __deref_opt_inout_bcount_opt(size)                      __deref_inout_bcount_opt(size)              __exceptthat __maybenull
#define __deref_opt_inout_ecount_part_opt(size,length)          __deref_inout_ecount_part_opt(size,length)  __exceptthat __maybenull
#define __deref_opt_inout_bcount_part_opt(size,length)          __deref_inout_bcount_part_opt(size,length)  __exceptthat __maybenull
#define __deref_opt_inout_ecount_full_opt(size)                 __deref_inout_ecount_full_opt(size)         __exceptthat __maybenull
#define __deref_opt_inout_bcount_full_opt(size)                 __deref_inout_bcount_full_opt(size)         __exceptthat __maybenull
#define __deref_opt_inout_z_opt                                 __deref_opt_inout_opt  __pre __deref __nullterminated __post __deref __nullterminated             
#define __deref_opt_inout_ecount_z_opt(size)                    __deref_opt_inout_ecount_opt(size)  __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_opt_inout_bcount_z_opt(size)                    __deref_opt_inout_bcount_opt(size)  __pre __deref __nullterminated __post __deref __nullterminated
#define __deref_opt_inout_nz_opt                                __deref_opt_inout_opt               
#define __deref_opt_inout_ecount_nz_opt(size)                   __deref_opt_inout_ecount_opt(size)  
#define __deref_opt_inout_bcount_nz_opt(size)                   __deref_opt_inout_bcount_opt(size)  

/*
-------------------------------------------------------------------------------
Advanced Annotation Definitions

Any of these may be used to directly annotate functions, and may be used in
combination with each other or with regular buffer macros. For an explanation
of each annotation, see the advanced annotations section.
-------------------------------------------------------------------------------
*/

#define __success(expr)                     __inner_success(expr)
#define __nullterminated                    __readableTo(sentinel(0))
#define __nullnullterminated
#define __reserved                          __pre __null
#define __checkReturn                       __inner_checkReturn
#define __typefix(ctype)                    __inner_typefix(ctype)
#define __override                          __inner_override
#define __callback                          __inner_callback
#define __format_string
#define __blocksOn(resource)                __inner_blocksOn(resource)
#define __control_entrypoint(category)      __inner_control_entrypoint(category)
#define __data_entrypoint(category)         __inner_data_entrypoint(category)

#ifndef __fallthrough
    __inner_fallthrough_dec
    #define __fallthrough __inner_fallthrough
#endif

#ifndef __analysis_assume
#ifdef _PREFAST_
#define __analysis_assume(expr) __assume(expr)
#else
#define __analysis_assume(expr) 
#endif
#endif

#ifdef  __cplusplus
}
#endif


