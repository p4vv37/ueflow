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
	FString ModelPath{ "D:\\git\\ueflow\\TFInUnreal\\Source\\ThirdParty\\model" };
	FString ModelSimplePath{ "D:\\git\\ueflow\\TFInUnreal\\Source\\ThirdParty\\model" };

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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 NetworkId{ 0 };

private:
	TUniquePtr<cppflow::model> Model;
	TUniquePtr<cppflow::model> ModelSimple;

	UTexture2D* WaterHeightTexture = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);
	UTexture2D* WhiteWaterTexture = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);

	std::vector<float> VGradient;
	std::vector<float> HGradient;

	UTexture2D* PrevMap = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);

	std::vector<float> WaterHeight = std::vector<float>(256 * 256, 0);
	std::vector<float> WhiteWaterData = std::vector<float>(256 * 256, 0);
	std::vector<float> InputGradient = std::vector<float>(256 * 256, 0);
	std::vector<float> InputRotationCos = std::vector<float>(256 * 256, 0);
	std::vector<float> InputRotationSin = std::vector<float>(256*256, 0);
	std::vector<float> DistanceField0 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField1 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField2 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField3 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField4 = std::vector<float>(256 * 256, 0);

	UMaterialInstanceDynamic* DynamicMaterial;

	int8 DisplayMode{ 0 };
	int8 ShapeId{ 0 };

	std::vector<double> Result;
};