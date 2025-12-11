#pragma once

#include "TextFeatureVector.generated.h"

USTRUCT(BlueprintType)
struct FTextFeatureVector
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<float> Values;
};