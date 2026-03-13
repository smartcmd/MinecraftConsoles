#include "stdafx.h"
#include "RemotePlayer.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/Mth.h"

RemotePlayer::RemotePlayer(Level *level, const wstring& name) : Player(level, name)
{
	// 4J - added initialisers
	hasStartedUsingItem = false;
	lSteps = 0;
	lx = ly = lz = lyr = lxr = 0.0;
	fallTime = 0.0f;

	app.DebugPrintf("Created RemotePlayer with name %ls\n", name.c_str() );

    heightOffset = 0;
    footSize = 0;

	noPhysics = true;

    bedOffsetY = 4 / 16.0f;

    viewScale = 10;
}

void RemotePlayer::setDefaultHeadHeight()
{
	heightOffset = 0;
}

bool RemotePlayer::hurt(DamageSource *source, float dmg)
{
	return true;
}

void RemotePlayer::lerpTo(double x, double y, double z, float yRot, float xRot, int steps)
{
//        heightOffset = 0;
    lx = x;
    ly = y;
    lz = z;
    lyr = yRot;
    lxr = xRot;

    lSteps = steps;
}

void RemotePlayer::tick()
{
    bedOffsetY = 0 / 16.0f;
    Player::tick();

    walkAnimSpeedO = walkAnimSpeed;
    double xxd = x - xo;
    double zzd = z - zo;
    float wst = Mth::sqrt(xxd * xxd + zzd * zzd) * 4;
    if (wst > 1) wst = 1;
    walkAnimSpeed += (wst - walkAnimSpeed) * 0.4f;
    walkAnimPos += walkAnimSpeed;

	if (!hasStartedUsingItem && isUsingItemFlag() && inventory->items[inventory->selected] != nullptr)
	{
		shared_ptr<ItemInstance> item = inventory->items[inventory->selected];
		startUsingItem(inventory->items[inventory->selected], Item::items[item->id]->getUseDuration(item));
		hasStartedUsingItem = true;
	}
	else if (hasStartedUsingItem && !isUsingItemFlag())
	{
		stopUsingItem();
		hasStartedUsingItem = false;
	}

	//        if (eatItem != null) {
	//            if (eatItemTickCount <= 25 && eatItemTickCount % 4 == 0) {
	//                spawnEatParticles(eatItem, 5);
	//            }
	//            eatItemTickCount--;
	//            if (eatItemTickCount <= 0) {
	//                spawnEatParticles(eatItem, 16);
	//                swing();
	//                eatItem = null;
	//            }
	//        }
}

float RemotePlayer::getShadowHeightOffs()
{
	return 0;
}

void RemotePlayer::aiStep()
{
    Player::serverAiStep();
    if (lSteps > 0)
	{
        double xt = x + (lx - x) / lSteps;
        double yt = y + (ly - y) / lSteps;
        double zt = z + (lz - z) / lSteps;

        double yrd = lyr - yRot;
        while (yrd < -180)
            yrd += 360;
        while (yrd >= 180)
            yrd -= 360;

        yRot += static_cast<float>((yrd) / lSteps);
        xRot += static_cast<float>((lxr - xRot) / lSteps);

        lSteps--;
        setPos(xt, yt, zt);
        setRot(yRot, xRot);
    }
    oBob = bob;

    float tBob = (float) Mth::sqrt(xd * xd + zd * zd);
    float tTilt = static_cast<float>(atan(-yd * 0.2f)) * 15.0f;
    if (tBob > 0.1f) tBob = 0.1f;
    if (!onGround || getHealth() <= 0) tBob = 0;
    if (onGround || getHealth() <= 0) tTilt = 0;
    bob += (tBob - bob) * 0.4f;
    tilt += (tTilt - tilt) * 0.8f;

}

// 4J Stu - Brought forward change from 1.3 to fix #64688 - Customer Encountered: TU7: Content: Art: Aura of enchanted item is not displayed for other players in online game
void RemotePlayer::setEquippedSlot(int slot, shared_ptr<ItemInstance> item)
{
    if (slot == 0)
	{
        inventory->items[inventory->selected] = item;
    }
	else
	{
        inventory->armor[slot - 1] = item;
    }
}

void RemotePlayer::animateRespawn()
{
//        Player.animateRespawn(this, level);
}

float RemotePlayer::getHeadHeight()
{
	return 1.82f;
}

Pos RemotePlayer::getCommandSenderWorldPosition()
{
    return new Pos(floor(x + .5), floor(y + .5), floor(z + .5));
}