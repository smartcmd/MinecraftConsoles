#pragma once
using namespace std;

#include "TutorialTask.h"

// 4J Stu - Tasks that involve using the controller
class ControllerTask : public TutorialTask
{
private:
	unordered_map<int, bool> completedMappings;
	unordered_map<int, bool> southpawCompletedMappings;
	bool m_bHasSouthpaw;
	unsigned int m_uiCompletionMask;
	int *m_iCompletionMaskA;
	int m_iCompletionMaskACount;
	bool CompletionMaskIsValid();
public:
	ControllerTask(Tutorial *tutorial, int descriptionId, bool enablePreCompletion,  bool showMinimumTime,
				int mappings[], unsigned int mappingsLength, int iCompletionMaskA[]=nullptr, int iCompletionMaskACount=0, int iSouthpawMappings[]=nullptr, unsigned int uiSouthpawMappingsCount=0);
	~ControllerTask();
	virtual bool isCompleted();
	virtual void setAsCurrentTask(bool active = true);

};