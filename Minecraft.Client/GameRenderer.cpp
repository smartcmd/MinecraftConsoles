#include "stdafx.h"
#include "GameRenderer.h"
#include "ItemInHandRenderer.h"
#include "LevelRenderer.h"
#include "Frustum.h"
#include "FrustumCuller.h"
#include "Textures.h"
#include "Tesselator.h"
#include "ParticleEngine.h"
#include "SmokeParticle.h"
#include "WaterDropParticle.h"
#include "GameMode.h"
#include "CreativeMode.h"
#include "Lighting.h"
#include "Options.h"
#include "MultiplayerLocalPlayer.h"
#include "GuiParticles.h"
#include "MultiPlayerLevel.h"
#include "Chunk.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.item.enchantment.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\net.minecraft.world.level.material.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\net.minecraft.world.level.chunk.h"
#include "..\Minecraft.World\net.minecraft.world.level.biome.h"
#include "..\Minecraft.World\net.minecraft.world.level.dimension.h"
#include "..\Minecraft.World\net.minecraft.world.phys.h"
#include "..\Minecraft.World\System.h"
#include "..\Minecraft.World\FloatBuffer.h"
#include "..\Minecraft.World\ThreadName.h"
#include "..\Minecraft.World\SparseLightStorage.h"
#include "..\Minecraft.World\CompressedTileStorage.h"
#include "..\Minecraft.World\SparseDataStorage.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\Facing.h"
#include "..\Minecraft.World\MobEffect.h"
#include "..\Minecraft.World\IntCache.h"
#include "..\Minecraft.World\SmoothFloat.h"
#include "..\Minecraft.World\MobEffectInstance.h"
#include "..\Minecraft.World\Item.h"
#include "Camera.h"
#include "..\Minecraft.World\SoundTypes.h"
#include "HumanoidModel.h"
#include "..\Minecraft.World\Item.h"
#include "..\Minecraft.World\compression.h"
#include "PS3\PS3Extras\ShutdownManager.h"
#include "BossMobGuiInfo.h"

#include "TexturePackRepository.h"
#include "TexturePack.h"
#include "TextureAtlas.h"
#include "Common/PostProcesser.h"

bool GameRenderer::anaglyph3d = false;
int GameRenderer::anaglyphPass = 0;

#ifdef MULTITHREAD_ENABLE
C4JThread*		GameRenderer::m_updateThread;
C4JThread::EventArray* GameRenderer::m_updateEvents;
bool GameRenderer::nearThingsToDo = false;
bool GameRenderer::updateRunning = false;
vector<byte *> GameRenderer::m_deleteStackByte;
vector<SparseLightStorage *> GameRenderer::m_deleteStackSparseLightStorage;
vector<CompressedTileStorage *> GameRenderer::m_deleteStackCompressedTileStorage;
vector<SparseDataStorage *> GameRenderer::m_deleteStackSparseDataStorage;
#endif
CRITICAL_SECTION GameRenderer::m_csDeleteStack;

ResourceLocation GameRenderer::RAIN_LOCATION = ResourceLocation(TN_ENVIRONMENT_RAIN);
ResourceLocation GameRenderer::SNOW_LOCATION = ResourceLocation(TN_ENVIRONMENT_SNOW);

GameRenderer::GameRenderer(Minecraft *mc)
{
	// 4J - added this block of initialisers
	renderDistance = 0;
	_tick = 0;
	hovered = nullptr;
	thirdDistance = 4;
	thirdDistanceO = 4;
	thirdRotation = 0;
	thirdRotationO = 0;
	thirdTilt = 0;
	thirdTiltO = 0;

	accumulatedSmoothXO = 0;
	accumulatedSmoothYO = 0;
	tickSmoothXO = 0;
	tickSmoothYO = 0;
	lastTickA = 0;

	cameraPos = Vec3::newPermanent(0.0f,0.0f,0.0f);

	fovOffset = 0;
	fovOffsetO = 0;
	cameraRoll = 0;
	cameraRollO = 0;
	for( int i = 0; i < 4; i++ )
	{
		fov[i] = 0.0f;
		oFov[i] = 0.0f;
		tFov[i] = 0.0f;
	}
	isInClouds = false;
	zoom = 1;
	zoom_x = 0;
	zoom_y = 0;
	rainXa = NULL;
	rainZa = NULL;
	lastActiveTime = Minecraft::currentTimeMillis();
	lastNsTime = 0;
	random = new Random();
	rainSoundTime = 0;
	xMod = 0;
	yMod = 0;
	lb = MemoryTracker::createFloatBuffer(16);
	fr = 0.0f;
	fg = 0.0f;
	fb = 0.0f;
	fogBrO = 0.0f;
	fogBr = 0.0f;
	cameraFlip = 0;
	_updateLightTexture = false;
	blr = 0.0f;
	blrt = 0.0f;
	blg = 0.0f;
	blgt = 0.0f;

	darkenWorldAmount = 0.0f;
	darkenWorldAmountO = 0.0f;

	m_fov=70.0f;

	// 4J Stu - Init these so they are setup before the tick
	for( int i = 0; i < 4; i++ )
	{
		fov[i] = oFov[i] = 1.0f;
	}

	this->mc = mc;
	itemInHandRenderer = NULL;

	// 4J-PB - set up the local players iteminhand renderers here - needs to be done with lighting enabled so that the render geometry gets compiled correctly
	glEnable(GL_LIGHTING);
	mc->localitemInHandRenderers[0] = new ItemInHandRenderer(mc);//itemInHandRenderer;
	mc->localitemInHandRenderers[1] = new ItemInHandRenderer(mc);
	mc->localitemInHandRenderers[2] = new ItemInHandRenderer(mc);
	mc->localitemInHandRenderers[3] = new ItemInHandRenderer(mc);
	glDisable(GL_LIGHTING);

	// 4J - changes brought forward from 1.8.2
	BufferedImage *img = new BufferedImage(16, 16, BufferedImage::TYPE_INT_RGB);
	for( int i = 0; i < NUM_LIGHT_TEXTURES; i++ )
	{
		lightTexture[i] = mc->textures->getTexture(img);		// 4J - changed to one light texture per level to support split screen
	}
	delete img;
#ifdef __PS3__
	// we're using the RSX now to upload textures to vram, so we need the main ram textures allocated from io space
	for(int i=0;i<NUM_LIGHT_TEXTURES;i++)
		lightPixels[i] = intArray((int*)RenderManager.allocIOMem(16*16*sizeof(int)), 16*16);
#else
	for(int i=0;i<NUM_LIGHT_TEXTURES;i++)
		lightPixels[i] = intArray(16*16);
#endif

#ifdef MULTITHREAD_ENABLE
	m_updateEvents  = new C4JThread::EventArray(eUpdateEventCount, C4JThread::EventArray::e_modeAutoClear);
	m_updateEvents->Set(eUpdateEventIsFinished);

	InitializeCriticalSection(&m_csDeleteStack);
	m_updateThread = new C4JThread(runUpdate, NULL, "Chunk update");
#ifdef __PS3__
	m_updateThread->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
#endif// __PS3__
	m_updateThread->SetProcessor(CPU_CORE_CHUNK_UPDATE);
	m_updateThread->Run();
#endif
}

// 4J Stu Added to go with 1.8.2 change
GameRenderer::~GameRenderer()
{
	if(rainXa != NULL) delete [] rainXa;
	if(rainZa != NULL) delete [] rainZa;
}

void GameRenderer::tick(bool first)		// 4J - add bFirst
{
	tickFov();
	tickLightTexture();		// 4J - change brought forward from 1.8.2
	fogBrO = fogBr;
	thirdDistanceO = thirdDistance;
	thirdRotationO = thirdRotation;
	thirdTiltO = thirdTilt;
	fovOffsetO = fovOffset;
	cameraRollO = cameraRoll;

	if (mc->options->smoothCamera)
	{
		// update player view in tick() instead of render() to maintain
		// camera movement regardless of FPS
		float ss = mc->options->sensitivity * 0.6f + 0.2f;
		float sens = (ss * ss * ss) * 8;
		tickSmoothXO = smoothTurnX.getNewDeltaValue(accumulatedSmoothXO, 0.05f * sens);
		tickSmoothYO = smoothTurnY.getNewDeltaValue(accumulatedSmoothYO, 0.05f * sens);
		lastTickA = 0;

		accumulatedSmoothXO = 0;
		accumulatedSmoothYO = 0;
	}

	if (mc->cameraTargetPlayer == NULL)
	{
		mc->cameraTargetPlayer = dynamic_pointer_cast<Mob>(mc->player);
	}

	float brr = mc->level->getBrightness(Mth::floor(mc->cameraTargetPlayer->x), Mth::floor(mc->cameraTargetPlayer->y), Mth::floor(mc->cameraTargetPlayer->z));
	float whiteness = (3 - mc->options->viewDistance) / 3.0f;
	float fogBrT = brr * (1 - whiteness) + whiteness;
	fogBr += (fogBrT - fogBr) * 0.1f;

	itemInHandRenderer->tick();

	PIXBeginNamedEvent(0,"Rain tick");
	tickRain();
	PIXEndNamedEvent();

	darkenWorldAmountO = darkenWorldAmount;
	if (BossMobGuiInfo::darkenWorld)
	{
		darkenWorldAmount += 1.0f / ((float) SharedConstants::TICKS_PER_SECOND * 1);
		if (darkenWorldAmount > 1)
		{
			darkenWorldAmount = 1;
		}
		BossMobGuiInfo::darkenWorld = false;
	}
	else if (darkenWorldAmount > 0)
	{
		darkenWorldAmount -= 1.0f / ((float) SharedConstants::TICKS_PER_SECOND * 4);
	}

	if( mc->player != mc->localplayers[ProfileManager.GetPrimaryPad()] ) return; // 4J added for split screen - only do rest of processing for once per frame

	_tick++;
}

void GameRenderer::pick(float a)
{
	if (mc->cameraTargetPlayer == NULL) return;
	if (mc->level == NULL) return;

	mc->crosshairPickMob = nullptr;

	double range = mc->gameMode->getPickRange();
	delete mc->hitResult;
	MemSect(31);
	mc->hitResult = mc->cameraTargetPlayer->pick(range, a);
	MemSect(0);

	// 4J - added - stop blocks right at the edge of the world from being pickable so we shouldn't be able to directly destroy or create anything there
	if( mc->hitResult )
	{
		int maxxz = ( ( mc->level->chunkSource->m_XZSize / 2 ) * 16 )  - 2;
		int minxz = ( -( mc->level->chunkSource->m_XZSize / 2 ) * 16 ) + 1;

		// Don't select the tops of the very edge blocks, or the sides of the next blocks in
		// 4J Stu - Only block the sides that are facing an outside block
		int hitx = mc->hitResult->x;
		int hitz = mc->hitResult->z;
		int face = mc->hitResult->f;
		if( face == Facing::WEST && hitx < 0 ) hitx -= 1;
		if( face == Facing::EAST && hitx > 0 ) hitx += 1;
		if( face == Facing::NORTH && hitz < 0 ) hitz -= 1;
		if( face == Facing::SOUTH && hitz > 0 ) hitz += 1;

		if( ( hitx < minxz ) || ( hitx > maxxz) ||
			( hitz < minxz ) || ( hitz > maxxz) )
		{
			delete mc->hitResult;
			mc->hitResult = NULL;
		}
	}

	double dist = range;
	Vec3 *from = mc->cameraTargetPlayer->getPos(a);

	if (mc->gameMode->hasFarPickRange())
	{
		dist = range = 6;
	}
	else
	{
		if (dist > 3) dist = 3;
		range = dist;
	}

	if (mc->hitResult != NULL)
	{
		dist = mc->hitResult->pos->distanceTo(from);
	}

	Vec3 *b = mc->cameraTargetPlayer->getViewVector(a);
	Vec3 *to = from->add(b->x * range, b->y * range, b->z * range);
	hovered = nullptr;
	float overlap = 1;
	vector<shared_ptr<Entity> > *objects = mc->level->getEntities(mc->cameraTargetPlayer, mc->cameraTargetPlayer->bb->expand(b->x * (range), b->y * (range), b->z * (range))->grow(overlap, overlap, overlap));
	double nearest = dist;

    for (auto& e : *objects )
    {
		if ( e == nullptr || !e->isPickable() ) continue;

		float rr = e->getPickRadius();
		AABB *bb = e->bb->grow(rr, rr, rr);
		HitResult *p = bb->clip(from, to);
		if (bb->contains(from))
		{
			if (0 < nearest || nearest == 0)
			{
				hovered = e;
				nearest = 0;
			}
		}
		else if (p != nullptr)
		{
			double dd = from->distanceTo(p->pos);
			if (e == mc->cameraTargetPlayer->riding != NULL)
			{
				if (nearest == 0)
				{
					hovered = e;
				}
			}
			else
			{
				hovered = e;
				nearest = dd;
			}
		}
		delete p;
	}

	if (hovered != NULL)
	{
		if (nearest < dist || (mc->hitResult == NULL))
		{
			if( mc->hitResult != NULL )
				delete mc->hitResult;
			mc->hitResult = new HitResult(hovered);
			if (hovered->instanceof(eTYPE_LIVINGENTITY))
			{
				mc->crosshairPickMob = dynamic_pointer_cast<LivingEntity>(hovered);
			}
		}
	}
}

void GameRenderer::SetFovVal(float fov)
{
	m_fov=fov;
}

float GameRenderer::GetFovVal()
{
	return m_fov;
}

void GameRenderer::tickFov()
{
	shared_ptr<LocalPlayer>player = dynamic_pointer_cast<LocalPlayer>(mc->cameraTargetPlayer);

	int playerIdx = player ? player->GetXboxPad() : 0;
	tFov[playerIdx] = player->getFieldOfViewModifier();

	oFov[playerIdx] = fov[playerIdx];
	fov[playerIdx] += (tFov[playerIdx] - fov[playerIdx]) * 0.5f;

	if (fov[playerIdx] > 1.5f) fov[playerIdx] = 1.5f;
	if (fov[playerIdx] < 0.1f) fov[playerIdx] = 0.1f;
}

float GameRenderer::getFov(float a, bool applyEffects)
{
	if (cameraFlip > 0 ) return 90;

	shared_ptr<LocalPlayer> player = dynamic_pointer_cast<LocalPlayer>(mc->cameraTargetPlayer);
	int playerIdx = player ? player->GetXboxPad() : 0;
	float fov = m_fov;//70;
	if (applyEffects)
	{
		fov += mc->options->fov * 40;
		fov *= oFov[playerIdx] + (this->fov[playerIdx] - oFov[playerIdx]) * a;
	}
	if (player->getHealth() <= 0)
	{
		float duration = player->deathTime + a;

		fov /= ((1 - 500 / (duration + 500)) * 2.0f + 1);
	}

	int t = Camera::getBlockAt(mc->level, player, a);
	if (t != 0 && Tile::tiles[t]->material == Material::water) fov = fov * 60 / 70;

	return fov + fovOffsetO + (fovOffset - fovOffsetO) * a;

}

void GameRenderer::bobHurt(float a)
{
	shared_ptr<LivingEntity> player = mc->cameraTargetPlayer;

	float hurt = player->hurtTime - a;

	if (player->getHealth() <= 0)
	{
		float duration = player->deathTime + a;

		glRotatef(40 - (40 * 200) / (duration + 200), 0, 0, 1);
	}

	if (hurt < 0) return;
	hurt /= player->hurtDuration;
	hurt = (float) Mth::sin(hurt * hurt * hurt * hurt * PI);

	float rr = player->hurtDir;


	glRotatef(-rr, 0, 1, 0);
	glRotatef(-hurt * 14, 0, 0, 1);
	glRotatef(+rr, 0, 1, 0);

}

void GameRenderer::bobView(float a)
{
	if (!mc->cameraTargetPlayer->instanceof(eTYPE_LIVINGENTITY)) return;

	shared_ptr<Player> player = dynamic_pointer_cast<Player>(mc->cameraTargetPlayer);

	float wda = player->walkDist - player->walkDistO;
	float b = -(player->walkDist + wda * a);
	float bob = player->oBob + (player->bob - player->oBob) * a;
	float tilt = player->oTilt + (player->tilt - player->oTilt) * a;
	glTranslatef((float) Mth::sin(b * PI) * bob * 0.5f, -(float) abs(Mth::cos(b * PI) * bob), 0);
	glRotatef((float) Mth::sin(b * PI) * bob * 3, 0, 0, 1);
	glRotatef((float) abs(Mth::cos(b * PI - 0.2f) * bob) * 5, 1, 0, 0);
	glRotatef((float) tilt, 1, 0, 0);
}

void GameRenderer::moveCameraToPlayer(float a)
{
	shared_ptr<LivingEntity> player = mc->cameraTargetPlayer;
	shared_ptr<LocalPlayer> localplayer = dynamic_pointer_cast<LocalPlayer>(mc->cameraTargetPlayer);
	float heightOffset = player->heightOffset - 1.62f;

	double x = player->xo + (player->x - player->xo) * a;
	double y = player->yo + (player->y - player->yo) * a - heightOffset;
	double z = player->zo + (player->z - player->zo) * a;


	glRotatef(cameraRollO + (cameraRoll - cameraRollO) * a, 0, 0, 1);

	if (player->isSleeping())
	{
		heightOffset += 1.0;
		glTranslatef(0.0f, 0.3f, 0);
		if (!mc->options->fixedCamera)
		{
			int t = mc->level->getTile(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
			if (t == Tile::bed_Id)
			{
				int data = mc->level->getData(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));

				int direction = data & 3;
				glRotatef((float)direction * 90,0.0f, 1.0f, 0.0f);
			}
			glRotatef(player->yRotO + (player->yRot - player->yRotO) * a + 180, 0, -1, 0);
			glRotatef(player->xRotO + (player->xRot - player->xRotO) * a, -1, 0, 0);
		}
	}
	// 4J-PB - changing this to be per player
	//else if (mc->options->thirdPersonView)
	else if (localplayer->ThirdPersonView())
	{
		double cameraDist = thirdDistanceO + (thirdDistance - thirdDistanceO) * a;

		if (mc->options->fixedCamera)
		{

			float rotationY = thirdRotationO + (thirdRotation - thirdRotationO) * a;
			float xRot = thirdTiltO + (thirdTilt - thirdTiltO) * a;

			glTranslatef(0, 0, (float) -cameraDist);
			glRotatef(xRot, 1, 0, 0);
			glRotatef(rotationY, 0, 1, 0);
		}
		else
		{
			// 4J - corrected bug where this used to just take player->xRot & yRot directly and so wasn't taking into account interpolation, allowing camera to go through walls
			float yRot = player->yRotO + (player->yRot - player->yRotO) * a;
			float xRot = player->xRotO + (player->xRot - player->xRotO) * a;

			// Thirdperson view values are now 0 for disabled, 1 for original mode, 2 for reversed.
			if( localplayer->ThirdPersonView() == 2 )
			{
				// Reverse y rotation - note that this is only used in doing collision to calculate our view
				// distance, the actual rotation itself is just below this else {} block
				yRot += 180.0f;
			}

			double xd = -Mth::sin(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI) * cameraDist;
			double zd = Mth::cos(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI) * cameraDist;
			double yd = -Mth::sin(xRot / 180 * PI) * cameraDist;

			if (localplayer->ThirdPersonView() == 2)
			{
				yd = Mth::sin(xRot / 180 * PI) * cameraDist;
			}

			for (int i = 0; i < 8; i++)
			{
				float xo = (float)((i & 1) * 2 - 1);
				float yo = (float)(((i >> 1) & 1) * 2 - 1);
				float zo = (float)(((i >> 2) & 1) * 2 - 1);

				xo *= 0.1f;
				yo *= 0.1f;
				zo *= 0.1f;

				// 4J - corrected bug here where zo was also added to x component
				HitResult *hr = mc->level->clip(Vec3::newTemp(x + xo, y + yo, z + zo), Vec3::newTemp(x - xd + xo, y - yd + yo, z - zd + zo));
				if (hr != NULL)
				{
					double dist = hr->pos->distanceTo(Vec3::newTemp(x, y, z));
					if (dist < cameraDist) cameraDist = dist;
					delete hr;
				}
			}

			glTranslatef(0, 0, (float) -cameraDist);
		}
	}
	else
	{
		glTranslatef(0, 0, -0.1f);
	}

	if (!mc->options->fixedCamera)
	{
		float pitch = player->xRotO + (player->xRot - player->xRotO) * a;
		if (localplayer->ThirdPersonView() == 2)
		{
			pitch = -pitch;
		}

		glRotatef(pitch, 1, 0, 0);
		if (localplayer->ThirdPersonView() == 2)
		{
			glRotatef(player->yRotO + (player->yRot - player->yRotO) * a, 0, 1, 0);
		}
		else
		{
			glRotatef(player->yRotO + (player->yRot - player->yRotO) * a + 180, 0, 1, 0);
		}
	}

	glTranslatef(0, heightOffset, 0);

	x = player->xo + (player->x - player->xo) * a;
	y = player->yo + (player->y - player->yo) * a - heightOffset;
	z = player->zo + (player->z - player->zo) * a;

	isInClouds = mc->levelRenderer->isInCloud(x, y, z, a);

}


void GameRenderer::zoomRegion(double zoom, double xa, double ya)
{
	zoom = zoom;
	zoom_x = xa;
	zoom_y = ya;
}

void GameRenderer::unZoomRegion()
{
	zoom = 1;
}

// 4J added as we have more complex adjustments to make for fov & aspect on account of viewports
void GameRenderer::getFovAndAspect(float& fov, float& aspect, float a, bool applyEffects)
{
	// 4J - split out aspect ratio and fov here so we can adjust for viewports - we might need to revisit these as
	// they are maybe be too generous for performance.
	aspect = mc->width / (float) mc->height;
	fov = getFov(a, applyEffects);

	if( ( mc->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_TOP ) ||
		( mc->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM ) )
	{
		aspect *= 2.0f;
		fov *= 0.7f;		// Reduce FOV to make things less fish-eye, at the expense of reducing vertical FOV from single player mode
	}
	else if( ( mc->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_LEFT ) ||
		( mc->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT) )
	{
		// Ideally I'd like to make the fov bigger here, but if I do then you an see that the arm isn't very long...
		aspect *= 0.5f;
	}
}

void GameRenderer::setupCamera(float a, int eye)
{
	renderDistance = (float)(16 * 16 >> (mc->options->viewDistance));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float stereoScale = 0.07f;
	if (mc->options->anaglyph3d) glTranslatef(-(eye * 2 - 1) * stereoScale, 0, 0);

	// 4J - have split out fov & aspect calculation so we can take into account viewports
	float aspect, fov;
	getFovAndAspect(fov, aspect, a, true);

	if (zoom != 1)
	{
		glTranslatef((float) zoom_x, (float) -zoom_y, 0);
		glScaled(zoom, zoom, 1);
	}
	gluPerspective(fov, aspect, 0.05f, renderDistance * 2);

	if (mc->gameMode->isCutScene())
	{
		float s = 1 / 1.5f;
		glScalef(1, s, 1);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (mc->options->anaglyph3d) glTranslatef((eye * 2 - 1) * 0.10f, 0, 0);

	bobHurt(a);

	// 4J-PB - this is a per-player option
	//if (mc->options->bobView) bobView(a);

	bool bNoLegAnim =(mc->player->getAnimOverrideBitmask()&(1<<HumanoidModel::eAnim_NoLegAnim))!=0;
	bool bNoBobbingAnim =(mc->player->getAnimOverrideBitmask()&(1<<HumanoidModel::eAnim_NoBobbing))!=0;

	if(app.GetGameSettings(mc->player->GetXboxPad(),eGameSetting_ViewBob) && !mc->player->abilities.flying && !bNoLegAnim && !bNoBobbingAnim) bobView(a);

	float pt = mc->player->oPortalTime + (mc->player->portalTime - mc->player->oPortalTime) * a;
	if (pt > 0)
	{
		int multiplier = 20;
		if (mc->player->hasEffect(MobEffect::confusion))
		{
			multiplier = 7;
		}

		float skew = 5 / (pt * pt + 5) - pt * 0.04f;
		skew *= skew;
		glRotatef((_tick + a) * multiplier, 0, 1, 1);
		glScalef(1 / skew, 1, 1);
		glRotatef(-(_tick + a) * multiplier, 0, 1, 1);
	}


	moveCameraToPlayer(a);

	if (cameraFlip > 0)
	{
		int i = cameraFlip - 1;
		if (i == 1) glRotatef(90, 0, 1, 0);
		if (i == 2) glRotatef(180, 0, 1, 0);
		if (i == 3) glRotatef(-90, 0, 1, 0);
		if (i == 4) glRotatef(90, 1, 0, 0);
		if (i == 5) glRotatef(-90, 1, 0, 0);
	}
}

void GameRenderer::renderItemInHand(float a, int eye)
{
	if (cameraFlip > 0) return;

	// 4J-JEV: I'm fairly confident this method would crash if the cameratarget isnt a local player anyway, but oh well.
	shared_ptr<LocalPlayer> localplayer = mc->cameraTargetPlayer->instanceof(eTYPE_LOCALPLAYER) ? dynamic_pointer_cast<LocalPlayer>(mc->cameraTargetPlayer) : nullptr;

	bool renderHand = true;

	// 4J-PB - to turn off the hand for screenshots, but not when the item held is a map
	if ( localplayer!=NULL)
	{
		shared_ptr<ItemInstance> item = localplayer->inventory->getSelected();
		if(!(item && item->getItem()->id==Item::map_Id)  && app.GetGameSettings(localplayer->GetXboxPad(),eGameSetting_DisplayHand)==0 ) renderHand = false;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float stereoScale = 0.07f;
	if (mc->options->anaglyph3d) glTranslatef(-(eye * 2 - 1) * stereoScale, 0, 0);

	// 4J - have split out fov & aspect calculation so we can take into account viewports
	float fov, aspect;
	getFovAndAspect(fov, aspect, a, false);

	if (zoom != 1)
	{
		glTranslatef((float) zoom_x, (float) -zoom_y, 0);
		glScaled(zoom, zoom, 1);
	}
	gluPerspective(fov, aspect, 0.05f, renderDistance * 2);

	if (mc->gameMode->isCutScene())
	{
		float s = 1 / 1.5f;
		glScalef(1, s, 1);
	}

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	if (mc->options->anaglyph3d) glTranslatef((eye * 2 - 1) * 0.10f, 0, 0);

	glPushMatrix();
	bobHurt(a);

	// 4J-PB - changing this to be per player
	//if (mc->options->bobView) bobView(a);
	bool bNoLegAnim =(localplayer->getAnimOverrideBitmask()&( (1<<HumanoidModel::eAnim_NoLegAnim) | (1<<HumanoidModel::eAnim_NoBobbing) ))!=0;
	if(app.GetGameSettings(localplayer->GetXboxPad(),eGameSetting_ViewBob) && !localplayer->abilities.flying && !bNoLegAnim) bobView(a);

	// 4J: Skip hand rendering if render hand is off
	if (renderHand)
	{
		// 4J-PB - changing this to be per player
		//if (!mc->options->thirdPersonView && !mc->cameraTargetPlayer->isSleeping())
		if (!localplayer->ThirdPersonView() && !mc->cameraTargetPlayer->isSleeping())
		{
			if (!mc->options->hideGui && !mc->gameMode->isCutScene())
			{
				turnOnLightLayer(a);
				PIXBeginNamedEvent(0,"Item in hand render");
				itemInHandRenderer->render(a);
				PIXEndNamedEvent();
				turnOffLightLayer(a);
			}
		}
	}
	glPopMatrix();

	// 4J-PB - changing this to be per player
	//if (!mc->options->thirdPersonView && !mc->cameraTargetPlayer->isSleeping())
	if (!localplayer->ThirdPersonView() && !mc->cameraTargetPlayer->isSleeping())
	{
		itemInHandRenderer->renderScreenEffect(a);
		bobHurt(a);
	}

	// 4J-PB - changing this to be per player
	//if (mc->options->bobView) bobView(a);
	if(app.GetGameSettings(localplayer->GetXboxPad(),eGameSetting_ViewBob) && !localplayer->abilities.flying && !bNoLegAnim) bobView(a);
}

// 4J - change brought forward from 1.8.2
void GameRenderer::turnOffLightLayer(double alpha)
{	// 4J - TODO
#if 0
	if (SharedConstants::TEXTURE_LIGHTING)
	{
		glClientActiveTexture(GL_TEXTURE1);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glClientActiveTexture(GL_TEXTURE0);
		glActiveTexture(GL_TEXTURE0);
	}
#endif
	RenderManager.TextureBindVertex(-1);
}

// 4J - change brought forward from 1.8.2
void GameRenderer::turnOnLightLayer(double alpha)
{	// 4J - TODO
#if 0
	if (SharedConstants::TEXTURE_LIGHTING)
	{
		glClientActiveTexture(GL_TEXTURE1);
		glActiveTexture(GL_TEXTURE1);
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		// float s = 1 / 16f / 15.0f*16/14.0f;
		float s = 1 / 16.0f / 15.0f * 15 / 16;
		glScalef(s, s, s);
		glTranslatef(8f, 8f, 8f);
		glMatrixMode(GL_MODELVIEW);

		mc->textures->bind(lightTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glColor4f(1, 1, 1, 1);
		glEnable(GL_TEXTURE_2D);
		glClientActiveTexture(GL_TEXTURE0);
		glActiveTexture(GL_TEXTURE0);
	}
#endif
	RenderManager.TextureBindVertex(getLightTexture(mc->player->GetXboxPad(), mc->level));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

// 4J - change brought forward from 1.8.2
void GameRenderer::tickLightTexture()
{
	blrt += (float)((Math::random() - Math::random()) * Math::random() * Math::random());
	blgt += (float)((Math::random() - Math::random()) * Math::random() * Math::random());
	blrt *= 0.9;
	blgt *= 0.9;
	blr += (blrt - blr) * 1;
	blg += (blgt - blg) * 1;
	_updateLightTexture = true;
}

void GameRenderer::updateLightTexture(float a)
{
    CachePlayerGammas();

    for (int j = 0; j < XUSER_MAX_COUNT; j++)
    {
        shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->localplayers[j];
        if (player == nullptr)
            continue;

        Level *level = player->level;

        const float skyDarken1 = level->getSkyDarken(1.0f);
        for (int i = 0; i < 256; i++)
        {
            const float darken = skyDarken1 * 0.95f + 0.05f;
            float sky = level->dimension->brightnessRamp[i / 16] * darken;
            const float block = level->dimension->brightnessRamp[i % 16] * (blr * 0.1f + 1.5f);

            if (level->skyFlashTime < 0)
            {
                sky = level->dimension->brightnessRamp[i / 16];
            }

            const float rs = sky * (skyDarken1 * 0.65f + 0.35f);
            const float gs = sky * (skyDarken1 * 0.65f + 0.35f);
            const float bs = sky;

            const float rb = block;
            const float gb = block * ((block * 0.6f + 0.4f) * 0.6f + 0.4f);
            const float bb = block * ((block * block) * 0.6f + 0.4f);

            float _r = (rs + rb);
            float _g = (gs + gb);
            float _b = (bs + bb);

            _r = _r * 0.96f + 0.03f;
            _g = _g * 0.96f + 0.03f;
            _b = _b * 0.96f + 0.03f;

            if (darkenWorldAmount > 0)
            {
                const float amount = darkenWorldAmountO + (darkenWorldAmount - darkenWorldAmountO) * a;
                _r = _r * (1.0f - amount) + (_r * 0.7f) * amount;
                _g = _g * (1.0f - amount) + (_g * 0.6f) * amount;
                _b = _b * (1.0f - amount) + (_b * 0.6f) * amount;
            }

            if (level->dimension->id == 1)
            {
                _r = (0.22f + rb * 0.75f);
                _g = (0.28f + gb * 0.75f);
                _b = (0.25f + bb * 0.75f);
            }

            if (player->hasEffect(MobEffect::nightVision))
            {
                const float scale = getNightVisionScale(player, a);
                float dist = 1.0f / _r;
                if (dist > (1.0f / _g))
                    dist = (1.0f / _g);
                if (dist > (1.0f / _b))
                    dist = (1.0f / _b);
                _r = _r * (1.0f - scale) + (_r * dist) * scale;
                _g = _g * (1.0f - scale) + (_g * dist) * scale;
                _b = _b * (1.0f - scale) + (_b * dist) * scale;
            }

            if (_r > 1.0f)
                _r = 1.0f;
            if (_r < 0.0f)
                _r = 0.0f;
            if (_g > 1.0f)
                _g = 1.0f;
            if (_g < 0.0f)
                _g = 0.0f;
            if (_b > 1.0f)
                _b = 1.0f;
            if (_b < 0.0f)
                _b = 0.0f;

            constexpr int alpha = 255;
            const int r = static_cast<int>(_r * 255);
            const int g = static_cast<int>(_g * 255);
            const int b = static_cast<int>(_b * 255);

#if ( defined _DURANGO || defined _WIN64 || __PSVITA__ )
            lightPixels[j][i] = alpha << 24 | b << 16 | g << 8 | r;
#elif ( defined _XBOX || defined __ORBIS__ )
            lightPixels[j][i] = alpha << 24 | r << 16 | g << 8 | b;
#else
            lightPixels[j][i] = r << 24 | g << 16 | b << 8 | alpha;
#endif
        }

        mc->textures->replaceTextureDirect(lightPixels[j], 16, 16, getLightTexture(j, level));
    }
}

float GameRenderer::ComputeGammaFromSlider(float slider0to100)
{
    float slider = slider0to100;
    slider = max(slider, 0.0f);
    slider = min(slider, 100.0f);

    if (slider > 50.0f)
        return 1.0f + (slider - 50.0f) / 50.0f * 3.0f; // 1.0 -> 4.0
    else
        return 1.0f - (50.0f - slider) / 50.0f * 0.85f; // 1.0 -> 0.15
}

void GameRenderer::CachePlayerGammas()
{
    for (int j = 0; j < XUSER_MAX_COUNT && j < NUM_LIGHT_TEXTURES; ++j)
    {
        std::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->localplayers[j];
        if (!player)
        {
            m_cachedGammaPerPlayer[j] = 1.0f;
            continue;
        }

        const float slider = app.GetGameSettings(j, eGameSetting_Gamma); // 0..100
        m_cachedGammaPerPlayer[j] = ComputeGammaFromSlider(slider);
    }
}

bool GameRenderer::ComputeViewportForPlayer(int j, D3D11_VIEWPORT &outViewport) const
{
    int active = 0;
    int indexMap[NUM_LIGHT_TEXTURES] = {-1, -1, -1, -1};
    for (int i = 0; i < XUSER_MAX_COUNT && i < NUM_LIGHT_TEXTURES; ++i)
    {
        if (Minecraft::GetInstance()->localplayers[i])
            indexMap[active++] = i;
    }

    if (active <= 1)
    {
        outViewport.TopLeftX = 0.0f;
        outViewport.TopLeftY = 0.0f;
        outViewport.Width = static_cast<FLOAT>(mc->width);
        outViewport.Height = static_cast<FLOAT>(mc->height);
        outViewport.MinDepth = 0.0f;
        outViewport.MaxDepth = 1.0f;
        return true;
    }

    int k = -1;
    for (int ord = 0; ord < active; ++ord)
        if (indexMap[ord] == j)
        {
            k = ord;
            break;
        }
    if (k < 0)
        return false;

    const float width = static_cast<float>(mc->width);
    const float height = static_cast<float>(mc->height);

    if (active == 2)
    {
        const float halfH = height * 0.5f;
        outViewport.TopLeftX = 0.0f;
        outViewport.Width = width;
        outViewport.MinDepth = 0.0f;
        outViewport.MaxDepth = 1.0f;
        if (k == 0)
        {
            outViewport.TopLeftY = 0.0f;
            outViewport.Height = halfH;
        }
        else
        {
            outViewport.TopLeftY = halfH;
            outViewport.Height = halfH;
        }
        return true;
    }
    else
    {
        const float halfW = width * 0.5f;
        const float halfH = height * 0.5f;
        const int row = (k >= 2) ? 1 : 0;
        const int col = (k % 2);
        outViewport.TopLeftX = col ? halfW : 0.0f;
        outViewport.TopLeftY = row ? halfH : 0.0f;
        outViewport.Width = halfW;
        outViewport.Height = halfH;
        outViewport.MinDepth = 0.0f;
        outViewport.MaxDepth = 1.0f;
        return true;
    }
}

uint32_t GameRenderer::BuildPlayerViewports(D3D11_VIEWPORT *outViewports, float *outGammas, UINT maxCount) const
{
    UINT count = 0;
    for (int j = 0; j < XUSER_MAX_COUNT && j < NUM_LIGHT_TEXTURES && count < maxCount; ++j)
    {
        if (!Minecraft::GetInstance()->localplayers[j])
            continue;
        D3D11_VIEWPORT vp;
        if (!ComputeViewportForPlayer(j, vp))
            continue;
        outViewports[count] = vp;
        outGammas[count] = m_cachedGammaPerPlayer[j];
        ++count;
    }
    return count;
}

void GameRenderer::ApplyGammaPostProcess() const
{
    D3D11_VIEWPORT vps[NUM_LIGHT_TEXTURES];
    float gammas[NUM_LIGHT_TEXTURES];
    const UINT n = BuildPlayerViewports(vps, gammas, NUM_LIGHT_TEXTURES);
    if (n == 0)
        return;

    bool anyEffect = false;
    for (UINT i = 0; i < n; ++i)
    {
        if (gammas[i] < 0.99f || gammas[i] > 1.01f)
        {
            anyEffect = true;
            break;
        }
    }
    if (!anyEffect)
        return;

    if (n == 1)
    {
        PostProcesser::GetInstance().SetGamma(gammas[0]);
        PostProcesser::GetInstance().Apply();
    }
    else
    {
        PostProcesser::GetInstance().CopyBackbuffer();
        for (UINT i = 0; i < n; ++i)
        {
            PostProcesser::GetInstance().SetGamma(gammas[i]);
            PostProcesser::GetInstance().SetViewport(vps[i]);
            PostProcesser::GetInstance().ApplyFromCopied();
        }
        PostProcesser::GetInstance().ResetViewport();
    }
}

float GameRenderer::getNightVisionScale(shared_ptr<Player> player, float a)
{
	int duration = player->getEffect(MobEffect::nightVision)->getDuration();
	if (duration > (SharedConstants::TICKS_PER_SECOND * 10))
	{
		return 1.0f;
	}
	else
	{
		float flash = max(0.0f, (float)duration - a);
		return .7f + Mth::sin(flash * PI * .05f) * .3f; // was:  .7 + sin(flash*pi*0.2) * .3
	}
}

// 4J added, so we can have a light texture for each player to support split screen
int GameRenderer::getLightTexture(int iPad, Level *level)
{
	// Turn the current dimenions id into an index from 0 to 2
	// int idx = level->dimension->id;
	// if( idx == -1 ) idx = 2;

	return lightTexture[iPad]; // 4J-JEV: Changing to Per Player lighting textures.
}

void GameRenderer::render(float a, bool bFirst)
{
	if( _updateLightTexture && bFirst) updateLightTexture(a);
	if (Display::isActive())
	{
		lastActiveTime = System::currentTimeMillis();
	}
	else
	{
		if (System::currentTimeMillis() - lastActiveTime > 500)
		{
			mc->pauseGame();
		}
	}

#if 0	// 4J - TODO
	if (mc->mouseGrabbed && focused) {
		mc->mouseHandler.poll();

		float ss = mc->options->sensitivity * 0.6f + 0.2f;
		float sens = (ss * ss * ss) * 8;
		float xo = mc->mouseHandler.xd * sens;
		float yo = mc->mouseHandler.yd * sens;

		int yAxis = 1;
		if (mc->options->invertYMouse) yAxis = -1;

		if (mc->options->smoothCamera) {

			xo = smoothTurnX.getNewDeltaValue(xo, .05f * sens);
			yo = smoothTurnY.getNewDeltaValue(yo, .05f * sens);

		}

		mc->player.turn(xo, yo * yAxis);
	}
#endif

	if (mc->noRender) return;
	GameRenderer::anaglyph3d = mc->options->anaglyph3d;

	glViewport(0, 0, mc->width, mc->height);	// 4J - added
	ScreenSizeCalculator ssc(mc->options, mc->width, mc->height);
	int screenWidth = ssc.getWidth();
	int screenHeight = ssc.getHeight();
	int xMouse = Mouse::getX() * screenWidth / mc->width;
	int yMouse = screenHeight - Mouse::getY() * screenHeight / mc->height - 1;

	int maxFps = getFpsCap(mc->options->framerateLimit);

	if (mc->level != NULL)
	{
		if (mc->options->framerateLimit == 0)
		{
			renderLevel(a, 0);
		}
		else
		{
			renderLevel(a, lastNsTime + 1000000000 / maxFps);
		}

		lastNsTime = System::nanoTime();

		if (!mc->options->hideGui || mc->screen != NULL)
		{
			mc->gui->render(a, mc->screen != NULL, xMouse, yMouse);
		}
	}
	else
	{
		glViewport(0, 0, mc->width, mc->height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		setupGuiScreen();

		lastNsTime = System::nanoTime();
	}


	if (mc->screen != NULL)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		mc->screen->render(xMouse, yMouse, a);
		if (mc->screen != NULL && mc->screen->particles != NULL) mc->screen->particles->render(a);
	}

	ApplyGammaPostProcess();
}

void GameRenderer::renderLevel(float a)
{
	renderLevel(a, 0);
}

#ifdef MULTITHREAD_ENABLE
// Request that an item be deleted, when it is safe to do so
void GameRenderer::AddForDelete(byte *deleteThis)
{
	EnterCriticalSection(&m_csDeleteStack);
	m_deleteStackByte.push_back(deleteThis);
}

void GameRenderer::AddForDelete(SparseLightStorage *deleteThis)
{
	EnterCriticalSection(&m_csDeleteStack);
	m_deleteStackSparseLightStorage.push_back(deleteThis);
}

void GameRenderer::AddForDelete(CompressedTileStorage *deleteThis)
{
	EnterCriticalSection(&m_csDeleteStack);
	m_deleteStackCompressedTileStorage.push_back(deleteThis);
}

void GameRenderer::AddForDelete(SparseDataStorage *deleteThis)
{
	EnterCriticalSection(&m_csDeleteStack);
	m_deleteStackSparseDataStorage.push_back(deleteThis);
}

void GameRenderer::FinishedReassigning()
{
	LeaveCriticalSection(&m_csDeleteStack);
}

int GameRenderer::runUpdate(LPVOID lpParam)
{
	Minecraft *minecraft = Minecraft::GetInstance();
	Vec3::CreateNewThreadStorage();
	AABB::CreateNewThreadStorage();
	IntCache::CreateNewThreadStorage();
	Tesselator::CreateNewThreadStorage(1024*1024);
	Compression::UseDefaultThreadStorage();
	RenderManager.InitialiseContext();
#ifdef _LARGE_WORLDS
	Chunk::CreateNewThreadStorage();
#endif
	Tile::CreateNewThreadStorage();

	ShutdownManager::HasStarted(ShutdownManager::eRenderChunkUpdateThread,m_updateEvents);
	while(ShutdownManager::ShouldRun(ShutdownManager::eRenderChunkUpdateThread))
	{
		//m_updateEvents->Clear(eUpdateEventIsFinished);
		//m_updateEvents->WaitForSingle(eUpdateCanRun,INFINITE);
		// 4J Stu - We Need to have this happen atomically to avoid deadlocks
		m_updateEvents->WaitForAll(INFINITE);

		if( !ShutdownManager::ShouldRun(ShutdownManager::eRenderChunkUpdateThread) )
		{
			break;
		}

		m_updateEvents->Set(eUpdateCanRun);

		//		PIXBeginNamedEvent(0,"Updating dirty chunks %d",(count++)&7);

		// Update chunks atomically until there aren't any very near ones left - they will be deferred for rendering
		// until the call to CBuffDeferredModeEnd if we have anything near to render here
		// Now limiting maximum number of updates that can be deferred as have noticed that with redstone clock circuits, it is possible to create
		// things that need constant updating, so if you stand near them, the render data Never gets updated and the game just keeps going until it runs out of render memory...
		int count = 0;
		static const int MAX_DEFERRED_UPDATES = 10;
		bool shouldContinue = false;
		do
		{
			shouldContinue = minecraft->levelRenderer->updateDirtyChunks();
			count++;
		} while ( shouldContinue && count < MAX_DEFERRED_UPDATES );

		//		while( minecraft->levelRenderer->updateDirtyChunks() )
		//			;
		RenderManager.CBuffDeferredModeEnd();

		// If any renderable tile entities were flagged in this last block of chunk(s) that were udpated, then change their
		// flags to say that this deferred chunk is over and they are actually safe to be removed now
		minecraft->levelRenderer->fullyFlagRenderableTileEntitiesToBeRemoved();

		// We've got stacks for things that can only safely be deleted whilst this thread isn't updating things - delete those things now
		EnterCriticalSection(&m_csDeleteStack);
		for(unsigned int i = 0; i < m_deleteStackByte.size(); i++ )
		{
			delete m_deleteStackByte[i];
		}
		m_deleteStackByte.clear();
		for(unsigned int i = 0; i < m_deleteStackSparseLightStorage.size(); i++ )
		{
			delete m_deleteStackSparseLightStorage[i];
		}
		m_deleteStackSparseLightStorage.clear();
		for(unsigned int i = 0; i < m_deleteStackCompressedTileStorage.size(); i++ )
		{
			delete m_deleteStackCompressedTileStorage[i];
		}
		m_deleteStackCompressedTileStorage.clear();
		for(unsigned int i = 0; i < m_deleteStackSparseDataStorage.size(); i++ )
		{
			delete m_deleteStackSparseDataStorage[i];
		}
		m_deleteStackSparseDataStorage.clear();
		LeaveCriticalSection(&m_csDeleteStack);

		//		PIXEndNamedEvent();

		AABB::resetPool();
		Vec3::resetPool();
		IntCache::Reset();
		m_updateEvents->Set(eUpdateEventIsFinished);
	}

	ShutdownManager::HasFinished(ShutdownManager::eRenderChunkUpdateThread);
	return 0;
}
#endif

void GameRenderer::EnableUpdateThread()
{
	// #ifdef __PS3__ // MGH - disable the update on PS3 for now
	// 	return;
	// #endif
#ifdef MULTITHREAD_ENABLE
	if( updateRunning) return;
	app.DebugPrintf("------------------EnableUpdateThread--------------------\n");
	updateRunning = true;
	m_updateEvents->Set(eUpdateCanRun);
	m_updateEvents->Set(eUpdateEventIsFinished);
#endif
}

void GameRenderer::DisableUpdateThread()
{
	// #ifdef __PS3__ // MGH - disable the update on PS3 for now
	// 	return;
	// #endif
#ifdef MULTITHREAD_ENABLE
	if( !updateRunning) return;
	app.DebugPrintf("------------------DisableUpdateThread--------------------\n");
	updateRunning = false;
	m_updateEvents->Clear(eUpdateCanRun);
	m_updateEvents->WaitForSingle(eUpdateEventIsFinished,INFINITE);
#endif
}

void GameRenderer::renderLevel(float a, int64_t until)
{
	//	if (updateLightTexture) updateLightTexture();	// 4J - TODO - Java 1.0.1 has this line enabled, should check why - don't want to put it in now in case it breaks split-screen

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	// Is this the primary player? Only do the updating of chunks if it is. This controls the creation of render data for each chunk - all of this we are only
	// going to do for the primary player, and the other players can just view whatever they have loaded in - we're sharing render data between players.
	bool updateChunks = ( mc->player == mc->localplayers[ProfileManager.GetPrimaryPad()] );

	//	if (mc->cameraTargetPlayer == NULL)	// 4J - removed condition as we want to update this is mc->player changes for different local players
	{
		mc->cameraTargetPlayer = mc->player;
	}
	pick(a);

	shared_ptr<LivingEntity> cameraEntity = mc->cameraTargetPlayer;
	LevelRenderer *levelRenderer = mc->levelRenderer;
	ParticleEngine *particleEngine = mc->particleEngine;
	double xOff = cameraEntity->xOld + (cameraEntity->x - cameraEntity->xOld) * a;
	double yOff = cameraEntity->yOld + (cameraEntity->y - cameraEntity->yOld) * a;
	double zOff = cameraEntity->zOld + (cameraEntity->z - cameraEntity->zOld) * a;

	for (int i = 0; i < 2; i++)
	{
		if (mc->options->anaglyph3d)
		{
			GameRenderer::anaglyphPass = i;
			if (GameRenderer::anaglyphPass == 0) glColorMask(false, true, true, false);
			else glColorMask(true, false, false, false);
		}


		glViewport(0, 0, mc->width, mc->height);
		setupClearColor(a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_CULL_FACE);

		setupCamera(a, i);
		Camera::prepare(mc->player, mc->player->ThirdPersonView() == 2);

		Frustum::getFrustum();
		if (mc->options->viewDistance < 2)
		{
			setupFog(-1, a);
			levelRenderer->renderSky(a);
			if(mc->skins->getSelected()->getId() == 1026 ) levelRenderer->renderHaloRing(a);
		}
		glEnable(GL_FOG);
		setupFog(1, a);

		if (mc->options->ambientOcclusion)
		{
			GL11::glShadeModel(GL11::GL_SMOOTH);
		}

		PIXBeginNamedEvent(0,"Culling");
		MemSect(31);
		//		Culler *frustum = new FrustumCuller();
		FrustumCuller frustObj;
		Culler *frustum = &frustObj;
		MemSect(0);
		frustum->prepare(xOff, yOff, zOff);

		mc->levelRenderer->cull(frustum, a);
		PIXEndNamedEvent();

#ifndef MULTITHREAD_ENABLE
		if ( (i == 0) && updateChunks ) // 4J - added updateChunks condition
		{
			int PIXPass = 0;
			PIXBeginNamedEvent(0,"Updating dirty chunks");
			do
			{
				PIXBeginNamedEvent(0,"Updating dirty chunks pass %d",PIXPass++);
				bool retval = mc->levelRenderer->updateDirtyChunks(cameraEntity, false);
				PIXEndNamedEvent();
				if( retval ) break;


				if (until == 0) break;

				int64_t diff = until - System::nanoTime();
				if (diff < 0) break;
				if (diff > 1000000000) break;
			} while (true);
			PIXEndNamedEvent();
		}
#endif


		if (cameraEntity->y < Level::genDepth)
		{
			prepareAndRenderClouds(levelRenderer, a);
		}
		Frustum::getFrustum();		// 4J added - re-calculate frustum as rendering the clouds does a scale & recalculates one that isn't any good for the rest of the level rendering

		setupFog(0, a);
		glEnable(GL_FOG);
		MemSect(31);
		mc->textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);	// 4J was L"/terrain.png"
		MemSect(0);
		Lighting::turnOff();
		PIXBeginNamedEvent(0,"Level render");
		levelRenderer->render(cameraEntity, 0, a, updateChunks);
		PIXEndNamedEvent();

		GL11::glShadeModel(GL11::GL_FLAT);

		if (cameraFlip == 0 )
		{
			Lighting::turnOn();
			PIXBeginNamedEvent(0,"Entity render");
			// 4J - for entities, don't include the "a" factor that interpolates from the old to new position, as the AABBs for the entities are already fully at the new position
			// This fixes flickering minecarts, and pigs that you are riding on
			frustum->prepare(cameraEntity->x,cameraEntity->y,cameraEntity->z);
			// 4J Stu - When rendering entities, in the end if the dragon is hurt or we have a lot of entities we can end up wrapping
			// our index into the temp Vec3 cache and overwrite the one that was storing the camera position
			// Fix for #77745 - TU9: Content: Gameplay: Items and mobs not belonging to end world are disappearing when Enderdragon is damaged.
			Vec3 *cameraPosTemp = cameraEntity->getPos(a);
			cameraPos->x = cameraPosTemp->x;
			cameraPos->y = cameraPosTemp->y;
			cameraPos->z = cameraPosTemp->z;
			levelRenderer->renderEntities(cameraPos, frustum, a);
#ifdef __PSVITA__
			// AP - make sure we're using the Alpha cut out effect for particles
			glEnable(GL_ALPHA_TEST);
#endif
			PIXEndNamedEvent();
			PIXBeginNamedEvent(0,"Particle render");
			turnOnLightLayer(a);		// 4J - brought forward from 1.8.2
			particleEngine->renderLit(cameraEntity, a, ParticleEngine::OPAQUE_LIST);
			Lighting::turnOff();
			setupFog(0, a);
			particleEngine->render(cameraEntity, a, ParticleEngine::OPAQUE_LIST);
			PIXEndNamedEvent();
			turnOffLightLayer(a);		// 4J - brought forward from 1.8.2

			if ( (mc->hitResult != NULL) && cameraEntity->isUnderLiquid(Material::water) && cameraEntity->instanceof(eTYPE_PLAYER) ) //&& !mc->options.hideGui)
			{
				shared_ptr<Player> player = dynamic_pointer_cast<Player>(cameraEntity);
				glDisable(GL_ALPHA_TEST);
				levelRenderer->renderHit(player, mc->hitResult, 0, player->inventory->getSelected(), a);
				glEnable(GL_ALPHA_TEST);
			}
		}

		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDepthMask(true);
		setupFog(0, a);
		glEnable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		MemSect(31);
		mc->textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);		// 4J was L"/terrain.png"
		MemSect(0);
		// 4J - have changed this fancy rendering option to work with our command buffers. The original used to use frame buffer flags to disable
		// writing to colour when doing the z-only pass, but that value gets obliterated by our command buffers. Using alpha blend function instead
		// to achieve the same effect.
		if (true)	// (mc->options->fancyGraphics)
		{
			if (mc->options->ambientOcclusion)
			{
				GL11::glShadeModel(GL11::GL_SMOOTH);
			}

			glBlendFunc(GL_ZERO, GL_ONE);
			PIXBeginNamedEvent(0,"Fancy second pass - writing z");
			int visibleWaterChunks = levelRenderer->render(cameraEntity, 1, a, updateChunks);
			PIXEndNamedEvent();
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (visibleWaterChunks > 0)
			{
				PIXBeginNamedEvent(0,"Fancy second pass - actual rendering");
				levelRenderer->render(cameraEntity, 1, a, updateChunks);	// 4J - chanaged, used to be renderSameAsLast but we don't support that anymore
				PIXEndNamedEvent();
			}

			GL11::glShadeModel(GL11::GL_FLAT);
		}
		else
		{
			PIXBeginNamedEvent(0,"Second pass level render");
			levelRenderer->render(cameraEntity, 1, a, updateChunks);
			PIXEndNamedEvent();
		}

		// 4J - added - have split out translucent particle rendering so that it happens after the water is rendered, primarily for fireworks
		PIXBeginNamedEvent(0,"Particle render (translucent)");
		Lighting::turnOn();
		turnOnLightLayer(a);		// 4J - brought forward from 1.8.2
		particleEngine->renderLit(cameraEntity, a, ParticleEngine::TRANSLUCENT_LIST);
		Lighting::turnOff();
		setupFog(0, a);
		particleEngine->render(cameraEntity, a, ParticleEngine::TRANSLUCENT_LIST);
		PIXEndNamedEvent();
		turnOffLightLayer(a);		// 4J - brought forward from 1.8.2
		////////////////////////// End of 4J added section

		glDepthMask(true);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);

		if ( (zoom == 1) && cameraEntity->instanceof(eTYPE_PLAYER) ) //&& !mc->options.hideGui)
		{
			if (mc->hitResult != NULL && !cameraEntity->isUnderLiquid(Material::water))
			{
				shared_ptr<Player> player = dynamic_pointer_cast<Player>(cameraEntity);
				glDisable(GL_ALPHA_TEST);
				levelRenderer->renderHitOutline(player, mc->hitResult, 0, a);
				glEnable(GL_ALPHA_TEST);
			}
		}

		/* 4J - moved rain rendering to after clouds so that it alpha blends onto them properly
		PIXBeginNamedEvent(0,"Rendering snow and rain");
		renderSnowAndRain(a);
		PIXEndNamedEvent();
		glDisable(GL_FOG);
		*/

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		levelRenderer->renderDestroyAnimation(Tesselator::getInstance(), dynamic_pointer_cast<Player>(cameraEntity), a);
		glDisable(GL_BLEND);

		if (cameraEntity->y >= Level::genDepth)
		{
			prepareAndRenderClouds(levelRenderer, a);
		}

		// 4J - rain rendering moved here so that it renders after clouds & can blend properly onto them
		setupFog(0, a);
		glEnable(GL_FOG);
		PIXBeginNamedEvent(0,"Rendering snow and rain");
		renderSnowAndRain(a);
		PIXEndNamedEvent();
		glDisable(GL_FOG);


		if (zoom == 1)
		{
			glClear(GL_DEPTH_BUFFER_BIT);
			renderItemInHand(a, i);
		}


		if (!mc->options->anaglyph3d)
		{
			return;
		}
	}
	glColorMask(true, true, true, false);
}

void GameRenderer::prepareAndRenderClouds(LevelRenderer *levelRenderer, float a)
{
	if (mc->options->isCloudsOn())
	{
		glPushMatrix();
		setupFog(0, a);
		glEnable(GL_FOG);
		PIXBeginNamedEvent(0,"Rendering clouds");
		levelRenderer->renderClouds(a);
		PIXEndNamedEvent();
		glDisable(GL_FOG);
		setupFog(1, a);
		glPopMatrix();
	}
}

void GameRenderer::tickRain()
{
	float rainLevel = mc->level->getRainLevel(1);

	if (!mc->options->fancyGraphics) rainLevel /= 2;
	if (rainLevel == 0) return;

	rainLevel /= ( mc->levelRenderer->activePlayers() + 1 );

	random->setSeed(_tick * 312987231l);
	shared_ptr<LivingEntity> player = mc->cameraTargetPlayer;
	Level *level = mc->level;

	int x0 = Mth::floor(player->x);
	int y0 = Mth::floor(player->y);
	int z0 = Mth::floor(player->z);

	int r = 10;

	double rainPosX = 0;
	double rainPosY = 0;
	double rainPosZ = 0;
	int rainPosSamples = 0;

	int rainCount = (int) (100 * rainLevel * rainLevel);
	if (mc->options->particles == 1)
	{
		rainCount >>= 1;
	} else if (mc->options->particles == 2)
	{
		rainCount = 0;
	}
	for (int i = 0; i < rainCount; i++)
	{
		int x = x0 + random->nextInt(r) - random->nextInt(r);
		int z = z0 + random->nextInt(r) - random->nextInt(r);
		int y = level->getTopRainBlock(x, z);
		int t = level->getTile(x, y - 1, z);
		Biome *biome = level->getBiome(x,z);
		if (y <= y0 + r && y >= y0 - r && biome->hasRain() && biome->getTemperature() >= 0.2f)
		{
			float xa = random->nextFloat();
			float za = random->nextFloat();
			if (t > 0)
			{
				if (Tile::tiles[t]->material == Material::lava)
				{
					mc->particleEngine->add( shared_ptr<SmokeParticle>( new SmokeParticle(level, x + xa, y + 0.1f - Tile::tiles[t]->getShapeY0(), z + za, 0, 0, 0) ) );
				}
				else
				{
					if (random->nextInt(++rainPosSamples) == 0)
					{
						rainPosX = x + xa;
						rainPosY = y + 0.1f - Tile::tiles[t]->getShapeY0();
						rainPosZ = z + za;
					}
					mc->particleEngine->add( shared_ptr<WaterDropParticle>( new WaterDropParticle(level, x + xa, y + 0.1f - Tile::tiles[t]->getShapeY0(), z + za) ) );
				}
			}
		}
	}


	if (rainPosSamples > 0 && random->nextInt(3) < rainSoundTime++)
	{
		rainSoundTime = 0;
		MemSect(24);
		if (rainPosY > player->y + 1 && level->getTopRainBlock(Mth::floor(player->x), Mth::floor(player->z)) > Mth::floor(player->y))
		{
			mc->level->playLocalSound(rainPosX, rainPosY, rainPosZ, eSoundType_AMBIENT_WEATHER_RAIN, 0.1f, 0.5f);
		}
		else
		{
			mc->level->playLocalSound(rainPosX, rainPosY, rainPosZ, eSoundType_AMBIENT_WEATHER_RAIN, 0.2f, 1.0f);
		}
		MemSect(0);
	}

}

// 4J - this whole function updated from 1.8.2
void GameRenderer::renderSnowAndRain(float a)
{
	float rainLevel = mc->level->getRainLevel(a);
	if (rainLevel <= 0) return;

	// 4J - rain is relatively low poly, but high fill-rate - better to clip it
	RenderManager.StateSetEnableViewportClipPlanes(true);

	turnOnLightLayer(a);

	if (rainXa == NULL)
	{
		rainXa = new float[32 * 32];
		rainZa = new float[32 * 32];

		for (int z = 0; z < 32; z++)
		{
			for (int x = 0; x < 32; x++)
			{
				float xa = x - 16;
				float za = z - 16;
				float d = Mth::sqrt(xa * xa + za * za);
				rainXa[z << 5 | x] = -za / d;
				rainZa[z << 5 | x] = xa / d;
			}
		}
	}

	shared_ptr<LivingEntity> player = mc->cameraTargetPlayer;
	Level *level = mc->level;

	int x0 = Mth::floor(player->x);
	int y0 = Mth::floor(player->y);
	int z0 = Mth::floor(player->z);

	Tesselator *t = Tesselator::getInstance();
	glDisable(GL_CULL_FACE);
	glNormal3f(0, 1, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 0.01f);

	MemSect(31);
	mc->textures->bindTexture(&SNOW_LOCATION);	// 4J was L"/environment/snow.png"
	MemSect(0);

	double xo = player->xOld + (player->x - player->xOld) * a;
	double yo = player->yOld + (player->y - player->yOld) * a;
	double zo = player->zOld + (player->z - player->zOld) * a;

	int yMin = Mth::floor(yo);


	int r = 5;
	// 4J - was if(mc.options.fancyGraphics) r = 10;
	switch( mc->levelRenderer->activePlayers() )
	{
	case 1:
	default:
		r = 9;
		break;
	case 2:
		r = 7;
		break;
	case 3:
		r = 5;
		break;
	case 4:
		r = 5;
		break;
	}

	// 4J - some changes made here to access biome through new interface that caches results in levelchunk flags, as an optimisation

	int mode = -1;
	float time = _tick + a;

	glColor4f(1, 1, 1, 1);

	for (int x = x0 - r; x <= x0 + r; x++)
		for (int z = z0 - r; z <= z0 + r; z++)
		{
			int rainSlot = (z - z0 + 16) * 32 + (x - x0 + 16);
			float xa = rainXa[rainSlot] * 0.5f;
			float za = rainZa[rainSlot] * 0.5f;

			// 4J - changes here brought forward from 1.8.2
			Biome *b = level->getBiome(x, z);
			if (!b->hasRain() && !b->hasSnow()) continue;

			int floor = level->getTopRainBlock(x, z);

			int yy0 = y0 - r;
			int yy1 = y0 + r;

			if (yy0 < floor) yy0 = floor;
			if (yy1 < floor) yy1 = floor;
			float s = 1;

			int yl = floor;
			if (yl < yMin) yl = yMin;

			if (yy0 != yy1)
			{
				random->setSeed((x * x * 3121 + x * 45238971) ^ (z * z * 418711 + z * 13761));

				// 4J - changes here brought forward from 1.8.2
				float temp = b->getTemperature();
				if (level->getBiomeSource()->scaleTemp(temp, floor) >= 0.15f)
				{
					if (mode != 0)
					{
						if (mode >= 0) t->end();
						mode = 0;
						mc->textures->bindTexture(&RAIN_LOCATION);
						t->begin();
					}

					float ra = (((_tick + x * x * 3121 + x * 45238971 + z * z * 418711 + z * 13761) & 31) + a) / 32.0f * (3 + random->nextFloat());

					double xd = (x + 0.5f) - player->x;
					double zd = (z + 0.5f) - player->z;
					float dd = (float) Mth::sqrt(xd * xd + zd * zd) / r;

					float br = 1;
					t->offset(-xo * 1, -yo * 1, -zo * 1);
#ifdef __PSVITA__
					// AP - this will set up the 4 vertices in half the time
					float Alpha = ((1 - dd * dd) * 0.5f + 0.5f) * rainLevel;
					int tex2 = (level->getLightColor(x, yl, z, 0) * 3 + 0xf000f0) / 4;
					t->tileRainQuad(x - xa + 0.5, yy0, z - za + 0.5, 0 * s, yy0 * s / 4.0f + ra * s,
						x + xa + 0.5, yy0, z + za + 0.5, 1 * s, yy0 * s / 4.0f + ra * s,
						x + xa + 0.5, yy1, z + za + 0.5, 1 * s, yy1 * s / 4.0f + ra * s,
						x - xa + 0.5, yy1, z - za + 0.5, 0 * s, yy1 * s / 4.0f + ra * s,
						br, br, br, Alpha, br, br, br, 0, tex2);
#else
					t->tex2(level->getLightColor(x, yl, z, 0));
					t->color(br, br, br, ((1 - dd * dd) * 0.5f + 0.5f) * rainLevel);
					t->vertexUV(x - xa + 0.5, yy0, z - za + 0.5, 0 * s, yy0 * s / 4.0f + ra * s);
					t->vertexUV(x + xa + 0.5, yy0, z + za + 0.5, 1 * s, yy0 * s / 4.0f + ra * s);
					t->color(br, br, br, 0.0f);														// 4J - added to soften the top visible edge of the rain
					t->vertexUV(x + xa + 0.5, yy1, z + za + 0.5, 1 * s, yy1 * s / 4.0f + ra * s);
					t->vertexUV(x - xa + 0.5, yy1, z - za + 0.5, 0 * s, yy1 * s / 4.0f + ra * s);
#endif
					t->offset(0, 0, 0);
					t->end();
				}
				else
				{
					if (mode != 1)
					{
						if (mode >= 0) t->end();
						mode = 1;
						mc->textures->bindTexture(&SNOW_LOCATION);
						t->begin();
					}
					float ra = (((_tick) & 511) + a) / 512.0f;
					float uo = random->nextFloat() + time * 0.01f * (float) random->nextGaussian();
					float vo = random->nextFloat() + time * (float) random->nextGaussian() * 0.001f;
					double xd = (x + 0.5f) - player->x;
					double zd = (z + 0.5f) - player->z;
					float dd = (float) sqrt(xd * xd + zd * zd) / r;
					float br = 1;
					t->offset(-xo * 1, -yo * 1, -zo * 1);
#ifdef __PSVITA__
					// AP - this will set up the 4 vertices in half the time
					float Alpha = ((1 - dd * dd) * 0.3f + 0.5f) * rainLevel;
					int tex2 = (level->getLightColor(x, yl, z, 0) * 3 + 0xf000f0) / 4;
					t->tileRainQuad(x - xa + 0.5, yy0, z - za + 0.5, 0 * s + uo, yy0 * s / 4.0f + ra * s + vo,
						x + xa + 0.5, yy0, z + za + 0.5, 1 * s + uo, yy0 * s / 4.0f + ra * s + vo,
						x + xa + 0.5, yy1, z + za + 0.5, 1 * s + uo, yy1 * s / 4.0f + ra * s + vo,
						x - xa + 0.5, yy1, z - za + 0.5, 0 * s + uo, yy1 * s / 4.0f + ra * s + vo,
						br, br, br, Alpha, br, br, br, Alpha, tex2);
#else
					t->tex2((level->getLightColor(x, yl, z, 0) * 3 + 0xf000f0) / 4);
					t->color(br, br, br, ((1 - dd * dd) * 0.3f + 0.5f) * rainLevel);
					t->vertexUV(x - xa + 0.5, yy0, z - za + 0.5, 0 * s + uo, yy0 * s / 4.0f + ra * s + vo);
					t->vertexUV(x + xa + 0.5, yy0, z + za + 0.5, 1 * s + uo, yy0 * s / 4.0f + ra * s + vo);
					t->vertexUV(x + xa + 0.5, yy1, z + za + 0.5, 1 * s + uo, yy1 * s / 4.0f + ra * s + vo);
					t->vertexUV(x - xa + 0.5, yy1, z - za + 0.5, 0 * s + uo, yy1 * s / 4.0f + ra * s + vo);
#endif
					t->offset(0, 0, 0);
				}
			}
		}

		if( mode >= 0 ) t->end();
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glAlphaFunc(GL_GREATER, 0.1f);
		turnOffLightLayer(a);

		RenderManager.StateSetEnableViewportClipPlanes(false);
}

// 4J - added forceScale parameter
void GameRenderer::setupGuiScreen(int forceScale /*=-1*/)
{
	ScreenSizeCalculator ssc(mc->options, mc->width, mc->height, forceScale);

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (float)ssc.rawWidth, (float)ssc.rawHeight, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);
}

void GameRenderer::setupClearColor(float a)
{
	Level *level = mc->level;
	shared_ptr<LivingEntity> player = mc->cameraTargetPlayer;

	float whiteness = 1.0f / (4 - mc->options->viewDistance);
	whiteness = 1 - (float) pow((double)whiteness, 0.25);

	Vec3 *skyColor = level->getSkyColor(mc->cameraTargetPlayer, a);
	float sr = (float) skyColor->x;
	float sg = (float) skyColor->y;
	float sb = (float) skyColor->z;

	Vec3 *fogColor = level->getFogColor(a);
	fr = (float) fogColor->x;
	fg = (float) fogColor->y;
	fb = (float) fogColor->z;

	if (mc->options->viewDistance < 2)
	{
		Vec3 *sunAngle = Mth::sin(level->getSunAngle(a)) > 0 ? Vec3::newTemp(-1, 0, 0) : Vec3::newTemp(1, 0, 0);
		float d = (float) player->getViewVector(a)->dot(sunAngle);
		if (d < 0) d = 0;
		if (d > 0)
		{
			float *c = level->dimension->getSunriseColor(level->getTimeOfDay(a), a);
			if (c != NULL)
			{
				d *= c[3];
				fr = fr * (1 - d) + c[0] * d;
				fg = fg * (1 - d) + c[1] * d;
				fb = fb * (1 - d) + c[2] * d;
			}
		}
	}

	fr += (sr - fr) * whiteness;
	fg += (sg - fg) * whiteness;
	fb += (sb - fb) * whiteness;

	float rainLevel = level->getRainLevel(a);
	if (rainLevel > 0)
	{
		float ba = 1 - rainLevel * 0.5f;
		float bb = 1 - rainLevel * 0.4f;
		fr *= ba;
		fg *= ba;
		fb *= bb;
	}
	float thunderLevel = level->getThunderLevel(a);
	if (thunderLevel > 0)
	{
		float ba = 1 - thunderLevel * 0.5f;
		fr *= ba;
		fg *= ba;
		fb *= ba;
	}

	int t = Camera::getBlockAt(mc->level, player, a);
	if (isInClouds)
	{
		Vec3 *cc = level->getCloudColor(a);
		fr = (float) cc->x;
		fg = (float) cc->y;
		fb = (float) cc->z;
	}
	else if (t != 0 && Tile::tiles[t]->material == Material::water)
	{
		float clearness = EnchantmentHelper::getOxygenBonus(player) * 0.2f;

		unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Under_Water_Clear_Colour );
		byte redComponent = ((colour>>16)&0xFF);
		byte greenComponent = ((colour>>8)&0xFF);
		byte blueComponent = ((colour)&0xFF);

		fr = (float)redComponent/256 + clearness;//0.02f;
		fg = (float)greenComponent/256 + clearness;//0.02f;
		fb = (float)blueComponent/256 + clearness;//0.2f;
	}
	else if (t != 0 && Tile::tiles[t]->material == Material::lava)
	{
		unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColor( eMinecraftColour_Under_Lava_Clear_Colour );
		byte redComponent = ((colour>>16)&0xFF);
		byte greenComponent = ((colour>>8)&0xFF);
		byte blueComponent = ((colour)&0xFF);

		fr = (float)redComponent/256;//0.6f;
		fg = (float)greenComponent/256;//0.1f;
		fb = (float)blueComponent/256;//0.00f;
	}

	float brr = fogBrO + (fogBr - fogBrO) * a;
	fr *= brr;
	fg *= brr;
	fb *= brr;

	double yy = (player->yOld + (player->y - player->yOld) * a) * level->dimension->getClearColorScale();		// 4J - getClearColorScale brought forward from 1.2.3

	if (player->hasEffect(MobEffect::blindness))
	{
		int duration = player->getEffect(MobEffect::blindness)->getDuration();
		if (duration < 20)
		{
			yy = yy * (1.0f - (float) duration / 20.0f);
		}
		else
		{
			yy = 0;
		}
	}

	if (yy < 1)
	{
		if (yy < 0) yy = 0;
		yy = yy * yy;
		fr *= yy;
		fg *= yy;
		fb *= yy;
	}

	if (darkenWorldAmount > 0)
	{
		float amount = darkenWorldAmountO + (darkenWorldAmount - darkenWorldAmountO) * a;
		fr = fr * (1.0f - amount) + (fr * .7f) * amount;
		fg = fg * (1.0f - amount) + (fg * .6f) * amount;
		fb = fb * (1.0f - amount) + (fb * .6f) * amount;
	}

	if (player->hasEffect(MobEffect::nightVision))
	{
		float scale = getNightVisionScale(mc->player, a);
		{
			float dist = FLT_MAX;				// MGH - changed this to avoid divide by zero
			if ( (fr > 0) && (dist > (1.0f / fr)) )
			{
				dist = (1.0f / fr);
			}
			if ( (fg > 0) && (dist > (1.0f / fg)) )
			{
				dist = (1.0f / fg);
			}
			if ( (fb > 0) && (dist > (1.0f / fb)) )
			{
				dist = (1.0f / fb);
			}
			fr = fr * (1.0f - scale) + (fr * dist) * scale;
			fg = fg * (1.0f - scale) + (fg * dist) * scale;
			fb = fb * (1.0f - scale) + (fb * dist) * scale;
		}
	}

	if (mc->options->anaglyph3d)
	{
		float frr = (fr * 30 + fg * 59 + fb * 11) / 100;
		float fgg = (fr * 30 + fg * 70) / (100);
		float fbb = (fr * 30 + fb * 70) / (100);

		fr = frr;
		fg = fgg;
		fb = fbb;
	}

	glClearColor(fr, fg, fb, 0.0f);

}

void GameRenderer::setupFog(int i, float alpha)
{
	shared_ptr<LivingEntity> player = mc->cameraTargetPlayer;

	// 4J - check for creative mode brought forward from 1.2.3
	bool creative = false;
	if ( player->instanceof(eTYPE_PLAYER) )
	{
		creative = (dynamic_pointer_cast<Player>(player))->abilities.instabuild;
	}

	if (i == 999)
	{
		__debugbreak();
		// 4J TODO
		/*
		glFog(GL_FOG_COLOR, getBuffer(0, 0, 0, 1));
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, 0);
		glFogf(GL_FOG_END, 8);

		if (GLContext.getCapabilities().GL_NV_fog_distance) {
		glFogi(NVFogDistance.GL_FOG_DISTANCE_MODE_NV, NVFogDistance.GL_EYE_RADIAL_NV);
		}

		glFogf(GL_FOG_START, 0);
		*/
		return;
	}

	glFog(GL_FOG_COLOR, getBuffer(fr, fg, fb, 1));
	glNormal3f(0, -1, 0);
	glColor4f(1, 1, 1, 1);

	int t = Camera::getBlockAt(mc->level, player, alpha);

	if (player->hasEffect(MobEffect::blindness))
	{
		float distance = 5.0f;
		int duration = player->getEffect(MobEffect::blindness)->getDuration();
		if (duration < 20)
		{
			distance = 5.0f + (renderDistance - 5.0f) * (1.0f - (float) duration / 20.0f);
		}

		glFogi(GL_FOG_MODE, GL_LINEAR);
		if (i < 0)
		{
			glFogf(GL_FOG_START, 0);
			glFogf(GL_FOG_END, distance * 0.8f);
		}
		else
		{
			glFogf(GL_FOG_START, distance * 0.25f);
			glFogf(GL_FOG_END, distance);
		}
		// 4J - TODO investigate implementing this
		//        if (GLContext.getCapabilities().GL_NV_fog_distance)
		//		{
		//            glFogi(NVFogDistance.GL_FOG_DISTANCE_MODE_NV, NVFogDistance.GL_EYE_RADIAL_NV);
		//        }
	}
	else if (isInClouds)
	{
		glFogi(GL_FOG_MODE, GL_EXP);
		glFogf(GL_FOG_DENSITY, 0.1f); // was 0.06
	}
	else if (t > 0 && Tile::tiles[t]->material == Material::water)
	{
		glFogi(GL_FOG_MODE, GL_EXP);
		if (player->hasEffect(MobEffect::waterBreathing))
		{
			glFogf(GL_FOG_DENSITY, 0.05f); // was 0.06
		}
		else
		{
			glFogf(GL_FOG_DENSITY, 0.1f - (EnchantmentHelper::getOxygenBonus(player) * 0.03f)); // was 0.06
		}
	}
	else if (t > 0 && Tile::tiles[t]->material == Material::lava)
	{
		glFogi(GL_FOG_MODE, GL_EXP);
		glFogf(GL_FOG_DENSITY, 2.0f); // was 0.06
	}
	else
	{
		float distance = renderDistance;
		if (!mc->level->dimension->hasCeiling)
		{
			// 4J - test for doing bedrockfog brought forward from 1.2.3
			if (mc->level->dimension->hasBedrockFog() && !creative)
			{
				double yy = ((player->getLightColor(alpha) & 0xf00000) >> 20) / 16.0 + (player->yOld + (player->y - player->yOld) * alpha + 4) / 32;
				if (yy < 1)
				{
					if (yy < 0) yy = 0;
					yy = yy * yy;
					float dist = 100 * (float) yy;
					if (dist < 5) dist = 5;
					if (distance > dist) distance = dist;
				}
			}
		}

		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, distance * 0.25f);
		glFogf(GL_FOG_END, distance);
		if (i < 0)
		{
			glFogf(GL_FOG_START, 0);
			glFogf(GL_FOG_END, distance * 0.8f);
		}
		else
		{
			glFogf(GL_FOG_START, distance * 0.25f);
			glFogf(GL_FOG_END, distance);
		}
		/* 4J - removed - TODO investigate
		if (GLContext.getCapabilities().GL_NV_fog_distance)
		{
		glFogi(NVFogDistance.GL_FOG_DISTANCE_MODE_NV, NVFogDistance.GL_EYE_RADIAL_NV);
		}
		*/

		if (mc->level->dimension->isFoggyAt((int) player->x, (int) player->z))
		{
			glFogf(GL_FOG_START, distance * 0.05f);
			glFogf(GL_FOG_END, min(distance, 16 * 16 * .75f) * .5f);
		}
	}

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT);

}

FloatBuffer *GameRenderer::getBuffer(float a, float b, float c, float d)
{
	lb->clear();
	lb->put(a)->put(b)->put(c)->put(d);
	lb->flip();
	return lb;
}

int GameRenderer::getFpsCap(int option)
{
	int maxFps = 200;
	if (option == 1) maxFps = 120;
	if (option == 2) maxFps = 35;
	return maxFps;
}

void GameRenderer::updateAllChunks()
{
	//	mc->levelRenderer->updateDirtyChunks(mc->cameraTargetPlayer, true);
}
