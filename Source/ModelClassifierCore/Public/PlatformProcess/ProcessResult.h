#pragma once

struct MODELCLASSIFIERCORE_API FProcessResult
{
public:
	int32 ReturnCode = 0;
	FString StdOut;
	FString StdErr;
	FString Output;
	bool bIsSuccess;
};
