#include "stdafx.h"
#include "ProcedureCompoundTask.h"

ProcedureCompoundTask::~ProcedureCompoundTask()
{
    for (auto it = m_taskSequence.begin(); it < m_taskSequence.end(); ++it)
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
    for (auto& task : m_taskSequence)
    {
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
	for(auto& task : m_taskSequence)
	{
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
	for(auto& task : m_taskSequence)
	{
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
		// Disable all constraints
		for(auto& task : m_taskSequence)
		{
			task->enableConstraints(false);
		}
	}
	bIsCompleted = allCompleted;
	return allCompleted;
}

void ProcedureCompoundTask::onCrafted(shared_ptr<ItemInstance> item)
{
    for(auto& task : m_taskSequence)
	{
		task->onCrafted(item);
	}
}

void ProcedureCompoundTask::handleUIInput(int iAction)
{
	for(auto task : m_taskSequence)
	{
		task->handleUIInput(iAction);
	}
}


void ProcedureCompoundTask::setAsCurrentTask(bool active /*= true*/)
{
	bool allCompleted = true;
	for(auto& task : m_taskSequence)
	{
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
	for(auto& task : m_taskSequence)
	{
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
	for(auto& task : m_taskSequence)
	{
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
	for(auto& task : m_taskSequence)
	{
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
	for(auto& task : m_taskSequence)
	{
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
	for(auto& task : m_taskSequence)
	{
		task->useItemOn(level, item, x, y, z, bTestUseOnly);
	}
}

void ProcedureCompoundTask::useItem(shared_ptr<ItemInstance> item, bool bTestUseOnly)
{
	for(auto& task : m_taskSequence)
	{
		task->useItem(item, bTestUseOnly);
	}
}

void ProcedureCompoundTask::onTake(shared_ptr<ItemInstance> item, unsigned int invItemCountAnyAux, unsigned int invItemCountThisAux)
{
	for(auto& task : m_taskSequence)
	{
		task->onTake(item, invItemCountAnyAux, invItemCountThisAux);
	}
}

void ProcedureCompoundTask::onStateChange(eTutorial_State newState)
{
	for(auto& task : m_taskSequence)
	{
		task->onStateChange(newState);
	}
}