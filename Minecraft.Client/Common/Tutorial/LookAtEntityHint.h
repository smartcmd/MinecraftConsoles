#pragma once
using namespace std;

#include "../../../Minecraft.World/Class.h"
#include "TutorialHint.h"

class ItemInstance;

class LookAtEntityHint : public TutorialHint
{
private:
	eINSTANCEOF m_type;
	int m_titleId;

public:
	LookAtEntityHint(eTutorial_Hint id, Tutorial *tutorial, int descriptionId, int titleId, eINSTANCEOF type);
	~LookAtEntityHint();

	virtual bool onLookAtEntity(eINSTANCEOF type);
};