#include "stdafx.h"
#include "Material.h"
#include "DecorationMaterial.h"
#include "GasMaterial.h"
#include "LiquidMaterial.h"
#include "PortalMaterial.h"
#include "WebMaterial.h"// 4J added, Java version just does a local alteration when instantiating the Material for webs to get the same thing

Material *Material::air = nullptr;
Material *Material::grass = nullptr;
Material *Material::dirt = nullptr;
Material *Material::wood = nullptr;
Material *Material::stone = nullptr;
Material *Material::metal = nullptr;
Material *Material::heavyMetal = nullptr;
Material *Material::water = nullptr;
Material *Material::lava = nullptr;
Material *Material::leaves = nullptr;
Material *Material::plant = nullptr;
Material *Material::replaceable_plant = nullptr;
Material *Material::sponge = nullptr;
Material *Material::cloth = nullptr;
Material *Material::fire = nullptr;
Material *Material::sand = nullptr;
Material *Material::decoration = nullptr;
Material *Material::clothDecoration = nullptr;
Material *Material::glass = nullptr;
Material *Material::buildable_glass = nullptr;
Material *Material::explosive = nullptr;
Material *Material::coral = nullptr;
Material *Material::ice = nullptr;
Material *Material::topSnow = nullptr;
Material *Material::snow = nullptr;
Material *Material::cactus = nullptr;
Material *Material::clay = nullptr;
Material *Material::vegetable = nullptr;
Material *Material::egg = nullptr;
Material *Material::portal = nullptr;
Material *Material::cake = nullptr;
Material *Material::piston = nullptr;
Material *Material::web = nullptr;

void Material::staticCtor()
{
	Material::air = new GasMaterial(MaterialColor::none);
	Material::grass = new Material(MaterialColor::grass);
	Material::dirt = new Material(MaterialColor::dirt);
	Material::wood = (new Material(MaterialColor::wood))->flammable();
	Material::stone = (new Material(MaterialColor::stone))->notAlwaysDestroyable();
	Material::metal = (new Material(MaterialColor::metal))->notAlwaysDestroyable();
	Material::heavyMetal = (new Material(MaterialColor::metal))->notAlwaysDestroyable()->notPushable();
	Material::water = (new LiquidMaterial(MaterialColor::water))->destroyOnPush();
	Material::lava = (new LiquidMaterial(MaterialColor::fire))->destroyOnPush();
	Material::leaves = (new Material(MaterialColor::plant))->flammable()->neverBuildable()->destroyOnPush();
	Material::plant = (new DecorationMaterial(MaterialColor::plant))->destroyOnPush();
	Material::replaceable_plant = (new DecorationMaterial(MaterialColor::plant))->flammable()->destroyOnPush()->replaceable();
	Material::sponge = new Material(MaterialColor::cloth);
	Material::cloth = (new Material(MaterialColor::cloth))->flammable();
	Material::fire = (new GasMaterial(MaterialColor::none))->destroyOnPush();
	Material::sand = new Material(MaterialColor::sand);
	Material::decoration = (new DecorationMaterial(MaterialColor::none))->destroyOnPush();
	Material::clothDecoration = (new DecorationMaterial(MaterialColor::cloth))->flammable();
	Material::glass = (new Material(MaterialColor::none))->neverBuildable()->makeDestroyedByHand();
	Material::buildable_glass = (new Material(MaterialColor::none))->makeDestroyedByHand();
	Material::explosive = (new Material(MaterialColor::fire))->flammable()->neverBuildable();
	Material::coral = (new Material(MaterialColor::plant))->destroyOnPush();
	Material::ice = (new Material(MaterialColor::ice))->neverBuildable()->makeDestroyedByHand();
	Material::topSnow = (new DecorationMaterial(MaterialColor::snow))->replaceable()->neverBuildable()->notAlwaysDestroyable()->destroyOnPush();
	Material::snow = (new Material(MaterialColor::snow))->notAlwaysDestroyable();
	Material::cactus = (new Material(MaterialColor::plant))->neverBuildable()->destroyOnPush();
	Material::clay = (new Material(MaterialColor::clay));
	Material::vegetable = (new Material(MaterialColor::plant))->destroyOnPush();
	Material::egg = ( new Material(MaterialColor::plant))->destroyOnPush();
	Material::portal = (new PortalMaterial(MaterialColor::none))->notPushable();
	Material::cake = (new Material(MaterialColor::none))->destroyOnPush();
	// 4J added WebMaterial, Java version just does a local alteration when instantiating the Material for webs to get the same thing
	Material::web = (new WebMaterial(MaterialColor::cloth))->notAlwaysDestroyable()->destroyOnPush();
	Material::piston  = (new Material(MaterialColor::stone))->notPushable();
}

Material::Material(MaterialColor *color)
{
	this->color = color;

	// 4J Stu - Default inits
	_flammable = false;
	_replaceable = false;
	_neverBuildable = false;
	_isAlwaysDestroyable = true;
	pushReaction = 0;
	destroyedByHand = false;
}

bool Material::isLiquid()
{
    return false;
}

bool Material::letsWaterThrough()
{
    return (!isLiquid() && !isSolid());
}

bool Material::isSolid()
{
    return true;
}

bool Material::blocksLight()
{
    return true;
}

bool Material::blocksMotion()
{
    return true;
}

Material *Material::neverBuildable()
{
    this->_neverBuildable = true;
    return this;
}

Material *Material::notAlwaysDestroyable()
{
	this->_isAlwaysDestroyable = false;
	return this;
}

Material *Material::flammable()
{
    this->_flammable = true;
    return this;
}

bool Material::isFlammable()
{
    return _flammable;
}

Material *Material::replaceable()
{
    this->_replaceable = true;
    return this;
}

bool Material::isReplaceable()
{
    return _replaceable;
}

bool Material::isSolidBlocking()
{
    if (_neverBuildable) return false;
    return blocksMotion();
}

bool Material::isAlwaysDestroyable()
{
    // these materials will always drop resources when destroyed, regardless
	// of player's equipment
    return _isAlwaysDestroyable;
}

int Material::getPushReaction()
{
	return pushReaction;
}

Material *Material::makeDestroyedByHand()
{
	this->destroyedByHand = true;
	return this;
}

bool Material::isDestroyedByHand()
{
	return destroyedByHand;
}

Material *Material::destroyOnPush()
{
	pushReaction = PUSH_DESTROY;
	return this;
}

Material *Material::notPushable()
{
	pushReaction = PUSH_BLOCK;
	return this;
}