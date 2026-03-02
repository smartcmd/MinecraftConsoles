#pragma once
using namespace std;

class Pos;


class SynchedEntityData
{
public:
	class DataItem
	{
		friend class SynchedEntityData;
	private:
		const int type;
		const int id;
		// 4J - there used to be one "value" type here of general type Object, just storing the different (used) varieties
		// here separately for us
		byte	value_byte;
		int		value_int;
		short	value_short;
		wstring	value_wstring;
		shared_ptr<ItemInstance> value_itemInstance;
		bool dirty;

	public:
		// There was one type here that took a generic Object type, using overloading here instead
		DataItem(int type, int id, byte value);
		DataItem(int type, int id, int value);
		DataItem(int type, int id, const wstring& value);
		DataItem(int type, int id, shared_ptr<ItemInstance> itemInstance);
		DataItem(int type, int id, short value);

		int getId();
		void setValue(byte value);
		void setValue(int value);
		void setValue(short value);
		void setValue(const wstring& value);
		void setValue(shared_ptr<ItemInstance> value);
		byte getValue_byte();
		int getValue_int();
		short getValue_short();
		wstring getValue_wstring();
		shared_ptr<ItemInstance> getValue_itemInstance();
		int getType();
		bool isDirty();
		void setDirty(bool dirty);
	};

public:
	static const int MAX_STRING_DATA_LENGTH = 64;
	static const int EOF_MARKER = 0x7f;

private:
	static const int TYPE_BYTE = 0;
	static const int TYPE_SHORT = 1;
	static const int TYPE_INT = 2;
	static const int TYPE_FLOAT = 3;
	static const int TYPE_STRING = 4;
	// special types (max possible value is 7):
	static const int TYPE_ITEMINSTANCE = 5;
	static const int TYPE_POS = 6;

private:
	bool m_isEmpty;

	// must have enough bits to fit the type
private:
	static const int TYPE_MASK = 0xe0;
	static const int TYPE_SHIFT = 5;

	// the id value must fit in the remaining bits
	static const int MAX_ID_VALUE = ~TYPE_MASK & 0xff;

	unordered_map<int, shared_ptr<DataItem> > itemsById;
	bool m_isDirty;

public:
	SynchedEntityData();

	// 4J - this function used to be a template, but there's only 3 varieties of use I've found so just hard-coding now, as
	// the original had some automatic Class to type sort of conversion that's a real pain for us to actually do
	void define(int id, byte value);
	void define(int id, const wstring& value);
	void define(int id, int value);
	void define(int id, short value);
	void defineNULL(int id, void *pVal);

	void checkId(int id);	// 4J - added to contain common code from overloaded define functions above
	byte getByte(int id);
	short getShort(int id);
	int getInteger(int id);
	float getFloat(int id);
	wstring getString(int id);
	shared_ptr<ItemInstance> getItemInstance(int id);
	Pos *getPos(int id);
	// 4J - using overloads rather than template here
	void set(int id, byte value);
	void set(int id, int value);
	void set(int id, short value);
	void set(int id, const wstring& value);
	void set(int id, shared_ptr<ItemInstance>);
	void markDirty(int id);
	bool isDirty();
	static void pack(vector<shared_ptr<DataItem> > *items, DataOutputStream *output); // TODO throws IOException
	vector<shared_ptr<DataItem> > *packDirty();
	void packAll(DataOutputStream *output); // throws IOException
	vector<shared_ptr<DataItem> > *getAll();

private:
	static void writeDataItem(DataOutputStream *output, shared_ptr<DataItem> dataItem); //throws IOException


public:
	static vector<shared_ptr<DataItem> > *unpack(DataInputStream *input); // throws IOException

	/**
	* Assigns values from a list of data items.
	* 
	* @param items
	*/
public:
	void assignValues(vector<shared_ptr<DataItem> > *items);
	bool isEmpty();

	// 4J Added
	int getSizeInBytes();
};