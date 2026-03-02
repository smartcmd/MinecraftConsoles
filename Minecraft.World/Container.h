#pragma once
using namespace std;

class ItemInstance;
class Player;

class Container
{
public:
    static const int LARGE_MAX_STACK_SIZE = 64;

    virtual unsigned int getContainerSize() = 0;
    virtual shared_ptr<ItemInstance> getItem(unsigned int slot) = 0;
    virtual shared_ptr<ItemInstance> removeItem(unsigned int slot, int count) = 0;
	virtual shared_ptr<ItemInstance> removeItemNoUpdate(int slot) = 0;
    virtual void setItem(unsigned int slot, shared_ptr<ItemInstance> item) = 0;
    virtual int getName() = 0;
    virtual int getMaxStackSize() = 0;
    virtual void setChanged() = 0;
    virtual bool stillValid(shared_ptr<Player> player) = 0;
	virtual void startOpen() = 0;
	virtual void stopOpen() = 0;
};