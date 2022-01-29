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
    mMesh = StaticMeshFinder.Object;
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
        return false;
    }


    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal;
    SpawnParams.bHideFromSceneOutliner = 0;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AStaticMeshActor* NewElement = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
    NewElement->GetStaticMeshComponent()->SetStaticMesh(mMesh);
    NewElement->SetActorHiddenInGame(false);
    NewElement->SetMobility(EComponentMobility::Movable);

    std::string stringPath = std::string(TCHAR_TO_UTF8(*ModelPath));
    cppflow::model model (stringPath);
    m_model = MakeUnique<cppflow::model>(std::string(TCHAR_TO_UTF8(*ModelPath)));

    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    auto water = ReadData("D:/dnn/data/0_0.158275115632_500__0.txt");
    auto whiteWater = ReadData("D:/dnn/data/0_0.158275115632_500_whitewater_0.txt");
    auto input = ReadData("D:/dnn/data/0_0.158275115632_500_input_0.txt");
    auto velocity = ReadData("D:/dnn/data/0_0.158275115632_500_velocity_0.txt");
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));
    UE_LOG(LogTemp, Warning, TEXT("********************************************"));


    /*
    if (NewElement->GetStaticMeshComponent()->GetStaticMesh()->RenderData->LODResources.Num() > 0)
    {
        FPositionVertexBuffer* VertexBuffer = &NewElement->GetStaticMeshComponent()->GetStaticMesh()->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer;
        if (VertexBuffer)
        {
            const int32 VertexCount = VertexBuffer->GetNumVertices();
            for (int32 Index = 0; Index < VertexCount; Index++)
            {
                VertexBuffer->VertexPosition(Index) += FVector(float(Index), float(100* Index), float(10000 * Index));
            }
        }
    }

    UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(this);
    RenderTarget->InitCustomFormat(2, 2, PF_B8G8R8A8, true);
    static uint8 rgbw[4 * 4] = {
                           0,   0,   255, 255,
                           0,   255,   0, 255,
                           255,   0,   0, 255,
                           255, 255, 255, 255

    };
    auto region = FUpdateTextureRegion2D(0, 0, 0, 0, 2, 2);
    const uint8 x = 4
        + region.SrcY * 2 * 4
        + region.SrcX * 4;
    ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
        [=](FRHICommandListImmediate& RHICmdList)
        {
            check(RenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture().IsValid());
            RHIUpdateTexture2D(
                RenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture(),
                0,
                region,
                2 * 4,
                &x
            );
        });

    FString PackageName;
    FString Name;
    FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
    AssetToolsModule.Get().CreateUniqueAssetName(mMesh->GetOutermost()->GetName(), TEXT("_Tex"), PackageName, Name);
    UTexture* NewObj = RenderTarget->ConstructTexture2D(CreatePackage(NULL, *PackageName), Name, mMesh->GetMaskedFlags(), CTF_Default, NULL);
    mMesh->GetMaterial(0)->GetTextureParameterValue(TEXT("TextureMap"), NewObj);
    */




    // Creates Texture2D to store TextureRenderTarget content
    Texture = UTexture2D::CreateTransient(256, 256, PF_R32_FLOAT);
    Texture = WriteDataToTexture(Texture, water);

    
    DynamicMaterial = NewElement->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0);
    DynamicMaterial->SetTextureParameterValue("TextureMap", Texture);
    DynamicMaterial->SetVectorParameterValue("Color", FColor::Red);

	return true;
}

void ATensorFlowNetwork::ApplyForce()
{
    mForceFramesLeft = 1;
}

// #pragma optimize( "", off )
// Called every frame
void ATensorFlowNetwork::UpdateScene()
{
}

#pragma optimize( "", on ) 

