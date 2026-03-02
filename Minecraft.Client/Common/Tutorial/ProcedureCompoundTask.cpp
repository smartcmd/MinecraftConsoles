#include "stdafx.h"
#include "ProcedureCompoundTask.h"

ProcedureCompoundTask::~ProcedureCompoundTask()
{
	for(AUTO_VAR(it, m_taskSequence.begin()); it < m_taskSequence.end(); ++it)
	{
		delete (*it);
	}
}

void ProcedureCompoundTask::AddTask(TutorialTask *task)
{
	if(task != NULL)
	{
		m_taskSequence.push_back(task);
	}
}

int ProcedureCompoundTask::getDescriptionId()
{
	if(bIsCompleted)
		return -1;

	// Return the id of the first task not completed
	int descriptionId = -1;
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		if(!task->isCompleted())
		{
			task->setAsCurrentTask(true);
			descriptionId = task->getDescriptionId();
			break;
		}
		else if(task->getCompletionAction() == e_Tutorial_Completion_Complete_State)
		{
			bIsCompleted = true;
			break;
		}
	}
	return descriptionId;
}

int ProcedureCompoundTask::getPromptId()
{
	if(bIsCompleted)
		return -1;

	// Return the id of the first task not completed
	int promptId = -1;
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		if(!task->isCompleted())
		{
			promptId = task->getPromptId();
			break;
		}
	}
	return promptId;
}

bool ProcedureCompoundTask::isCompleted()
{
	// Return whether all tasks are completed

	bool allCompleted = true;
	bool isCurrentTask = true;
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;

		if(allCompleted && isCurrentTask)
		{
			if(task->isCompleted())
			{
				if(task->getCompletionAction() == e_Tutorial_Completion_Complete_State)
				{
					allCompleted = true;
					break;
				}
			}
			else
			{
				task->setAsCurrentTask(true);
				allCompleted = false;
				isCurrentTask = false;
			}
		}
		else if (!allCompleted)
		{
			task->setAsCurrentTask(false);
		}
	}

	if(allCompleted)
	{
		//Disable all constraints
		itEnd = m_taskSequence.end();
		for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
		{
			TutorialTask *task = *it;
			task->enableConstraints(false);
		}
	}
	bIsCompleted = allCompleted;
	return allCompleted;
}

void ProcedureCompoundTask::onCrafted(shared_ptr<ItemInstance> item)
{
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		task->onCrafted(item);
	}
}

void ProcedureCompoundTask::handleUIInput(int iAction)
{
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		task->handleUIInput(iAction);
	}
}


void ProcedureCompoundTask::setAsCurrentTask(bool active /*= true*/)
{
	bool allCompleted = true;
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		if(allCompleted && !task->isCompleted())
		{
			task->setAsCurrentTask(true);
			allCompleted = false;
		}
		else if (!allCompleted)
		{
			task->setAsCurrentTask(false);
		}
	}
}

bool ProcedureCompoundTask::ShowMinimumTime()
{
	if(bIsCompleted)
		return false;

	bool showMinimumTime = false;
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		if(!task->isCompleted())
		{
			showMinimumTime = task->ShowMinimumTime();
			break;
		}
	}
	return showMinimumTime;
}

bool ProcedureCompoundTask::hasBeenActivated()
{
	if(bIsCompleted)
		return true;

	bool hasBeenActivated = false;
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		if(!task->isCompleted())
		{
			hasBeenActivated = task->hasBeenActivated();
			break;
		}
	}
	return hasBeenActivated;
}

void ProcedureCompoundTask::setShownForMinimumTime()
{
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		if(!task->isCompleted())
		{
			task->setShownForMinimumTime();
			break;
		}
	}
}

bool ProcedureCompoundTask::AllowFade()
{
	if(bIsCompleted)
		return true;

	bool allowFade = true;
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		if(!task->isCompleted())
		{
			allowFade = task->AllowFade();
			break;
		}
	}
	return allowFade;
}

void ProcedureCompoundTask::useItemOn(Level *level, shared_ptr<ItemInstance> item, int x, int y, int z,bool bTestUseOnly)
{
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		task->useItemOn(level, item, x, y, z, bTestUseOnly);
	}
}

void ProcedureCompoundTask::useItem(shared_ptr<ItemInstance> item, bool bTestUseOnly)
{
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		task->useItem(item, bTestUseOnly);
	}
}

void ProcedureCompoundTask::onTake(shared_ptr<ItemInstance> item, unsigned int invItemCountAnyAux, unsigned int invItemCountThisAux)
{
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		task->onTake(item, invItemCountAnyAux, invItemCountThisAux);
	}
}

void ProcedureCompoundTask::onStateChange(eTutorial_State newState)
{
	AUTO_VAR(itEnd, m_taskSequence.end());
	for(AUTO_VAR(it, m_taskSequence.begin()); it < itEnd; ++it)
	{
		TutorialTask *task = *it;
		task->onStateChange(newState);
	}
}