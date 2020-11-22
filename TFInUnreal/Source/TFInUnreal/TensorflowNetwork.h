// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ThirdParty/include/TFLibrary.h"
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/StaticMeshActor.h" 
#include "Components/SphereComponent.h" 
#include "Components/StaticMeshComponent.h" 
#include "Engine/World.h" 
#include "Misc/Paths.h"
#include "TensorFlowNetwork.generated.h"

UCLASS(BlueprintType, Blueprintable, Category = "TensorFlow")
class TFINUNREAL_API ATensorFlowNetwork : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATensorFlowNetwork();
	UPROPERTY()
		TMap < FString, UStaticMesh*> mMeshes;
	// Called every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString ModelPath{ "Source/ThirdParty/model.pb" };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float forceAngle{0};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float force{0};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool disablePhysics{ 1 };

	UFUNCTION(BlueprintCallable)
		bool InitializeModel();

	UFUNCTION(BlueprintCallable)
		void UpdateScene();
	UFUNCTION(BlueprintCallable)
		void ApplyForce();
private:
	float mSeconds{ 0 };
	int mForceFramesLeft{ 0 };
	TFNetwork m_network;
	TArray<AStaticMeshActor*> m_elementsList;
	int m_numberOfFrames, m_numberOfBlocks;
};
