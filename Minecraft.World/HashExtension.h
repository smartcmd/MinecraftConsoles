#pragma once
#ifdef _XBOX
//using namespace std;
namespace std
{
	namespace tr1
	{

		template<class T>
		class hash< shared_ptr<T> >
		{
		public:
			size_t operator()(const shared_ptr<T>& key) const
			{
				return (size_t)key.get();
			}
		};
	}
}
#endif