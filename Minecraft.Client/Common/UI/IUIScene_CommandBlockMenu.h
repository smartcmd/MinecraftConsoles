#pragma once
#include "../Minecraft.World/net.minecraft.world.level.tile.entity.h"

class IUIScene_CommandBlockMenu
{
public:
    void Initialise(CommandBlockEntity *commandBlock);

protected:
	void ConfirmButtonClicked();

	virtual wstring GetCommand();
	virtual void SetCommand(wstring command);
	virtual int GetPad();

private:
	CommandBlockEntity *m_commandBlock;
};