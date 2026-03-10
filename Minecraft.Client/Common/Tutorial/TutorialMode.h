#pragma once
using namespace std;

#include "..\..\MultiPlayerGameMode.h"
#include "Tutorial.h"

class TutorialMode : public MultiPlayerGameMode
{
protected:
	Tutorial *tutorial;
	int m_iPad;

	// Function to make this an abstract class
	virtual bool isImplemented() = 0;
public:
	TutorialMode(int iPad, Minecraft *minecraft, ClientConnection *connection);
	virtual ~TutorialMode();

    virtual void startDestroyBlock(int x, int y, int z, int face);
    virtual bool destroyBlock(int x, int y, int z, int face);
    virtual void tick();
    virtual bool useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face, Vec3 *hit, bool bTestUseOnly=false, bool *pbUsedItem=nullptr);
    virtual void attack(shared_ptr<Player> player, shared_ptr<Entity> entity);

	virtual bool isInputAllowed(int mapping);

	Tutorial *getTutorial() { return tutorial; }
};