#include "stdafx.h"

#include "..\..\..\Minecraft.World\net.minecraft.world.item.crafting.h"
#include "..\..\..\Minecraft.World\net.minecraft.world.level.tile.h"
#include "..\..\..\Minecraft.World\net.minecraft.stats.h"
#include "..\..\LocalPlayer.h"
#include "IUIScene_CraftingMenu.h"

Recipy::_eGroupType IUIScene_CraftingMenu::m_GroupTypeMapping4GridA[IUIScene_CraftingMenu::m_iMaxGroup2x2]=
{
	Recipy::eGroupType_Structure,
	Recipy::eGroupType_Tool,
	Recipy::eGroupType_Food,
	Recipy::eGroupType_Mechanism,
	Recipy::eGroupType_Transport,
	Recipy::eGroupType_Decoration,
};

Recipy::_eGroupType IUIScene_CraftingMenu::m_GroupTypeMapping9GridA[IUIScene_CraftingMenu::m_iMaxGroup3x3]=
{
	Recipy::eGroupType_Structure,
	Recipy::eGroupType_Tool,
	Recipy::eGroupType_Food,
	Recipy::eGroupType_Armour,
	Recipy::eGroupType_Mechanism,
	Recipy::eGroupType_Transport,
	Recipy::eGroupType_Decoration,
};


LPCWSTR IUIScene_CraftingMenu::m_GroupIconNameA[m_iMaxGroup3x3]=
{
	L"Structures",//Recipy::eGroupType_Structure,
	L"Tools",//Recipy::eGroupType_Tool,
	L"Food",//Recipy::eGroupType_Food,
	L"Armour",//Recipy::eGroupType_Armour,
	L"Mechanisms",//Recipy::eGroupType_Mechanism,
	L"Transport",//Recipy::eGroupType_Transport,
	L"Decoration",//Recipy::eGroupType_Decoration,
};

IUIScene_CraftingMenu::_eGroupTab IUIScene_CraftingMenu::m_GroupTabBkgMapping2x2A[m_iMaxGroup2x2]=
{
	eGroupTab_Left,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Right,
};

IUIScene_CraftingMenu::_eGroupTab IUIScene_CraftingMenu::m_GroupTabBkgMapping3x3A[m_iMaxGroup3x3]=
{
	eGroupTab_Left,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Middle,
	eGroupTab_Right,
};


// mapping array to map the base objects to their description string
// This should map the enums
// enum
// {
// 	eBaseItemType_undefined=0,
// 	eBaseItemType_sword,
// 	eBaseItemType_shovel,
// 	eBaseItemType_pickaxe,
// 	eBaseItemType_hatchet,
// 	eBaseItemType_hoe,
// 	eBaseItemType_door,
// 	eBaseItemType_helmet,
// 	eBaseItemType_chestplate,
// 	eBaseItemType_leggings,
// 	eBaseItemType_boots,
// 	eBaseItemType_ingot,
// 	eBaseItemType_rail,
// 	eBaseItemType_block,
// 	eBaseItemType_pressureplate,
// 	eBaseItemType_stairs,
// 	eBaseItemType_cloth,
// 	eBaseItemType_dyepowder,
//  eBaseItemType_structplanks
// 	eBaseItemType_structblock,
// 	eBaseItemType_slab,
// 	eBaseItemType_halfslab,
// 	eBaseItemType_torch,
// 	eBaseItemType_bow,
// 	eBaseItemType_pockettool,
// 	eBaseItemType_utensil,
//
// }
// eBaseItemType;

IUIScene_CraftingMenu::IUIScene_CraftingMenu()
{
	m_iCurrentSlotHIndex=0;
	m_iCurrentSlotVIndex=1;

	for(int i=0;i<m_iMaxHSlotC;i++)
	{
		CanBeMadeA[i].iCount=0;
		CanBeMadeA[i].iItemBaseType=0;
	}
	memset(CanBeMadeA,0,sizeof(CANBEMADE)*m_iMaxHSlotC);
	m_iRecipeC=0;
	m_iGroupIndex=0;

	for(int i=0;i<m_iMaxDisplayedVSlotC;i++)
	{
		iVSlotIndexA[i]=i; // start with 0,1,2
	}

	m_iDisplayDescription=DISPLAY_INVENTORY;
	m_iIngredientsC=0;
}

LPCWSTR IUIScene_CraftingMenu::GetGroupNameText(int iGroupType)
{
	switch(iGroupType)
	{
	case ShapedRecipy::eGroupType_Tool:
		return app.GetString( IDS_GROUPNAME_TOOLS );
	case ShapedRecipy::eGroupType_Food:
		return app.GetString( IDS_GROUPNAME_FOOD);
	case ShapedRecipy::eGroupType_Structure:
		return app.GetString( IDS_GROUPNAME_STRUCTURES );
	case ShapedRecipy::eGroupType_Armour:
		return app.GetString( IDS_GROUPNAME_ARMOUR );
	case ShapedRecipy::eGroupType_Mechanism:
		return app.GetString( IDS_GROUPNAME_MECHANISMS );
	case ShapedRecipy::eGroupType_Transport:
		return app.GetString( IDS_GROUPNAME_TRANSPORT );
	case ShapedRecipy::eGroupType_Decoration:
	default:
		return app.GetString( IDS_GROUPNAME_DECORATIONS );
	}
}

bool IUIScene_CraftingMenu::handleKeyDown(int iPad, int iAction, bool bRepeat)
{
	bool bHandled = false;

	if(m_bIgnoreKeyPresses) return bHandled;

	// ignore key repeats of the X key - because it's X to open this menu, it can come through as a repeat on opening
	if(bRepeat &&(iAction==ACTION_MENU_X))
	{
		return S_OK;
	}

	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( pMinecraft->localgameModes[getPad()] != NULL )
	{
		Tutorial *tutorial = pMinecraft->localgameModes[getPad()]->getTutorial();
		if(tutorial != NULL)
		{
			tutorial->handleUIInput(iAction);
			if(ui.IsTutorialVisible(getPad()) && !tutorial->isInputAllowed(iAction))
			{
				return S_OK;
			}
		}
	}


	switch(iAction)
	{
	case ACTION_MENU_X:

		// change the display
		m_iDisplayDescription++;
		if(m_iDisplayDescription==DISPLAY_MAX) m_iDisplayDescription=DISPLAY_INVENTORY;
		ui.PlayUISFX(eSFX_Focus);
		UpdateMultiPanel();
		UpdateTooltips();
		break;
	case ACTION_MENU_PAUSEMENU:
	case ACTION_MENU_B:
		ui.ShowTooltip( iPad, eToolTipButtonX, false );
		ui.ShowTooltip( iPad, eToolTipButtonB, false );
		ui.ShowTooltip( iPad, eToolTipButtonA, false );
		ui.ShowTooltip( iPad, eToolTipButtonRB, false );
		// kill the crafting xui
		//ui.PlayUISFX(eSFX_Back);
		ui.CloseUIScenes(iPad);

		bHandled = true;
		break;
	case ACTION_MENU_A:
#ifdef __ORBIS__
	case ACTION_MENU_TOUCHPAD_PRESS:
#endif
		// Do some crafting!
		if(m_pPlayer && m_pPlayer->inventory)
		{
			//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
			Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
			// Force a make if the debug is on
			if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
			{
				if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
				{
					int iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];

					int iRecipe= CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
					shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);
					//int iIcon=pTempItemInst->getItem()->getIcon(pTempItemInst->getAuxValue());

					if( pMinecraft->localgameModes[iPad] != NULL)
					{
						Tutorial *tutorial = pMinecraft->localgameModes[iPad]->getTutorial();
						if(tutorial != NULL)
						{
							tutorial->onCrafted(pTempItemInst);
						}
					}

					pMinecraft->localgameModes[iPad]->handleCraftItem(iRecipe,m_pPlayer);

					if(m_pPlayer->inventory->add(pTempItemInst)==false)
					{
						// no room in inventory, so throw it down
						m_pPlayer->drop(pTempItemInst);
					}
					// play a sound
					//pMinecraft->soundEngine->playUI( L"random.pop", 1.0f, 1.0f);
					ui.PlayUISFX(eSFX_Craft);
				}
			}
			else if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
			{
				int iSlot;
				if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
				{
					iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
				}
				else
				{
					iSlot=0;
				}
				int iRecipe= CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
				shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);
				//int iIcon=pTempItemInst->getItem()->getIcon(pTempItemInst->getAuxValue());

				if( pMinecraft->localgameModes[iPad] != NULL )
				{
					Tutorial *tutorial = pMinecraft->localgameModes[iPad]->getTutorial();
					if(tutorial != NULL)
					{
						tutorial->createItemSelected(pTempItemInst, pRecipeIngredientsRequired[iRecipe].bCanMake[iPad]);
					}
				}

				if(pRecipeIngredientsRequired[iRecipe].bCanMake[iPad])
				{
					pTempItemInst->onCraftedBy(m_pPlayer->level, dynamic_pointer_cast<Player>( m_pPlayer->shared_from_this() ), pTempItemInst->count );
					// TODO 4J Stu - handleCraftItem should do a lot more than what it does, loads of the "can we craft" code should also probably be
					// shifted to the GameMode
					pMinecraft->localgameModes[iPad]->handleCraftItem(iRecipe,m_pPlayer);

					// play a sound
					//pMinecraft->soundEngine->playUI( L"random.pop", 1.0f, 1.0f);
					ui.PlayUISFX(eSFX_Craft);

					if(pTempItemInst->id != Item::fireworksCharge_Id && pTempItemInst->id != Item::fireworks_Id)
					{
						// and remove those resources from your inventory
						for(int i=0;i<pRecipeIngredientsRequired[iRecipe].iIngC;i++)
						{
							for(int j=0;j<pRecipeIngredientsRequired[iRecipe].iIngValA[i];j++)
							{
								shared_ptr<ItemInstance> ingItemInst = nullptr;
								// do we need to remove a specific aux value?
								if(pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]!=Recipes::ANY_AUX_VALUE)
								{
									ingItemInst = m_pPlayer->inventory->getResourceItem( pRecipeIngredientsRequired[iRecipe].iIngIDA[i],pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i] );
									m_pPlayer->inventory->removeResource(pRecipeIngredientsRequired[iRecipe].iIngIDA[i],pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i]);
								}
								else
								{
									ingItemInst = m_pPlayer->inventory->getResourceItem( pRecipeIngredientsRequired[iRecipe].iIngIDA[i] );
									m_pPlayer->inventory->removeResource(pRecipeIngredientsRequired[iRecipe].iIngIDA[i]);
								}

								// 4J Stu - Fix for #13097 - Bug: Milk Buckets are removed when crafting Cake
								if (ingItemInst != NULL)
								{
									if (ingItemInst->getItem()->hasCraftingRemainingItem())
									{
										// replace item with remaining result
										m_pPlayer->inventory->add( shared_ptr<ItemInstance>( new ItemInstance(ingItemInst->getItem()->getCraftingRemainingItem()) ) );
									}

								}
							}
						}

						// 4J Stu - Fix for #13119 - We should add the item after we remove the ingredients
						if(m_pPlayer->inventory->add(pTempItemInst)==false )
						{
							// no room in inventory, so throw it down
							m_pPlayer->drop(pTempItemInst);
						}

						//4J Gordon: Achievements
						switch(pTempItemInst->id )
						{
						case Tile::workBench_Id:		m_pPlayer->awardStat(GenericStats::buildWorkbench(),		GenericStats::param_buildWorkbench());		break;
						case Item::pickAxe_wood_Id:		m_pPlayer->awardStat(GenericStats::buildPickaxe(),			GenericStats::param_buildPickaxe());		break;
						case Tile::furnace_Id:			m_pPlayer->awardStat(GenericStats::buildFurnace(),			GenericStats::param_buildFurnace());		break;
						case Item::hoe_wood_Id:			m_pPlayer->awardStat(GenericStats::buildHoe(),				GenericStats::param_buildHoe());			break;
						case Item::bread_Id:			m_pPlayer->awardStat(GenericStats::makeBread(),				GenericStats::param_makeBread());			break;
						case Item::cake_Id:				m_pPlayer->awardStat(GenericStats::bakeCake(),				GenericStats::param_bakeCake());			break;
						case Item::pickAxe_stone_Id:	m_pPlayer->awardStat(GenericStats::buildBetterPickaxe(),	GenericStats::param_buildBetterPickaxe());	break;
						case Item::sword_wood_Id:		m_pPlayer->awardStat(GenericStats::buildSword(),			GenericStats::param_buildSword());			break;
						case Tile::dispenser_Id:		m_pPlayer->awardStat(GenericStats::dispenseWithThis(),		GenericStats::param_dispenseWithThis());	break;
						case Tile::enchantTable_Id:		m_pPlayer->awardStat(GenericStats::enchantments(),			GenericStats::param_enchantments());		break;
						case Tile::bookshelf_Id:		m_pPlayer->awardStat(GenericStats::bookcase(),				GenericStats::param_bookcase());			break;
						}

						// We've used some ingredients from our inventory, so update the recipes we can make
						CheckRecipesAvailable();
						// don't reset the vertical slots - we want to stay where we are
						UpdateVerticalSlots();
						UpdateHighlight();
					}
				}
				else
				{
					//pMinecraft->soundEngine->playUI( L"btn.back", 1.0f, 1.0f);
					ui.PlayUISFX(eSFX_CraftFail);
				}
			}
		}
		break;

	case ACTION_MENU_LEFT_SCROLL:
		// turn off the old group tab
		showTabHighlight(m_iGroupIndex,false);

		if(m_iGroupIndex==0)
		{
			if(m_iContainerType==RECIPE_TYPE_3x3)
			{
				m_iGroupIndex=m_iMaxGroup3x3-1;
			}
			else
			{
				m_iGroupIndex=m_iMaxGroup2x2-1;
			}
		}
		else
		{
			m_iGroupIndex--;
		}
		// turn on the new group
		showTabHighlight(m_iGroupIndex,true);

		m_iCurrentSlotHIndex=0;
		m_iCurrentSlotVIndex=1;

		CheckRecipesAvailable();
		// reset the vertical slots
		iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
		iVSlotIndexA[1]=0;
		iVSlotIndexA[2]=1;
		ui.PlayUISFX(eSFX_Focus);
		UpdateVerticalSlots();
		UpdateHighlight();
		setGroupText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));

		break;
	case ACTION_MENU_RIGHT_SCROLL:
		// turn off the old group tab
		showTabHighlight(m_iGroupIndex,false);

		m_iGroupIndex++;
		if(m_iContainerType==RECIPE_TYPE_3x3)
		{
			if(m_iGroupIndex==m_iMaxGroup3x3) m_iGroupIndex=0;
		}
		else
		{
			if(m_iGroupIndex==m_iMaxGroup2x2) m_iGroupIndex=0;
		}
		// turn on the new group
		showTabHighlight(m_iGroupIndex,true);

		m_iCurrentSlotHIndex=0;
		m_iCurrentSlotVIndex=1;
		CheckRecipesAvailable();
		// reset the vertical slots
		iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
		iVSlotIndexA[1]=0;
		iVSlotIndexA[2]=1;
		ui.PlayUISFX(eSFX_Focus);
		UpdateVerticalSlots();
		UpdateHighlight();
		setGroupText(GetGroupNameText(m_pGroupA[m_iGroupIndex]));
		break;
	}

	// 4J-Tomk - check if we've only got one vertical scroll slot (480, splits & Vita)
	bool bNoScrollSlots = false;
	if(m_bSplitscreen ||(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()))
	{
		bNoScrollSlots = true;
	}
#ifdef __PSVITA__
	bNoScrollSlots = true;
#endif

	// 4J Stu - We did used to swap the thumsticks based on Southpaw in this scene, but ONLY in this scene
	switch(iAction)
	{
	case ACTION_MENU_OTHER_STICK_UP:
		scrollDescriptionUp();
		break;
	case ACTION_MENU_OTHER_STICK_DOWN:
		scrollDescriptionDown();
		break;
	case ACTION_MENU_RIGHT:
		{
			int iOldHSlot=m_iCurrentSlotHIndex;

			m_iCurrentSlotHIndex++;
			if(m_iCurrentSlotHIndex>=m_iCraftablesMaxHSlotC) m_iCurrentSlotHIndex=0;
			m_iCurrentSlotVIndex=1;
			// clear the indices
			iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
			iVSlotIndexA[1]=0;
			iVSlotIndexA[2]=1;

			UpdateVerticalSlots();
			UpdateHighlight();
			// re-enable the old hslot
			if(CanBeMadeA[iOldHSlot].iCount>0)
			{
				setShowCraftHSlot(iOldHSlot,true);
			}
			ui.PlayUISFX(eSFX_Focus);
			bHandled = true;
		}
		break;
	case ACTION_MENU_LEFT:
		{
			if(m_iCraftablesMaxHSlotC!=0)
			{
				int iOldHSlot=m_iCurrentSlotHIndex;
				if(m_iCurrentSlotHIndex==0) m_iCurrentSlotHIndex=m_iCraftablesMaxHSlotC-1;
				else m_iCurrentSlotHIndex--;
				m_iCurrentSlotVIndex=1;
				// clear the indices
				iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
				iVSlotIndexA[1]=0;
				iVSlotIndexA[2]=1;

				UpdateVerticalSlots();
				UpdateHighlight();
				// re-enable the old hslot
				if(CanBeMadeA[iOldHSlot].iCount>0)
				{
					setShowCraftHSlot(iOldHSlot,true);
				}
				ui.PlayUISFX(eSFX_Focus);
			}
			bHandled = true;
		}
		break;
	case ACTION_MENU_UP:
		{
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
			{
				if(bNoScrollSlots)
				{
					if(iVSlotIndexA[1]==0)
					{
						iVSlotIndexA[1]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
					}
					else
					{
						iVSlotIndexA[1]--;
					}
					ui.PlayUISFX(eSFX_Focus);
				}
				else
					if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>2)
					{
						{
							if(m_iCurrentSlotVIndex!=0)
							{
								// just move the highlight
								m_iCurrentSlotVIndex--;
								ui.PlayUISFX(eSFX_Focus);
							}
							else
							{
								//move the slots
								iVSlotIndexA[2]=iVSlotIndexA[1];
								iVSlotIndexA[1]=iVSlotIndexA[0];
								// on 0 and went up, so cycle the values
								if(iVSlotIndexA[0]==0)
								{
									iVSlotIndexA[0]=CanBeMadeA[m_iCurrentSlotHIndex].iCount-1;
								}
								else
								{
									iVSlotIndexA[0]--;
								}
								ui.PlayUISFX(eSFX_Focus);
							}
						}
					}
					else
					{
						if(m_iCurrentSlotVIndex!=1)
						{
							// just move the highlight
							m_iCurrentSlotVIndex--;
							ui.PlayUISFX(eSFX_Focus);
						}
					}
					UpdateVerticalSlots();
					UpdateHighlight();
			}

		}
		break;
	case ACTION_MENU_DOWN:
		{
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
			{
				if(bNoScrollSlots)
				{
					if(iVSlotIndexA[1]==(CanBeMadeA[m_iCurrentSlotHIndex].iCount-1))
					{
						iVSlotIndexA[1]=0;
					}
					else
					{
						iVSlotIndexA[1]++;
					}
					ui.PlayUISFX(eSFX_Focus);

				}
				else
					if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>2)
					{
						if(m_iCurrentSlotVIndex!=2)
						{
							m_iCurrentSlotVIndex++;
							ui.PlayUISFX(eSFX_Focus);
						}
						else
						{
							iVSlotIndexA[0]=iVSlotIndexA[1];
							iVSlotIndexA[1]=iVSlotIndexA[2];
							if(iVSlotIndexA[m_iCurrentSlotVIndex]==(CanBeMadeA[m_iCurrentSlotHIndex].iCount-1))
							{
								iVSlotIndexA[2]=0;
							}
							else
							{
								iVSlotIndexA[2]++;
							}
							ui.PlayUISFX(eSFX_Focus);
						}
					}
					else
					{
						if(m_iCurrentSlotVIndex!=(CanBeMadeA[m_iCurrentSlotHIndex].iCount))
						{
							m_iCurrentSlotVIndex++;
							ui.PlayUISFX(eSFX_Focus);
						}
					}
					UpdateVerticalSlots();
					UpdateHighlight();
			}
		}
		break;
	}

	return bHandled;
}

//////////////////////////////////////////////////////////////////////////
//
//	CheckRecipesAvailable
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::CheckRecipesAvailable()
{
	int iHSlotBrushControl=0;

	// clear the current list
	memset(CanBeMadeA,0,sizeof(CANBEMADE)*m_iCraftablesMaxHSlotC);

	hideAllHSlots();

	if(m_pPlayer && m_pPlayer->inventory)
	{
		// dump out the inventory
		/*		for (unsigned int k = 0; k < m_pPlayer->inventory->items.length; k++)
		{
		if (m_pPlayer->inventory->items[k] != NULL)
		{
		wstring itemstring=m_pPlayer->inventory->items[k]->toString();

		//printf("--- Player has ");
		OutputDebugStringW(itemstring.c_str());
		//printf(" with Aux val = %d, base type = %d, Material = %d\n",m_pPlayer->inventory->items[k]->getAuxValue(),m_pPlayer->inventory->items[k]->getItem()->getBaseItemType(),m_pPlayer->inventory->items[k]->getItem()->getMaterial());
		}
		}
		*/
		RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
		Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
		int iRecipeC=(int)recipes->size();
        auto itRecipe = recipes->begin();

        // dump out the recipe products

		// 		for (int i = 0; i < iRecipeC; i++)
		// 		{
		// 			shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[i].pRecipy->assemble(NULL);
		// 			if (pTempItemInst != NULL)
		// 			{
		// 				wstring itemstring=pTempItemInst->toString();
		//
		// 				printf("Recipe [%d] = ",i);
		// 				OutputDebugStringW(itemstring.c_str());
		// 				if(pTempItemInst->id!=0)
		// 				{
		// 					if(pTempItemInst->id<256)
		// 					{
		// 						Tile *pTile=Tile::tiles[pTempItemInst->id];
		// 						printf("[TILE] ID\t%d\tAux val\t%d\tBase type\t%d\tMaterial\t%d\t Count=%d\n",pTempItemInst->id, pTempItemInst->getAuxValue(),pTile->getBaseItemType(),pTile->getMaterial(),pTempItemInst->GetCount());
		// 					}
		// 					else
		// 					{
		// 						printf("ID\t%d\tAux val\t%d\tBase type\t%d\tMaterial\t%d Count=%d\n",pTempItemInst->id, pTempItemInst->getAuxValue(),pTempItemInst->getItem()->getBaseItemType(),pTempItemInst->getItem()->getMaterial(),pTempItemInst->GetCount());
		// 					}
		//
		// 				}
		// 			}
		// 		}

		for(int i=0;i<iRecipeC;i++)
		{

			Recipy *r = *itRecipe;

			// If this recipe isn't in the current grouptype, skip it
			if(r->getGroup()!=m_pGroupA[m_iGroupIndex])
			{
				itRecipe++;
				pRecipeIngredientsRequired[i].bCanMake[getPad()]=false;
				continue;
			}
			// if we are in the inventory menu, then we have 2x2 crafting available only
			if((m_iContainerType==RECIPE_TYPE_2x2) && (pRecipeIngredientsRequired[i].iType==RECIPE_TYPE_3x3))
			{
				// need a crafting table for this recipe
				itRecipe++;
				pRecipeIngredientsRequired[i].bCanMake[getPad()]=false;
				continue;
			}
			// clear the mask showing which ingredients are missing
			pRecipeIngredientsRequired[i].usBitmaskMissingGridIngredients[getPad()]=0;

			//bool bCanMakeRecipe=true;
			bool *bFoundA= new bool [pRecipeIngredientsRequired[i].iIngC];
			for(int j=0;j<pRecipeIngredientsRequired[i].iIngC;j++)
			{
				bFoundA[j]=false;
				int iTotalCount=0;

				// Does the player have this ingredient?
				for (unsigned int k = 0; k < m_pPlayer->inventory->items.length; k++)
				{
					if (m_pPlayer->inventory->items[k] != NULL)
					{
						// do they have the ingredient, and the aux value matches, and enough off it?
						if((m_pPlayer->inventory->items[k]->id == pRecipeIngredientsRequired[i].iIngIDA[j]) &&
							// check if the ingredient required doesn't care about the aux value, or if it does, does the inventory item aux match it
								((pRecipeIngredientsRequired[i].iIngAuxValA[j]==Recipes::ANY_AUX_VALUE) || (pRecipeIngredientsRequired[i].iIngAuxValA[j]==m_pPlayer->inventory->items[k]->getAuxValue()))
								)
						{
							// do they have enough? We need to check the whole inventory, since they may have enough in different slots (milk isn't milkx3, but milk,milk,milk)
							if(m_pPlayer->inventory->items[k]->GetCount()>=pRecipeIngredientsRequired[i].iIngValA[j])
							{
								// they have enough with one slot
								bFoundA[j]=true;
							}
							else
							{
								// look at the combined value from the whole inventory

								for(unsigned int l=0;l<m_pPlayer->inventory->items.length;l++)
								{
									if (m_pPlayer->inventory->items[l] != NULL)
									{
										if(
											(m_pPlayer->inventory->items[l]->id == pRecipeIngredientsRequired[i].iIngIDA[j]) &&
											( (pRecipeIngredientsRequired[i].iIngAuxValA[j]==Recipes::ANY_AUX_VALUE) || (pRecipeIngredientsRequired[i].iIngAuxValA[j]==m_pPlayer->inventory->items[l]->getAuxValue() ))
											)
										{
											iTotalCount+=m_pPlayer->inventory->items[l]->GetCount();
										}
									}
								}

								if(iTotalCount>=pRecipeIngredientsRequired[i].iIngValA[j])
								{
									bFoundA[j]=true;
								}
							}

							// 4J Stu - TU-1 hotfix
							// Fix for #13143 - Players are able to craft items they do not have enough ingredients for if they store the ingredients in multiple, smaller stacks
							break;
						}
					}
				}
				// if bFoundA[j] is false, then we didn't have enough of the ingredient required by the recipe, so mark the grid items we're short of
				if(bFoundA[j]==false)
				{
					int iMissing = pRecipeIngredientsRequired[i].iIngValA[j]-iTotalCount;
					int iGridIndex=0;
					while(iMissing!=0)
					{
						// need to check if there is an aux val and match that
						if(((pRecipeIngredientsRequired[i].uiGridA[iGridIndex]&0x00FFFFFF)==pRecipeIngredientsRequired[i].iIngIDA[j]) &&
							((pRecipeIngredientsRequired[i].iIngAuxValA[j]==Recipes::ANY_AUX_VALUE) ||(pRecipeIngredientsRequired[i].iIngAuxValA[j]== ((pRecipeIngredientsRequired[i].uiGridA[iGridIndex]&0xFF000000)>>24))) )
						{
							// this grid entry is the ingredient we don't have enough of
							pRecipeIngredientsRequired[i].usBitmaskMissingGridIngredients[getPad()]|=1<<iGridIndex;
							iMissing--;
						}
						iGridIndex++;
					}
				}
			}

			// so can we make it?
			bool bCanMake=true;
			for(int j=0;j<pRecipeIngredientsRequired[i].iIngC;j++)
			{
				if(bFoundA[j]==false)
				{
					bCanMake=false;
					break;
				}
			}

			pRecipeIngredientsRequired[i].bCanMake[getPad()]=bCanMake;

			// Add the recipe to the CanBeMade list of lists
			if(iHSlotBrushControl<=m_iCraftablesMaxHSlotC)
			{
				bool bFound=false;
				shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[i].pRecipy->assemble(nullptr);
				//int iIcon=pTempItemInst->getItem()->getIcon(pTempItemInst->getAuxValue());
				int iID=pTempItemInst->getItem()->id;
				int iBaseType;

				if(iID<256) // is it a tile?
				{
					iBaseType=Tile::tiles[iID]->getBaseItemType();
				}
				else
				{
					iBaseType=pTempItemInst->getItem()->getBaseItemType();
				}

				// ignore for the misc base type - these have not been placed in a base type group
				if(iBaseType!=Item::eBaseItemType_undefined)
				{
					for(int k=0;k<iHSlotBrushControl;k++)
					{
						// if the item base type is the same as one already in, then add it to that list
						if(CanBeMadeA[k].iItemBaseType==iBaseType)
						{
							// base item type already in our list
							bFound=true;
							if(CanBeMadeA[k].iCount<m_iMaxVSlotC)
							{
								CanBeMadeA[k].iRecipeA[CanBeMadeA[k].iCount++]=i;
							}
							else
							{
								app.DebugPrintf("Need more V slots\n");
							}
							break;
						}
					}
				}

				if(!bFound)
				{
					if(iHSlotBrushControl<m_iCraftablesMaxHSlotC)
					{
						// add to the list
						CanBeMadeA[iHSlotBrushControl].iItemBaseType=iBaseType;
						CanBeMadeA[iHSlotBrushControl].iRecipeA[CanBeMadeA[iHSlotBrushControl].iCount++]=i;
						iHSlotBrushControl++;
					}
					else
					{
						app.DebugPrintf("Need more H slots - ");
#ifndef _CONTENT_PACKAGE
						OutputDebugStringW(app.GetString(pTempItemInst->getDescriptionId()));
#endif
						app.DebugPrintf("\n");

					}
				}
			}
			else
			{
				app.DebugPrintf("Need more HSlots\n");
			}

			delete [] bFoundA;
			itRecipe++;
		}
	}

	// run through the canbemade list and update the icons displayed
	int iIndex=0;
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

	while((iIndex<m_iCraftablesMaxHSlotC) && CanBeMadeA[iIndex].iCount!=0)
	{
		shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[CanBeMadeA[iIndex].iRecipeA[0]].pRecipy->assemble(nullptr);
		assert(pTempItemInst->id!=0);
		unsigned int uiAlpha;

		if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
		{
			uiAlpha = 31;
		}
		else
		{
			if(pRecipeIngredientsRequired[CanBeMadeA[iIndex].iRecipeA[0]].bCanMake[getPad()])
			{
				uiAlpha = 31;
			}
			else
			{
				uiAlpha= 16;
			}
		}

		// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
		// rather than the dynamic one for the player
		if( pTempItemInst->id == Item::clock_Id || pTempItemInst->id == Item::compass_Id )
		{
			pTempItemInst->setAuxValue( 255 );
		}
		setCraftHSlotItem(getPad(),iIndex,pTempItemInst,uiAlpha);

		iIndex++;
	}

	// 4J-PB - Removed - UpdateTooltips will do this
	// Update tooltips
	/*if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
	{
		ui.ShowTooltip( getPad(), eToolTipButtonA, true );
		// 4J-PB - not implemented !
		//ui.EnableTooltip( getPad(), eToolTipButtonA, true );
	}
	else
	{
		ui.ShowTooltip( getPad(), eToolTipButtonA, false );
	}*/
}

//////////////////////////////////////////////////////////////////////////
//
//	UpdateHighlight
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateHighlight()
{
	updateHighlightAndScrollPositions();

	bool bCanBeMade=CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0;
	if(bCanBeMade)
	{
		//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
		Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
		int iSlot;
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
		{
			iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
		}
		else
		{
			iSlot=0;
		}
		shared_ptr<ItemInstance> pTempItemInstAdditional=pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].pRecipy->assemble(nullptr);

		// special case for the torch coal/charcoal
		int id=pTempItemInstAdditional->getDescriptionId();
		LPCWSTR itemstring;

		switch(id)
		{
		case IDS_TILE_TORCH:
			{
				if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].iIngAuxValA[0]==1)
				{
					itemstring=app.GetString( IDS_TILE_TORCHCHARCOAL );
				}
				else
				{
					itemstring=app.GetString( IDS_TILE_TORCHCOAL );
				}
			}
			break;
		case IDS_ITEM_FIREBALL:
			{
				if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].iIngAuxValA[2]==1)
				{
					itemstring=app.GetString( IDS_ITEM_FIREBALLCHARCOAL );
				}
				else
				{
					itemstring=app.GetString( IDS_ITEM_FIREBALLCOAL );
				}
			}
			break;
		default:
			itemstring=app.GetString(id  );
			break;
		}

		setItemText(itemstring);
	}
	else
	{
		setItemText(L"");
	}
	UpdateDescriptionText(bCanBeMade);
	DisplayIngredients();

	UpdateMultiPanel();

	UpdateTooltips();
}

//////////////////////////////////////////////////////////////////////////
//
//	UpdateVerticalSlots
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateVerticalSlots()
{
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

	// update the vertical items for the current horizontal slot
	hideAllVSlots();

	// could have either 1 or 2 vertical slots, above and below the horizontal slot
	if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
	{
		// turn off the horizontal one since we could be cycling through others
		setShowCraftHSlot(m_iCurrentSlotHIndex,false);
		int iSlots=(CanBeMadeA[m_iCurrentSlotHIndex].iCount>2)?3:2;

		// 4J-Tomk - check if we've only got one vertical scroll slot (480, splits & Vita)
		bool bNoScrollSlots = false;
		if(m_bSplitscreen ||(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()))
		{
			bNoScrollSlots = true;
		}
#ifdef __PSVITA__
		bNoScrollSlots = true;
#endif

		for(int i=0;i<iSlots;i++)
		{
			// 4J this check determines if the crafting scene has only one vertical scroll slot
			if(bNoScrollSlots)
			{
				if(i!=1) continue;
			}
			shared_ptr<ItemInstance> pTempItemInstAdditional=pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iVSlotIndexA[i]]].pRecipy->assemble(nullptr);

			assert(pTempItemInstAdditional->id!=0);
			unsigned int uiAlpha;

			if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
			{
				uiAlpha = 31;
			}
			else
			{
				if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iVSlotIndexA[i]]].bCanMake[getPad()])
				{
					uiAlpha = 31;
				}
				else
				{
					uiAlpha= 16;
				}
			}

			// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
			// rather than the dynamic one for the player
			if( pTempItemInstAdditional->id == Item::clock_Id || pTempItemInstAdditional->id == Item::compass_Id )
			{
				pTempItemInstAdditional->setAuxValue( 255 );
			}

			setCraftVSlotItem(getPad(),i,pTempItemInstAdditional,uiAlpha);

			updateVSlotPositions(iSlots, i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//	DisplayIngredients
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::DisplayIngredients()
{
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

	// hide the previous ingredients
	hideAllIngredientsSlots();

	if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
	{
		int iSlot,iRecipy;
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
		{
			iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
			iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
		}
		else
		{
			iSlot=0;
			iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[0];
		}

		// show the 2x2 or 3x3 to make the current item
		int iBoxWidth=(m_iContainerType==RECIPE_TYPE_2x2)?2:3;
		int iRecipe=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
		bool bCanMakeRecipe  = pRecipeIngredientsRequired[iRecipe].bCanMake[getPad()];
		shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[iRecipe].pRecipy->assemble(nullptr);

		m_iIngredientsC=pRecipeIngredientsRequired[iRecipe].iIngC;

		// update the ingredients required - these will all be hidden until cycled by the user
		for(int i=0;i<pRecipeIngredientsRequired[iRecipe].iIngC;i++)
		{
			int id=pRecipeIngredientsRequired[iRecipe].iIngIDA[i];
			int iAuxVal=pRecipeIngredientsRequired[iRecipe].iIngAuxValA[i];
			Item *item = Item::items[id];

			shared_ptr<ItemInstance> itemInst= shared_ptr<ItemInstance>(new ItemInstance(item,pRecipeIngredientsRequired[iRecipe].iIngValA[i],iAuxVal));

			// 4J-PB - a very special case - the bed can use any kind of wool, so we can't use the item description
			// and the same goes for the painting
			int idescID;

			if( ((pTempItemInst->id==Item::bed_Id)		&&(id==Tile::wool_Id)) ||
				((pTempItemInst->id==Item::painting_Id)	&&(id==Tile::wool_Id)) )
			{
				idescID=IDS_ANY_WOOL;
			}
			else if((pTempItemInst->id==Item::fireworksCharge_Id) && (id==Item::dye_powder_Id))
			{
				idescID=IDS_ITEM_DYE_POWDER;
				iAuxVal = 1;
			}
			else
			{
				idescID=itemInst->getDescriptionId();
			}
			setIngredientDescriptionText(i,app.GetString(idescID));


			if( (iAuxVal & 0xFF) == 0xFF) // 4J Stu - If the aux value is set to match any
				iAuxVal = 0;

			// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
			// rather than the dynamic one for the player
			if( id == Item::clock_Id || id == Item::compass_Id )
			{
				iAuxVal = 0xFF;
			}
			itemInst->setAuxValue(iAuxVal);

			setIngredientDescriptionItem(getPad(),i,itemInst);
			setIngredientDescriptionRedBox(i,false);
		}

		// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
		// rather than the dynamic one for the player
		if( pTempItemInst->id == Item::clock_Id || pTempItemInst->id == Item::compass_Id )
		{
			pTempItemInst->setAuxValue( 255 );
		}

		// don't grey out the output icon
		setCraftingOutputSlotItem(getPad(), pTempItemInst);

		if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
		{
			setCraftingOutputSlotRedBox(false);
		}
		else
		{
			if(bCanMakeRecipe==false)
			{
				setCraftingOutputSlotRedBox(true);
			}
			else
			{
				setCraftingOutputSlotRedBox(false);
			}

		}
		for (int x = 0; x < iBoxWidth; x++)
		{
			for (int y = 0; y < iBoxWidth; y++)
			{
				int index = x+y*iBoxWidth;
				if(pRecipeIngredientsRequired[iRecipy].uiGridA[x+y*3]!=0)
				{
					int id=pRecipeIngredientsRequired[iRecipy].uiGridA[x+y*3]&0x00FFFFFF;
					assert(id!=0);
					int iAuxVal=(pRecipeIngredientsRequired[iRecipy].uiGridA[x+y*3]&0xFF000000)>>24;

					// 4J Stu - For clocks and compasses we set the aux value to a special one that signals we should use a default texture
					// rather than the dynamic one for the player
					if( id == Item::clock_Id || id == Item::compass_Id )
					{
						iAuxVal = 0xFF;
					}
					else if( pTempItemInst->id==Item::fireworksCharge_Id && id == Item::dye_powder_Id)
					{
						iAuxVal = 1;
					}
					shared_ptr<ItemInstance> itemInst= shared_ptr<ItemInstance>(new ItemInstance(id,1,iAuxVal));
					setIngredientSlotItem(getPad(),index,itemInst);
					// show the ingredients we don't have if we can't make the recipe
					if(app.DebugSettingsOn() && app.GetGameSettingsDebugMask(ProfileManager.GetPrimaryPad())&(1L<<eDebugSetting_CraftAnything))
					{
						setIngredientSlotRedBox(index, false);
					}
					else
					{
						if((pRecipeIngredientsRequired[iRecipy].usBitmaskMissingGridIngredients[getPad()]&(1<<(x+y*3)))!=0)
						{
							setIngredientSlotRedBox(index, true);
						}
						else
						{
							setIngredientSlotRedBox(index, false);
						}
					}
				}
				else
				{
					setIngredientSlotRedBox(index, false);
					setIngredientSlotItem(getPad(),index,nullptr);
				}
			}
		}
	}
	else
	{
		setCraftingOutputSlotItem(getPad(), nullptr);
		setCraftingOutputSlotRedBox(false);
		m_iIngredientsC=0;
		int iIngredientsSlots;
		// if it's a 2x2 , only clear the 4 m_pCraftingIngredientA slots
		if(m_iContainerType==RECIPE_TYPE_2x2)
		{
			iIngredientsSlots=4;
		}
		else
		{
			iIngredientsSlots=m_iIngredients3x3SlotC;
		}

		for(int i=0;i<iIngredientsSlots;i++)
		{
			setIngredientSlotRedBox(i, false);
			setIngredientSlotItem(getPad(),i,nullptr);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//	UpdateDescriptionText
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateDescriptionText(bool bCanBeMade)
{
	int iIDSString=0;
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

	if(bCanBeMade)
	{
		int iSlot;//,iRecipy;
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
		{
			iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
			//iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
		}
		else
		{
			iSlot=0;
			//iRecipy=CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[0];
		}

		shared_ptr<ItemInstance> pTempItemInst=pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].pRecipy->assemble(nullptr);
		int iID=pTempItemInst->getItem()->id;
		int iAuxVal=pTempItemInst->getAuxValue();
		int iBaseType;

		if(iID<256) // is it a tile?
		{
			iBaseType=Tile::tiles[iID]->getBaseItemType();

			iIDSString = Tile::tiles[iID]->getUseDescriptionId();
		}
		else
		{
			iBaseType=pTempItemInst->getItem()->getBaseItemType();

			iIDSString = pTempItemInst->getUseDescriptionId();
		}

		// A few special cases where the description required is specific to crafting, rather than the normal description
		if(iBaseType!=Item::eBaseItemType_undefined)
		{
			switch(iBaseType)
			{
			case Item::eBaseItemType_cloth:
				switch(iAuxVal)
				{
				case 0:
					iIDSString=IDS_DESC_WOOLSTRING;
					break;
				}
				break;
			}
		}

		// set the string mapped to by the base object mapping array

		if(iIDSString>=0)
		{
			// this is an html control now, so set the font size and colour
			//wstring wsText=app.GetString(iIDSString);
			wstring wsText=app.FormatHTMLString(getPad(),app.GetString(iIDSString));

			// 12 for splitscreen, 14 for normal
			EHTMLFontSize size = eHTMLSize_Normal;
			if(m_bSplitscreen ||(!RenderManager.IsHiDef() && !RenderManager.IsWidescreen()))
			{
				size = eHTMLSize_Splitscreen;
			}
			wchar_t startTags[64];
			swprintf(startTags,64,L"<font color=\"#%08x\"><P ALIGN=LEFT>",app.GetHTMLColour(eHTMLColor_Black));
			wsText= startTags + wsText + L"</P>";

			setDescriptionText(wsText.c_str());
		}
		else
		{
			/// Missing string!
#ifdef _DEBUG
			setDescriptionText(L"This is some placeholder description text about the craftable item.");
#else
			setDescriptionText(L"");
#endif
		}
	}
	else
	{
		setDescriptionText(L"");
	}
}

//////////////////////////////////////////////////////////////////////////
//
//	UpdateTooltips
//
//////////////////////////////////////////////////////////////////////////
void IUIScene_CraftingMenu::UpdateTooltips()
{
	//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
	Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();
	// Update tooltips

	bool bDisplayCreate;

	if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
	{
		int iSlot;
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
		{
			iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
		}
		else
		{
			iSlot=0;
		}

		if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].bCanMake[getPad()])
		{
			bDisplayCreate=true;
		}
		else
		{
			bDisplayCreate=false;
		}
	}
	else
	{
		bDisplayCreate=false;
	}


	switch(m_iDisplayDescription)
	{
	case DISPLAY_INVENTORY:
		ui.SetTooltips( getPad(), bDisplayCreate?IDS_TOOLTIPS_CREATE:-1,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_DESCRIPTION,-1,-1,-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
		break;
	case DISPLAY_DESCRIPTION:
		ui.SetTooltips( getPad(), bDisplayCreate?IDS_TOOLTIPS_CREATE:-1,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INGREDIENTS,-1,-1,-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
		break;
	case DISPLAY_INGREDIENTS:
		ui.SetTooltips( getPad(), bDisplayCreate?IDS_TOOLTIPS_CREATE:-1,IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_SHOW_INVENTORY,-1,-1,-1,-2, IDS_TOOLTIPS_CHANGE_GROUP);
		break;
	}

	/*if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
	{
		int iSlot;
		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
		{
			iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
		}
		else
		{
			iSlot=0;
		}

		if(pRecipeIngredientsRequired[CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot]].bCanMake[getPad()])
		{
			ui.EnableTooltip( getPad(), eToolTipButtonA, true );
		}
		else
		{
			ui.EnableTooltip( getPad(), eToolTipButtonA, false );
		}
	}
	else
	{
		ui.ShowTooltip( getPad(), eToolTipButtonA, false );
	}*/
}

void IUIScene_CraftingMenu::HandleInventoryUpdated()
{
	// Check which recipes are available with the resources we have
	CheckRecipesAvailable();
	UpdateVerticalSlots();
	UpdateHighlight();
	UpdateTooltips();
}

bool IUIScene_CraftingMenu::isItemSelected(int itemId)
{
	bool isSelected = false;
	if(m_pPlayer && m_pPlayer->inventory)
	{
		//RecipyList *recipes = ((Recipes *)Recipes::getInstance())->getRecipies();
		Recipy::INGREDIENTS_REQUIRED *pRecipeIngredientsRequired=Recipes::getInstance()->getRecipeIngredientsArray();

		if(CanBeMadeA[m_iCurrentSlotHIndex].iCount!=0)
		{
			int iSlot;
			if(CanBeMadeA[m_iCurrentSlotHIndex].iCount>1)
			{
				iSlot=iVSlotIndexA[m_iCurrentSlotVIndex];
			}
			else
			{
				iSlot=0;
			}
			int iRecipe= CanBeMadeA[m_iCurrentSlotHIndex].iRecipeA[iSlot];
			ItemInstance *pTempItemInst = (ItemInstance *)pRecipeIngredientsRequired[iRecipe].pRecipy->getResultItem();

			if(pTempItemInst->id == itemId)
			{
				isSelected = true;
			}
		}
	}
	return isSelected;
}
