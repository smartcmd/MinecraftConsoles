#pragma once
using namespace std;
// 4J Stu - Not updated to 1.8.2 as we don't use this
class KeyMapping
{
public:
	wstring name;
	int key;
	int defaultKey;
	KeyMapping(const wstring& name, int key);
	void resetToDefault();
};