#include "stdafx.h"

#include "FoodConstants.h"

const int FoodConstants::MAX_FOOD = 20;
const float FoodConstants::MAX_SATURATION = (float) FoodConstants::MAX_FOOD;
const float FoodConstants::START_SATURATION = (float) FoodConstants::MAX_SATURATION / 4.0f;
const float FoodConstants::SATURATION_FLOOR = FoodConstants::MAX_SATURATION / 8.0f;

// this value modifies how quickly food is dropped
const float FoodConstants::EXHAUSTION_DROP = 4.0f;

// number of game ticks to change health because of food
const int FoodConstants::HEALTH_TICK_COUNT = 80;

const int FoodConstants::HEAL_LEVEL = 18;
const int FoodConstants::STARVE_LEVEL = 0;

// some saturation guidelines
const float FoodConstants::FOOD_SATURATION_POOR = .1f;
const float FoodConstants::FOOD_SATURATION_LOW = .3f;
const float FoodConstants::FOOD_SATURATION_NORMAL = .6f;
const float FoodConstants::FOOD_SATURATION_GOOD = .8f;
const float FoodConstants::FOOD_SATURATION_MAX = 1.0f;
const float FoodConstants::FOOD_SATURATION_SUPERNATURAL = 1.2f;

// some exhaustion guidelines
const float FoodConstants::EXHAUSTION_HEAL = 3.0f;
const float FoodConstants::EXHAUSTION_JUMP = .2f;
const float FoodConstants::EXHAUSTION_SPRINT_JUMP = FoodConstants::EXHAUSTION_JUMP * 4;
const float FoodConstants::EXHAUSTION_MINE = .025f;
const float FoodConstants::EXHAUSTION_ATTACK = .3f;
const float FoodConstants::EXHAUSTION_DAMAGE = .1f;
const float FoodConstants::EXHAUSTION_WALK = .010f;
const float FoodConstants::EXHAUSTION_SPRINT = FoodConstants::EXHAUSTION_WALK * 10;
const float FoodConstants::EXHAUSTION_SWIM = .015f;