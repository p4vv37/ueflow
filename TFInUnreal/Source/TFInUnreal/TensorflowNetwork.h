// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "../ThirdParty/include/TFLibrary.h"

#include "cppflow/ops.h"
#include "cppflow/model.h"
#include "cppflow/datatype.h"

#include <chrono>
#include <vector>
#include <string>
#include <sstream>

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h" 
#include "Misc/Paths.h"
#include "Engine/World.h" 
#include "Engine/TextureRenderTarget2D.h" 
#include "Rendering/SlateRenderer.h" 
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
	FString ModelPath{ FPaths::ProjectContentDir() + "model_complex" };
	FString ModelSimplePath{ FPaths::ProjectContentDir() + "model_simple" };

	UFUNCTION(BlueprintCallable)
		bool InitializeModel();
	UFUNCTION(BlueprintCallable)
		void UpdateScene();
	UFUNCTION(BlueprintCallable)
		void ChangeDisplayMode(const int NewMode);
	UFUNCTION(BlueprintCallable)
		void ChangeShape(const int NewShapeId);


	// Old: remove after updating blueprints.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Rotation{ 0 };
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float Velocity{ 1.0 };
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 NetworkId{ 1 };

private:
	TUniquePtr<cppflow::model> Model;
	TUniquePtr<cppflow::model> ModelSimple;

	UTexture2D* WaterHeightTexture = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);
	UTexture2D* WhiteWaterTexture = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT); 

	std::vector<float> VGradient;
	std::vector<float> HGradient;

	UTexture2D* PrevMap = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);

	std::vector<float> WaterHeightData = std::vector<float>(256 * 256, 0);
	std::vector<float> PreviewData = std::vector<float>(256 * 256, 0);
	std::vector<float> WhiteWaterData = std::vector<float>(256 * 256, 0);
	std::vector<float> InputGradient = std::vector<float>(256 * 256, 0);
	std::vector<float> InputRotationCos = std::vector<float>(256 * 256, 0);
	std::vector<float> InputRotationSin = std::vector<float>(256*256, 0);
	std::vector<float> InputRotationGrad = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField0 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField1 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField2 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField3 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceField4 = std::vector<float>(256 * 256, 0);
	std::vector<float> DistanceFieldSmall = std::vector<float>(64 * 64, 0);

	UMaterialInstanceDynamic* DynamicMaterial;

	int8 DisplayMode{ 0 };
	int8 ShapeId{ 0 };

	std::vector<double> Result;
};