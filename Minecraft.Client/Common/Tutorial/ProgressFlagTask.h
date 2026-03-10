#pragma once
using namespace std;
#include "Tutorial.h"
#include "TutorialTask.h"

class ProgressFlagTask : public TutorialTask
{
public:
	enum EProgressFlagType
	{
		e_Progress_Set_Flag,
		e_Progress_Flag_On,
	};
private:
	char *flags; // Not a member of this object
	char m_mask;
	EProgressFlagType m_type;
public:
	ProgressFlagTask(char *flags, char mask, EProgressFlagType type, Tutorial *tutorial ) :
	TutorialTask(tutorial, -1, false, nullptr ),
		flags( flags ), m_mask( mask ), m_type( type )
	{}

	virtual bool isCompleted();
};