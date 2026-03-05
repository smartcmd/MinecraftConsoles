#include "stdafx.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "net.minecraft.world.entity.ai.attributes.h"
#include "net.minecraft.world.entity.boss.h"
#include "net.minecraft.world.entity.monster.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.phys.h"
#include "net.minecraft.world.damagesource.h"
#include "BasicTypeContainers.h"
#include "..\Minecraft.Client\Textures.h"
#include "net.minecraft.world.entity.boss.enderdragon.h"
#include "net.minecraft.world.level.pathfinder.h"
#include "SharedConstants.h"
#include "EnderDragon.h"

#define PRINT_DRAGON_STATE_CHANGE_MESSAGES 1



// 4J Added for new dragon behaviour
const int EnderDragon::CRYSTAL_COUNT = 8;
const int EnderDragon::FLAME_TICKS = 60;
const float EnderDragon::FLAME_ANGLE = 22.5f;
const int EnderDragon::FLAME_PASSES = 4; // How many times it covers FLAME_ANGLE in FLAME_TICKS
const int EnderDragon::FLAME_FREQUENCY = 2; // Every FLAME_FREQUENCY ticks it sets fire to blocks while doing a flame pass
const int EnderDragon::FLAME_RANGE = 10;

const int EnderDragon::ATTACK_TICKS = SharedConstants::TICKS_PER_SECOND * 2; // Time for the dragon roar to play

const int EnderDragon::SITTING_ATTACK_Y_VIEW_RANGE = 10; // The player must be now lower and no higher than the dragon by this amount
const int EnderDragon::SITTING_ATTACK_VIEW_RANGE = EnderDragon::FLAME_RANGE * 2;
const int EnderDragon::SITTING_ATTACK_RANGE = EnderDragon::FLAME_RANGE * 2;
const int EnderDragon::SITTING_POST_ATTACK_IDLE_TICKS = 40;
const int EnderDragon::SITTING_SCANNING_IDLE_TICKS = 100;
const int EnderDragon::SITTING_FLAME_ATTACKS_COUNT = 4; // How many times the dragons does the scan/roar/flame cycle before flying off

// The percentage of max health that the dragon will take while in the "Sitting" states before flying away
const float EnderDragon::SITTING_ALLOWED_DAMAGE_PERCENTAGE = 0.25f;

void EnderDragon::_init()
{
	// 4J Stu - This function call had to be moved here from the Entity ctor to ensure that
	// the derived version of the function is called
	this->defineSynchedData();
	registerAttributes();
	setHealth(getMaxHealth());

	xTarget = yTarget = zTarget = 0.0;
	posPointer = -1;
	oFlapTime = 0;
	flapTime = 0;
	newTarget = false;
	inWall = false;
	attackTarget = nullptr;
	dragonDeathTime = 0;
	nearestCrystal = nullptr;

	// 4J Stu - Added for new dragon behaviour
	m_remainingCrystalsCount = CRYSTAL_COUNT;
	m_fireballCharge = 0;
	m_holdingPatternAngle = 0.0f;
	m_holdingPatternClockwise = true;
	setSynchedAction(e_EnderdragonAction_HoldingPattern);
	m_actionTicks = 0;
	m_sittingDamageReceived = 0;
	m_headYRot = 0.0;
	m_acidArea = AABB::newPermanent(-4,-10,-3,6,3,3);
	m_flameAttacks = 0;

	for (int i = 0; i < positionsLength; i++)
	{
		positions[i][0] = 0;
		positions[i][1] = 0;
		positions[i][2] = 0;
	}

	m_nodes = new NodeArray(24);
	openSet = new BinaryHeap();
	m_currentPath = NULL;
}

EnderDragon::EnderDragon(Level *level) : Mob(level)
{
	_init();

	setSize(16, 8);

	noPhysics = true;
	fireImmune = true;

	yTarget = 100;

	m_iGrowlTimer=100;

	noCulling = true;
}

// 4J - split off from ctor so we can use shared_from_this()
void EnderDragon::AddParts()
{
	head = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"head", 6, 6) );
	neck = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"neck", 6, 6) ); // 4J Added
	body = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"body", 8, 8) );
	tail1 = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail", 4, 4) );
	tail2 = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail", 4, 4) );
	tail3 = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"tail", 4, 4) );
	wing1 = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"wing", 4, 4) );
	wing2 = shared_ptr<MultiEntityMobPart>( new MultiEntityMobPart(dynamic_pointer_cast<MultiEntityMob>(shared_from_this()), L"wing", 4, 4) );

	subEntities.push_back(head);
	subEntities.push_back(neck); // 4J Added
	subEntities.push_back(body);
	subEntities.push_back(tail1);
	subEntities.push_back(tail2);
	subEntities.push_back(tail3);
	subEntities.push_back(wing1);
	subEntities.push_back(wing2);
}

EnderDragon::~EnderDragon()
{
	if(m_nodes->data != NULL)
	{
		for(unsigned int i = 0; i < m_nodes->length; ++i)
		{
			if(m_nodes->data[i]!=NULL) delete m_nodes->data[i];
		}
		delete [] m_nodes->data;
	}
	delete openSet;
	if( m_currentPath != NULL ) delete m_currentPath;
}

void EnderDragon::registerAttributes()
{
	Mob::registerAttributes();

	getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(200);
}

void EnderDragon::defineSynchedData()
{
	Mob::defineSynchedData();

	// 4J Added for new dragon behaviour
	entityData->define(DATA_ID_SYNCHED_ACTION, e_EnderdragonAction_HoldingPattern);
}

void EnderDragon::getLatencyPos(doubleArray result, int step, float a)
{
	if (getHealth() <= 0)
	{
		a = 0;
	}

	a = 1 - a;

	int p0 = (posPointer - step * 1) & 63;
	int p1 = (posPointer - step * 1 - 1) & 63;

	// positions is a ring buffer of size positionsLength (64) storing positional information per tick
	// positions[i][0] is y rotation
	// positions[i][1] is y position
	// positions[i][2] is currently always 0

	double yr0 = positions[p0][0];
	double yrd = Mth::wrapDegrees(positions[p1][0] - yr0);
	result[0] = yr0 + yrd * a;

	yr0 = positions[p0][1];
	yrd = positions[p1][1] - yr0;

	result[1] = yr0 + yrd * a;
	result[2] = positions[p0][2] + (positions[p1][2] - positions[p0][2]) * a;
}

void EnderDragon::aiStep()
{
	if (level->isClientSide)
	{
		// 4J Stu - If saved when dead we need to make sure that the actual health is updated correctly on the client
		// Fix for TU9: Content: Gameplay: Enderdragon respawns after loading game which was previously saved at point of hes death
		setHealth(getHealth());

		float flap = Mth::cos(flapTime * PI * 2);
		float oldFlap = Mth::cos(oFlapTime * PI * 2);

		if (oldFlap <= -0.3f && flap >= -0.3f) 
		{
			level->playLocalSound(x, y, z, eSoundType_MOB_ENDERDRAGON_MOVE, 1, 0.8f + random->nextFloat() * .3f, false, 100.0f);
		}
		// play a growl every now and then
		if(! (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
			getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
			getSynchedAction() == e_EnderdragonAction_Sitting_Attacking))
		{
			m_iGrowlTimer--;
			if(m_iGrowlTimer<0)
			{
				level->playLocalSound(x, y, z, eSoundType_MOB_ENDERDRAGON_GROWL, 0.5f, 0.8f + random->nextFloat() * .3f, false, 100.0f);
				m_iGrowlTimer=200+(random->nextInt(200));
			}
		}
	}

	oFlapTime = flapTime;

	if (getHealth() <= 0)
	{
		//            level.addParticle("explode", x + random.nextFloat() * bbWidth * 2 - bbWidth, y + random.nextFloat() * bbHeight, z + random.nextFloat() * bbWidth * 2 - bbWidth, 0, 0, 0);
		float xo = (random->nextFloat() - 0.5f) * 8;
		float yo = (random->nextFloat() - 0.5f) * 4;
		float zo = (random->nextFloat() - 0.5f) * 8;
		level->addParticle(eParticleType_largeexplode, x + xo, y + 2 + yo, z + zo, 0, 0, 0);
		return;
	}

	checkCrystals();

	float flapSpeed = 0.2f / (sqrt(xd * xd + zd * zd) * 10.0f + 1);
	flapSpeed *= (float) pow(2.0, yd);
	if (	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
	{
		//app.DebugPrintf("flapSpeed is %f\n", flapSpeed);
		//flapTime += flapSpeed * 2;
		flapTime += 0.1f;
	}
	else if (inWall)
	{
		flapTime += flapSpeed * 0.5f;
	}
	else
	{
		flapTime += flapSpeed;
	}

	yRot = Mth::wrapDegrees(yRot);

	if (posPointer < 0)
	{
		for (int i = 0; i < positionsLength; i++)
		{
			positions[i][0] = yRot;
			positions[i][1] = y;
		}
	}

	if (++posPointer == positionsLength) posPointer = 0;
	positions[posPointer][0] = yRot;
	positions[posPointer][1] = y;


	if (level->isClientSide)
	{
		if (lSteps > 0)
		{
			double xt = x + (lx - x) / lSteps;
			double yt = y + (ly - y) / lSteps;
			double zt = z + (lz - z) / lSteps;

			// 4J Stu - The movement is so small that this head animation doesn't look good
			//if( getSynchedAction() == e_EnderdragonAction_Sitting_Flaming )
			//{
			//	double yrd = lyr - (yRot + m_headYRot);
			//	while (yrd < -180)
			//		yrd += 360;
			//	while (yrd >= 180)
			//		yrd -= 360;

			//	m_headYRot += (yrd) / lSteps;
			//}
			//else
			{
				double yrd = Mth::wrapDegrees(lyr - yRot);

				m_headYRot = 0.0;
				yRot += (yrd) / lSteps;
			}
			xRot += (lxr - xRot) / lSteps;

			lSteps--;
			this->setPos(xt, yt, zt);
			this->setRot(yRot, xRot);

			/*
			* List<AABB> collisions = level.getCubes(this, bb.shrink(1 / 32.0, 0, 1 /
			* 32.0)); if (collisions.size() > 0) { double yTop = 0; for (int i = 0; i <
			* collisions.size(); i++) { AABB ab = collisions.get(i); if (ab.y1 > yTop) yTop
			* = ab.y1; } yt += yTop - bb.y0; setPos(xt, yt, zt); }
			*/

		}

		if( getSynchedAction() == e_EnderdragonAction_Landing || (getSynchedAction() == e_EnderdragonAction_Sitting_Flaming && tickCount%2==0) )
		{
			double xP = 0.0;
			double yP = 0.0;
			double zP = 0.0;
			Vec3 *v = getHeadLookVector(1); //getViewVector(1);
			//app.DebugPrintf("View vector is (%f,%f,%f) - lsteps %d\n", v->x, v->y, v->z, lSteps);
			//unsigned int d = 0;
			//for(unsigned int d = 1; d < 3; ++d)
			{
				Vec3 *vN = v->normalize();
				vN->yRot(-PI/4);
				for(unsigned int i = 0; i < 8; ++i)
				{
					if(getSynchedAction() == e_EnderdragonAction_Landing)
					{
						//for(unsigned int j = 0; j < 6; ++j)
						{
							xP = head->x;// - vN->x * d;
							yP = head->bb->y0 + head->bbHeight / 2;// - vN->y * d; //head->y + head->bbHeight / 2 + 0.5f - v->y * d;
							zP = head->z;// - vN->z * d;
							xP += (level->random->nextBoolean()?1:-1) * level->random->nextFloat()/2;
							yP += (level->random->nextBoolean()?1:-1) * level->random->nextFloat()/2;
							zP += (level->random->nextBoolean()?1:-1) * level->random->nextFloat()/2;
							level->addParticle(eParticleType_dragonbreath, xP, yP, zP, (-vN->x * 0.08) + xd, (-vN->y * 0.3) + yd, (-vN->z * 0.08) + zd);
						}
					}
					else
					{
						double yVelocity = 0.6;
						double xzVelocity = 0.08;
						for(unsigned int j = 0; j < 6; ++j)
						{
							xP = head->x;// - vN->x * d;
							yP = head->bb->y0 + head->bbHeight / 2;// - vN->y * d; //head->y + head->bbHeight / 2 + 0.5f - v->y * d;
							zP = head->z;// - vN->z * d;
							xP += (level->random->nextBoolean()?1:-1) * level->random->nextFloat()/2;
							yP += (level->random->nextBoolean()?1:-1) * level->random->nextFloat()/2;
							zP += (level->random->nextBoolean()?1:-1) * level->random->nextFloat()/2;
							level->addParticle(eParticleType_dragonbreath, xP, yP, zP, -vN->x * xzVelocity*j, -vN->y * yVelocity, -vN->z * xzVelocity*j);
						}
					}
					vN->yRot(PI/(2*8) );
				}
			}
		}
		else if( getSynchedAction() == e_EnderdragonAction_Sitting_Attacking )
		{
			// AP - changed this to use playLocalSound because no sound could be heard with playSound (cos it's a stub function)
			level->playLocalSound(x, y, z, eSoundType_MOB_ENDERDRAGON_GROWL, 0.5f, 0.8f + random->nextFloat() * .3f, false, 100.0f);
		}
	}
	else
	{
		double xdd = xTarget - x;
		double ydd = yTarget - y;
		double zdd = zTarget - z;

		double dist = xdd * xdd + ydd * ydd + zdd * zdd;

		if( getSynchedAction() == e_EnderdragonAction_Sitting_Flaming )
		{
			--m_actionTicks;
			if(m_actionTicks <= 0)
			{
				if( m_flameAttacks >= SITTING_FLAME_ATTACKS_COUNT)
				{
					setSynchedAction(e_EnderdragonAction_Takeoff);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
					app.DebugPrintf("Dragon action is now: Takeoff\n");
#endif
					newTarget = true;
				}
				else
				{
					setSynchedAction(e_EnderdragonAction_Sitting_Scanning);
					attackTarget = level->getNearestPlayer( shared_from_this(), SITTING_ATTACK_VIEW_RANGE, SITTING_ATTACK_Y_VIEW_RANGE );
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
					app.DebugPrintf("Dragon action is now: SittingScanning\n");
#endif
				}
			}
		}
		else if( getSynchedAction() == e_EnderdragonAction_Sitting_Scanning )
		{
			attackTarget = level->getNearestPlayer( shared_from_this(), SITTING_ATTACK_VIEW_RANGE, SITTING_ATTACK_Y_VIEW_RANGE );

			++m_actionTicks;
			if( attackTarget != NULL )
			{
				if(m_actionTicks > SITTING_SCANNING_IDLE_TICKS/4)
				{
					setSynchedAction(e_EnderdragonAction_Sitting_Attacking);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
					app.DebugPrintf("Dragon action is now: SittingAttacking\n");
#endif
					m_actionTicks = ATTACK_TICKS;
				}
			}
			else
			{
				if(m_actionTicks >= SITTING_SCANNING_IDLE_TICKS)
				{
					setSynchedAction(e_EnderdragonAction_Takeoff);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
					app.DebugPrintf("Dragon action is now: Takeoff\n");
#endif
					newTarget = true;
				}
			}
		}
		else if( getSynchedAction() == e_EnderdragonAction_Sitting_Attacking )
		{
			--m_actionTicks;
			if(m_actionTicks <= 0)
			{
				++m_flameAttacks;
				setSynchedAction(e_EnderdragonAction_Sitting_Flaming);
				attackTarget = level->getNearestPlayer( shared_from_this(), SITTING_ATTACK_VIEW_RANGE, SITTING_ATTACK_Y_VIEW_RANGE );
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
				app.DebugPrintf("Dragon action is now: SittingFlaming\n");
#endif
				m_actionTicks = FLAME_TICKS;
			}
		}
		else if( !newTarget && getSynchedAction() == e_EnderdragonAction_Takeoff)
		{
			int eggHeight = level->getTopSolidBlock(PODIUM_X_POS,PODIUM_Z_POS); //level->getHeightmap(4,4);

			float dist = distanceToSqr(PODIUM_X_POS, eggHeight, PODIUM_Z_POS);
			if(dist > (10.0f * 10.0f) )
			{
				setSynchedAction(e_EnderdragonAction_HoldingPattern);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
				app.DebugPrintf("Dragon action is now: HoldingPattern\n");
#endif
			}
		}
		else if (newTarget || ( (getSynchedAction() != e_EnderdragonAction_Landing && dist < 10 * 10) || dist < 1) || dist > 150 * 150 || horizontalCollision || verticalCollision)
		{
			findNewTarget();
		}

		if( getSynchedAction() == e_EnderdragonAction_Sitting_Flaming || getSynchedAction() == e_EnderdragonAction_Landing )
		{
			if( m_actionTicks < (FLAME_TICKS - 10) )
			{
				vector<shared_ptr<Entity> > *targets = level->getEntities(shared_from_this(), m_acidArea);

				if ( targets )
				{
					for (auto& it : *targets )
					{
						if ( it->instanceof(eTYPE_LIVINGENTITY))
						{
							//app.DebugPrintf("Attacking entity with acid\n");
							shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(it);
							e->hurt(DamageSource::dragonbreath, 2);
						}
					}
				}
			}
		}
		if( getSynchedAction() == e_EnderdragonAction_Sitting_Flaming )
		{
			// No movement
		}
		else if( getSynchedAction() == e_EnderdragonAction_Sitting_Scanning )
		{
			if( attackTarget != NULL)
			{
				Vec3 *aim = Vec3::newTemp((attackTarget->x - x), 0, (attackTarget->z - z))->normalize();
				Vec3 *dir = Vec3::newTemp(sin(yRot * PI / 180), 0, -cos(yRot * PI / 180))->normalize();
				float dot = (float)dir->dot(aim);
				float angleDegs = acos(dot)*180/PI;
				angleDegs = angleDegs + 0.5f;

				if( angleDegs < 0 || angleDegs > 10 )
				{
					double xdd = attackTarget->x - head->x;
					//double ydd = (attackTarget->bb->y0 + attackTarget->bbHeight / 2) - (head->y + head->bbHeight / 2);
					double zdd = attackTarget->z - head->z;

					double yRotT = (180) - atan2(xdd, zdd) * 180 / PI;
					double yRotD = Mth::wrapDegrees(yRotT - yRot);

					if (yRotD > 50) yRotD = 50;
					if (yRotD < -50) yRotD = -50;

					double xd = xTarget - x;
					double zd = zTarget - z;
					yRotA *= 0.80f;

					float rotSpeed = sqrt(xd * xd + zd * zd) * 1 + 1;
					double distToTarget = sqrt(xd * xd + zd * zd) * 1 + 1;
					if (distToTarget > 40) distToTarget = 40;
					yRotA += yRotD * ((0.7f / distToTarget) / rotSpeed);
					yRot += yRotA;
				}
				else
				{
					//setSynchedAction(e_EnderdragonAction_Sitting_Flaming);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
					//app.DebugPrintf("Dragon action is now : SittingFlaming\n");
#endif
					//m_actionTicks = FLAME_TICKS;
				}
			}
			else
			{
				//setSynchedAction(e_EnderdragonAction_Sitting_Flaming);
				//app.DebugPrintf("Dragon action is now : SittingFlaming\n");
				//m_actionTicks = 0;
			}
		}
		else if( getSynchedAction() == e_EnderdragonAction_Sitting_Attacking )
		{

		}
		else
		{
			// 			double xTargetO = xTarget;
			// 			double yTargetO = yTarget;
			// 			double zTargetO = zTarget;
			if (getSynchedAction() == e_EnderdragonAction_StrafePlayer && attackTarget != NULL && m_currentPath != NULL && m_currentPath->isDone())
			{
				xTarget = attackTarget->x;
				zTarget = attackTarget->z;

				double xd = xTarget - x;
				double zd = zTarget - z;
				double sd = sqrt(xd * xd + zd * zd);
				double ho = 0.4f + sd / 80.0f - 1;
				if (ho > 10) ho = 10;
				yTarget = attackTarget->bb->y0 + ho;
			}
			else
			{
				//xTarget += random->nextGaussian() * 2;
				//zTarget += random->nextGaussian() * 2;
			}
			ydd = ydd / (sqrt(xdd * xdd + zdd * zdd));
			float max = 0.6f;
			if(getSynchedAction() == e_EnderdragonAction_Landing) max = 1.5f;
			if (ydd < -max) ydd = -max;
			if (ydd > max) ydd = max;
			yd += (ydd) * 0.1f;
			while (yRot < -180)
				yRot += 180 * 2;
			while (yRot >= 180)
				yRot -= 180 * 2;


			double yRotT = (180) - atan2(xdd, zdd) * 180 / PI;
			double yRotD = yRotT - yRot;
			while (yRotD < -180)
				yRotD += 180 * 2;
			while (yRotD >= 180)
				yRotD -= 180 * 2;


			if (yRotD > 50) yRotD = 50;
			if (yRotD < -50) yRotD = -50;

			Vec3 *aim = Vec3::newTemp((xTarget - x), (yTarget - y), (zTarget - z))->normalize();
			Vec3 *dir = Vec3::newTemp(sin(yRot * PI / 180), yd, -cos(yRot * PI / 180))->normalize();
			float dot = (float) (dir->dot(aim) + 0.5f) / 1.5f;
			if (dot < 0) dot = 0;

			yRotA *= 0.80f;

			float rotSpeed = sqrt(xd * xd + zd * zd) * 1 + 1;
			double distToTarget = sqrt(xd * xd + zd * zd) * 1 + 1;
			if (distToTarget > 40) distToTarget = 40;
			if(getSynchedAction() == e_EnderdragonAction_Landing)
			{
				yRotA += yRotD * (distToTarget / rotSpeed);
			}
			else
			{
				yRotA += yRotD * ((0.7f / distToTarget) / rotSpeed);
			}
			yRot += yRotA * 0.1f;

			float span = (float) (2.0f / (distToTarget + 1));
			float speed = 0.06f;
			moveRelative(0, -1, speed * (dot * span + (1 - span)));
			if (inWall)
			{
				move(xd * 0.8f, yd * 0.8f, zd * 0.8f);
			}
			else
			{
				move(xd, yd, zd);

			}

			Vec3 *actual = Vec3::newTemp(xd, yd, zd)->normalize();
			float slide = (float) (actual->dot(dir) + 1) / 2.0f;
			slide = 0.8f + 0.15f * slide;


			xd *= slide;
			zd *= slide;
			yd *= 0.91f;
		}
	}

	yBodyRot = yRot;

	head->bbWidth = head->bbHeight = 1; // 4J Stu - Replaced what was "head" with "neck" //3;
	neck->bbWidth = neck->bbHeight = 3;
	tail1->bbWidth = tail1->bbHeight = 2;
	tail2->bbWidth = tail2->bbHeight = 2;
	tail3->bbWidth = tail3->bbHeight = 2;
	body->bbHeight = 3;
	body->bbWidth = 5;
	wing1->bbHeight = 2;
	wing1->bbWidth = 4;
	wing2->bbHeight = 3;
	wing2->bbWidth = 4;

	//double latencyPosAcomponents[3],latencyPosBcomponents[3];
	//doubleArray latencyPosA = doubleArray(latencyPosAcomponents,3);
	//doubleArray latencyPosB = doubleArray(latencyPosBcomponents,3);
	//getLatencyPos(latencyPosA, 5, 1);
	//getLatencyPos(latencyPosB, 10, 1);

	//float tilt = (float) (latencyPosA[1] - latencyPosB[1]) * 10 / 180.0f * PI;
	float tilt = (float) getTilt(1) / 180.0f * PI;
	float ccTilt = cos(tilt);

	// 4J Stu - ssTilt was negative sin(tilt), but this causes the bounding boxes of the parts to head in the wrong y direction
	// i.e. head moves up when tilting forward, and down when tilting backwards
	float ssTilt = sin(tilt);


	float rot1 = yRot * PI / 180;
	float ss1 = sin(rot1);
	float cc1 = cos(rot1);

	body->tick();
	body->moveTo(x + ss1 * 0.5f, y, z - cc1 * 0.5f, 0, 0);
	wing1->tick();
	wing1->moveTo(x + cc1 * 4.5f, y + 2, z + ss1 * 4.5f, 0, 0);
	wing2->tick();
	wing2->moveTo(x - cc1 * 4.5f, y + 2, z - ss1 * 4.5f, 0, 0);

	if (!level->isClientSide) checkAttack();
	if (!level->isClientSide && hurtDuration == 0)
	{
		knockBack(level->getEntities(shared_from_this(), wing1->bb->grow(4, 2, 4)->move(0, -2, 0)));
		knockBack(level->getEntities(shared_from_this(), wing2->bb->grow(4, 2, 4)->move(0, -2, 0)));
		hurt(level->getEntities(shared_from_this(), neck->bb->grow(1, 1, 1)));
		hurt(level->getEntities(shared_from_this(), head->bb->grow(1, 1, 1)));
	}

	double p1components[3];
	doubleArray p1 = doubleArray(p1components, 3);
	getLatencyPos(p1, 5, 1);

	{
		//double p0components[3];
		//doubleArray p0 = doubleArray(p0components, 3);
		//getLatencyPos(p0, 0, 1);

		double yRotDiff = getHeadYRotDiff(1);

		float ss = sin((yRot + yRotDiff) * PI / 180 - yRotA * 0.01f);
		float cc = cos((yRot + yRotDiff) * PI / 180 - yRotA * 0.01f);
		head->tick();
		neck->tick();
		double yOffset = getHeadYOffset(1); // (p0[1] - p1[1]) * 1

		// 4J Stu - Changed the head entity to only be the head, and not include the neck parts
		head->moveTo(x + ss * 6.5f * ccTilt, y + yOffset + ssTilt * 6.5f, z - cc * 6.5f * ccTilt, 0, 0);

		// Neck position is where the java code used to move the "head" object which was head and neck
		neck->moveTo(x + ss * 5.5f * ccTilt, y + yOffset + ssTilt * 5.5f, z - cc * 5.5f * ccTilt, 0, 0);

		double acidX = x + ss * 9.5f * ccTilt;
		double acidY = y + yOffset + ssTilt * 10.5f;
		double acidZ = z - cc * 9.5f * ccTilt;
		m_acidArea->set(acidX - 5, acidY - 17, acidZ - 5, acidX + 5, acidY + 4, acidZ + 5);

		//app.DebugPrintf("\nDragon is %s, yRot = %f, yRotA = %f, ss = %f, cc = %f, ccTilt = %f\n",level->isClientSide?"client":"server", yRot, yRotA, ss, cc, ccTilt);
		//app.DebugPrintf("Body (%f,%f,%f) to (%f,%f,%f)\n", body->bb->x0, body->bb->y0, body->bb->z0, body->bb->x1, body->bb->y1, body->bb->z1);
		//app.DebugPrintf("Neck (%f,%f,%f) to (%f,%f,%f)\n", neck->bb->x0, neck->bb->y0, neck->bb->z0, neck->bb->x1, neck->bb->y1, neck->bb->z1);
		//app.DebugPrintf("Head (%f,%f,%f) to (%f,%f,%f)\n", head->bb->x0, head->bb->y0, head->bb->z0, head->bb->x1, head->bb->y1, head->bb->z1);
		//app.DebugPrintf("Acid (%f,%f,%f) to (%f,%f,%f)\n\n", m_acidArea->x0, m_acidArea->y0, m_acidArea->z0, m_acidArea->x1, m_acidArea->y1, m_acidArea->z1);
	}

	// Curls/straightens the tail
	for (int i = 0; i < 3; i++)
	{
		shared_ptr<MultiEntityMobPart> part = nullptr;

		if (i == 0) part = tail1;
		if (i == 1) part = tail2;
		if (i == 2) part = tail3;

		double p0components[3];
		doubleArray p0 = doubleArray(p0components, 3);
		getLatencyPos(p0, 12 + i * 2, 1);

		float rot = yRot * PI / 180 + rotWrap(p0[0] - p1[0]) * PI / 180 * (1);
		float ss = sin(rot);
		float cc = cos(rot);

		float dd1 = 1.5f;
		float dd = (i + 1) * 2.0f;
		part->tick();
		part->moveTo(x - (ss1 * dd1 + ss * dd) * ccTilt, y + (p0[1] - p1[1]) * 1 - (dd + dd1) * ssTilt + 1.5f, z + (cc1 * dd1 + cc * dd) * ccTilt, 0, 0);
	}


	// 4J Stu - Fireball attack taken from Ghast
	if (!level->isClientSide)
	{
		double maxDist = 64.0f;
		if (getSynchedAction() == e_EnderdragonAction_StrafePlayer && attackTarget != NULL && attackTarget->distanceToSqr(shared_from_this()) < maxDist * maxDist) 
		{
			if (this->canSee(attackTarget))
			{
				m_fireballCharge++;
				Vec3 *aim = Vec3::newTemp((attackTarget->x - x), 0, (attackTarget->z - z))->normalize();
				Vec3 *dir = Vec3::newTemp(sin(yRot * PI / 180), 0, -cos(yRot * PI / 180))->normalize();
				float dot = (float)dir->dot(aim);
				float angleDegs = acos(dot)*180/PI;
				angleDegs = angleDegs + 0.5f;

				if (m_fireballCharge >= 20 && ( angleDegs >= 0 && angleDegs < 10 ))
				{
					double d = 1;
					Vec3 *v = getViewVector(1);
					float startingX = head->x - v->x * d;
					float startingY = head->y + head->bbHeight / 2 + 0.5f;
					float startingZ = head->z - v->z * d;

					double xdd = attackTarget->x - startingX;
					double ydd = (attackTarget->bb->y0 + attackTarget->bbHeight / 2) - (startingY + head->bbHeight / 2);
					double zdd = attackTarget->z - startingZ;

					level->levelEvent(nullptr, LevelEvent::SOUND_GHAST_FIREBALL, (int) x, (int) y, (int) z, 0);
					shared_ptr<DragonFireball> ie = shared_ptr<DragonFireball>( new DragonFireball(level, dynamic_pointer_cast<Mob>( shared_from_this() ), xdd, ydd, zdd) );
					ie->x = startingX;
					ie->y = startingY;
					ie->z = startingZ;
					level->addEntity(ie);
					m_fireballCharge = 0;

					app.DebugPrintf("Finding new target due to having fired a fireball\n");		
					if( m_currentPath != NULL )
					{
						while(!m_currentPath->isDone())
						{
							m_currentPath->next();
						}
					}
					newTarget = true;
					findNewTarget();
				}
			} 
			else 
			{
				if (m_fireballCharge > 0) m_fireballCharge--;
			}
		}
		else
		{
			if (m_fireballCharge > 0) m_fireballCharge--;
		}
	}
	// End fireball attack

	if (!level->isClientSide)
	{
		inWall = checkWalls(head->bb) | checkWalls(neck->bb) | checkWalls(body->bb);
	}
}

void EnderDragon::checkCrystals()
{
	if (nearestCrystal != NULL)
	{
		if (nearestCrystal->removed)
		{
			if (!level->isClientSide)
			{
				hurt(head, DamageSource::explosion(NULL), 10);
			}

			nearestCrystal = nullptr;
		}
		else if (tickCount % 10 == 0)
		{
			if (getHealth() < getMaxHealth()) setHealth(getHealth() + 1);
		}
	}

	if (random->nextInt(10) == 0)
	{
		float maxDist = 32;
		vector<shared_ptr<Entity> > *crystals = level->getEntitiesOfClass(typeid(EnderCrystal), bb->grow(maxDist, maxDist, maxDist));
		if ( crystals )
		{
			shared_ptr<EnderCrystal> crystal = nullptr;
			double nearest = Double::MAX_VALUE;
			for (auto& it : *crystals )
			{
				shared_ptr<EnderCrystal> ec = dynamic_pointer_cast<EnderCrystal>(it);
				double dist = ec->distanceToSqr(shared_from_this());
				if (dist < nearest)
				{
					nearest = dist;
					crystal = ec;
				}
			}
			delete crystals;
			nearestCrystal = crystal;
		}
	}
}

void EnderDragon::checkAttack()
{
	//if (tickCount % 20 == 0)
	{
		// 		Vec3 *v = getViewVector(1);
		// 		double xdd = 0;
		// 		double ydd = -1;
		// 		double zdd = 0;

		//            double x = (body.bb.x0 + body.bb.x1) / 2;
		//            double y = (body.bb.y0 + body.bb.y1) / 2 - 2;
		//            double z = (body.bb.z0 + body.bb.z1) / 2;

	}
}

void EnderDragon::knockBack(vector<shared_ptr<Entity> > *entities)
{
	double xm = (body->bb->x0 + body->bb->x1) / 2;
	//        double ym = (body.bb.y0 + body.bb.y1) / 2;
	double zm = (body->bb->z0 + body->bb->z1) / 2;

	if ( entities )
	{
		for ( auto& it : *entities )
		{
			if (it->instanceof(eTYPE_LIVINGENTITY))
			{
				shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(it);
				double xd = e->x - xm;
				double zd = e->z - zm;
				double dd = xd * xd + zd * zd;
				e->push(xd / dd * 4, 0.2f, zd / dd * 4);
			}
		}
	}
}

void EnderDragon::hurt(vector<shared_ptr<Entity> > *entities)
{
	if ( entities )
	{
		for ( auto& it : *entities )
		{

			if ( it->instanceof(eTYPE_LIVINGENTITY))
			{
				shared_ptr<LivingEntity> e = dynamic_pointer_cast<LivingEntity>(it);
				DamageSource* damageSource = DamageSource::mobAttack(dynamic_pointer_cast<LivingEntity>(shared_from_this()));
				if ( damageSource )
				{
					e->hurt(damageSource, 10);
					delete damageSource;
				}
			}
		}
	}
}

void EnderDragon::findNewTarget()
{
	shared_ptr<Player> playerNearestToEgg = nullptr;

	// Update current action
	switch(getSynchedAction())
	{
	case e_EnderdragonAction_Takeoff:
	case e_EnderdragonAction_HoldingPattern:
		{
			if(!newTarget && m_currentPath != NULL && m_currentPath->isDone())
			{
				// Distance is 64, which is the radius of the circle
				int eggHeight = max(level->seaLevel + 5, level->getTopSolidBlock(PODIUM_X_POS,PODIUM_Z_POS)); //level->getHeightmap(4,4);
				playerNearestToEgg = level->getNearestPlayer(PODIUM_X_POS, eggHeight, PODIUM_Z_POS, 64.0);
				double dist = 64.0f;
				if(playerNearestToEgg != NULL)
				{
					dist = playerNearestToEgg->distanceToSqr(PODIUM_X_POS, eggHeight, PODIUM_Z_POS);
					dist /= (8*8*8);
				}
				//app.DebugPrintf("Adjusted dist is %f\n", dist);

				if( random->nextInt(m_remainingCrystalsCount + 3) == 0 )
				{
					setSynchedAction(e_EnderdragonAction_LandingApproach);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
					app.DebugPrintf("Dragon action is now: LandingApproach\n");
#endif
				}
				// More likely to strafe a player if they are close to the egg, or there are not many crystals remaining
				else if( playerNearestToEgg != NULL && (random->nextInt( abs(dist) + 2 ) == 0 || random->nextInt( m_remainingCrystalsCount + 2 ) == 0) )
				{
					setSynchedAction(e_EnderdragonAction_StrafePlayer);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
					app.DebugPrintf("Dragon action is now: StrafePlayer\n");
#endif
				}
			}
		}
		break;
	case e_EnderdragonAction_StrafePlayer:
		// Always return to the holding pattern after strafing
		if(m_currentPath == NULL || (m_currentPath->isDone() && newTarget) )
		{
			setSynchedAction(e_EnderdragonAction_HoldingPattern);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
			app.DebugPrintf("Dragon action is now: HoldingPattern\n");
#endif
		}
		break;
	case e_EnderdragonAction_Landing:
		//		setSynchedAction(e_EnderdragonAction_Sitting_Flaming);
		//#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
		//		app.DebugPrintf("Dragon action is now: SittingFlaming\n");
		//#endif
		//		m_actionTicks = FLAME_TICKS;

		m_flameAttacks = 0;
		setSynchedAction(e_EnderdragonAction_Sitting_Scanning);
		attackTarget = level->getNearestPlayer( shared_from_this(), SITTING_ATTACK_VIEW_RANGE, SITTING_ATTACK_Y_VIEW_RANGE );
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
		app.DebugPrintf("Dragon action is now: SittingScanning\n");
#endif
		m_actionTicks = 0;
		break;
	};

	newTarget = false;

	//if (random->nextInt(2) == 0 && level->players.size() > 0)
	if(getSynchedAction() == e_EnderdragonAction_StrafePlayer && playerNearestToEgg != NULL)
	{
		attackTarget = playerNearestToEgg;
		strafeAttackTarget();
	}
	else if(getSynchedAction() == e_EnderdragonAction_LandingApproach)
	{
		// Generate a new path if we don't currently have one
		if( m_currentPath == NULL || m_currentPath->isDone() )
		{
			int currentNodeIndex = findClosestNode();

			// To get the angle to the player correct when landing, head to a node diametrically opposite the player, then swoop in to 4,4
			int eggHeight = max( level->seaLevel + 5, level->getTopSolidBlock(PODIUM_X_POS,PODIUM_Z_POS) ); //level->getHeightmap(4,4);
			playerNearestToEgg = level->getNearestPlayer(PODIUM_X_POS, eggHeight, PODIUM_Z_POS, 128.0);

			int targetNodeIndex = 0 ;
			if(playerNearestToEgg != NULL)
			{
				Vec3 *aim = Vec3::newTemp(playerNearestToEgg->x, 0, playerNearestToEgg->z)->normalize();
				//app.DebugPrintf("Final marker node near (%f,%d,%f)\n", -aim->x*40,105,-aim->z*40 );
				targetNodeIndex = findClosestNode(-aim->x*40,105.0,-aim->z*40);
			}
			else
			{
				targetNodeIndex = findClosestNode(40.0, eggHeight, 0.0);
			}
			Node finalNode(PODIUM_X_POS, eggHeight, PODIUM_Z_POS);

			if(m_currentPath != NULL) delete m_currentPath;
			m_currentPath = findPath(currentNodeIndex,targetNodeIndex, &finalNode);

			// Always skip the first node (as that's where we are already)
			if(m_currentPath != NULL) m_currentPath->next();
		}

		m_actionTicks = 0;

		navigateToNextPathNode();

		if(m_currentPath != NULL && m_currentPath->isDone())
		{					
			setSynchedAction(e_EnderdragonAction_Landing);
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
			app.DebugPrintf("Dragon action is now: Landing\n");
#endif
		}
	}
	else if(getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Attacking ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Scanning)
	{
		// Does no movement
	}
	else
	{
		// Default is e_EnderdragonAction_HoldingPattern
		// Generate a new path if we don't currently have one
		if(m_currentPath == NULL || m_currentPath->isDone() )
		{
			int currentNodeIndex = findClosestNode();
			int targetNodeIndex = currentNodeIndex;
			//if(random->nextInt(4) == 0) m_holdingPatternClockwise = !m_holdingPatternClockwise;

			if( getSynchedAction() == e_EnderdragonAction_Takeoff)
			{
				Vec3 *v = getHeadLookVector(1);
				targetNodeIndex = findClosestNode(-v->x*40,105.0,-v->z*40);
			}
			else
			{
				if(random->nextInt(8) == 0)
				{
					m_holdingPatternClockwise = !m_holdingPatternClockwise;
					targetNodeIndex = targetNodeIndex + 6;
				}

				if(m_holdingPatternClockwise) targetNodeIndex = targetNodeIndex + 1;
				else targetNodeIndex = targetNodeIndex - 1;
			}

			if(m_remainingCrystalsCount <= 0)
			{
				// If no crystals left, navigate only between nodes 12-19
				targetNodeIndex -= 12;
				targetNodeIndex = targetNodeIndex&7;	// 4J-RR - was %8, but that could create a result of -1 here when targetNodeIndex was 11
				targetNodeIndex += 12;
			}
			else
			{
				// If crystals are left, navigate only between nodes 0-11
				targetNodeIndex = targetNodeIndex%12;
				if(targetNodeIndex < 0) targetNodeIndex += 12;
			}

			if(m_currentPath != NULL) delete m_currentPath;
			m_currentPath = findPath(currentNodeIndex,targetNodeIndex);		

			// Always skip the first node (as that's where we are already)
			if(m_currentPath != NULL) m_currentPath->next();
		}

		navigateToNextPathNode();

		if(getSynchedAction() != e_EnderdragonAction_StrafePlayer) attackTarget = nullptr;
	}
}

float EnderDragon::rotWrap(double d)
{
	while (d >= 180)
		d -= 360;
	while (d < -180)
		d += 360;
	return (float) d;
}

bool EnderDragon::checkWalls(AABB *bb)
{
	int x0 = Mth::floor(bb->x0);
	int y0 = Mth::floor(bb->y0);
	int z0 = Mth::floor(bb->z0);
	int x1 = Mth::floor(bb->x1);
	int y1 = Mth::floor(bb->y1);
	int z1 = Mth::floor(bb->z1);
	bool hitWall = false;
	bool destroyedTile = false;
	for (int x = x0; x <= x1; x++)
	{
		for (int y = y0; y <= y1; y++)
		{
			for (int z = z0; z <= z1; z++)
			{
				int t = level->getTile(x, y, z);
				// 4J Stu - Don't remove fire
				if (t == 0 || t == Tile::fire_Id)
				{

				}
				else if (t == Tile::obsidian_Id || t == Tile::endStone_Id || t == Tile::unbreakable_Id || !level->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING))
				{
					hitWall = true;
				}
				else
				{
					destroyedTile = level->removeTile(x, y, z) || destroyedTile;
				}
			}
		}
	}

	if (destroyedTile)
	{
		double x = bb->x0 + (bb->x1 - bb->x0) * random->nextFloat();
		double y = bb->y0 + (bb->y1 - bb->y0) * random->nextFloat();
		double z = bb->z0 + (bb->z1 - bb->z0) * random->nextFloat();
		level->addParticle(eParticleType_largeexplode, x, y, z, 0, 0, 0);
	}

	return hitWall;
}

bool EnderDragon::hurt(shared_ptr<MultiEntityMobPart> MultiEntityMobPart, DamageSource *source, float damage)
{
	if (MultiEntityMobPart != head)
	{
		damage = damage / 4 + 1;
	}

	//float rot1 = yRot * PI / 180;
	//float ss1 = sin(rot1);
	//float cc1 = cos(rot1);

	//xTarget = x + ss1 * 5 + (random->nextFloat() - 0.5f) * 2;
	//yTarget = y + random->nextFloat() * 3 + 1;
	//zTarget = z - cc1 * 5 + (random->nextFloat() - 0.5f) * 2;
	//attackTarget = NULL;

	if ( source->getEntity() != NULL && source->getEntity()->instanceof(eTYPE_PLAYER) || source->isExplosion() )
	{
		int healthBefore = getHealth();
		reallyHurt(source, damage);

		//if(!level->isClientSide) app.DebugPrintf("Health is now %d\n", health);
		if( getHealth() <= 0 &&
			!(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
			getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
			getSynchedAction() == e_EnderdragonAction_Sitting_Attacking) )
		{
			setHealth(1);

			if( setSynchedAction(e_EnderdragonAction_LandingApproach) )
			{
				if( m_currentPath != NULL )
				{
					while(!m_currentPath->isDone())
					{
						m_currentPath->next();
					}
				}
				app.DebugPrintf("Dragon should be dead, so landing.\n");
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
				app.DebugPrintf("Dragon action is now: LandingApproach\n");
#endif
				findNewTarget();
			}
		}

		if(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
			getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
			getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
		{
			m_sittingDamageReceived += healthBefore - getHealth();

			if(m_sittingDamageReceived > (SITTING_ALLOWED_DAMAGE_PERCENTAGE*getMaxHealth() ) )
			{
				m_sittingDamageReceived = 0;
				setSynchedAction(e_EnderdragonAction_Takeoff);
				newTarget = true;
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
				app.DebugPrintf("Dragon action is now: Takeoff\n");
#endif
			}
		}
	}
	return true;
}

bool EnderDragon::hurt(DamageSource *source, float damage)
{
	return false;
}

bool EnderDragon::reallyHurt(DamageSource *source, float damage)
{
	return Mob::hurt(source, damage);
}

void EnderDragon::tickDeath()
{
	if(	getSynchedAction() != e_EnderdragonAction_Sitting_Flaming &&
		getSynchedAction() != e_EnderdragonAction_Sitting_Scanning &&
		getSynchedAction() != e_EnderdragonAction_Sitting_Attacking)
	{
		if(!level->isClientSide) setHealth(1);
		return;
	}

	dragonDeathTime++;
	if (dragonDeathTime >= 180 && dragonDeathTime <= 200)
	{
		float xo = (random->nextFloat() - 0.5f) * 8;
		float yo = (random->nextFloat() - 0.5f) * 4;
		float zo = (random->nextFloat() - 0.5f) * 8;
		level->addParticle(eParticleType_hugeexplosion, x + xo, y + 2 + yo, z + zo, 0, 0, 0);
	}
	if (!level->isClientSide)
	{
		if (dragonDeathTime > 150 && dragonDeathTime % 5 == 0)
		{
			int xpCount = 1000;
			while (xpCount > 0)
			{
				int newCount = ExperienceOrb::getExperienceValue(xpCount);
				xpCount -= newCount;
				level->addEntity(shared_ptr<ExperienceOrb>( new ExperienceOrb(level, x, y, z, newCount) ));
			}
		}
		if (dragonDeathTime == 1) 
		{
			level->globalLevelEvent(LevelEvent::SOUND_DRAGON_DEATH, (int) x, (int) y, (int) z, 0);
		}
	}
	move(0, 0.1f, 0);
	yBodyRot = yRot += 20.0f;

	if (dragonDeathTime == 200 && !level->isClientSide)
	{
		//level->levelEvent(NULL, LevelEvent::ENDERDRAGON_KILLED, (int) x, (int) y, (int) z, 0);

		int xpCount = 2000;
		while (xpCount > 0)
		{
			int newCount = ExperienceOrb::getExperienceValue(xpCount);
			xpCount -= newCount;
			level->addEntity(shared_ptr<ExperienceOrb>( new ExperienceOrb(level, x, y, z, newCount)));
		}
		int xo = 5 + random->nextInt(2) * 2 - 1;
		int zo = 5 + random->nextInt(2) * 2 - 1;
		if (random->nextInt(2) == 0)
		{
			xo = 0;
		}
		else
		{
			zo = 0;
		}
		// 4J-PB changed to center this between the pillars
		spawnExitPortal(0,0);//Mth::floor(x), Mth::floor(z));
		remove();
	}
}

void EnderDragon::spawnExitPortal(int x, int z)
{
	int y = level->seaLevel;

	TheEndPortal::allowAnywhere(true);

	int r = 4;
	for (int yy = y - 1; yy <= y + 32; yy++)
	{
		for (int xx = x - r; xx <= x + r; xx++)
		{
			for (int zz = z - r; zz <= z + r; zz++)
			{
				double xd = xx - x;
				double zd = zz - z;
				double d = sqrt(xd * xd + zd * zd);
				if (d <= r - 0.5)
				{
					if (yy < y)
					{
						if (d > r - 1 - 0.5)
						{
						}
						else
						{
							level->setTileAndUpdate(xx, yy, zz, Tile::unbreakable_Id);
						}
					}
					else if (yy > y)
					{
						level->setTileAndUpdate(xx, yy, zz, 0);
					}
					else
					{
						if (d > r - 1 - 0.5)
						{
							level->setTileAndUpdate(xx, yy, zz, Tile::unbreakable_Id);
						}
						else
						{
							level->setTileAndUpdate(xx, yy, zz, Tile::endPortalTile_Id);
						}
					}
				}
			}
		}
	}

	level->setTileAndUpdate(x, y + 0, z, Tile::unbreakable_Id);
	level->setTileAndUpdate(x, y + 1, z, Tile::unbreakable_Id);
	level->setTileAndUpdate(x, y + 2, z, Tile::unbreakable_Id);
	level->setTileAndUpdate(x - 1, y + 2, z, Tile::torch_Id);
	level->setTileAndUpdate(x + 1, y + 2, z, Tile::torch_Id);
	level->setTileAndUpdate(x, y + 2, z - 1, Tile::torch_Id);
	level->setTileAndUpdate(x, y + 2, z + 1, Tile::torch_Id);
	level->setTileAndUpdate(x, y + 3, z, Tile::unbreakable_Id);
	level->setTileAndUpdate(x, y + 4, z, Tile::dragonEgg_Id);

	// 4J-PB - The podium can be floating with nothing under it, so put some whiteStone under it if this is the case
	for (int yy = y - 5; yy < y - 1; yy++)
	{
		for (int xx = x - (r - 1); xx <= x + (r - 1); xx++)
		{
			for (int zz = z - (r - 1); zz <= z + (r - 1); zz++)
			{
				if(level->isEmptyTile(xx,yy,zz))
				{
					level->setTileAndUpdate(xx, yy, zz, Tile::endStone_Id);
				}
			}
		}
	}

	TheEndPortal::allowAnywhere(false);
}

void EnderDragon::checkDespawn()
{
}

vector<shared_ptr<Entity> > *EnderDragon::getSubEntities()
{
	return &subEntities;
}

bool EnderDragon::isPickable()
{
	return false;
}

Level *EnderDragon::getLevel()
{
	return level;
}

int EnderDragon::getAmbientSound()
{
	return eSoundType_MOB_ENDERDRAGON_GROWL; //"mob.enderdragon.growl";
}

int EnderDragon::getHurtSound()
{
	return eSoundType_MOB_ENDERDRAGON_HIT; //"mob.enderdragon.hit";
}

float EnderDragon::getSoundVolume()
{
	return 5;
}

// 4J Added for new dragon behaviour
bool EnderDragon::setSynchedAction(EEnderdragonAction action, bool force /*= false*/)
{
	bool validTransition = false;
	// Check if this is a valid state transition
	switch(getSynchedAction())
	{
	case e_EnderdragonAction_HoldingPattern:
		switch(action)
		{
		case e_EnderdragonAction_StrafePlayer:
		case e_EnderdragonAction_LandingApproach:
			validTransition = true;
			break;
		};
		break;
	case e_EnderdragonAction_StrafePlayer:
		switch(action)
		{
		case e_EnderdragonAction_HoldingPattern:
		case e_EnderdragonAction_LandingApproach:
			validTransition = true;
			break;
		};
		break;
	case e_EnderdragonAction_LandingApproach:
		switch(action)
		{
		case e_EnderdragonAction_Landing:
			validTransition = true;
			break;
		};
		break;
	case e_EnderdragonAction_Landing:
		switch(action)
		{
		case e_EnderdragonAction_Sitting_Flaming:
		case e_EnderdragonAction_Sitting_Scanning:
			validTransition = true;
			break;
		};
		break;
	case e_EnderdragonAction_Takeoff:
		switch(action)
		{
		case e_EnderdragonAction_HoldingPattern:
			validTransition = true;
			break;
		};
		break;
	case e_EnderdragonAction_Sitting_Flaming:
		switch(action)
		{
		case e_EnderdragonAction_Sitting_Scanning:
		case e_EnderdragonAction_Sitting_Attacking:
		case e_EnderdragonAction_Takeoff:
			validTransition = true;
			break;
		};
		break;
	case e_EnderdragonAction_Sitting_Scanning:
		switch(action)
		{
		case e_EnderdragonAction_Sitting_Flaming:
		case e_EnderdragonAction_Sitting_Attacking:
		case e_EnderdragonAction_Takeoff:
			validTransition = true;
			break;
		};
		break;
	case e_EnderdragonAction_Sitting_Attacking:
		switch(action)
		{
		case e_EnderdragonAction_Sitting_Flaming:
		case e_EnderdragonAction_Sitting_Scanning:
		case e_EnderdragonAction_Takeoff:
			validTransition = true;
			break;
		};
		break;
	};

	if( force || validTransition )
	{
		entityData->set(DATA_ID_SYNCHED_ACTION, action);
	}
	else
	{
		app.DebugPrintf("EnderDragon: Invalid state transition from %d to %d\n", getSynchedAction(), action);
	}

	return force || validTransition;
}

EnderDragon::EEnderdragonAction EnderDragon::getSynchedAction()
{
	return (EEnderdragonAction)entityData->getInteger(DATA_ID_SYNCHED_ACTION);
}

void EnderDragon::handleCrystalDestroyed(DamageSource *source)
{
	AABB *tempBB = AABB::newTemp(PODIUM_X_POS,84.0,PODIUM_Z_POS,PODIUM_X_POS+1.0,85.0,PODIUM_Z_POS+1.0);
	vector<shared_ptr<Entity> > *crystals = level->getEntitiesOfClass(typeid(EnderCrystal), tempBB->grow(48, 40, 48));
	m_remainingCrystalsCount = (int)crystals->size() - 1;
	if(m_remainingCrystalsCount < 0) m_remainingCrystalsCount = 0;
	delete crystals;

	app.DebugPrintf("Crystal count is now %d\n",m_remainingCrystalsCount);

	//--m_remainingCrystalsCount;

	if(m_remainingCrystalsCount%2 == 0)
	{
		if(setSynchedAction(e_EnderdragonAction_LandingApproach))
		{
			if( m_currentPath != NULL )
			{
				while(!m_currentPath->isDone())
				{
					m_currentPath->next();
				}
			}
			m_actionTicks = 1;
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
			app.DebugPrintf("Dragon action is now: LandingApproach\n");
#endif
		}
	}
	else if(source->getEntity() != NULL && source->getEntity()->instanceof(eTYPE_PLAYER))
	{
		if(setSynchedAction(e_EnderdragonAction_StrafePlayer))
		{
			attackTarget = dynamic_pointer_cast<Player>(source->getEntity());
#if PRINT_DRAGON_STATE_CHANGE_MESSAGES
			app.DebugPrintf("Dragon action is now: StrafePlayer\n");
#endif
			strafeAttackTarget();
		}
	}
}

void EnderDragon::strafeAttackTarget()
{
	app.DebugPrintf("Setting path to strafe attack target\n");
	int currentNodeIndex = findClosestNode();
	int targetNodeIndex = findClosestNode(attackTarget->x,attackTarget->y,attackTarget->z);

	int finalXTarget = attackTarget->x;
	int finalZTarget = attackTarget->z;

	double xd = finalXTarget - x;
	double zd = finalZTarget - z;
	double sd = sqrt(xd * xd + zd * zd);
	double ho = 0.4f + sd / 80.0f - 1;
	if (ho > 10) ho = 10;
	int finalYTarget = attackTarget->bb->y0 + ho;

	Node finalNode(finalXTarget, finalYTarget, finalZTarget);

	if(m_currentPath != NULL) delete m_currentPath;
	m_currentPath = findPath(currentNodeIndex,targetNodeIndex, &finalNode);

	if(m_currentPath != NULL)
	{
		// Always skip the first node (as that's where we are already)
		m_currentPath->next();

		navigateToNextPathNode();
	}
}

void EnderDragon::navigateToNextPathNode()
{
	if(m_currentPath != NULL && !m_currentPath->isDone())
	{
		Vec3 *curr = m_currentPath->currentPos();

		m_currentPath->next();
		xTarget = curr->x;

		if(getSynchedAction() == e_EnderdragonAction_LandingApproach && m_currentPath->isDone())
		{
			// When heading to the last node on the landing approach, we want the yCoord to be exact
			yTarget = curr->y;
		}
		else
		{
			do
			{
				yTarget = curr->y + random->nextFloat() * 20;
			} while( yTarget < (curr->y) );
		}
		zTarget = curr->z;
		app.DebugPrintf("Path node pos is (%f,%f,%f)\n",curr->x,curr->y,curr->z);
		app.DebugPrintf("Setting new target to (%f,%f,%f)\n",xTarget, yTarget, zTarget);
	}
}

int EnderDragon::findClosestNode()
{
	// Setup all the nodes on the first time this is called
	if(m_nodes->data[0] == NULL)
	{
		// Path nodes for navigation
		// 0 - 11 are the outer ring at 60 blocks from centre
		// 12 - 19 are the middle ring at 40 blocks from centre
		// 20 - 23 are the inner ring at 20 blocks from centre
		int nodeX=0;
		int nodeY=0;
		int nodeZ=0;
		int multiplier = 0;
		for(unsigned int i = 0; i < 24; ++i)
		{
			int yAdjustment = 5;
			multiplier = i;
			if(i < 12)
			{
				nodeX=60 * Mth::cos(2*(-PI+(PI/12)*multiplier));
				nodeZ=60 * Mth::sin(2*(-PI+(PI/12)*multiplier));
			}
			else if(i < 20)
			{
				multiplier -= 12;
				nodeX=40 * Mth::cos(2*(-PI+(PI/8)*multiplier));
				nodeZ=40 * Mth::sin(2*(-PI+(PI/8)*multiplier));
				yAdjustment += 10; // Make the target well above the top of the towers
			}
			else
			{
				multiplier -= 20;
				nodeX=20 * Mth::cos(2*(-PI+(PI/4)*multiplier));
				nodeZ=20 * Mth::sin(2*(-PI+(PI/4)*multiplier));
			}
			// Fix for #77202 - TU9: Content: Gameplay: The Ender Dragon sometimes flies through terrain
			// Add minimum height
			nodeY = max( (level->seaLevel + 10), level->getTopSolidBlock(nodeX, nodeZ) + yAdjustment );

			app.DebugPrintf("Node %d is at (%d,%d,%d)\n", i, nodeX, nodeY, nodeZ);

			m_nodes->data[i] = new Node(nodeX,nodeY,nodeZ);

			//level->setTile(nodeX,nodeY,nodeZ,Tile::obsidian_Id);
		}

		m_nodeAdjacency[0] = (1<<11) | (1<<1) | (1<<12);
		m_nodeAdjacency[1] = (1<<0) | (1<<2) | (1<<13);
		m_nodeAdjacency[2] = (1<<1) | (1<<3) | (1<<13);
		m_nodeAdjacency[3] = (1<<2) | (1<<4) | (1<<14);
		m_nodeAdjacency[4] = (1<<3) | (1<<5) | (1<<15);
		m_nodeAdjacency[5] = (1<<4) | (1<<6) | (1<<15);
		m_nodeAdjacency[6] = (1<<5) | (1<<7) | (1<<16);
		m_nodeAdjacency[7] = (1<<6) | (1<<8) | (1<<17);
		m_nodeAdjacency[8] = (1<<7) | (1<<9) | (1<<17);
		m_nodeAdjacency[9] = (1<<8) | (1<<10) | (1<<18);
		m_nodeAdjacency[10] = (1<<9) | (1<<11) | (1<<19);
		m_nodeAdjacency[11] = (1<<10) | (1<<0) | (1<<19);

		m_nodeAdjacency[12] = (1<<0) | (1<<13) | (1<<20) | (1<<19);
		m_nodeAdjacency[13] = (1<<1) | (1<<2) | (1<<14) | (1<<21) | (1<<20) | (1<<12);
		m_nodeAdjacency[14] = (1<<3) | (1<<15) | (1<<21) | (1<<13);
		m_nodeAdjacency[15] = (1<<4) | (1<<5) | (1<<16) | (1<<22) | (1<<21) | (1<<14);
		m_nodeAdjacency[16] = (1<<6) | (1<<17) | (1<<22) | (1<<15);
		m_nodeAdjacency[17] = (1<<7) | (1<<8) | (1<<18) | (1<<23) | (1<<22) | (1<<16);
		m_nodeAdjacency[18] = (1<<9) | (1<<19) | (1<<23) | (1<<17);
		m_nodeAdjacency[19] = (1<<10) | (1<<11) | (1<<12) | (1<<20) | (1<<23) | (1<<18);

		m_nodeAdjacency[20] = (1<<12) | (1<<13) | (1<<21) | (1<<22) | (1<<23) | (1<<19);
		m_nodeAdjacency[21] = (1<<14) | (1<<15) | (1<<22) | (1<<23) | (1<<20) | (1<<13);
		m_nodeAdjacency[22] = (1<<15) | (1<<16) | (1<<17) | (1<<23) | (1<<20) | (1<<21);
		m_nodeAdjacency[23] = (1<<17) | (1<<18) | (1<<19) | (1<<20) | (1<<21) | (1<<22);
	}

	return findClosestNode(x,y,z);
}

int EnderDragon::findClosestNode(double tX, double tY, double tZ)
{
	float closestDist = 100.0f;
	int closestIndex = 0;
	Node *currentPos = new Node((int) floor(tX), (int) floor(tY), (int) floor(tZ));
	int startIndex = 0;
	if(m_remainingCrystalsCount <= 0)
	{
		// If not crystals are left then we try and stay in the middle ring and avoid the outer ring
		startIndex = 12;
	}
	for(unsigned int i = startIndex; i < 24; ++i)
	{
		if( m_nodes->data[i]  != NULL )
		{
			float dist = m_nodes->data[i]->distanceTo(currentPos);
			if(dist < closestDist)
			{
				closestDist = dist;
				closestIndex = i;
			}
		}
	}
	delete currentPos;
	return closestIndex;
}

// 4J Stu - A* taken from PathFinder and modified
Path *EnderDragon::findPath(int startIndex, int endIndex, Node *finalNode /* = NULL */)
{
	for(unsigned int i = 0; i < 24; ++i)
	{
		Node *n = m_nodes->data[i];
		n->closed = false;
		n->f = 0;
		n->g = 0;
		n->h = 0;
		n->cameFrom = NULL;
		n->heapIdx = -1;
	}

	Node *from = m_nodes->data[startIndex];
	Node *to = m_nodes->data[endIndex];

	from->g = 0;
	from->h = from->distanceTo(to);
	from->f = from->h;

	openSet->clear();
	openSet->insert(from);

	Node *closest = from;

	int minimumNodeIndex = 0;
	if(m_remainingCrystalsCount <= 0)
	{
		// If not crystals are left then we try and stay in the middle ring and avoid the outer ring
		minimumNodeIndex = 12;
	}

	while (!openSet->isEmpty())
	{
		Node *x = openSet->pop();

		if (x->equals(to))
		{
			app.DebugPrintf("Found path from %d to %d\n", startIndex, endIndex);
			if(finalNode != NULL)
			{
				finalNode->cameFrom = to;
				to = finalNode;
			}
			return reconstruct_path(from, to);
		}

		if (x->distanceTo(to) < closest->distanceTo(to))
		{
			closest = x;
		}
		x->closed = true;

		unsigned int xIndex = 0;
		for(unsigned int i = 0; i < 24; ++i)
		{
			if(m_nodes->data[i] == x)
			{
				xIndex = i;
				break;
			}
		}

		for (int i = minimumNodeIndex; i < 24; i++)
		{
			if(m_nodeAdjacency[xIndex] & (1<<i))
			{
				Node *y = m_nodes->data[i];

				if(y->closed) continue;

				float tentative_g_score = x->g + x->distanceTo(y);
				if (!y->inOpenSet() || tentative_g_score < y->g)
				{
					y->cameFrom = x;
					y->g = tentative_g_score;
					y->h = y->distanceTo(to);
					if (y->inOpenSet())
					{
						openSet->changeCost(y, y->g + y->h);
					}
					else
					{
						y->f = y->g + y->h;
						openSet->insert(y);
					}
				}
			}
		}
	}

	if (closest == from) return NULL;
	app.DebugPrintf("Failed to find path from %d to %d\n", startIndex, endIndex);
	if(finalNode != NULL)
	{
		finalNode->cameFrom = closest;
		closest = finalNode;
	}
	return reconstruct_path(from, closest);
}

// function reconstruct_path(came_from,current_node)
Path *EnderDragon::reconstruct_path(Node *from, Node *to)
{
	int count = 1;
	Node *n = to;
	while (n->cameFrom != NULL)
	{
		count++;
		n = n->cameFrom;
	}

	NodeArray nodes = NodeArray(count);
	n = to;
	nodes.data[--count] = n;
	while (n->cameFrom != NULL) 
	{
		n = n->cameFrom;
		nodes.data[--count] = n;
	}
	Path *ret = new Path(nodes);
	delete [] nodes.data;
	return ret;
}

void EnderDragon::addAdditonalSaveData(CompoundTag *entityTag) 
{
	app.DebugPrintf("Adding EnderDragon additional save data\n");
	entityTag->putShort(L"RemainingCrystals", m_remainingCrystalsCount);
	entityTag->putInt(L"DragonState", (int)getSynchedAction() );

	Mob::addAdditonalSaveData(entityTag);
}

void EnderDragon::readAdditionalSaveData(CompoundTag *tag) 
{
	app.DebugPrintf("Reading EnderDragon additional save data\n");
	m_remainingCrystalsCount = tag->getShort(L"RemainingCrystals");
	if(!tag->contains(L"RemainingCrystals")) m_remainingCrystalsCount = CRYSTAL_COUNT;

	if(tag->contains(L"DragonState")) setSynchedAction( (EEnderdragonAction)tag->getInt(L"DragonState"), true);

	Mob::readAdditionalSaveData(tag);
}

float EnderDragon::getTilt(float a)
{
	float tilt = 0.0f;
	//if(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
	//	getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
	//	getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
	//{
	//	tilt = -25.0f;
	//	xRot = -25.0f;
	//}
	//else
	{
		double latencyPosAcomponents[3],latencyPosBcomponents[3];
		doubleArray latencyPosA = doubleArray(latencyPosAcomponents,3);
		doubleArray latencyPosB = doubleArray(latencyPosBcomponents,3);
		getLatencyPos(latencyPosA, 5, a);
		getLatencyPos(latencyPosB, 10, a);

		tilt = (latencyPosA[1] - latencyPosB[1]) * 10;
	}
	//app.DebugPrintf("Tilt is %f\n", tilt);

	return tilt;
}

double EnderDragon::getHeadYOffset(float a)
{
	double headYOffset = 0.0;
	if(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
	{
		headYOffset = -1.0;
	}
	else
	{
		double p1components[3];
		doubleArray p1 = doubleArray(p1components, 3);
		getLatencyPos(p1, 5, 1);

		double p0components[3];
		doubleArray p0 = doubleArray(p0components, 3);
		getLatencyPos(p0, 0, 1);

		headYOffset = (p0[1] - p1[1]) * 1;
	}
	//app.DebugPrintf("headYOffset is %f\n", headYOffset);
	return headYOffset;
}

double EnderDragon::getHeadYRotDiff(float a)
{
	double result = 0.0;
	//if(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
	//	getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
	//	getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
	//{
	//	result = m_headYRot;
	//}
	return result;
}

double EnderDragon::getHeadPartYOffset(int partIndex, doubleArray bodyPos, doubleArray partPos)
{
	double result = 0.0;
	if( getSynchedAction() == e_EnderdragonAction_Landing || getSynchedAction() == e_EnderdragonAction_Takeoff )
	{
		int eggHeight = level->getTopSolidBlock(PODIUM_X_POS,PODIUM_Z_POS); //level->getHeightmap(4,4);
		float dist = sqrt( distanceToSqr(PODIUM_X_POS, eggHeight, PODIUM_Z_POS) )/4;
		if( dist < 1.0f ) dist = 1.0f;
		result = partIndex / dist;
		//app.DebugPrintf("getHeadPartYOffset - dist = %f, result = %f (%d)\n", dist, result, partIndex);
	}
	else if(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
	{
		result = partIndex;
	}
	else
	{
		if(partIndex == 6)
		{
			result = 0.0;
		}
		else
		{
			result = partPos[1] - bodyPos[1];
		}
	}
	//app.DebugPrintf("Part %d is at %f\n", partIndex, result);
	return result;
}

double EnderDragon::getHeadPartYRotDiff(int partIndex, doubleArray bodyPos, doubleArray partPos)
{
	double result = 0.0;
	//if(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
	//	getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
	//	getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
	//{
	//	result = m_headYRot / (7 - partIndex);
	//}
	//else
	{
		result = partPos[0] - bodyPos[0];
	}
	//app.DebugPrintf("Part %d is at %f\n", partIndex, result);
	return result;
}

Vec3 *EnderDragon::getHeadLookVector(float a)
{
	Vec3 *result = NULL;

	if( getSynchedAction() == e_EnderdragonAction_Landing || getSynchedAction() == e_EnderdragonAction_Takeoff )
	{
		int eggHeight = level->getTopSolidBlock(PODIUM_X_POS,PODIUM_Z_POS); //level->getHeightmap(4,4);
		float dist = sqrt(distanceToSqr(PODIUM_X_POS, eggHeight, PODIUM_Z_POS))/4;
		if( dist < 1.0f ) dist = 1.0f;
		// The 6.0f is dragon->getHeadPartYOffset(6, start, p)
		float yOffset = 6.0f / dist;

		double xRotTemp = xRot;
		double rotScale = 1.5f;
		xRot = -yOffset * rotScale * 5.0f;

		double yRotTemp = yRot;
		yRot += getHeadYRotDiff(a);

		result = getViewVector(a);

		xRot = xRotTemp;
		yRot = yRotTemp;
	}
	else if(	getSynchedAction() == e_EnderdragonAction_Sitting_Flaming ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Scanning ||
		getSynchedAction() == e_EnderdragonAction_Sitting_Attacking)
	{
		double xRotTemp = xRot;
		double rotScale = 1.5f;
		// The 6.0f is dragon->getHeadPartYOffset(6, start, p)
		xRot = -6.0f * rotScale * 5.0f;

		double yRotTemp = yRot;
		yRot += getHeadYRotDiff(a);

		result = getViewVector(a);

		xRot = xRotTemp;
		yRot = yRotTemp;
	}
	else
	{
		result = getViewVector(a);
	}
	return result;
}
