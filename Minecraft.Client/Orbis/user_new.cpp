/* SCE CONFIDENTIAL
 PlayStation(R)4 Programmer Tool Runtime Library Release 01.600.051
 * Copyright (C) 2013 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <new>
#include <cstdlib>
#include <cstdio>

void *user_new(std::size_t size) throw(std::bad_alloc);
void *user_new(std::size_t size, const std::nothrow_t& x) throw();
void *user_new_array(std::size_t size) throw(std::bad_alloc);
void *user_new_array(std::size_t size, const std::nothrow_t& x) throw();
void user_delete(void *ptr) throw();
void user_delete(void *ptr, const std::nothrow_t& x) throw();
void user_delete_array(void *ptr) throw();
void user_delete_array(void *ptr, const std::nothrow_t& x) throw();

//E Replace operator new.
//J operator new と置き換わる
void *user_new(std::size_t size) throw(std::bad_alloc)
{
	void *ptr;

	if (size == 0)
		size = 1;

	while ((ptr = (void *)std::malloc(size)) == nullptr) {
		//E Obtain new_handler
		//J new_handler を取得する
		std::new_handler handler = std::get_new_handler();

		//E When new_handler is a nullptr pointer, bad_alloc is send. If not, new_handler is called.
		//J new_handler が nullptr ポインタの場合、bad_alloc を送出する、そうでない場合、new_handler を呼び出す
		if (!handler)
		{
			assert(0);//throw std::bad_alloc();
		}
		else
			(*handler)();
	}
	return ptr;
}

//E Replace operator new(std::nothrow).
//J operator(std::nothrow) と置き換わる
void *user_new(std::size_t size, const std::nothrow_t& x) throw()
{
// 	void *ptr;
// 
// 	(void)x;
// 
// 	if (size == 0)
// 		size = 1;
// 
// 	while ((ptr = (void *)std::malloc(size)) == nullptr) {
// 		//E Obtain new_handler
// 		//J new_handler を取得する
// 		std::new_handler handler = std::get_new_handler();
// 
// 		//E When new_handler is a nullptr pointer, nullptr is returned.
// 		//J new_handler が nullptr ポインタの場合、nullptr を返す
// 		if (!handler)
// 			return nullptr;
// 
// 		//E Call new_handler. If new_handler sends bad_alloc, nullptr is returned.
// 		//J new_handler を呼び出す、new_handler が bad_alloc を送出した場合、nullptr を返す
// 		try {
// 			(*handler)();
// 		} catch (std::bad_alloc) {
// 			return nullptr;
// 		}
// 	}
// 	return ptr;
	assert(0);
	return nullptr;
}

//E Replace operator new[].
//J operator new[] と置き換わる
void *user_new_array(std::size_t size) throw(std::bad_alloc)
{
	return user_new(size);
}

//E Replace operator new[](std::nothrow).
//J operator new[](std::nothrow) と置き換わる
void *user_new_array(std::size_t size, const std::nothrow_t& x) throw()
{
	return user_new(size, x);
}

//E Replace operator delete.
//J operator delete と置き換わる
void user_delete(void *ptr) throw()
{
	//E In the case of the nullptr pointer, no action will be taken.
	//J nullptr ポインタの場合、何も行わない
	if (ptr != nullptr)
		std::free(ptr);
}

//E Replace operator delete(std::nothrow).
//J operator delete(std::nothrow) と置き換わる
void user_delete(void *ptr, const std::nothrow_t& x) throw()
{
	(void)x;

	user_delete(ptr);
}

//E Replace operator delete[].
//J operator delete[] と置き換わる
void user_delete_array(void *ptr) throw()
{
	user_delete(ptr);
}

//E Replace operator delete[](std::nothrow).
//J operator delete[](std::nothrow) と置き換わる
void user_delete_array(void *ptr, const std::nothrow_t& x) throw()
{
	user_delete(ptr, x);
}
