#include "stdafx.h"
#include "Gui.h"
#include "ItemRenderer.h"
#include "GameRenderer.h"
#include "Options.h"
#include "MultiplayerLocalPlayer.h"
#include "Textures.h"
#include "TextureAtlas.h"
#include "GameMode.h"
#include "Lighting.h"
#include "ChatScreen.h"
#include "MultiPlayerLevel.h"
#include "..\Minecraft.World\JavaMath.h"
#include "..\Minecraft.World\net.minecraft.world.entity.player.h"
#include "..\Minecraft.World\net.minecraft.world.effect.h"
#include "..\Minecraft.World\net.minecraft.world.food.h"
#include "..\Minecraft.World\net.minecraft.world.item.h"
#include "..\Minecraft.World\net.minecraft.world.level.h"
#include "..\Minecraft.World\LevelData.h"
#include "..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\Minecraft.World\System.h"
#include "..\Minecraft.World\Language.h"
#include "EntityRenderDispatcher.h"
#include "..\Minecraft.World\Dimension.h"
#include "..\Minecraft.World\net.minecraft.world.entity.boss.enderdragon.h"
#include "EnderDragonRenderer.h"
#include "..\Minecraft.World\net.minecraft.h"
#include "..\Minecraft.World\net.minecraft.world.h"
#include "..\Minecraft.World\LevelChunk.h"
#include "..\Minecraft.World\Biome.h"

ResourceLocation Gui::PUMPKIN_BLUR_LOCATION = ResourceLocation(TN__BLUR__MISC_PUMPKINBLUR);

#define RENDER_HUD 0
//#ifndef _XBOX
//#undef RENDER_HUD
//#define RENDER_HUD 1
//#endif

float Gui::currentGuiBlendFactor = 1.0f;	// 4J added
float Gui::currentGuiScaleFactor = 1.0f;	// 4J added
ItemRenderer *Gui::itemRenderer = new ItemRenderer();

Gui::Gui(Minecraft *minecraft)
{
	// 4J - initialisers added
    random = new Random();
	tickCount = 0;
    overlayMessageTime = 0;
    animateOverlayMessageColor = false;
	progress = 0.0f;
	tbr = 1.0f;
	fAlphaIncrementPerCent=255.0f/100.0f;

	this->minecraft = minecraft;

	lastTickA = 0.0f;
}

void Gui::render(float a, bool mouseFree, int xMouse, int yMouse)
{
	// 4J Stu - I have copied this code for XUI_BaseScene. If/when it gets changed it should be broken out
	// 4J - altered to force full screen mode to 3X scaling, and any split screen modes to 2X scaling. This is so that the further scaling by 0.5 that
	// happens in split screen modes results in a final scaling of 1 rather than 1.5.
	int splitYOffset;// = 20;	// This offset is applied when doing the 2X scaling above to move the gui out of the way of the tool tips
	int guiScale;// = ( minecraft->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN ? 3 : 2 );
	int iPad=minecraft->player->GetXboxPad();
	int iWidthOffset=0,iHeightOffset=0; // used to get the interface looking right on a 2 player split screen game

 	// 4J-PB - selected the gui scale based on the slider settings
	if(minecraft->player->m_iScreenSection == C4JRender::VIEWPORT_TYPE_FULLSCREEN)
	{
		guiScale=app.GetGameSettings(iPad,eGameSetting_UISize) + 2;
	}
	else
	{
		guiScale=app.GetGameSettings(iPad,eGameSetting_UISizeSplitscreen) + 2;
	}


	ScreenSizeCalculator ssc(minecraft->options, minecraft->width, minecraft->height, guiScale );
	int screenWidth = ssc.getWidth();
	int screenHeight = ssc.getHeight();
	int iSafezoneXHalf=0,iSafezoneYHalf=0,iSafezoneTopYHalf=0;
	int iTooltipsYOffset=0;
	int quickSelectWidth=182;
	int quickSelectHeight=22;
	float fScaleFactorWidth=1.0f,fScaleFactorHeight=1.0f;
	bool bTwoPlayerSplitscreen=false;
	currentGuiScaleFactor = (float) guiScale;		// Keep static copy of scale so we know how gui coordinates map to physical pixels - this is also affected by the viewport

	switch(guiScale)
	{
	case 3:
		splitYOffset = 0;
		break;
	case 4:
		splitYOffset = -5;
		break;
	default: // 2
		splitYOffset = 10;
		break;
	}

	// Check which screen section this player is in
	switch(minecraft->player->m_iScreenSection)
	{
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
		// single player
		iSafezoneXHalf = screenWidth/20; // 5%
		iSafezoneYHalf = screenHeight/20; // 5%
		iSafezoneTopYHalf = iSafezoneYHalf;
		iTooltipsYOffset=40+splitYOffset;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
		iSafezoneXHalf = screenWidth/10; // 5%  (need to treat the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset;
		iSafezoneTopYHalf = screenHeight/10;
		fScaleFactorWidth=0.5f;
		iWidthOffset=(int)((float)screenWidth*(1.0f - fScaleFactorWidth));
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
		iSafezoneXHalf = screenWidth/10; // 5% (need to treat the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset + screenHeight/10;// 5%  (need to treat the whole screen is 2x this screen)
		iSafezoneTopYHalf = 0;
		fScaleFactorWidth=0.5f;
		iWidthOffset=(int)((float)screenWidth*(1.0f - fScaleFactorWidth));
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_LEFT:
		iSafezoneXHalf = screenWidth/10; // 5% (the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset + screenHeight/10;// 5% (need to treat the whole screen is 2x this screen)
		iSafezoneTopYHalf = screenHeight/10;
		fScaleFactorHeight=0.5f;
		iHeightOffset=screenHeight;
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_SPLIT_RIGHT:
		iSafezoneXHalf = 0;
		iSafezoneYHalf = splitYOffset + screenHeight/10;// 5% (need to treat the whole screen is 2x this screen)
		iSafezoneTopYHalf = splitYOffset + screenHeight/10;
		fScaleFactorHeight=0.5f;
		iHeightOffset=screenHeight;
		iTooltipsYOffset=44;
		bTwoPlayerSplitscreen=true;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_LEFT:
		iSafezoneXHalf = screenWidth/10; // 5% (the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset;
		iSafezoneTopYHalf = screenHeight/10;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_TOP_RIGHT:
		iSafezoneXHalf = 0;
		iSafezoneYHalf = splitYOffset; // 5%
		iSafezoneTopYHalf = screenHeight/10;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_LEFT:
		iSafezoneXHalf = screenWidth/10; // 5%  (the whole screen is 2x this screen)
		iSafezoneYHalf = splitYOffset + screenHeight/10; // 5% (the whole screen is 2x this screen)
		iSafezoneTopYHalf = 0;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;
	case C4JRender::VIEWPORT_TYPE_QUADRANT_BOTTOM_RIGHT:
		iSafezoneXHalf = 0;
		iSafezoneYHalf = splitYOffset + screenHeight/10; // 5%  (the whole screen is 2x this screen)
		iSafezoneTopYHalf = 0;
		iTooltipsYOffset=44;
		currentGuiScaleFactor *= 0.5f;
		break;

	}

	// 4J-PB - turn off the slot display if a xui menu is up, or if we're autosaving
	bool bDisplayGui=!ui.GetMenuDisplayed(iPad) && !(app.GetXuiAction(iPad)==eAppAction_AutosaveSaveGameCapturedThumbnail);

	// if tooltips are off, set the y offset to zero
	if(app.GetGameSettings(iPad,eGameSetting_Tooltips)==0 && bDisplayGui)
	{
		switch(minecraft->player->m_iScreenSection)
		{
		case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
			iTooltipsYOffset=screenHeight/10;
			break;
		default:
			//iTooltipsYOffset=screenHeight/10;
			switch(guiScale)
			{
			case 3:
				iTooltipsYOffset=28;//screenHeight/10;
				break;
			case 4:
				iTooltipsYOffset=28;//screenHeight/10;
				break;
			default: // 2
				iTooltipsYOffset=14;//screenHeight/10;
				break;
			}
			break;
		}
	}

	// 4J-PB - Turn off interface if eGameSetting_DisplayHUD is off - for screen shots/videos.
	if ( app.GetGameSettings(iPad,eGameSetting_DisplayHUD)==0 )
	{
		bDisplayGui = false;
	}

    Font *font = minecraft->font;


    minecraft->gameRenderer->setupGuiScreen(guiScale);



    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// 4J - added - this did actually get set in renderVignette but that code is currently commented out

    if (Minecraft::useFancyGraphics())
	{
		renderVignette(minecraft->player->getBrightness(a), screenWidth, screenHeight);
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// Display the pumpkin screen effect
	/////////////////////////////////////////////////////////////////////////////////////

    shared_ptr<ItemInstance> headGear = minecraft->player->inventory->getArmor(3);

	// 4J-PB - changing this to be per player
	//if (!minecraft->options->thirdPersonView && headGear != NULL && headGear->id == Tile::pumpkin_Id) renderPumpkin(screenWidth, screenHeight);
	if ((minecraft->player->ThirdPersonView()==0) && headGear != NULL && headGear->id == Tile::pumpkin_Id) renderPumpkin(screenWidth, screenHeight);
	if (!minecraft->player->hasEffect(MobEffect::confusion))
	{
		float pt = minecraft->player->oPortalTime + (minecraft->player->portalTime - minecraft->player->oPortalTime) * a;
		if (pt > 0)
		{
			renderTp(pt, screenWidth, screenHeight);
		}
	}

	if (!minecraft->gameMode->isCutScene())
	{
		if(bDisplayGui && bTwoPlayerSplitscreen)
		{
			// need to apply scale factors depending on the mode
			glPushMatrix();
			glScalef(fScaleFactorWidth, fScaleFactorHeight, fScaleFactorWidth);
		}
#if RENDER_HUD
		/////////////////////////////////////////////////////////////////////////////////////
		// Display the quick select background, the quick select selection, and the crosshair
		/////////////////////////////////////////////////////////////////////////////////////

		glColor4f(1, 1, 1, 1);

		// 4J - this is where to set the blend factor for gui things
		// use the primary player's settings
		unsigned char ucAlpha=app.GetGameSettings(ProfileManager.GetPrimaryPad(),eGameSetting_InterfaceOpacity);

		// If the user has started to navigate their quickselect bar, ignore the alpha setting, and display at default value
		float fVal=fAlphaIncrementPerCent*(float)ucAlpha;
		if(ucAlpha<80)
		{
			// check if we have the timer running for the opacity
			unsigned int uiOpacityTimer=app.GetOpacityTimer(iPad);
			if(uiOpacityTimer!=0)
			{
				if(uiOpacityTimer<10)
				{
					float fStep=(80.0f-(float)ucAlpha)/10.0f;
					fVal=fAlphaIncrementPerCent*(80.0f-((10.0f-(float)uiOpacityTimer)*fStep));
				}
				else
				{
					fVal=fAlphaIncrementPerCent*80.0f;
				}
			}
			else
			{
				fVal=fAlphaIncrementPerCent*(float)ucAlpha;
			}
		}
		else
		{
			fVal=fAlphaIncrementPerCent*(float)ucAlpha;
		}

		RenderManager.StateSetBlendFactor(0xffffff |(((unsigned int)fVal)<<24));
		currentGuiBlendFactor = fVal / 255.0f;
	//	RenderManager.StateSetBlendFactor(0x40ffffff);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

		blitOffset = -90;

		/////////////////////////////////////////////////////////////////////////////////////
		// Display the quick select background, the quick select selection, and the crosshair
		/////////////////////////////////////////////////////////////////////////////////////
		if(bDisplayGui)
		{
			MemSect(31);
			minecraft->textures->bindTexture(TN_GUI_GUI);	// 4J was L"/gui/gui.png"
			MemSect(0);

			shared_ptr<Inventory> inventory = minecraft->player->inventory;
			if(bTwoPlayerSplitscreen)
			{
				// need to apply scale factors depending on the mode

				// 4J Stu - Moved this push and scale further up as we still need to do it for the few HUD components not replaced by xui
				//glPushMatrix();
				//glScalef(fScaleFactorWidth, fScaleFactorHeight, fScaleFactorWidth);

				// 4J-PB - move into the safe zone, and account for 2 player splitscreen
				blit(iWidthOffset + (screenWidth - quickSelectWidth)/2, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset , 0, 0, 182, 22);
				blit(iWidthOffset + (screenWidth - quickSelectWidth)/2 - 1 + inventory->selected * 20, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset  - 1, 0, 22, 24, 22);
			}
			else
			{
				blit(iWidthOffset + screenWidth / 2 - quickSelectWidth / 2, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset , 0, 0, 182, 22);
				blit(iWidthOffset + screenWidth / 2 - quickSelectWidth / 2 - 1 + inventory->selected * 20, iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset  - 1, 0, 22, 24, 22);
			}


			MemSect(31);
			minecraft->textures->bindTexture(TN_GUI_ICONS);//L"/gui/icons.png"));
			MemSect(0);
			glEnable(GL_BLEND);
			RenderManager.StateSetBlendFactor(0xffffff |(((unsigned int)fVal)<<24));
			glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
			//glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ONE_MINUS_SRC_COLOR);
			// 4J Stu - We don't want to adjust the cursor by the safezone, we want it centred
			if(bTwoPlayerSplitscreen)
			{
				blit(iWidthOffset + screenWidth / 2 - 7, (iHeightOffset + screenHeight) / 2 - 7, 0, 0, 16, 16);
			}
			else
			{
				blit(screenWidth / 2 - 7, screenHeight / 2 - 7, 0, 0, 16, 16);
			}
			glDisable(GL_BLEND);

	// 		if(bTwoPlayerSplitscreen)
	// 		{
	// 			glPopMatrix();
	// 		}

		}

		bool blink = minecraft->player->invulnerableTime / 3 % 2 == 1;
		if (minecraft->player->invulnerableTime < 10) blink = false;
		int iHealth = minecraft->player->getHealth();
		int iLastHealth = minecraft->player->lastHealth;
		random->setSeed(tickCount * 312871);

		bool foodBlink = false;
		FoodData *foodData = minecraft->player->getFoodData();
		int food = foodData->getFoodLevel();
		int oldFood = foodData->getLastFoodLevel();

// 		if (false) //(true)
// 		{
// 			renderBossHealth();
// 		}

		/////////////////////////////////////////////////////////////////////////////////////
		// Display the experience, food, armour, health and the air bubbles
		/////////////////////////////////////////////////////////////////////////////////////
		if(bDisplayGui)
		{
			// 4J - added blend for fading gui
			glEnable(GL_BLEND);
			glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);

			if (minecraft->gameMode->canHurtPlayer())
			{
				int xLeft, xRight;
				// 4J Stu - TODO Work out proper positioning for splitscreen
				if(bTwoPlayerSplitscreen)
				{
					xLeft = iWidthOffset + (screenWidth - quickSelectWidth)/2;
					xRight = iWidthOffset + (screenWidth + quickSelectWidth)/2;
				}
				else
				{
					xLeft = (screenWidth - quickSelectWidth)/2;
					xRight = (screenWidth + quickSelectWidth) / 2;
				}

				// render experience bar
				int xpNeededForNextLevel = minecraft->player->getXpNeededForNextLevel();
				if (xpNeededForNextLevel > 0)
				{
					int w = 182;

					int progress = (int) (minecraft->player->experienceProgress * (float) (w + 1));

					int yo = screenHeight - iSafezoneYHalf - iTooltipsYOffset - 8;
					if(bTwoPlayerSplitscreen)
					{
						yo+=iHeightOffset;
					}
					blit(xLeft, yo, 0, 64, w, 5);
					if (progress > 0)
					{
						blit(xLeft, yo, 0, 69, progress, 5);
					}
				}

				int yLine1, yLine2;
				if(bTwoPlayerSplitscreen)
				{
					//yo = iHeightOffset + screenHeight - 10 - iSafezoneYHalf - iTooltipsYOffset;
					yLine1 = iHeightOffset + screenHeight - 18 - iSafezoneYHalf - iTooltipsYOffset;
					yLine2 = yLine1 - 10;
				}
				else
				{
					//yo = screenHeight - 10 - iSafezoneYHalf - iTooltipsYOffset;
					yLine1 = screenHeight - 18 - iSafezoneYHalf - iTooltipsYOffset;
					yLine2 = yLine1 - 10;
				}

				double maxHealth = minecraft->localplayers[iPad]->getAttribute(SharedMonsterAttributes.MAX_HEALTH);

				double totalAbsorption = minecraft->localplayers[iPad]->getAbsorptionAmount();
				int numHealthRows = Mth.ceil((maxHealth + totalAbsorption) / 2 / (float) NUM_HEARTS_PER_ROW);
				int healthRowHeight = Math.max(10 - (numHealthRows - 2), 3);
				int yLine2 = yLine1 - (numHealthRows - 1) * healthRowHeight - 10;
				absorption = totalAbsorption;

				int armor = minecraft->player->getArmorValue();
				int heartOffsetIndex = -1;
				if (minecraft->player->hasEffect(MobEffect::regeneration))
				{
					heartOffsetIndex = tickCount % (int) ceil(maxHealth + 5);
				}

				// render health and armor
				//minecraft.profiler.push("armor");
				for (int i = 0; i < Player::MAX_HEALTH / 2; i++)
				{
					if (armor > 0)
					{
						int xo = xLeft + i * 8;
						if (i * 2 + 1 < armor) blit(xo, yLine2, 16 + 2 * 9, 9, 9, 9);
						if (i * 2 + 1 == armor) blit(xo, yLine2, 16 + 1 * 9, 9, 9, 9);
						if (i * 2 + 1 > armor) blit(xo, yLine2, 16 + 0 * 9, 9, 9, 9);
					}
				}

				//minecraft.profiler.popPush("health");
				for (int i = Mth.ceil((maxHealth + totalAbsorption) / 2) - 1; i >= 0; i--)
				{
					int healthTexBaseX = 16;
					if (minecraft.player.hasEffect(MobEffect.poison))
					{
						healthTexBaseX += 4 * 9;
					}
					else if (minecraft.player.hasEffect(MobEffect.wither))
					{
						healthTexBaseX += 8 * 9;
					}

					int bg = 0;
					if (blink) bg = 1;
					int rowIndex = Mth.ceil((i + 1)  / (float) NUM_HEARTS_PER_ROW) - 1;
					int xo = xLeft + (i % NUM_HEARTS_PER_ROW) * 8;
					int yo = yLine1 - rowIndex * healthRowHeight;
					if (currentHealth <= 4)
					{
						yo += random.nextInt(2);
					}

					if (i == heartOffsetIndex)
					{
						yo -= 2;
					}

					int y0 = 0;

					// No hardcore on console
					/*if (minecraft->level.getLevelData().isHardcore())
					{
						y0 = 5;
					}*/

					blit(xo, yo, 16 + bg * 9, 9 * y0, 9, 9);
					if (blink)
					{
						if (i * 2 + 1 < oldHealth) blit(xo, yo, healthTexBaseX + 6 * 9, 9 * y0, 9, 9);
						if (i * 2 + 1 == oldHealth) blit(xo, yo, healthTexBaseX + 7 * 9, 9 * y0, 9, 9);
					}

					if (absorption > 0)
					{
						if (absorption == totalAbsorption && totalAbsorption % 2 == 1)
						{
							blit(xo, yo, healthTexBaseX + 17 * 9, 9 * y0, 9, 9);
						}
						else
						{
							blit(xo, yo, healthTexBaseX + 16 * 9, 9 * y0, 9, 9);
						}
						absorption -= 2;
					}
					else
					{
						if (i * 2 + 1 < currentHealth) blit(xo, yo, healthTexBaseX + 4 * 9, 9 * y0, 9, 9);
						if (i * 2 + 1 == currentHealth) blit(xo, yo, healthTexBaseX + 5 * 9, 9 * y0, 9, 9);
					}
				}

				std::shared_ptr<Entity> riding = minecraft->localplayers[iPad].get()->riding;
				std::shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(riding);
				if (riding == NULL)
				{
					// render food
					for (int i = 0; i < FoodConstants::MAX_FOOD / 2; i++)
					{
						int yo = yLine1;


						int texBaseX = 16;
						int bg = 0;
						if (minecraft->player->hasEffect(MobEffect::hunger))
						{
							texBaseX += 4 * 9;
							bg = 13;
						}

						if (minecraft->player->getFoodData()->getSaturationLevel() <= 0)
						{
							if ((tickCount % (food * 3 + 1)) == 0)
							{
								yo += random->nextInt(3) - 1;
							}
						}

						if (foodBlink) bg = 1;
						int xo = xRight - i * 8 - 9;
						blit(xo, yo, 16 + bg * 9, 9 * 3, 9, 9);
						if (foodBlink)
						{
							if (i * 2 + 1 < oldFood) blit(xo, yo, texBaseX + 6 * 9, 9 * 3, 9, 9);
							if (i * 2 + 1 == oldFood) blit(xo, yo, texBaseX + 7 * 9, 9 * 3, 9, 9);
						}
						if (i * 2 + 1 < food) blit(xo, yo, texBaseX + 4 * 9, 9 * 3, 9, 9);
						if (i * 2 + 1 == food) blit(xo, yo, texBaseX + 5 * 9, 9 * 3, 9, 9);
					}
				}
				else if (living != nullptr)
				{
					// Render mount health

					int riderCurrentHealth = (int) ceil(living.get()->GetHealth());
					float maxRiderHealth = living->GetMaxHealth();
					int hearts = (int) (maxRiderHealth + .5f) / 2;
					if (hearts > 30)
					{
						hearts = 30;
					}

					int yo = yLine1;
					int baseHealth = 0;

					while (hearts > 0)
					{
						int rowHearts = min(hearts, 10);
						hearts -= rowHearts;

						for (int i = 0; i < rowHearts; i++)
						{
							int texBaseX = 52;
							int bg = 0;

							if (foodBlink) bg = 1;
							int xo = xRight - i * 8 - 9;
							blit(xo, yo, texBaseX + bg * 9, 9 * 1, 9, 9);
							if (i * 2 + 1 + baseHealth < riderCurrentHealth) blit(xo, yo, texBaseX + 4 * 9, 9 * 1, 9, 9);
							if (i * 2 + 1 + baseHealth == riderCurrentHealth) blit(xo, yo, texBaseX + 5 * 9, 9 * 1, 9, 9);
						}
						yo -= 10;
						baseHealth += 20;
					}
				}

				// render air bubbles
				if (minecraft->player->isUnderLiquid(Material::water))
				{
					int count = (int) ceil((minecraft->player->getAirSupply() - 2) * 10.0f / Player::TOTAL_AIR_SUPPLY);
					int extra = (int) ceil((minecraft->player->getAirSupply()) * 10.0f / Player::TOTAL_AIR_SUPPLY) - count;
					for (int i = 0; i < count + extra; i++)
					{
						// Air bubbles
						if (i < count) blit(xRight - i * 8 - 9, yLine2, 16, 9 * 2, 9, 9);
						else blit(xRight - i * 8 - 9, yLine2, 16 + 9, 9 * 2, 9, 9);
					}
				}
			}

		}

		// 4J-PB - turn off the slot display if a xui menu is up

		////////////////////////////
		// render the slot contents
		////////////////////////////
		if(bDisplayGui)
		{
	//		glDisable(GL_BLEND);		4J - removed - we want to be able to fade our gui

			glEnable(GL_RESCALE_NORMAL);

			Lighting::turnOnGui();


			int x,y;

			for (int i = 0; i < 9; i++)
			{
				if(bTwoPlayerSplitscreen)
				{
					x = iWidthOffset + screenWidth / 2 - 9 * 10 + i * 20 + 2;
					y = iHeightOffset + screenHeight - iSafezoneYHalf - iTooltipsYOffset - 16 - 3 + 22;
				}
				else
				{
					x = screenWidth / 2 - 9 * 10 + i * 20 + 2;
					y = screenHeight - iSafezoneYHalf - iTooltipsYOffset - 16 - 3 + 22;
				}
				this->renderSlot(i, x, y, a);
			}
			Lighting::turnOff();
			glDisable(GL_RESCALE_NORMAL);
		}
#endif // RENDER_HUD

		// 4J - do render of crouched player. This code is largely taken from the inventory render of the player, with some special hard-coded positions
		// worked out by hand from the xui implementation of the crouch icon

		if(app.GetGameSettings(iPad,eGameSetting_AnimatedCharacter))
		{
			//int playerIdx = minecraft->player->GetXboxPad();

			static int characterDisplayTimer[4] = {0};
			if( !bDisplayGui )
			{
				characterDisplayTimer[iPad] = 0;
			}
			else if( minecraft->player->isSneaking() )
			{
				characterDisplayTimer[iPad] = 30;
			}
			else if( minecraft->player->isSprinting() )
			{
				characterDisplayTimer[iPad] = 30;
			}
			else if( minecraft->player->abilities.flying)
			{
				characterDisplayTimer[iPad] = 5; // quickly get rid of the player display if they stop flying
			}
			else if( characterDisplayTimer[iPad] > 0 )
			{
				--characterDisplayTimer[iPad];
			}
			bool displayCrouch = minecraft->player->isSneaking() || ( characterDisplayTimer[iPad] > 0 );
			bool displaySprint = minecraft->player->isSprinting() || ( characterDisplayTimer[iPad] > 0 );
			bool displayFlying = minecraft->player->abilities.flying || ( characterDisplayTimer[iPad] > 0 );

			if( bDisplayGui && (displayCrouch || displaySprint || displayFlying) )
			{
				EntityRenderDispatcher::instance->prepare(minecraft->level, minecraft->textures, minecraft->font, minecraft->cameraTargetPlayer, minecraft->crosshairPickMob, minecraft->options, a);
				glEnable(GL_RESCALE_NORMAL);
				glEnable(GL_COLOR_MATERIAL);

				// 4J - TomK now using safe zone values directly instead of the magic number calculation that lived here before (which only worked for medium scale, the other two were off!)
				int xo = iSafezoneXHalf + 10;
				int yo = iSafezoneTopYHalf + 10;

#ifdef __PSVITA__
				// align directly with corners, there are no safe zones on vita
				xo = 10;
				yo = 10;
#endif

				glPushMatrix();
				glTranslatef((float)xo, (float)yo, 50);
				float ss = 12;
				glScalef(-ss, ss, ss);
				glRotatef(180, 0, 0, 1);

				float oyr = minecraft->player->yRot;
				float oyrO = minecraft->player->yRotO;
				float oxr = minecraft->player->xRot;
				int ofire = minecraft->player->onFire;
				bool ofireflag = minecraft->player->getSharedFlag(Entity::FLAG_ONFIRE);

				float xd = -40;
				float yd = 10;

				// 4J Stu - This is all based on the inventory player renderer, with changes to ensure that capes render correctly
				// by minimising the changes to member variables of the player which are all related

				glRotatef(45 + 90, 0, 1, 0);
				Lighting::turnOn();
				glRotatef(-45 - 90, 0, 1, 0);

				glRotatef(-(float) atan(yd / 40.0f ) * 20, 1, 0, 0);
				float bodyRot = (minecraft->player->yBodyRotO + (minecraft->player->yBodyRot - minecraft->player->yBodyRotO));
				// Fixed rotation angle of degrees, adjusted by bodyRot to negate the rotation that occurs in the renderer
				// bodyRot in the rotation below is a simplification of "180 - (180 - bodyRot)" where the first 180 is EntityRenderDispatcher::instance->playerRotY that we set below
				// and (180 - bodyRot) is the angle of rotation that is performed within the mob renderer
				glRotatef( bodyRot - ( (float) atan(xd / 40.0f) * 20), 0, 1, 0);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

				// Set head rotation to body rotation to make head static
				minecraft->player->yRot = bodyRot;
				minecraft->player->yRotO = minecraft->player->yRot;
				minecraft->player->xRot = -(float) atan(yd / 40.0f) * 20;

				minecraft->player->onFire = 0;
				minecraft->player->setSharedFlag(Entity::FLAG_ONFIRE, false);

				// 4J - TomK don't offset the player. it's easier to align it with the safe zones that way!
				//glTranslatef(0, minecraft->player->heightOffset, 0);
				glTranslatef(0, 0, 0);
				EntityRenderDispatcher::instance->playerRotY = 180;
				EntityRenderDispatcher::instance->isGuiRender = true;
				EntityRenderDispatcher::instance->render(minecraft->player, 0, 0, 0, 0, 1);
				EntityRenderDispatcher::instance->isGuiRender = false;

				minecraft->player->yRot = oyr;
				minecraft->player->yRotO = oyrO;
				minecraft->player->xRot = oxr;
				minecraft->player->onFire = ofire;
				minecraft->player->setSharedFlag(Entity::FLAG_ONFIRE,ofireflag);
				glPopMatrix();
				Lighting::turnOff();
				glDisable(GL_RESCALE_NORMAL);
			}
		}
	}

#if RENDER_HUD
	// Moved so the opacity blend is applied to it
	if (bDisplayGui && minecraft->gameMode->hasExperience() && minecraft->player->experienceLevel > 0)
	{
		if (true)
		{
			bool blink = false;
			int col = blink ? 0xffffff : 0x80ff20;
			wchar_t formatted[10];
			swprintf(formatted, 10, L"%d",minecraft->player->experienceLevel);

			wstring str = formatted;
			int x = iWidthOffset + (screenWidth - font->width(str)) / 2;
			int y = screenHeight - iSafezoneYHalf - iTooltipsYOffset;
			// If we're in creative mode, we don't need to offset the XP display so much
			if (minecraft->gameMode->canHurtPlayer())
			{
				y-=18;
			}
			else
			{
				y-=13;
			}

			if(bTwoPlayerSplitscreen)
			{
				y+=iHeightOffset;
			}
			//int y = screenHeight - 31 - 4;
			font->draw(str, x + 1, y, 0x000000);
			font->draw(str, x - 1, y, 0x000000);
			font->draw(str, x, y + 1, 0x000000);
			font->draw(str, x, y - 1, 0x000000);
			// 			font->draw(str, x + 1, y + 1, 0x000000);
			// 			font->draw(str, x - 1, y + 1, 0x000000);
			// 			font->draw(str, x + 1, y - 1, 0x000000);
			// 			font->draw(str, x - 1, y - 1, 0x000000);
			font->draw(str, x, y, col);
		}
	}
#endif // RENDER_HUD

	// 4J - added to disable blends, which we have enabled previously to allow gui fading
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // if the player is falling asleep we render a dark overlay
    if (minecraft->player->getSleepTimer() > 0)
	{
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_ALPHA_TEST);
        int timer = minecraft->player->getSleepTimer();
        float amount = (float) timer / (float) Player::SLEEP_DURATION;
        if (amount > 1)
		{
            // waking up
            amount = 1.0f - ((float) (timer - Player::SLEEP_DURATION) / (float) Player::WAKE_UP_DURATION);
        }

        int color = (int) (220.0f * amount) << 24 | (0x101020);
        fill(0, 0, screenWidth/fScaleFactorWidth, screenHeight/fScaleFactorHeight, color);
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_DEPTH_TEST);
    }

	// 4J-PB - Request from Mojang to have a red death screen
	if (!minecraft->player->isAlive())
	{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_ALPHA_TEST);
		int timer = minecraft->player->getDeathFadeTimer();
		float amount = (float) timer / (float) Player::DEATHFADE_DURATION;

		int color = (int) (220.0f * amount) << 24 | (0x200000);
		fill(0, 0, screenWidth/fScaleFactorWidth, screenHeight/fScaleFactorHeight, color);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_DEPTH_TEST);

	}


	//        {
	//            String str = "" + minecraft.player.getFoodData().getExhaustionLevel() + ", " + minecraft.player.getFoodData().getSaturationLevel();
	//            int x = (screenWidth - font.width(str)) / 2;
	//            int y = screenHeight - 64;
	//            font.draw(str, x + 1, y, 0xffffff);
	//        }

#ifndef _FINAL_BUILD
	MemSect(31);
    if (minecraft->options->renderDebug)
	{
        glPushMatrix();
        if (Minecraft::warezTime > 0) glTranslatef(0, 32, 0);
        font->drawShadow(ClientConstants::VERSION_STRING + L" (" + minecraft->fpsString + L")", iSafezoneXHalf+2, 20, 0xffffff);
        font->drawShadow(L"Seed: " + std::to_wstring(minecraft->level->getLevelData()->getSeed() ), iSafezoneXHalf+2, 32 + 00, 0xffffff);
        font->drawShadow(minecraft->gatherStats1(), iSafezoneXHalf+2, 32 + 10, 0xffffff);
        font->drawShadow(minecraft->gatherStats2(), iSafezoneXHalf+2, 32 + 20, 0xffffff);
        font->drawShadow(minecraft->gatherStats3(), iSafezoneXHalf+2, 32 + 30, 0xffffff);
        font->drawShadow(minecraft->gatherStats4(), iSafezoneXHalf+2, 32 + 40, 0xffffff);

		// TERRAIN FEATURES
		int iYPos=82;

		if(minecraft->level->dimension->id==0)
		{
			wstring wfeature[eTerrainFeature_Count];

			wfeature[eTerrainFeature_Stronghold] = L"Stronghold: ";
			wfeature[eTerrainFeature_Mineshaft] = L"Mineshaft: ";
			wfeature[eTerrainFeature_Village] = L"Village: ";
			wfeature[eTerrainFeature_Ravine] = L"Ravine: ";

			for(int i=0;i<app.m_vTerrainFeatures.size();i++)
			{
				FEATURE_DATA *pFeatureData=app.m_vTerrainFeatures[i];

				wstring itemInfo = L"[" + std::to_wstring( pFeatureData->x*16 ) + L", " + std::to_wstring( pFeatureData->z*16 ) + L"] ";
				wfeature[pFeatureData->eTerrainFeature] += itemInfo;
			}

			for( int i = eTerrainFeature_Stronghold; i < (int) eTerrainFeature_Count; i++ )
			{
				font->drawShadow(wfeature[i], iSafezoneXHalf + 2, iYPos, 0xffffff);
				iYPos+=10;
			}
		}

		//font->drawShadow(minecraft->gatherStats5(), iSafezoneXHalf+2, 32 + 10, 0xffffff);
       {
			/* 4J - removed
            long max = Runtime.getRuntime().maxMemory();
            long total = Runtime.getRuntime().totalMemory();
            long free = Runtime.getRuntime().freeMemory();
            long used = total - free;
            String msg = "Used memory: " + (used * 100 / max) + "% (" + (used / 1024 / 1024) + "MB) of " + (max / 1024 / 1024) + "MB";
            drawString(font, msg, screenWidth - font.width(msg) - 2, 2, 0xe0e0e0);
            msg = "Allocated memory: " + (total * 100 / max) + "% (" + (total / 1024 / 1024) + "MB)";
            drawString(font, msg, screenWidth - font.width(msg) - 2, 12, 0xe0e0e0);
			*/
        }
		// 4J Stu - Moved these so that they don't overlap
		double xBlockPos = floor(minecraft->player->x);
		double yBlockPos = floor(minecraft->player->y);
		double zBlockPos = floor(minecraft->player->z);
        drawString(font, L"x: " + std::to_wstring(minecraft->player->x) + L"/ Head: " + std::to_wstring(static_cast<int>(xBlockPos)) + L"/ Chunk: " + std::to_wstring(minecraft->player->xChunk), iSafezoneXHalf+2, iYPos + 8 * 0, 0xe0e0e0);
        drawString(font, L"y: " + std::to_wstring(minecraft->player->y) + L"/ Head: " + std::to_wstring(static_cast<int>(yBlockPos)), iSafezoneXHalf+2, iYPos + 8 * 1, 0xe0e0e0);
        drawString(font, L"z: " + std::to_wstring(minecraft->player->z) + L"/ Head: " + std::to_wstring(static_cast<int>(zBlockPos)) + L"/ Chunk: " + std::to_wstring(minecraft->player->zChunk), iSafezoneXHalf+2, iYPos + 8 * 2, 0xe0e0e0);
		drawString(font, L"f: " + std::to_wstring(Mth::floor(minecraft->player->yRot * 4.0f / 360.0f + 0.5) & 0x3) + L"/ yRot: " + std::to_wstring(minecraft->player->yRot), iSafezoneXHalf+2, iYPos + 8 * 3, 0xe0e0e0);
		iYPos += 8*4;

		int px = Mth::floor(minecraft->player->x);
		int py = Mth::floor(minecraft->player->y);
		int pz = Mth::floor(minecraft->player->z);
		if (minecraft->level != NULL && minecraft->level->hasChunkAt(px, py, pz))
		{
			LevelChunk *chunkAt = minecraft->level->getChunkAt(px, pz);
			Biome *biome = chunkAt->getBiome(px & 15, pz & 15, minecraft->level->getBiomeSource());
			drawString(
				font,
				L"b: " + biome->m_name + L" (" + std::to_wstring(biome->id) + L")", iSafezoneXHalf+2, iYPos, 0xe0e0e0);
		}

        glPopMatrix();
    }
	MemSect(0);
#endif

	lastTickA = a;
	// 4J Stu - This is now displayed in a xui scene
#if 0
	// Jukebox CD message
    if (overlayMessageTime > 0)
	{
        float t = overlayMessageTime - a;
        int alpha = (int) (t * 256 / 20);
        if (alpha > 255) alpha = 255;
        if (alpha > 0)
		{
            glPushMatrix();

			if(bTwoPlayerSplitscreen)
			{
				glTranslatef((float)((screenWidth / 2)+iWidthOffset), ((float)(screenHeight+iHeightOffset)) - iTooltipsYOffset -12 -iSafezoneYHalf, 0);
			}
			else
			{
				glTranslatef(((float)screenWidth) / 2, ((float)screenHeight) - iTooltipsYOffset - 12 -iSafezoneYHalf, 0);
			}
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            int col = 0xffffff;
            if (animateOverlayMessageColor)
			{
                col = Color::HSBtoRGB(t / 50.0f, 0.7f, 0.6f) & 0xffffff;
            }
			// 4J-PB - this is the string displayed when cds are placed in a jukebox
            font->draw(overlayMessageString,-font->width(overlayMessageString) / 2, -20, col + (alpha << 24));
            glDisable(GL_BLEND);
            glPopMatrix();
        }
    }
#endif

    unsigned int max = 10;
    bool isChatting = false;
    if (dynamic_cast<ChatScreen *>(minecraft->screen) != NULL)
	{
        max = 20;
        isChatting = true;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_ALPHA_TEST);

#if defined(_WINDOWS64)
    glPushMatrix();
    glTranslatef(0.0f, static_cast<float>(screenHeight - iSafezoneYHalf - iTooltipsYOffset - 16 - 3 + 22) - 24.0f, 0.0f);

	if(bDisplayGui)
	{
		int iPad=minecraft->player->GetXboxPad();
		for (unsigned int i = 0; i < guiMessages[iPad].size() && i < max; i++)
		{
			if (guiMessages[iPad][i].ticks < 20 * 10 || isChatting)
			{
				double t = guiMessages[iPad][i].ticks / (20 * 10.0);
				t = 1 - t;
				t = t * 10;
				if (t < 0) t = 0;
				if (t > 1) t = 1;
				t = t * t;
				int alpha = static_cast<int>(255 * t);
				if (isChatting) alpha = 255;

				if (alpha > 0)
				{
					int x = iSafezoneXHalf+2;
					int y = -(static_cast<int>(i)) * 9;
					if(bTwoPlayerSplitscreen)
					{
						y+= iHeightOffset;
					}

					wstring msg = guiMessages[iPad][i].string;
					this->fill(0, y - 1, screenWidth/fScaleFactorWidth, y + 8, (alpha / 2) << 24);
					glEnable(GL_BLEND);
					font->drawShadow(msg, iSafezoneXHalf+4, y, 0xffffff + (alpha << 24));
				}
			}
		}
	}
    glPopMatrix();
#endif

	// 4J Stu - Copied over but not used
#if 0
	if (minecraft.player instanceof MultiplayerLocalPlayer && minecraft.options.keyPlayerList.isDown)
	{
		ClientConnection connection = ((MultiplayerLocalPlayer) minecraft.player).connection;
		List<PlayerInfo> playerInfos = connection.playerInfos;
		int slots = connection.maxPlayers;

		int rows = slots;
		int cols = 1;
		while (rows > 20) {
			cols++;
			rows = (slots + cols - 1) / cols;
		}

		/*
		* int fakeCount = 39; while (playerInfos.size() > fakeCount)
		* playerInfos.remove(playerInfos.size() - 1); while (playerInfos.size() <
		* fakeCount) playerInfos.add(new PlayerInfo("fiddle"));
		*/

		int slotWidth = 300 / cols;
		if (slotWidth > 150) slotWidth = 150;

		int xxo = (screenWidth - cols * slotWidth) / 2;
		int yyo = 10;
		fill(xxo - 1, yyo - 1, xxo + slotWidth * cols, yyo + 9 * rows, 0x80000000);
		for (int i = 0; i < slots; i++) {
			int xo = xxo + i % cols * slotWidth;
			int yo = yyo + i / cols * 9;

			fill(xo, yo, xo + slotWidth - 1, yo + 8, 0x20ffffff);
			glColor4f(1, 1, 1, 1);
			glEnable(GL_ALPHA_TEST);

			if (i < playerInfos.size()) {
				PlayerInfo pl = playerInfos.get(i);
				font.drawShadow(pl.name, xo, yo, 0xffffff);
				minecraft.textures.bind(minecraft.textures.loadTexture("/gui/icons.png"));
				int xt = 0;
				int yt = 0;
				xt = 0;
				yt = 0;
				if (pl.latency < 0) yt = 5;
				else if (pl.latency < 150) yt = 0;
				else if (pl.latency < 300) yt = 1;
				else if (pl.latency < 600) yt = 2;
				else if (pl.latency < 1000) yt = 3;
				else yt = 4;

				blitOffset += 100;
				blit(xo + slotWidth - 12, yo, 0 + xt * 10, 176 + yt * 8, 10, 8);
				blitOffset -= 100;
			}
		}
	}
#endif

	if(bDisplayGui && bTwoPlayerSplitscreen)
	{
		// pop the scaled matrix
		glPopMatrix();
	}

	glColor4f(1, 1, 1, 1);
    glDisable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
}

// Moved to the xui base scene
// void Gui::renderBossHealth(void)
// {
// 	if (EnderDragonRenderer::bossInstance == NULL) return;
//
// 	shared_ptr<EnderDragon> boss = EnderDragonRenderer::bossInstance;
// 	EnderDragonRenderer::bossInstance = NULL;
//
// 	Minecraft *pMinecraft=Minecraft::GetInstance();
//
// 	Font *font = pMinecraft->font;
//
// 	ScreenSizeCalculator ssc(pMinecraft->options, pMinecraft->width_phys, pMinecraft->height_phys);
// 	int screenWidth = ssc.getWidth();
//
// 	int w = 182;
// 	int xLeft = screenWidth / 2 - w / 2;
//
// 	int progress = (int) (boss->getSynchedHealth() / (float) boss->getMaxHealth() * (float) (w + 1));
//
// 	int yo = 12;
// 	blit(xLeft, yo, 0, 74, w, 5);
// 	blit(xLeft, yo, 0, 74, w, 5);
// 	if (progress > 0)
// 	{
// 		blit(xLeft, yo, 0, 79, progress, 5);
// 	}
//
// 	wstring msg = L"Boss health - NON LOCALISED";
// 	font->drawShadow(msg, screenWidth / 2 - font->width(msg) / 2, yo - 10, 0xff00ff);
// 	glColor4f(1, 1, 1, 1);
// 	glBindTexture(GL_TEXTURE_2D, pMinecraft->textures->loadTexture(TN_GUI_ICONS) );//"/gui/icons.png"));
//
// }

void Gui::renderPumpkin(int w, int h)
{
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, 1);
    glDisable(GL_ALPHA_TEST);

	MemSect(31);
    minecraft->textures->bindTexture(&PUMPKIN_BLUR_LOCATION);
	MemSect(0);
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV((float)(0), (float)( h), (float)( -90), (float)( 0), (float)( 1));
    t->vertexUV((float)(w), (float)( h), (float)( -90), (float)( 1), (float)( 1));
    t->vertexUV((float)(w), (float)( 0), (float)( -90), (float)( 1), (float)( 0));
    t->vertexUV((float)(0), (float)( 0), (float)( -90), (float)( 0), (float)( 0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);

}

void Gui::renderVignette(float br, int w, int h)
{
    br = 1 - br;
    if (br < 0) br = 0;
    if (br > 1) br = 1;
    tbr += (br - tbr) * 0.01f;

#if 0  // 4J - removed - TODO put back when we have blend functions implemented
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
    glColor4f(tbr, tbr, tbr, 1);
    glBindTexture(GL_TEXTURE_2D, minecraft->textures->loadTexture(TN__BLUR__MISC_VIGNETTE));//L"%blur%/misc/vignette.png"));
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV((float)(0), (float)( h), (float)( -90), (float)( 0), (float)( 1));
    t->vertexUV((float)(w), (float)( h), (float)( -90), (float)( 1), (float)( 1));
    t->vertexUV((float)(w), (float)( 0), (float)( -90), (float)( 1), (float)( 0));
    t->vertexUV((float)(0), (float)( 0), (float)( -90), (float)( 0), (float)( 0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1, 1, 1, 1);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
}

void Gui::renderTp(float br, int w, int h)
{
    if (br < 1)
	{
        br = br * br;
        br = br * br;
        br = br * 0.8f + 0.2f;
    }

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1, 1, 1, br);
	MemSect(31);
    minecraft->textures->bindTexture(&TextureAtlas::LOCATION_BLOCKS);
	MemSect(0);

	Icon *slot = Tile::portalTile->getTexture(Facing::UP);
    float u0 = slot->getU0();
    float v0 = slot->getV0();
    float u1 = slot->getU1();
    float v1 = slot->getV1();
    Tesselator *t = Tesselator::getInstance();
    t->begin();
    t->vertexUV((float)(0), (float)( h), (float)( -90), (float)( u0), (float)( v1));
    t->vertexUV((float)(w), (float)( h), (float)( -90), (float)( u1), (float)( v1));
    t->vertexUV((float)(w), (float)( 0), (float)( -90), (float)( u1), (float)( v0));
    t->vertexUV((float)(0), (float)( 0), (float)( -90), (float)( u0), (float)( v0));
    t->end();
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);
    glColor4f(1, 1, 1, 1);

}

void Gui::renderSlot(int slot, int x, int y, float a)
{
    shared_ptr<ItemInstance> item = minecraft->player->inventory->items[slot];
    if (item == NULL) return;

    float pop = item->popTime - a;
    if (pop > 0)
	{
        glPushMatrix();
        float squeeze = 1 + pop / (float) Inventory::POP_TIME_DURATION;
        glTranslatef((float)(x + 8), (float)(y + 12), 0);
        glScalef(1 / squeeze, (squeeze + 1) / 2, 1);
        glTranslatef((float)-(x + 8), (float)-(y + 12), 0);
    }

    itemRenderer->renderAndDecorateItem(minecraft->font, minecraft->textures, item, x, y);

    if (pop > 0)
	{
        glPopMatrix();
    }

    itemRenderer->renderGuiItemDecorations(minecraft->font, minecraft->textures, item, x, y);

}

void Gui::tick()
{
    if (overlayMessageTime > 0) overlayMessageTime--;
    tickCount++;

	for(int iPad=0;iPad<XUSER_MAX_COUNT;iPad++)
	{
		// 4J Stu - Fix for #10929 - MP LAB: Network Disconnects: Host does not receive an error message stating the client left the game when viewing the Pause Menu.
		// We don't show the guiMessages when a menu is up, so don't fade them out
		if(!ui.GetMenuDisplayed(iPad))
		{
            for (auto& it : guiMessages[iPad])
            {
				it.ticks++;
			}
		}
	}
}

void Gui::clearMessages(int iPad)
{
	if(iPad==-1)
	{
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(minecraft->localplayers[i])
			{
				guiMessages[i].clear();
			}
		}
	}
	else
	{
		guiMessages[iPad].clear();
	}
}


void Gui::addMessage(const wstring& _string,int iPad,bool bIsDeathMessage)
{
	wstring string = _string;	// 4J - Take copy of input as it is const
	//int iScale=1;

	//if((minecraft->player->m_iScreenSection==C4JRender::VIEWPORT_TYPE_SPLIT_TOP) ||
	//	(minecraft->player->m_iScreenSection==C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM))
	//{
	//	iScale=2;
	//}

 //   while (minecraft->font->width(string) > (m_iMaxMessageWidth*iScale))
	//{
 //       unsigned int i = 1;
 //       while (i < string.length() && minecraft->font->width(string.substr(0, i + 1)) <= (m_iMaxMessageWidth*iScale))
	//	{
 //           i++;
 //       }
	//	int iLast=string.find_last_of(L" ",i);

	//	// if a space was found, include the space on this line
	//	if(iLast!=i)
	//	{
	//		iLast++;
	//	}
	//	addMessage(string.substr(0, iLast), iPad);
	//	string = string.substr(iLast);
 //   }

	int maximumChars;

	switch(minecraft->player->m_iScreenSection)
	{
	case C4JRender::VIEWPORT_TYPE_SPLIT_TOP:
	case C4JRender::VIEWPORT_TYPE_SPLIT_BOTTOM:
	case C4JRender::VIEWPORT_TYPE_FULLSCREEN:
		if(RenderManager.IsHiDef())
		{
			maximumChars = 105;
		}
		else
		{
			maximumChars = 55;
		}
#ifdef __PSVITA__
		maximumChars = 90;
#endif
		switch(XGetLanguage())
		{
		case XC_LANGUAGE_JAPANESE:
		case XC_LANGUAGE_TCHINESE:
		case XC_LANGUAGE_KOREAN:
			if(RenderManager.IsHiDef())
			{
				maximumChars = 70;
			}
			else
			{
				maximumChars = 35;
			}
#ifdef __PSVITA__
			maximumChars = 55;
#endif
			break;
		}
		break;
	default:
		 maximumChars = 55;
		 switch(XGetLanguage())
		 {
		 case XC_LANGUAGE_JAPANESE:
		 case XC_LANGUAGE_TCHINESE:
		 case XC_LANGUAGE_KOREAN:
			 maximumChars = 35;
			 break;
		 }
		break;
	}


	while (string.length() > maximumChars)
	{
        unsigned int i = 1;
        while (i < string.length() && (i + 1) <= maximumChars)
		{
            i++;
        }
		int iLast=(int)string.find_last_of(L" ",i);
		switch(XGetLanguage())
		{
			case XC_LANGUAGE_JAPANESE:
			case XC_LANGUAGE_TCHINESE:
			case XC_LANGUAGE_KOREAN:
				iLast = maximumChars;
				break;
			default:
				iLast=(int)string.find_last_of(L" ",i);
				break;
		}

		// if a space was found, include the space on this line
		if(iLast!=i)
		{
			iLast++;
		}
		addMessage(string.substr(0, iLast), iPad, bIsDeathMessage);
		string = string.substr(iLast);
    }

	if(iPad==-1)
	{
		// add to all
		for(int i=0;i<XUSER_MAX_COUNT;i++)
		{
			if(minecraft->localplayers[i] && !(bIsDeathMessage && app.GetGameSettings(i,eGameSetting_DeathMessages)==0))
			{
				guiMessages[i].insert(guiMessages[i].begin(), GuiMessage(string));
				while (guiMessages[i].size() > 50)
				{
					guiMessages[i].pop_back();
				}
			}
		}
	}
	else if(!(bIsDeathMessage && app.GetGameSettings(iPad,eGameSetting_DeathMessages)==0))
	{
		guiMessages[iPad].insert(guiMessages[iPad].begin(), GuiMessage(string));
		while (guiMessages[iPad].size() > 50)
		{
			guiMessages[iPad].pop_back();
		}
	}


}

// 4J Added
float Gui::getOpacity(int iPad, DWORD index)
{
	float opacityPercentage = 0;
	if (guiMessages[iPad].size() > index && guiMessages[iPad][index].ticks < 20 * 10)
	{
		double t = guiMessages[iPad][index].ticks / (20 * 10.0);
		t = 1 - t;
		t = t * 10;
		if (t < 0) t = 0;
		if (t > 1) t = 1;
		t = t * t;
		opacityPercentage = t;
	}
	return opacityPercentage;
}

float Gui::getJukeboxOpacity(int iPad)
{
	float t = overlayMessageTime - lastTickA;
    int alpha = (int) (t * 256 / 20);
    if (alpha > 255) alpha = 255;
	alpha /= 255;

	return alpha;
}

void Gui::setNowPlaying(const wstring& string)
{
//	overlayMessageString = L"Now playing: " + string;
	overlayMessageString = app.GetString(IDS_NOWPLAYING) + string;
    overlayMessageTime = 20 * 3;
    animateOverlayMessageColor = true;
}

void Gui::displayClientMessage(int messageId, int iPad)
{
    //Language *language = Language::getInstance();
    wstring languageString = app.GetString(messageId);//language->getElement(messageId);

    addMessage(languageString, iPad);
}

// 4J Added
void Gui::renderGraph(int dataLength, int dataPos, __int64 *dataA, float dataAScale, int dataAWarning, __int64 *dataB, float dataBScale, int dataBWarning)
{
	int height = minecraft->height;
	// This causes us to cover xScale*dataLength pixels in the horizontal
	int xScale = 1;
	if(dataA != NULL && dataB != NULL) xScale = 2;

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (float)minecraft->width, (float)height, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);

	glLineWidth(1);
	glDisable(GL_TEXTURE_2D);
	Tesselator *t = Tesselator::getInstance();

	t->begin(GL_LINES);
	for (int i = 0; i < dataLength; i++)
	{
		int col = ((i - dataPos) & (dataLength - 1)) * 255 / dataLength;
		int cc = col * col / 255;
		cc = cc * cc / 255;
		int cc2 = cc * cc / 255;
		cc2 = cc2 * cc2 / 255;

		if( dataA != NULL )
		{
			if (dataA[i] > dataAWarning)
			{
				t->color(0xff000000 + cc * 65536);
			}
			else
			{
				t->color(0xff000000 + cc * 256);
			}

			__int64 aVal = dataA[i] / dataAScale;

			t->vertex((float)(xScale*i + 0.5f), (float)( height - aVal + 0.5f), (float)( 0));
			t->vertex((float)(xScale*i + 0.5f), (float)( height + 0.5f), (float)( 0));
		}

		if( dataB != NULL )
		{
			if (dataB[i]>dataBWarning)
			{
				t->color(0xff000000 + cc * 65536 + cc * 256 + cc * 1);
			}
			else
			{
				t->color(0xff808080 + cc/2 * 256);
			}

			__int64 bVal = dataB[i] / dataBScale;

			t->vertex((float)(xScale*i + (xScale - 1) + 0.5f), (float)( height - bVal + 0.5f), (float)( 0));
			t->vertex((float)(xScale*i + (xScale - 1) + 0.5f), (float)( height + 0.5f), (float)( 0));
		}
	}
	t->end();

	glEnable(GL_TEXTURE_2D);
}

void Gui::renderStackedGraph(int dataPos, int dataLength, int dataSources, __int64 (*func)(unsigned int dataPos, unsigned int dataSource) )
{
	int height = minecraft->height;

	glClear(GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, (float)minecraft->width, (float)height, 0, 1000, 3000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -2000);

	glLineWidth(1);
	glDisable(GL_TEXTURE_2D);
	Tesselator *t = Tesselator::getInstance();

	t->begin(GL_LINES);
	__int64 thisVal = 0;
	__int64 topVal = 0;
	for (int i = 0; i < dataLength; i++)
	{
		thisVal = 0;
		topVal = 0;
		int col = ((i - dataPos) & (dataLength - 1)) * 255 / dataLength;
		int cc = col * col / 255;
		cc = cc * cc / 255;
		int cc2 = cc * cc / 255;
		cc2 = cc2 * cc2 / 255;


		for(unsigned int source = 0; source < dataSources; ++source )
		{
			thisVal = func( i, source );

			if( thisVal > 0 )
			{
				float vary = (float)source/dataSources;
				int fColour = floor(vary * 0xffffff);

				int colour = 0xff000000 + fColour;
				//printf("Colour is %x\n", colour);
				t->color(colour);

				t->vertex((float)(i + 0.5f), (float)( height - topVal - thisVal + 0.5f), (float)( 0));
				t->vertex((float)(i + 0.5f), (float)( height - topVal + 0.5f), (float)( 0));

				topVal += thisVal;
			}
		}

		// Draw some horizontals
		for(unsigned int horiz = 1; horiz < 7; ++horiz )
		{
			t->color(0xff000000);

			t->vertex((float)(0 + 0.5f), (float)( height - (horiz*100) + 0.5f), (float)( 0));
			t->vertex((float)(dataLength + 0.5f), (float)( height - (horiz*100) + 0.5f), (float)( 0));
		}
	}
	t->end();

	glEnable(GL_TEXTURE_2D);
}
