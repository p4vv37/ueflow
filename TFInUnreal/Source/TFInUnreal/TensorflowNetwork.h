// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "../ThirdParty/include/TFLibrary.h"

#include "cppflow/ops.h"
#include "cppflow/model.h"
#include "cppflow/datatype.h"

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/StaticMeshActor.h" 
#include "Components/SphereComponent.h" 
#include "Components/StaticMeshComponent.h" 
#include "Engine/World.h" 
#include "Misc/Paths.h"
#include "Misc/FileHelper.h" 
#include "Misc/Paths.h" 
#include "Containers/RingBuffer.h" 
#include <chrono>
#include <thread>
#include "TensorFlowNetwork.generated.h"


namespace utils {
	inline float  GetAngle(const float& sin, const float& cos) { return atan2(sin, cos) * 180.0f / PI; }
	inline void GetSinCos(const float& angle, float& s, float& c) { s = sin(angle); c = cos(angle); }
} // utils


UCLASS(BlueprintType, Blueprintable, Category = "TensorFlow")
class TFINUNREAL_API ATensorFlowNetwork : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATensorFlowNetwork();
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UStaticMesh* mMesh;
	// Called every frame
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString ModelPath { "Source/ThirdParty/generator/saved_model.pb" };

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float forceAngle{0};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float force{0};
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		bool disablePhysics{ 1 };

	UFUNCTION(BlueprintCallable)
		bool InitializeModel();
		//void UpdateTextureRegion(FTexture2DRHIRef TextureRHI, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D Region, uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, TFunction<void(uint8* SrcData)> DataCleanupFunc);

	UFUNCTION(BlueprintCallable)
		void UpdateScene();
	UFUNCTION(BlueprintCallable)
		void ApplyForce();
private:
	UTexture2D* Texture;
	UMaterialInstanceDynamic* DynamicMaterial;
	float mSeconds{ 0 };
	int mForceFramesLeft{ 0 };

	float m_positionsIndex{ 0.0f };
	float m_orientationsIndex{ 0.0f };
	TArray<float> m_lastFrame;
	TArray<FString*> m_models;
	TUniquePtr<cppflow::model> m_model;
	TArray<AStaticMeshActor*> m_elementsList;
	int m_numberOfFrames{ -1 }, m_numberOfBlocks{ -1 };
};
