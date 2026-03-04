#include "stdafx.h"
#include "LivingEntityRenderer.h"
#include "Lighting.h"
#include "Cube.h"
#include "ModelPart.h"
#include "EntityRenderDispatcher.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\Minecraft.World\Arrow.h"
#include "..\Minecraft.World\Mth.h"
#include "..\Minecraft.World\Player.h"


ResourceLocation LivingEntityRenderer::ENCHANT_GLINT_LOCATION = ResourceLocation(TN__BLUR__MISC_GLINT);
int LivingEntityRenderer::MAX_ARMOR_LAYERS = 4;

LivingEntityRenderer::LivingEntityRenderer(Model *model, float shadow)
{
	this->model = model;
	shadowRadius = shadow;
	armor = NULL;
}

void LivingEntityRenderer::setArmor(Model *armor)
{
	this->armor = armor;
}

float LivingEntityRenderer::rotlerp(float from, float to, float a)
{
	float diff = to - from;
	while (diff < -180)
		diff += 360;
	while (diff >= 180)
		diff -= 360;
	return from + a * diff;
}

void LivingEntityRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	shared_ptr<LivingEntity> mob = dynamic_pointer_cast<LivingEntity>(_mob);

	glPushMatrix();
	glDisable(GL_CULL_FACE);

	model->attackTime = getAttackAnim(mob, a);
	if (armor != NULL) armor->attackTime = model->attackTime;
	model->riding = mob->isRiding();
	if (armor != NULL) armor->riding = model->riding;
	model->young = mob->isBaby();
	if (armor != NULL) armor->young = model->young;

	/*try*/
	{
		float bodyRot = rotlerp(mob->yBodyRotO, mob->yBodyRot, a);
		float headRot = rotlerp(mob->yHeadRotO, mob->yHeadRot, a);

		if (mob->isRiding() && mob->riding->instanceof(eTYPE_LIVINGENTITY))
		{
			shared_ptr<LivingEntity> riding = dynamic_pointer_cast<LivingEntity>(mob->riding);
			bodyRot = rotlerp(riding->yBodyRotO, riding->yBodyRot, a);

			float headDiff = Mth::wrapDegrees(headRot - bodyRot);
			if (headDiff < -85) headDiff = -85;
			if (headDiff >= 85) headDiff = +85;
			bodyRot = headRot - headDiff;
			if (headDiff * headDiff > 50 * 50)
			{
				bodyRot += headDiff * 0.2f;
			}
		}

		float headRotx = (mob->xRotO + (mob->xRot - mob->xRotO) * a);

		setupPosition(mob, x, y, z);

		float bob = getBob(mob, a);
		setupRotations(mob, bob, bodyRot, a);

		float fScale = 1 / 16.0f;
		glEnable(GL_RESCALE_NORMAL);
		glScalef(-1, -1, 1);

		scale(mob, a);
		glTranslatef(0, -24 * fScale - 0.125f / 16.0f, 0);

		float ws = mob->walkAnimSpeedO + (mob->walkAnimSpeed - mob->walkAnimSpeedO) * a;
		float wp = mob->walkAnimPos - mob->walkAnimSpeed * (1 - a);
		if (mob->isBaby())
		{
			wp *= 3.0f;
		}

		if (ws > 1) ws = 1;

		glEnable(GL_ALPHA_TEST);
		model->prepareMobModel(mob, wp, ws, a);
		renderModel(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale);

		for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
		{
			int armorType = prepareArmor(mob, i, a);
			if (armorType > 0)
			{
				armor->prepareMobModel(mob, wp, ws, a);
				armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale, true);
				if ((armorType & 0xf0) == 16)
				{
					prepareSecondPassArmor(mob, i, a);
					armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale, true);
				}
				// 4J - added condition here for rendering player as part of the gui. Avoiding rendering the glint here as it involves using its own blending, and for gui rendering
				// we are globally blending to be able to offer user configurable gui opacity. Note that I really don't know why GL_BLEND is turned off at the end of the first
				// armour layer anyway, or why alpha testing is turned on... but we definitely don't want to be turning blending off during the gui render.
				if( !entityRenderDispatcher->isGuiRender )
				{
					if ((armorType & 0xf) == 0xf) 
					{
						float time = mob->tickCount + a;
						bindTexture(&ENCHANT_GLINT_LOCATION);
						glEnable(GL_BLEND);
						float br = 0.5f;
						glColor4f(br, br, br, 1);
						glDepthFunc(GL_EQUAL);
						glDepthMask(false);

						for (int j = 0; j < 2; j++)
						{
							glDisable(GL_LIGHTING);
							float brr = 0.76f;
							glColor4f(0.5f * brr, 0.25f * brr, 0.8f * brr, 1);
							glBlendFunc(GL_SRC_COLOR, GL_ONE);
							glMatrixMode(GL_TEXTURE);
							glLoadIdentity();
							float uo = time * (0.001f + j * 0.003f) * 20;
							float ss = 1 / 3.0f;
							glScalef(ss, ss, ss);
							glRotatef(30 - (j) * 60.0f, 0, 0, 1);
							glTranslatef(0, uo, 0);
							glMatrixMode(GL_MODELVIEW);
							armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale, false);
						}

						glColor4f(1, 1, 1, 1);
						glMatrixMode(GL_TEXTURE);
						glDepthMask(true);
						glLoadIdentity();
						glMatrixMode(GL_MODELVIEW);
						glEnable(GL_LIGHTING);
						glDisable(GL_BLEND);
						glDepthFunc(GL_LEQUAL);

					}
					glDisable(GL_BLEND);
				}
				glEnable(GL_ALPHA_TEST);
			}
		}
		glDepthMask(true);

		additionalRendering(mob, a);
		float br = mob->getBrightness(a);
		int overlayColor = getOverlayColor(mob, br, a);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);

		if (((overlayColor >> 24) & 0xff) > 0 || mob->hurtTime > 0 || mob->deathTime > 0)
		{
			glDisable(GL_TEXTURE_2D);
			glDisable(GL_ALPHA_TEST);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDepthFunc(GL_EQUAL);

			// 4J - changed these renders to not use the compiled version of their models, because otherwise the render states set
			// about (in particular the depth & alpha test) don't work with our command buffer versions
			if (mob->hurtTime > 0 || mob->deathTime > 0)
			{
				glColor4f(br, 0, 0, 0.4f);
				model->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale, false);
				for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
				{
					if (prepareArmorOverlay(mob, i, a) >= 0)
					{
						glColor4f(br, 0, 0, 0.4f);
						armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale, false);
					}
				}
			}

			if (((overlayColor >> 24) & 0xff) > 0)
			{
				float r = ((overlayColor >> 16) & 0xff) / 255.0f;
				float g = ((overlayColor >> 8) & 0xff) / 255.0f;
				float b = ((overlayColor) & 0xff) / 255.0f;
				float aa = ((overlayColor >> 24) & 0xff) / 255.0f;
				glColor4f(r, g, b, aa);
				model->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale, false);
				for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
				{
					if (prepareArmorOverlay(mob, i, a) >= 0)
					{
						glColor4f(r, g, b, aa);
						armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, fScale, false);
					}
				}
			}

			glDepthFunc(GL_LEQUAL);
			glDisable(GL_BLEND);
			glEnable(GL_ALPHA_TEST);
			glEnable(GL_TEXTURE_2D);
		}
		glDisable(GL_RESCALE_NORMAL);
	}
	/* catch (Exception e)
	{
	e.printStackTrace();
	}*/

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_CULL_FACE);

	glPopMatrix();

	MemSect(31);
	renderName(mob, x, y, z);
	MemSect(0);
}

void LivingEntityRenderer::renderModel(shared_ptr<LivingEntity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale)
{
	bindTexture(mob);
	if (!mob->isInvisible())
	{
		model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
	}
	else if(!mob->isInvisibleTo(dynamic_pointer_cast<Player>(Minecraft::GetInstance()->player)))
	{
		glPushMatrix();
		glColor4f(1, 1, 1, 0.15f);
		glDepthMask(false);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glAlphaFunc(GL_GREATER, 1.0f / 255.0f);
		model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
		glDisable(GL_BLEND);
		glAlphaFunc(GL_GREATER, .1f);
		glPopMatrix();
		glDepthMask(true);
	}
	else 
	{
		model->setupAnim(wp, ws, bob, headRotMinusBodyRot, headRotx, scale, mob);
	}
}

void LivingEntityRenderer::setupPosition(shared_ptr<LivingEntity> mob, double x, double y, double z)
{
	glTranslatef((float) x, (float) y, (float) z);
}

void LivingEntityRenderer::setupRotations(shared_ptr<LivingEntity> mob, float bob, float bodyRot, float a)
{
	glRotatef(180 - bodyRot, 0, 1, 0);
	if (mob->deathTime > 0)
	{
		float fall = (mob->deathTime + a - 1) / 20.0f * 1.6f;
		fall = sqrt(fall);
		if (fall > 1) fall = 1;
		glRotatef(fall * getFlipDegrees(mob), 0, 0, 1);
	}
	else
	{
		wstring name = mob->getAName();
		if (name == L"Dinnerbone" || name == L"Grumm")
		{
			if ( !mob->instanceof(eTYPE_PLAYER) || !dynamic_pointer_cast<Player>(mob)->isCapeHidden() )
			{
				glTranslatef(0, mob->bbHeight + 0.1f, 0);
				glRotatef(180, 0, 0, 1);
			}
		}
	}
}

float LivingEntityRenderer::getAttackAnim(shared_ptr<LivingEntity> mob, float a)
{
	return mob->getAttackAnim(a);
}

float LivingEntityRenderer::getBob(shared_ptr<LivingEntity> mob, float a)
{
	return (mob->tickCount + a);
}

void LivingEntityRenderer::additionalRendering(shared_ptr<LivingEntity> mob, float a)
{

}

void LivingEntityRenderer::renderArrows(shared_ptr<LivingEntity> mob, float a)
{
	int arrowCount = mob->getArrowCount();
	if (arrowCount > 0)
	{
		shared_ptr<Entity> arrow = shared_ptr<Entity>(new Arrow(mob->level, mob->x, mob->y, mob->z));
		Random random = Random(mob->entityId);
		Lighting::turnOff();
		for (int i = 0; i < arrowCount; i++)
		{
			glPushMatrix();
			ModelPart *modelPart = model->getRandomModelPart(random);
			Cube *cube = modelPart->cubes[random.nextInt(modelPart->cubes.size())];
			modelPart->translateTo(1 / 16.0f);
			float xd = random.nextFloat();
			float yd = random.nextFloat();
			float zd = random.nextFloat();
			float xo = (cube->x0 + (cube->x1 - cube->x0) * xd) / 16.0f;
			float yo = (cube->y0 + (cube->y1 - cube->y0) * yd) / 16.0f;
			float zo = (cube->z0 + (cube->z1 - cube->z0) * zd) / 16.0f;
			glTranslatef(xo, yo, zo);
			xd = xd * 2 - 1;
			yd = yd * 2 - 1;
			zd = zd * 2 - 1;
			if (true)
			{
				xd *= -1;
				yd *= -1;
				zd *= -1;
			}
			float sd = (float) sqrt(xd * xd + zd * zd);
			arrow->yRotO = arrow->yRot = (float) (atan2(xd, zd) * 180 / PI);
			arrow->xRotO = arrow->xRot = (float) (atan2(yd, sd) * 180 / PI);
			double x = 0;
			double y = 0;
			double z = 0;
			float yRot = 0;
			entityRenderDispatcher->render(arrow, x, y, z, yRot, a);
			glPopMatrix();
		}
		Lighting::turnOn();
	}
}

int LivingEntityRenderer::prepareArmorOverlay(shared_ptr<LivingEntity> mob, int layer, float a)
{
	return prepareArmor(mob, layer, a);
}

int LivingEntityRenderer::prepareArmor(shared_ptr<LivingEntity> mob, int layer, float a) 
{
	return -1;
}

void LivingEntityRenderer::prepareSecondPassArmor(shared_ptr<LivingEntity> mob, int layer, float a)
{
}

float LivingEntityRenderer::getFlipDegrees(shared_ptr<LivingEntity> mob)
{
	return 90;
}

int LivingEntityRenderer::getOverlayColor(shared_ptr<LivingEntity> mob, float br, float a)
{
	return 0;
}

void LivingEntityRenderer::scale(shared_ptr<LivingEntity> mob, float a)
{
}

void LivingEntityRenderer::renderName(shared_ptr<LivingEntity> mob, double x, double y, double z)
{
	if (shouldShowName(mob) || Minecraft::renderDebug())
	{
		float size = 1.60f;
		float s = 1 / 60.0f * size;
		double dist = mob->distanceToSqr(entityRenderDispatcher->cameraEntity);

		float maxDist = mob->isSneaking() ? 32 : 64;

		if (dist < maxDist * maxDist)
		{
			wstring msg = mob->getDisplayName();

			if (!msg.empty())
			{
				if (mob->isSneaking())
				{
					if ( app.GetGameSettings(eGameSetting_DisplayHUD)==0 )
					{
						// 4J-PB - turn off gamertag render 
						return;
					}

					if(app.GetGameHostOption(eGameHostOption_Gamertags)==0)
					{
						// turn off gamertags if the host has set them off
						return;
					}

					Font *font = getFont();
					glPushMatrix();
					glTranslatef((float) x + 0, (float) y + mob->bbHeight + 0.5f, (float) z);
					glNormal3f(0, 1, 0);

					glRotatef(-entityRenderDispatcher->playerRotY, 0, 1, 0);
					glRotatef(entityRenderDispatcher->playerRotX, 1, 0, 0);

					glScalef(-s, -s, s);
					glDisable(GL_LIGHTING);

					glTranslatef(0, 0.25f / s, 0);
					glDepthMask(false);
					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					Tesselator *t = Tesselator::getInstance();

					glDisable(GL_TEXTURE_2D);
					t->begin();
					int w = font->width(msg) / 2;
					t->color(0.f, 0.f, 0.f, 0.25f);
					t->vertex(-w - 1, -1, 0);
					t->vertex(-w - 1, +8, 0);
					t->vertex(+w + 1, +8, 0);
					t->vertex(+w + 1, -1, 0);
					t->end();
					glEnable(GL_TEXTURE_2D);
					glDepthMask(true);
					font->draw(msg, -font->width(msg) / 2, 0, 0x20ffffff);
					glEnable(GL_LIGHTING);
					glDisable(GL_BLEND);
					glColor4f(1, 1, 1, 1);
					glPopMatrix();
				}
				else
				{
					renderNameTags(mob, x, y, z, msg, s, dist);
				}
			}
		}
	}
}

bool LivingEntityRenderer::shouldShowName(shared_ptr<LivingEntity> mob)
{
	return Minecraft::renderNames() && mob != entityRenderDispatcher->cameraEntity && !mob->isInvisibleTo(Minecraft::GetInstance()->player) && mob->rider.lock() == NULL;
}

void LivingEntityRenderer::renderNameTags(shared_ptr<LivingEntity> mob, double x, double y, double z, const wstring &msg, float scale, double dist)
{
	if (mob->isSleeping())
	{
		renderNameTag(mob, msg, x, y - 1.5f, z, 64);
	}
	else
	{
		renderNameTag(mob, msg, x, y, z, 64);
	}
}

// 4J Added parameter for color here so that we can colour players names
void LivingEntityRenderer::renderNameTag(shared_ptr<LivingEntity> mob, const wstring &name, double x, double y, double z, int maxDist, int color /*= 0xff000000*/)
{
	if ( app.GetGameSettings(eGameSetting_DisplayHUD)==0 )
	{
		// 4J-PB - turn off gamertag render
		return;
	}

	if(app.GetGameHostOption(eGameHostOption_Gamertags)==0)
	{
		// turn off gamertags if the host has set them off
		return;
	}

	float dist = mob->distanceTo(entityRenderDispatcher->cameraEntity);

	if (dist > maxDist ) 
	{
		return;
	}

	Font *font = getFont();

    constexpr float size = 1.60f;
    constexpr float s = 1 / 60.0f * size;

	glPushMatrix();
	glTranslatef(static_cast<float>(x) + 0, static_cast<float>(y) + mob->bbHeight + 0.5f, static_cast<float>(z));
	glNormal3f(0, 1, 0);

	glRotatef(-this->entityRenderDispatcher->playerRotY, 0, 1, 0);
	glRotatef(this->entityRenderDispatcher->playerRotX, 1, 0, 0);

	glScalef(-s, -s, s);
	glDisable(GL_LIGHTING);

	// 4J Stu - If it's beyond readable distance, then just render a coloured box
	int readableDist = PLAYER_NAME_READABLE_FULLSCREEN;
	if( !RenderManager.IsHiDef() )
	{
		readableDist = PLAYER_NAME_READABLE_DISTANCE_SD;
	}
	else if ( app.GetLocalPlayerCount() > 2 )
	{
		readableDist = PLAYER_NAME_READABLE_DISTANCE_SPLITSCREEN;
	}

	float textOpacity = 1.0f;
	if( dist >= readableDist )
	{
		int diff = dist - readableDist;

		textOpacity /= (diff/2);

		if( diff > readableDist ) textOpacity = 0.0f;
	}

	if( textOpacity < 0.0f ) textOpacity = 0.0f;
	if( textOpacity > 1.0f ) textOpacity = 1.0f;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	Tesselator *t = Tesselator::getInstance();

	int offs = 0;

	wstring playerName;
	WCHAR wchName[2];
	
	if(mob->instanceof(eTYPE_PLAYER))
	{
		shared_ptr<Player> player = dynamic_pointer_cast<Player>(mob);

		if(app.isXuidDeadmau5( player->getXuid() ) ) offs = -10;

#if defined(__PS3__) || defined(__ORBIS__)
		// Check we have all the font characters for this player name
		switch(player->GetPlayerNameValidState())
		{
		case Player::ePlayerNameValid_NotSet:
			if(font->AllCharactersValid(name))
			{
				playerName=name;
				player->SetPlayerNameValidState(true);
			}
			else
			{
				memset(wchName,0,sizeof(WCHAR)*2);
				swprintf(wchName, 2, L"%d",player->getPlayerIndex()+1);		
				playerName=wchName;
				player->SetPlayerNameValidState(false);
			}
			break;
		case Player::ePlayerNameValid_True:
			playerName=name;
			break;
		case Player::ePlayerNameValid_False:
			memset(wchName,0,sizeof(WCHAR)*2);
			swprintf(wchName, 2, L"%d",player->getPlayerIndex()+1);		
			playerName=wchName;	
			break;
		}
#else
		playerName = name;
#endif
	}
	else
	{
		playerName = name;
	}

	if( textOpacity > 0.0f )
	{
		glColor4f(1.0f,1.0f,1.0f,textOpacity);

		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);

		glDisable(GL_TEXTURE_2D);

		t->begin();
		int w = font->width(playerName) / 2;

		if( textOpacity < 1.0f )
		{
			t->color(color, 255 * textOpacity);
		}
		else
		{
			t->color(0.0f, 0.0f, 0.0f, 0.25f);
		}
		t->vertex((float)(-w - 1), (float)( -1 + offs), (float)( 0));
		t->vertex((float)(-w - 1), (float)( +8 + offs + 1), (float)( 0));
		t->vertex((float)(+w + 1), (float)( +8 + offs + 1), (float)( 0));
		t->vertex((float)(+w + 1), (float)( -1 + offs), (float)( 0));
		t->end();

		glEnable(GL_DEPTH_TEST);
		glDepthMask(true);
		glDepthFunc(GL_ALWAYS);
		glLineWidth(2.0f);
		t->begin(GL_LINE_STRIP);
		t->color(color, 255 * textOpacity);
		t->vertex((float)(-w - 1), (float)( -1 + offs), (float)( 0));
		t->vertex((float)(-w - 1), (float)( +8 + offs + 1), (float)( 0));
		t->vertex((float)(+w + 1), (float)( +8 + offs + 1), (float)( 0));
		t->vertex((float)(+w + 1), (float)( -1 + offs), (float)( 0));
		t->vertex((float)(-w - 1), (float)( -1 + offs), (float)( 0));
		t->end();
		glDepthFunc(GL_LEQUAL);
		glDepthMask(false);
		glDisable(GL_DEPTH_TEST);

		glEnable(GL_TEXTURE_2D);
		font->draw(playerName, -font->width(playerName) / 2, offs, 0x20ffffff);
		glEnable(GL_DEPTH_TEST);

		glDepthMask(true);
	}

	if( textOpacity < 1.0f )
	{
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glDisable(GL_TEXTURE_2D);
		glDepthFunc(GL_ALWAYS);
		t->begin();
		int w = font->width(playerName) / 2;
		t->color(color, 255);
		t->vertex((float)(-w - 1), (float)( -1 + offs), (float)( 0));
		t->vertex((float)(-w - 1), (float)( +8 + offs), (float)( 0));
		t->vertex((float)(+w + 1), (float)( +8 + offs), (float)( 0));
		t->vertex((float)(+w + 1), (float)( -1 + offs), (float)( 0));
		t->end();		
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_2D);

		glTranslatef(0.0f, 0.0f, -0.04f);
	}

	if( textOpacity > 0.0f )
	{
		int textColor = ( ( (int)(textOpacity*255) << 24 ) | 0xffffff );
		font->draw(playerName, -font->width(playerName) / 2, offs, textColor);
	}

	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glColor4f(1, 1, 1, 1);
	glPopMatrix();
}