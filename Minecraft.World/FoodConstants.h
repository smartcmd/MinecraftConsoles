#pragma once

class FoodConstants
{
public:
	static const int MAX_FOOD;
	static const float MAX_SATURATION;
	static const float START_SATURATION;
	static const float SATURATION_FLOOR;

	// this value modifies how quickly food is dropped
	static const float EXHAUSTION_DROP;

	// number of game ticks to change health because of food
	static const int HEALTH_TICK_COUNT;
	static const int QUICK_HEALTH_TICK_COUNT;
	static const int HEAL_LEVEL;
	static const int STARVE_LEVEL;

	// some saturation guidelines
	static const float FOOD_SATURATION_POOR;
	static const float FOOD_SATURATION_LOW;
	static const float FOOD_SATURATION_NORMAL;
	static const float FOOD_SATURATION_GOOD;
	static const float FOOD_SATURATION_MAX;
	static const float FOOD_SATURATION_SUPERNATURAL;

	// some exhaustion guidelines
	static const float EXHAUSTION_HEAL;
	static const float EXHAUSTION_JUMP;
	static const float EXHAUSTION_SPRINT_JUMP;
	static const float EXHAUSTION_MINE;
	static const float EXHAUSTION_ATTACK;
	static const float EXHAUSTION_DAMAGE;
	static const float EXHAUSTION_WALK;
	static const float EXHAUSTION_SPRINT;
	static const float EXHAUSTION_SWIM;
};