#pragma once

class GameRules
{
private:
	class GameRule
	{
	private:
		wstring value;
		bool booleanValue;
		int intValue;
		double doubleValue;

	public:
		GameRule(const wstring &startValue);

		void set(const wstring &newValue);
		wstring get();
		bool getBoolean();
		int getInt();
		double getDouble();
	};

public:
	// 4J: Originally strings
	// default rules
	static const int RULE_DOFIRETICK;
	static const int RULE_MOBGRIEFING;
	static const int RULE_KEEPINVENTORY;
	static const int RULE_DOMOBSPAWNING;
	static const int RULE_DOMOBLOOT;
	static const int RULE_DOTILEDROPS;
	static const int RULE_COMMANDBLOCKOUTPUT;
	static const int RULE_NATURAL_REGENERATION;
	static const int RULE_DAYLIGHT;

private:
	unordered_map<wstring, GameRule *> rules;

public:
	GameRules();
	~GameRules();

	bool getBoolean(const int rule);

	// 4J: Removed unused functions
	/*void set(const wstring &ruleName, const wstring &newValue);
	void registerRule(const wstring &name, const wstring &startValue);
	wstring get(const wstring &ruleName);
	int getInt(const wstring &ruleName);
	double getDouble(const wstring &ruleName);
	CompoundTag *createTag();
	void loadFromTag(CompoundTag *tag);
	vector<wstring> *getRuleNames();
	bool contains(const wstring &rule);*/
};