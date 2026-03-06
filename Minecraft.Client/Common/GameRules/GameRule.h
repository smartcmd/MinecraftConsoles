#pragma once
using namespace std;

#include <unordered_map>

class CompoundTag;
class GameRuleDefinition;
class Connection;

// A game rule maintains the state for one particular definition
class GameRule
{
public:
	typedef struct _ValueType
	{
		union{
			int64_t				i64;
			int					i;
			char				c;
			bool				b;
			float				f;
			double				d;
			GameRule			*gr;
		};
		bool isPointer;

		_ValueType()
		{
			i64 = 0;
			isPointer = false;
		}
	} ValueType;

private:
	GameRuleDefinition *m_definition;
	Connection *m_connection;

public:
	typedef unordered_map<wstring,ValueType> stringValueMapType;
	stringValueMapType m_parameters; // These are the members of this rule that maintain it's state

public:
	GameRule(GameRuleDefinition *definition, Connection *connection = NULL);
	virtual ~GameRule();

	Connection *getConnection() { return m_connection; }

	ValueType getParameter(const wstring &parameterName);
	void setParameter(const wstring &parameterName,ValueType value);
	GameRuleDefinition *getGameRuleDefinition();

	// All the hooks go here
	void onUseTile(int tileId, int x, int y, int z);
	void onCollectItem(shared_ptr<ItemInstance> item);

	// 4J-JEV: For saving.
	//CompoundTag *toTags(unordered_map<GameRuleDefinition *, int> *map);
	//static GameRule *fromTags(Connection *c, CompoundTag *cTag, vector<GameRuleDefinition *> *grds);

	void write(DataOutputStream *dos);
	void read(DataInputStream *dos);
};