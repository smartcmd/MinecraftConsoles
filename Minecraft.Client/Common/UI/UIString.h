#pragma once

#include <set>
#include <functional>


#ifndef __PS3__

typedef function<wstring(void)> StringBuilder;

#else

class StringBuilderCore
{
public:
	virtual wstring getString() = 0;
};

struct StringBuilder
{
	shared_ptr<StringBuilderCore> m_coreBuilder;
	virtual wstring operator()() { return m_coreBuilder->getString(); }
	StringBuilder() {}
	StringBuilder(StringBuilderCore *core) { m_coreBuilder = shared_ptr<StringBuilderCore>(core); }
};

class IdsStringBuilder : public StringBuilderCore
{
	const int m_ids;
public:
	IdsStringBuilder(int ids) : m_ids(ids) {}
	virtual wstring getString(void) { return app.GetString(m_ids); }
};
#endif

using namespace std;

class UIString
{
protected:
	static int s_currentLanguage;
	static int s_currentLocale;

public:
	static bool setCurrentLanguage();
	static int getCurrentLanguage();

protected:
	class UIStringCore : public enable_shared_from_this<UIStringCore>
	{
	private:
		int				m_lastSetLanguage;
		int				m_lastSetLocale;

		int				m_lastUpdatedLanguage;
		int				m_lastUpdatedLocale;

		wstring			m_wstrCache;

		bool			m_bIsConstant;

		StringBuilder	m_fStringBuilder;
		
	public:
		UIStringCore(StringBuilder wstrBuilder);
		UIStringCore(const wstring &str);

		wstring &getString();

		bool hasNewString();
		bool update(bool force);

		bool needsUpdating();
		void setUpdated();
	};	

	shared_ptr<UIStringCore> m_core;

public:
	UIString();
	
	UIString(int ids); // Create a dynamic UI string from a string id value.

	UIString(StringBuilder wstrBuilder); // Create a dynamic UI string with a custom update function.
	
	// Create a UIString with a constant value.
	UIString(const  string &constant);
	UIString(const wstring &constant); 
	UIString(const wchar_t *constant);

	~UIString();

	bool	empty();
	bool	compare(const UIString &uiString);

	bool	needsUpdating();	// Language has been change since the last time setUpdated was called.
	void	setUpdated();		// The new text has been used.

	wstring	&getString();

	const wchar_t *c_str();
	unsigned int length();
};