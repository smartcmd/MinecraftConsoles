#include "stdafx.h"
#include <functional>

#include "Hasher.h"

#include "StringHelpers.h"

Hasher::Hasher(wstring &salt)
{
	std::setlocale(LC_ALL, "");
	this->salt = salt;
}

wstring Hasher::getHash(wstring &name)
{
	// 4J Stu - Removed try/catch
	//try {
		wstring s = wstring( salt ).append( name );
		//MessageDigest m;
		//m = MessageDigest.getInstance("MD5");
		//m.update(s.getBytes(), 0, s.length());
		//return new BigInteger(1, m.digest()).toString(16);

		// TODO 4J Stu - Will this hash us with the same distribution as the MD5?
		MD5 chk;
		std::wstring wsd = convStringToWstring(std::string(chk.digestString(const_cast<char*>(wstringtofilename(s)))));

		return _toString( wsd );
	//}
	//catch (NoSuchAlgorithmException e)
	//{
	//	throw new RuntimeException(e);
	//}
}