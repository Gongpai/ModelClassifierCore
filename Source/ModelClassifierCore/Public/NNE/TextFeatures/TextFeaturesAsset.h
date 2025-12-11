// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ClipTokens.h"
#include "TextFeatureVector.h"
#include "Engine/DataAsset.h"
#include "TextFeaturesAsset.generated.h"

/**
 * 
 */
UCLASS()
class MODELCLASSIFIERCORE_API UTextFeaturesAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Collect labelsclip_tokens
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	TArray<FString> Labels;

	// Collect clip tokens
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	TMap<FString, FClipTokens> ClipTokens;
	
	// Collect pre-computed text features (Each label has a feature vector.)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Classification")
	TArray<FTextFeatureVector> TextFeatures;
	
	// Metadata
	UPROPERTY(VisibleAnywhere, Category = "Classification")
	int32 InputTokenLength;
	
	UPROPERTY(VisibleAnywhere, Category = "Classification")
	int32 OutputTokenLength;

	UPROPERTY(VisibleAnywhere, Category = "Classification")
	FDateTime GeneratedTime;
};