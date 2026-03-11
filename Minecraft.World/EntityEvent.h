#pragma once

class EntityEvent
{
public:
	static const BYTE JUMP = 1;
	static const BYTE HURT = 2;
	//New
	static const BYTE HURT_CRITICAL = 19;
	static const BYTE DEATH = 3;
	static const BYTE START_ATTACKING = 4;
	static const BYTE STOP_ATTACKING = 5;

	static const BYTE TAMING_FAILED = 6;
	static const BYTE TAMING_SUCCEEDED = 7;
	static const BYTE SHAKE_WETNESS = 8;

	static const BYTE USE_ITEM_COMPLETE = 9;

	static const BYTE EAT_GRASS = 10;
	static const BYTE OFFER_FLOWER = 11;
	static const BYTE LOVE_HEARTS = 12;
	static const BYTE VILLAGER_ANGRY = 13;
	static const BYTE VILLAGER_HAPPY = 14;
	static const BYTE WITCH_HAT_MAGIC = 15;
	static const BYTE ZOMBIE_CONVERTING = 16;

	static const BYTE FIREWORKS_EXPLODE = 17;

	static const BYTE IN_LOVE_HEARTS = 18;
};