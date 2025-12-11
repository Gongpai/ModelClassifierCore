#include "ProgressDebug.h"
#include <cmath>

namespace ModelClassifierCore
{
	void ProgressDebug::DebugLog(float Progress, FString Message)
	{
		float Percentage = Progress * 100.0f;
		UE_LOG(LogTemp, Display, TEXT("%s: %d%s"), *Message, static_cast<int>(std::floor(Percentage)), *FString("%"));
	}
}