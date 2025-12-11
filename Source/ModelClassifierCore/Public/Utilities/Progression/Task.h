#pragma once

#include "TaskProcess.h"

namespace ModelClassifierCore
{
	class FTaskProcess;
	
	class MODELCLASSIFIERCORE_API FTask
	{
		FTaskProcess* TaskProcess;
		float Count = 0.0f;
		
	public:
		TMap<FString, bool> TaskComplete;

		void SetTaskComplete(FString TaskName)
		{
			if (!TaskComplete.Contains(TaskName))
			{
				TaskComplete[TaskName] = true;
				Count++;
			}
		}
		
		float GetProgress()
		{
			return Count / TaskComplete.Num();
		}
	};
}
