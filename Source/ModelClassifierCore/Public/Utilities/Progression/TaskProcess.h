#pragma once
#include "Task.h"

//DECLARE_MULTICAST_DELEGATE(FAdd)

namespace ModelClassifierCore
{
	class MODELCLASSIFIERCORE_API FTaskProcess
	{
		TArray<FTask*> Tasks;
	public:

		float GetProgress()
		{
			float Progress = 0.0f;
			for (int i = 0; i < Tasks.Num(); ++i)
			{
				Progress += Tasks[i]->GetProgress() / Tasks.Num();
			}

			return Progress;
		}

		void Reset()
		{
			
		}
	};
}
