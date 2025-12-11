#pragma once

#include "ProcessResult.h"

class MODELCLASSIFIERCORE_API FPlatformProcessHelper
{
public:
	static FProcessResult ExecCommand(const FString& Command, const FString& Args);
	static FProcessResult WaitCommand(const FString& Command, const FString& Args);
};
