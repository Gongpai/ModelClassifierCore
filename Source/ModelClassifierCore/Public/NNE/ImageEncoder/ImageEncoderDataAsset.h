// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ImageEncoderDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class MODELCLASSIFIERCORE_API UImageEncoderDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	// Image Normalization
	UPROPERTY(EditAnywhere, Category = "Normalization")
	TArray<double> Mean;
	
	UPROPERTY(EditAnywhere, Category = "Normalization")
	TArray<double> Std;
};
