// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

#define MULTIPLIER 100.0f
#define SLOWDOWN 1.0f

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "Engine/TextureRenderTarget2D.h" 
#include "Rendering/SlateRenderer.h" 
#include "AssetToolsModule.h"
#include "RHICommandList.h"


UTexture2D* WriteDataToTexture(UTexture2D* ParamsTex, std::vector<float> data)
{
    const int32 NumPixelsInTexture = ParamsTex->GetSizeX()* ParamsTex->GetSizeY();  // Includes empty space at the end, not used by any chunks
    TArray<float> NewPixels = TArray<float>();
    float ColorDefault = 0;
    constexpr SIZE_T PIXEL_DATA_SIZE = sizeof(float);
    NewPixels.Init(ColorDefault, NumPixelsInTexture);


    for (int32 VoxelIndex1DInChunk = 0; VoxelIndex1DInChunk < NumPixelsInTexture; ++VoxelIndex1DInChunk)
    {
        NewPixels[VoxelIndex1DInChunk] = (data[VoxelIndex1DInChunk] + 3) * 512;
    }

    FTexture2DMipMap& Mip0 = ParamsTex->PlatformData->Mips[0];
    void* TextureData = Mip0.BulkData.Lock(LOCK_READ_WRITE);
    FMemory::Memcpy(TextureData, NewPixels.GetData(), PIXEL_DATA_SIZE * NumPixelsInTexture);
    Mip0.BulkData.Unlock();
    ParamsTex->UpdateResource();
    return ParamsTex;
}



// Sets default values
ATensorFlowNetwork::ATensorFlowNetwork()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshFinder(TEXT("StaticMesh'/Game/Meshes/waterSurface.waterSurface'"));
    Mesh = StaticMeshFinder.Object;
}


std::vector<float> ReadData(const FString path) {
    std::fstream in(*path);
    std::string line;
    std::vector<float> result;

    while (std::getline(in, line))
    {
        float value;
        std::stringstream ss(line);

        ss >> value;
        result.push_back(value);
    }
    UE_LOG(LogTemp, Warning, TEXT("%d"), result.size());
    return result;
}

bool ATensorFlowNetwork::InitializeModel()
{
    if (!FPaths::DirectoryExists(ModelPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Model path incorrect: no saved model found in %s"), *ModelPath);
        return false;
    }


    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal;
    SpawnParams.bHideFromSceneOutliner = 0;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AStaticMeshActor* NewElement = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
    NewElement->GetStaticMeshComponent()->SetStaticMesh(Mesh);
    NewElement->SetActorHiddenInGame(false);
    NewElement->SetMobility(EComponentMobility::Movable);

    std::string stringPath = std::string(TCHAR_TO_UTF8(*ModelPath));
    cppflow::model model (stringPath);
    Model = MakeUnique<cppflow::model>(std::string(TCHAR_TO_UTF8(*ModelPath)));

    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    
    auto water = ReadData("D:/dnn/data/0_0.158275115632_500__0.txt");
    for (int n = 0; n < water.size(); n++) {
        water[n] += 10;
        water[n] /= 20;
    }

    auto whiteWater = ReadData("D:/dnn/data/0_0.158275115632_500_whitewater_0.txt");
    for (int n = 0; n < whiteWater.size(); n++) {
        whiteWater[n] /= 7.0;
    }
    
    auto input = ReadData("D:/dnn/data/0_0.158275115632_500_input_0.txt");
    auto velocity = ReadData("D:/dnn/data/0_0.158275115632_500_velocity_0.txt");
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));


    // Creates Texture2D to store TextureRenderTarget content
    WaterHeight = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);
    WaterHeight = WriteDataToTexture(WaterHeight, water);

    WhiteWater = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);
    WhiteWater = WriteDataToTexture(WhiteWater, whiteWater);

    
    DynamicMaterial = NewElement->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0);
    DynamicMaterial->SetTextureParameterValue("HeightMap", WaterHeight);
    DynamicMaterial->SetTextureParameterValue("WhitewaterMap", WhiteWater);
    DynamicMaterial->SetVectorParameterValue("Color", FColor::Red);

	return true;
}

// #pragma optimize( "", off )
void ATensorFlowNetwork::UpdateScene()
{
}
// #pragma optimize( "", off )
void ATensorFlowNetwork::ChangeDisplayMode(const int NewMode)
{
    if (NewMode == 0) {
        DynamicMaterial->SetScalarParameterValue("HeightDisplay", 1.0);
        DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 1.0);
    }
    else if (NewMode == 1) {
        DynamicMaterial->SetScalarParameterValue("HeightDisplay", 1.0);
        DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 0.0);
    }
    else if (NewMode == 2) {
        DynamicMaterial->SetScalarParameterValue("HeightDisplay", 0.0);
        DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 1.0);
    }
    DisplayMode = NewMode;
}

// #pragma optimize( "", on ) 

