#pragma once
#include "ClipTokens.generated.h"

USTRUCT(BlueprintType)
struct MODELCLASSIFIERCORE_API FClipTokens
{
	GENERATED_BODY()
	
public:
	FClipTokens(){}
	FClipTokens(FString InLabel, TArray<int64> InTokens) : Label(InLabel), Tokens(InTokens) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Label;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<int64> Tokens;
};