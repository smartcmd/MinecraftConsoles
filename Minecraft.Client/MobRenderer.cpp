#include "stdafx.h"
#include "MobRenderer.h"
#include "MultiPlayerLocalPlayer.h"
#include "..\Minecraft.World\net.minecraft.world.entity.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.entity.projectile.h"
#include "..\Minecraft.World\StringHelpers.h"
#include "..\Minecraft.World\Mth.h"
#include "entityRenderDispatcher.h"

MobRenderer::MobRenderer(Model *model, float shadow) : EntityRenderer()
{
    this->model = model;
    this->shadowRadius = shadow;

	this->armor = NULL;
}

void MobRenderer::setArmor(Model *armor)
{
	this->armor = armor;
}

float MobRenderer::rotlerp(float from, float to, float a)
{
	float diff = to - from;
	while (diff < -180)
		diff += 360;
	while (diff >= 180)
		diff -= 360;
	return from + a * diff;
}

void MobRenderer::render(shared_ptr<Entity> _mob, double x, double y, double z, float rot, float a)
{
	// 4J - added - this used to use generics so the input parameter could be a mob (or derived type), but we aren't
	// able to do that so dynamically casting to get the more specific type here.
	shared_ptr<Mob> mob = dynamic_pointer_cast<Mob>(_mob);
    glPushMatrix();
    glDisable(GL_CULL_FACE);

    model->attackTime = getAttackAnim(mob, a);
    if (armor != NULL) armor->attackTime = model->attackTime;
    model->riding = mob->isRiding();
    if (armor != NULL) armor->riding = model->riding;
	model->young = mob->isBaby();
	if (armor != NULL) armor->young = model->young;

	// 4J - removed try/catch
//    try
//	{
        float bodyRot = rotlerp(mob->yBodyRotO, mob->yBodyRot, a);
        float headRot = rotlerp(mob->yHeadRotO, mob->yHeadRot, a);

		if (mob->isRiding() && dynamic_pointer_cast<Mob>(mob->riding))
		{
			shared_ptr<Mob> riding = dynamic_pointer_cast<Mob>(mob->riding);
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

        float _scale = 1 / 16.0f;
        glEnable(GL_RESCALE_NORMAL);
        glScalef(-1, -1, 1);

        scale(mob, a);
        glTranslatef(0, -24 * _scale - 0.125f / 16.0f, 0);


        float ws = mob->walkAnimSpeedO + (mob->walkAnimSpeed - mob->walkAnimSpeedO) * a;
        float wp = mob->walkAnimPos - mob->walkAnimSpeed * (1 - a);
		if (mob->isBaby())
		{
			wp *= 3.0f;
		}

        if (ws > 1) ws = 1;

		MemSect(31);
        bindTexture(mob->customTextureUrl, mob->getTexture());
		MemSect(0);
        glEnable(GL_ALPHA_TEST);

        model->prepareMobModel(mob, wp, ws, a);
        renderModel(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale);
        for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
		{
			int armorType = prepareArmor(mob, i, a);
			if (armorType > 0)
			{
				armor->prepareMobModel(mob, wp, ws, a);
				armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale, true);
				if ((armorType & 0xf0) == 16)
				{
					prepareSecondPassArmor(mob, i, a);
					armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale, true);
				}
				// 4J - added condition here for rendering player as part of the gui. Avoiding rendering the glint here as it involves using its own blending, and for gui rendering
				// we are globally blending to be able to offer user configurable gui opacity. Note that I really don't know why GL_BLEND is turned off at the end of the first
				// armour layer anyway, or why alpha testing is turned on... but we definitely don't want to be turning blending off during the gui render.
				if( !entityRenderDispatcher->isGuiRender )
				{
					if ((armorType & 0xf) == 0xf)		//MGH - fix for missing enchantment glow
					{
						float time = mob->tickCount + a;
						bindTexture(TN__BLUR__MISC_GLINT); // 4J was "%blur%/misc/glint.png"
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
							armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
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
                model->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
                for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
				{
                    if (prepareArmorOverlay(mob, i, a) >= 0)
					{
                        glColor4f(br, 0, 0, 0.4f);
                        armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
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
                model->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
                for (int i = 0; i < MAX_ARMOR_LAYERS; i++)
				{
                    if (prepareArmorOverlay(mob, i, a) >= 0)
					{
                        glColor4f(r, g, b, aa);
                        armor->render(mob, wp, ws, bob, headRot - bodyRot, headRotx, _scale, false);
                    }
                }
            }

            glDepthFunc(GL_LEQUAL);
            glDisable(GL_BLEND);
            glEnable(GL_ALPHA_TEST);
            glEnable(GL_TEXTURE_2D);
        }
        glDisable(GL_RESCALE_NORMAL);
//    }
//catch (Exception e) {
 //       // System.out.println("Failed: " + modelNames[model]);
 //       e.printStackTrace();
 //   }
	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

    glEnable(GL_CULL_FACE);

    glPopMatrix();

	MemSect(31);
    renderName(mob, x, y, z);
	MemSect(0);
}

void MobRenderer::renderModel(shared_ptr<Entity> mob, float wp, float ws, float bob, float headRotMinusBodyRot, float headRotx, float scale)
{
	shared_ptr<Player> player = dynamic_pointer_cast<Player>(Minecraft::GetInstance()->player);

	bindTexture(mob->customTextureUrl, mob->getTexture());
	if (!mob->isInvisible())
	{
		model->render(mob, wp, ws, bob, headRotMinusBodyRot, headRotx, scale, true);
	}
	else if ( !mob->isInvisibleTo(player) )
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
		model->setupAnim(wp, ws, bob, headRotMinusBodyRot, headRotx, scale);//, mob);
	}
}

void MobRenderer::setupPosition(shared_ptr<Mob> mob, double x, double y, double z)
{
	glTranslatef((float) x, (float) y, (float) z);
}

void MobRenderer::setupRotations(shared_ptr<Mob> mob, float bob, float bodyRot, float a)
{
    glRotatef(180 - bodyRot, 0, 1, 0);
    if (mob->deathTime > 0) 
	{
        float fall = (mob->deathTime + a - 1) / 20.0f * 1.6f;
        fall = (float)sqrt(fall);
        if (fall > 1) fall = 1;
        glRotatef(fall * getFlipDegrees(mob), 0, 0, 1);
    }
}

float MobRenderer::getAttackAnim(shared_ptr<Mob> mob, float a)
{
	 return mob->getAttackAnim(a);
}

float MobRenderer::getBob(shared_ptr<Mob> mob, float a)
{
	return (mob->tickCount + a);
}

void MobRenderer::additionalRendering(shared_ptr<Mob> mob, float a)
{
}

int MobRenderer::prepareArmorOverlay(shared_ptr<Mob> mob, int layer, float a)
{
	return prepareArmor(mob, layer, a);
}

int MobRenderer::prepareArmor(shared_ptr<Mob> mob, int layer, float a)
{
	return -1;
}

void MobRenderer::prepareSecondPassArmor(shared_ptr<Mob> mob, int layer, float a)
{
}

float MobRenderer::getFlipDegrees(shared_ptr<Mob> mob)
{
	return 90;
}

int MobRenderer::getOverlayColor(shared_ptr<Mob> mob, float br, float a)
{
	return 0;
}

void MobRenderer::scale(shared_ptr<Mob> mob, float a)
{
}

void MobRenderer::renderName(shared_ptr<Mob> mob, double x, double y, double z)
{
    if (Minecraft::renderDebug())
	{
        //renderNameTag(mob, _toString<int>(mob->entityId), x, y, z, 64);
    }
}

// 4J Added parameter for color here so that we can colour players names
void MobRenderer::renderNameTag(shared_ptr<Mob> mob, const wstring& OriginalName, double x, double y, double z, int maxDist, int color /*= 0xffffffff*/)
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

    if (dist > maxDist)
	{
        return;
    }

    Font *font = getFont();

    float size = 1.60f;
    float s = 1 / 60.0f * size;

    glPushMatrix();
    glTranslatef((float) x + 0, (float) y + 2.3f, (float) z);
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
	shared_ptr<Player> player = dynamic_pointer_cast<Player>(mob);
	if (player != NULL && app.isXuidDeadmau5( player->getXuid() ) ) offs = -10;

	wstring playerName;
	WCHAR wchName[2];

#if defined(__PS3__) || defined(__ORBIS__)
	// Check we have all the font characters for this player name
	switch(player->GetPlayerNameValidState())
	{
	case Player::ePlayerNameValid_NotSet:
		if(font->AllCharactersValid(OriginalName))
		{
			playerName=OriginalName;
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
		playerName=OriginalName;
		break;
	case Player::ePlayerNameValid_False:
		memset(wchName,0,sizeof(WCHAR)*2);
		swprintf(wchName, 2, L"%d",player->getPlayerIndex()+1);		
		playerName=wchName;	
		break;
	}

#else
	playerName=OriginalName;
#endif

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
