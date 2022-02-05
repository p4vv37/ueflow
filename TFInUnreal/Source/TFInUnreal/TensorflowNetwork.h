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
	inline void GetSinCos(const float& angle, float& s, float& c) { s = sin(angle); c = cos(angle); }
} // utils


UCLASS(BlueprintType, Blueprintable, Category = "TensorFlow")
class TFINUNREAL_API ATensorFlowNetwork : public AActor
{
	GENERATED_BODY()
	
public:	
	ATensorFlowNetwork();
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UStaticMesh* Mesh;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString ModelPath { "Source/ThirdParty/generator" };

	UFUNCTION(BlueprintCallable)
		bool InitializeModel();
	UFUNCTION(BlueprintCallable)
		void UpdateScene();
	UFUNCTION(BlueprintCallable)
		void ChangeDisplayMode(const int NewMode);


	// Old: remove after updating blueprints.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Rotation{ 0 };
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Velocity{ 1.0 };

private:
	TUniquePtr<cppflow::model> Model;

	UTexture2D* WaterHeight; // (256, 256, 1)
	UTexture2D* WhiteWater; // (256, 256, 1)

	UTexture2D* StarADistanceMap; // (256, 256, 1)
	UTexture2D* CircleDistanceMap; // (256, 256, 1)
	UTexture2D* TriangleDistanceMap; // (256, 256, 1)
	UTexture2D* SquareDistanceMap; // (256, 256, 1)
	UTexture2D* StarBDistanceMap; // (256, 256, 1)

	std::vector<float> VGradient;
	std::vector<float> HGradient;

	UTexture2D* PrevMap;

	std::vector<float> InputGradient;
	std::vector<float> InputRotationCos;
	std::vector<float> InputRotationSin; 

	int8 DisplayMode{ 0 };
	UMaterialInstanceDynamic* DynamicMaterial;

	bool Puk{ false };
	std::vector<double> Result;
};
