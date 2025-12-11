#pragma once
#include "SettingData.h"

struct MODELCLASSIFIERCORE_API FSettingDataMap
{
public:
	TMap<FString, TMap<FString, FSettingData<TSharedPtr<void>>>> Data;
};
