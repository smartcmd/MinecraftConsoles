#pragma once

class FlatLayerInfo
{
private:
	int height;
	int id;
	int data;
	int start;

	void _init(int height, int id);

public:
	FlatLayerInfo(int height, int id);
	FlatLayerInfo(int height, int id, int data);

	int getHeight();
	void setHeight(int height);
	int getId();
	void setId(int id);
	int getData();
	void setData(int data);
	int getStart();
	void setStart(int start);
	wstring toString();
};