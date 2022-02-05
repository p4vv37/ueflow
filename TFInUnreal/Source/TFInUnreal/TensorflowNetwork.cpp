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
        NewPixels[VoxelIndex1DInChunk] = data[VoxelIndex1DInChunk];
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
    for (int x = 0; x < 64; x++) {

        for (int y = 0; y < 64; y++) {
            HGradient.push_back(y);
            VGradient.push_back(x);
        }
    }

    DistanceField = ReadData("D:/git/ueflow/TFInUnreal/Source/ThirdParty/distances/distance_0.txt");

    for (int x = 0; x < 64 * 64; x++) {
        InputGradient.push_back(x);
        InputRotationCos.push_back(x);
        InputRotationSin.push_back(x);
    }

    WaterHeight = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);
    WaterHeight = WriteDataToTexture(WaterHeight, water);

    WhiteWater = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);
    WhiteWater = WriteDataToTexture(WhiteWater, whiteWater);

    PrevMap = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);

    DynamicMaterial = NewElement->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0);
    DynamicMaterial->SetTextureParameterValue("HeightMap", WaterHeight);
    DynamicMaterial->SetTextureParameterValue("WhitewaterMap", WhiteWater);
    DynamicMaterial->SetTextureParameterValue("PrevMap", PrevMap);
    DynamicMaterial->SetVectorParameterValue("Color", FColor::Red);

	return true;
}

// #pragma optimize( "", off )
void ATensorFlowNetwork::UpdateScene()
{

    for (int x = 0; x < 64 * 64; x++) {
        InputGradient[x] = (0.5 - (-std::cos(Rotation) * (HGradient[x] - 32) * Velocity / (8 * 64) - std::sin(Rotation) * (VGradient[x] - 32) * Velocity / (8 * 64)));

        float RotationDelta = std::atan2(HGradient[x] - 32, VGradient[x] - 32.5) - Rotation;
        InputRotationCos[x] = 0.5 + 0.5 * std::cos(RotationDelta);
        InputRotationSin[x] = 0.5 + 0.5 * std::sin(RotationDelta);
    }

    if (DisplayMode < 3) {
        return;
    }

    std::vector<std::vector<float>*> inputs {
        &InputGradient,
        &InputRotationCos,
        &InputRotationSin
    };

    if (DisplayMode < 6) {
        std::vector<float> UpscaledInput;
        UpscaledInput.reserve(256 * 256);

        for (int idx = 0; idx < 64; idx++) {
            std::vector<float> row;
            row.reserve(256);
            for (int PixelId = 64 * idx; PixelId < 64 + 64 * idx; PixelId++) {
                float pixel = (*inputs[DisplayMode - 3])[PixelId];
                row.push_back(pixel);
                row.push_back(pixel);
                row.push_back(pixel);
                row.push_back(pixel);
            }
            UpscaledInput.insert(UpscaledInput.end(), row.begin(), row.end());
            UpscaledInput.insert(UpscaledInput.end(), row.begin(), row.end());
            UpscaledInput.insert(UpscaledInput.end(), row.begin(), row.end());
            UpscaledInput.insert(UpscaledInput.end(), row.begin(), row.end());
        }
        PrevMap = WriteDataToTexture(PrevMap, UpscaledInput);
        return;
    }

    std::vector<float> x_pos_data(64 * 64 * 3, 0.0);

    for (int x = 0; x < 64 * 64; x++) {
        x_pos_data[x] = InputGradient[x];
    }
    for (int x = 0; x < 64 * 64; x++) {
        x_pos_data[x + 64] = InputRotationCos[x];
    }
    for (int x = 0; x < 64 * 64; x++) {
        x_pos_data[x + 64 + 64] = InputRotationSin[x];
    }

    std::vector<float> x_pos_data2;
    for (int x = 0; x < 64 * 64 * 3; x++) {
        x_pos_data2.push_back(0);
    }
    for (int x = 0; x < 64 * 64; x++) {
        x_pos_data2[x * 3] = InputGradient[x];
    }
    cppflow::tensor x_pos(x_pos_data2, { 1, 64, 64, 3 });

    std::vector<float> x_n_data2;
    for (int x = 0; x < 256 * 256 * 3; x++) {
        x_n_data2.push_back(0);
    }
    for (int x = 0; x < 256 * 256; x++) {
        x_n_data2[x * 3 + 2] = DistanceField[x];
    }

    cppflow::tensor x_n(x_n_data2, { 1, 256, 256, 3 });
    auto test = x_n.get_data<float>();

    auto begin = std::chrono::high_resolution_clock::now();
    auto output = (*Model)({ {"serving_default_x_n:0", x_n}, {"serving_default_x_pos:0", x_pos} }, { "StatefulPartitionedCall:0" });
    auto end = std::chrono::high_resolution_clock::now();

    UE_LOG(LogTemp, Warning, TEXT("Network time: %d"), std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());

    Result = output[0].get_data<double>();
    auto testowanie = output[0].shape().get_data<int>();

    std::vector<float> result(256 * 256, 0.0);
    for (int x = 0; x < 256 * 256; x++) {
        result[x] = Result[x * 2];
    }

    WhiteWater = WriteDataToTexture(WhiteWater, result);
    PrevMap = WriteDataToTexture(PrevMap, result);
    DynamicMaterial->SetTextureParameterValue("PrevMap", PrevMap);
    Puk = testowanie.size() == 4;

}
// #pragma optimize( "", off )
void ATensorFlowNetwork::ChangeDisplayMode(const int NewMode)
{
    if (NewMode == 0) {
        DynamicMaterial->SetScalarParameterValue("HeightDisplay", 1.0);
        DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 1.0);
        DynamicMaterial->SetScalarParameterValue("previewDisplay", 0.0);
    }
    else if (NewMode == 1) {
        DynamicMaterial->SetScalarParameterValue("HeightDisplay", 1.0);
        DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 0.0);
        DynamicMaterial->SetScalarParameterValue("previewDisplay", 0.0);
    }
    else if (NewMode == 2) {
        DynamicMaterial->SetScalarParameterValue("HeightDisplay", 0.0);
        DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 1.0);
        DynamicMaterial->SetScalarParameterValue("previewDisplay", 0.0);
    }
    else if (NewMode >= 3) {
        DynamicMaterial->SetScalarParameterValue("HeightDisplay", 0.0);
        DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 0.0);
        DynamicMaterial->SetScalarParameterValue("previewDisplay", 1.0);
    }
    DisplayMode = NewMode;
}

// #pragma optimize( "", on ) 

