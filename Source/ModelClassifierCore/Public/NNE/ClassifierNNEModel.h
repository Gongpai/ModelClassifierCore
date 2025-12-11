// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NNEModelData.h"
#include "Engine/DataAsset.h"
#include "ClassifierNNEModel.generated.h"

/**
 * 
 */
UCLASS()
class MODELCLASSIFIERCORE_API UClassifierNNEModel : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UNNEModelData> ImageEncoderNNEModel;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UNNEModelData> TextEncoderNNEModel;
};
