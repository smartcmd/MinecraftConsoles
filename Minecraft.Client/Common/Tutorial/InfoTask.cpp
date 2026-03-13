#include "stdafx.h"
#include <string>
#include <unordered_map>
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "Tutorial.h"
#include "TutorialConstraints.h"
#include "InfoTask.h"
#include "../../../Minecraft.World/Material.h"
#include "../../Windows64/KeyboardMouseInput.h"
#include "Common/UI/UI.h"

InfoTask::InfoTask(Tutorial *tutorial, int descriptionId, int promptId /*= -1*/, bool requiresUserInput /*= false*/,
                   int iMapping /*= 0*/, ETelemetryChallenges telemetryEvent /*= eTelemetryTutorial_NoEvent*/)
	: TutorialTask( tutorial, descriptionId, false, nullptr, true, false, false )
{
	if(requiresUserInput == true)
	{
		constraints.push_back( new InputConstraint( iMapping ) );
	}
	completedMappings[iMapping]=false;

	m_promptId = promptId;
	tutorial->addMessage( m_promptId );

	m_eTelemetryEvent = telemetryEvent;
}

bool InfoTask::isCompleted()
{
	if( bIsCompleted )
		return true;

	if( tutorial->m_hintDisplayed )
		return false;

	if( !bHasBeenActivated || !m_bShownForMinimumTime )
		return false;

	bool bAllComplete = true;

	Minecraft *pMinecraft = Minecraft::GetInstance();

	// If the player is under water then allow all keypresses so they can jump out
	if( pMinecraft->localplayers[tutorial->getPad()]->isUnderLiquid(Material::water) ) return false;

	if(ui.GetMenuDisplayed(tutorial->getPad()))
	{
		// If a menu is displayed, then we use the handleUIInput to complete the task
		bAllComplete = true;
		for( auto& it : completedMappings )
		{
			bool current = it.second;
			if(!current)
			{
				bAllComplete = false;
				break;
			}
		}
	}
	else
	{
		int iCurrent=0;

		for( auto& it : completedMappings )
		{
			bool current = it.second;
			if(!current)
			{
#ifdef _WINDOWS64
				if (InputManager.GetValue(pMinecraft->player->GetXboxPad(), it.first) > 0 || g_KBMInput.IsKeyDown(VK_SPACE))
#else
				if( InputManager.GetValue(pMinecraft->player->GetXboxPad(), it.first) > 0)
#endif
				{
					it.second = true;
					bAllComplete=true;
				}
				else
				{
					bAllComplete = false;
				}
			}
			iCurrent++;
		}
	}

	if(bAllComplete==true)
	{
		sendTelemetry();
		enableConstraints(false, true);
	}
	bIsCompleted = bAllComplete;
	return bAllComplete;
}

int InfoTask::getPromptId()
{
	if( m_bShownForMinimumTime )
		return m_promptId;
	else
		return -1;
}

void InfoTask::setAsCurrentTask(bool active /*= true*/)
{
	enableConstraints( active );
	TutorialTask::setAsCurrentTask(active);
}

void InfoTask::handleUIInput(int iAction)
{
	if(bHasBeenActivated)
	{
		for( auto& it : completedMappings )
		{
			if( iAction == it.first )
			{
				it.second = true;
			}
		}
	}
}


void InfoTask::sendTelemetry()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( m_eTelemetryEvent != eTelemetryChallenges_Unknown )
	{
		bool firstPlay = true;
		// We only store first play for some of the events
		switch(m_eTelemetryEvent)
		{
		case eTelemetryTutorial_Complete:
			firstPlay = !tutorial->getCompleted( eTutorial_Telemetry_Complete );
			tutorial->setCompleted( eTutorial_Telemetry_Complete );
			break;
		};
		TelemetryManager->RecordEnemyKilledOrOvercome(pMinecraft->player->GetXboxPad(), 0, 0, 0, 0, 0, 0, m_eTelemetryEvent);
	}
}
