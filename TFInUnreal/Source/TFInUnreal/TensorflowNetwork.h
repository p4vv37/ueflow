// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ThirdParty/include/TFLibrary.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TensorFlowNetwork.generated.h"

UCLASS(BlueprintType, Blueprintable, Category = "TensorFlow")
class TFINUNREAL_API UTensorFlowNetwork : public UObject
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UTensorFlowNetwork();

	// Called every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString ModelPath{ TEXT("model.pb") };

	UFUNCTION(BlueprintCallable)
		bool InitializeModel();

	UFUNCTION(BlueprintCallable)
		void UpdateScene();
private:
	TFNetwork m_network;
};
