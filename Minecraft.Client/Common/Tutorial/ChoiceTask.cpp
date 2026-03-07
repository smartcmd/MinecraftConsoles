#include "stdafx.h"
#include <string>
#include <unordered_map>
#include "../../Minecraft.h"
#include "../../MultiPlayerLocalPlayer.h"
#include "Tutorial.h"
#include "TutorialConstraints.h"
#include "ChoiceTask.h"
#include "../../../Minecraft.World/Material.h"
#include "../../Windows64/KeyboardMouseInput.h"

ChoiceTask::ChoiceTask(Tutorial *tutorial, int descriptionId, int promptId /*= -1*/, bool requiresUserInput /*= false*/,
	int iConfirmMapping /*= 0*/, int iCancelMapping /*= 0*/,
	eTutorial_CompletionAction cancelAction /*= e_Tutorial_Completion_None*/, ETelemetryChallenges telemetryEvent /*= eTelemetryTutorial_NoEvent*/)
	: TutorialTask( tutorial, descriptionId, false, NULL, true, false, false )
{
	if(requiresUserInput == true)
	{
		constraints.push_back( new InputConstraint( iConfirmMapping ) );
		constraints.push_back( new InputConstraint( iCancelMapping ) );
	}
	m_iConfirmMapping = iConfirmMapping;
	m_iCancelMapping = iCancelMapping;
	m_bConfirmMappingComplete = false;
	m_bCancelMappingComplete = false;

	m_cancelAction = cancelAction;

	m_promptId = promptId;
	tutorial->addMessage( m_promptId );

	m_eTelemetryEvent = telemetryEvent;
}

bool ChoiceTask::isCompleted()
{
	Minecraft* pMinecraft = Minecraft::GetInstance();
	if (!pMinecraft || !pMinecraft->player)
		return false;

	int xboxPad = pMinecraft->player->GetXboxPad();

	if( m_bConfirmMappingComplete || m_bCancelMappingComplete )
	{
		sendTelemetry();
		enableConstraints(false, true);
		return true;
	}	
	
	if(ui.GetMenuDisplayed(tutorial->getPad()))
	{
		// If a menu is displayed, then we use the handleUIInput to complete the task
	}
	else
	{
		// If the player is under water then allow all keypresses so they can jump out
		if (pMinecraft->localplayers[tutorial->getPad()]->isUnderLiquid(Material::water)) return false;
#ifdef _WINDOWS64
		if (!m_bConfirmMappingComplete &&
			(InputManager.GetValue(xboxPad, m_iConfirmMapping) > 0
				|| g_KBMInput.IsKeyDown(VK_RETURN)))
#else
		if (!m_bConfirmMappingComplete &&
			InputManager.GetValue(xboxPad, m_iConfirmMapping) > 0)
#endif
		{
			m_bConfirmMappingComplete = true;
		}

#ifdef _WINDOWS64
		if (!m_bCancelMappingComplete &&
			(InputManager.GetValue(xboxPad, m_iCancelMapping) > 0
				|| g_KBMInput.IsKeyDown('B')))
#else
		if (!m_bCancelMappingComplete &&
			InputManager.GetValue(xboxPad, m_iCancelMapping) > 0)
#endif
		{
			m_bCancelMappingComplete = true;
		}

		if (m_bConfirmMappingComplete || m_bCancelMappingComplete)
		{
			sendTelemetry();
			enableConstraints(false, true);
		}
		return m_bConfirmMappingComplete || m_bCancelMappingComplete;
	}
}

eTutorial_CompletionAction ChoiceTask::getCompletionAction()
{
	if(m_bCancelMappingComplete)
	{
		return m_cancelAction;
	}
	else
	{
		return e_Tutorial_Completion_None;
	}
}

int ChoiceTask::getPromptId()
{
	if( m_bShownForMinimumTime )
		return m_promptId;
	else
		return -1;
}

void ChoiceTask::setAsCurrentTask(bool active /*= true*/)
{
	enableConstraints( active );
	TutorialTask::setAsCurrentTask(active);
}

void ChoiceTask::handleUIInput(int iAction)
{
	if(bHasBeenActivated && m_bShownForMinimumTime)
	{
		if( iAction == m_iConfirmMapping)
		{
			m_bConfirmMappingComplete = true;
		}
		else if(iAction == m_iCancelMapping)
		{
			m_bCancelMappingComplete = true;
		}
	}
}

void ChoiceTask::sendTelemetry()
{
	Minecraft *pMinecraft = Minecraft::GetInstance();

	if( m_eTelemetryEvent != eTelemetryChallenges_Unknown )
	{
		bool firstPlay = true;
		// We only store first play for some of the events
		switch(m_eTelemetryEvent)
		{
		case eTelemetryTutorial_TrialStart:
			firstPlay = !tutorial->getCompleted( eTutorial_Telemetry_TrialStart );
			tutorial->setCompleted( eTutorial_Telemetry_TrialStart );
			break;
		case eTelemetryTutorial_Halfway:
			firstPlay = !tutorial->getCompleted( eTutorial_Telemetry_Halfway );
			tutorial->setCompleted( eTutorial_Telemetry_Halfway );
			break;
		};

		TelemetryManager->RecordEnemyKilledOrOvercome(pMinecraft->player->GetXboxPad(), 0, 0, 0, 0, 0, 0, m_eTelemetryEvent);
	}
}
