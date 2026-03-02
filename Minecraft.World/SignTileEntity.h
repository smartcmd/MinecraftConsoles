#pragma once
using namespace std;

#include "TileEntity.h"

#define MAX_SIGN_LINES 4

class SignTileEntity : public TileEntity
{
public:
	eINSTANCEOF GetType() { return eTYPE_SIGNTILEENTITY; }
	static TileEntity *create() { return new SignTileEntity(); }
public:
	static const int MAX_LINE_LENGTH;

public:
	SignTileEntity();
	virtual ~SignTileEntity();
	wstring GetMessage(int i)								{ return m_wsmessages[i];}
	wstring *GetMessages()									{ return m_wsmessages;}
	void SetMessage(int iIndex,wstring &wsText);
	int GetSelectedLine()									{return m_iSelectedLine;}
	void SetSelectedLine(int iLine)							{m_iSelectedLine=iLine;}
	bool IsVerified()										{return m_bVerified;}
	void SetVerified(bool bVerified)						{m_bVerified=bVerified;}
	bool IsCensored()										{return m_bCensored;}
	void SetCensored(bool bCensored)						{m_bCensored=bCensored;}
public:

private:
	shared_ptr<Player> playerWhoMayEdit;
	bool _isEditable;
	bool m_bVerified;
	bool m_bCensored;
	int m_iSelectedLine;

	wstring m_wsmessages[MAX_SIGN_LINES];

public:
	virtual void save(CompoundTag *tag);
	virtual void load(CompoundTag *tag);
	virtual shared_ptr<Packet> getUpdatePacket();
	bool isEditable();
	void setEditable(bool isEditable);
	void setAllowedPlayerEditor(shared_ptr<Player> player);
	shared_ptr<Player> getPlayerWhoMayEdit();
	virtual void setChanged();
	static int StringVerifyCallback(LPVOID lpParam,STRING_VERIFY_RESPONSE *pResults);

	// 4J Added
	virtual shared_ptr<TileEntity> clone();
};