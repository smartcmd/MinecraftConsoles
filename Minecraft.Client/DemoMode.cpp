#include "stdafx.h"
#include "DemoMode.h"
#include "../Minecraft.World/net.minecraft.world.level.h"

DemoMode::DemoMode(Minecraft *minecraft) : SurvivalMode(minecraft)
{
	demoHasEnded = false;
    demoEndedReminder = 0;
}

void DemoMode::tick()
{
    SurvivalMode::tick();

/* 4J - TODO - seems unlikely we need this demo mode anyway
    int64_t time = minecraft->level->getTime();
    int64_t day = (time / Level::TICKS_PER_DAY) + 1;

    demoHasEnded = (time > (500 + Level::TICKS_PER_DAY * DEMO_DAYS));
    if (demoHasEnded)
	{
        demoEndedReminder++;
    }

    if ((time % Level::TICKS_PER_DAY) == 500)
	{
        if (day <= (DEMO_DAYS + 1))
		{
            minecraft->gui->displayClientMessage(L"demo.day." + std::to_wstring(day));
        }
    }
	else if (day == 1)
	{
        Options *options = minecraft->options;
        wstring message;

        if (time == 100) {
            minecraft.gui.addMessage("Seed: " + minecraft.level.getSeed());
            message = language.getElement("demo.help.movement");
            message = String.format(message, Keyboard.getKeyName(options.keyUp.key), Keyboard.getKeyName(options.keyLeft.key), Keyboard.getKeyName(options.keyDown.key),
                    Keyboard.getKeyName(options.keyRight.key));
        } else if (time == 175) {
            message = language.getElement("demo.help.jump");
            message = String.format(message, Keyboard.getKeyName(options.keyJump.key));
        } else if (time == 250) {
            message = language.getElement("demo.help.inventory");
            message = String.format(message, Keyboard.getKeyName(options.keyBuild.key));
        }
        if (message != null) {
            minecraft.gui.addMessage(message);
        }
    } else if (day == DEMO_DAYS) {
        if ((time % Level.TICKS_PER_DAY) == 22000) {
            minecraft.gui.displayClientMessage("demo.day.warning");
        }
    }
*/
}

void DemoMode::outputDemoReminder()
{
/* 4J - TODO
    if (demoEndedReminder > 100) {
        minecraft.gui.displayClientMessage("demo.reminder");
        demoEndedReminder = 0;
    }
	*/
}

void DemoMode::startDestroyBlock(int x, int y, int z, int face)
{
    if (demoHasEnded)
	{
        outputDemoReminder();
        return;
    }
    SurvivalMode::startDestroyBlock(x, y, z, face);
}

void DemoMode::continueDestroyBlock(int x, int y, int z, int face)
{
    if (demoHasEnded)
	{
        return;
    }
    SurvivalMode::continueDestroyBlock(x, y, z, face);
}

bool DemoMode::destroyBlock(int x, int y, int z, int face)
{
    if (demoHasEnded)
	{
        return false;
    }
    return SurvivalMode::destroyBlock(x, y, z, face);
}

bool DemoMode::useItem(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item)
{
    if (demoHasEnded)
	{
        outputDemoReminder();
        return false;
    }
    return SurvivalMode::useItem(player, level, item);
}

bool DemoMode::useItemOn(shared_ptr<Player> player, Level *level, shared_ptr<ItemInstance> item, int x, int y, int z, int face)
{
    if (demoHasEnded) {
        outputDemoReminder();
        return false;
    }
    return SurvivalMode::useItemOn(player, level, item, x, y, z, face);
}

void DemoMode::attack(shared_ptr<Player> player, shared_ptr<Entity> entity)
{
    if (demoHasEnded)
	{
        outputDemoReminder();
        return;
    }
    SurvivalMode::attack(player, entity);
}
