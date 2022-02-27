// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

#define MULTIPLIER 100.0f
#define SLOWDOWN 1.0f

#include "RHICommandList.h"


UTexture2D* WriteDataToTexture(UTexture2D* ParamsTex, std::vector<float> data)
{
    const int32 NumPixelsInTexture = ParamsTex->GetSizeX()* ParamsTex->GetSizeY();  // Includes empty space at the end, not used by any chunks
    TArray<float> NewPixels = TArray<float>();
    float ColorDefault = 0;
    constexpr SIZE_T PIXEL_DATA_SIZE = sizeof(float);
    NewPixels.Init(ColorDefault, NumPixelsInTexture);


    for (int32 PixelIndex1DInChunk = 0; PixelIndex1DInChunk < NumPixelsInTexture; ++PixelIndex1DInChunk)
    {
        NewPixels[PixelIndex1DInChunk] = data[PixelIndex1DInChunk];
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
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AStaticMeshActor* NewElement = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnParams);
    NewElement->GetStaticMeshComponent()->SetStaticMesh(Mesh);
    NewElement->SetActorHiddenInGame(false);
    NewElement->SetMobility(EComponentMobility::Movable);
    NewElement->GetStaticMeshComponent()->bCastVolumetricTranslucentShadow = true;

    Model = MakeUnique<cppflow::model>(std::string(TCHAR_TO_UTF8(*ModelPath)));
    ModelSimple = MakeUnique<cppflow::model>(std::string(TCHAR_TO_UTF8(*ModelSimplePath)));

    for (int x = 0; x < 64; x++) {

        for (int y = 0; y < 64; y++) {
            HGradient.push_back(y);
            VGradient.push_back(x);
        }
    }

    WaterHeightTexture = WriteDataToTexture(WaterHeightTexture, WaterHeightData);

    WhiteWaterTexture = WriteDataToTexture(WhiteWaterTexture, WhiteWaterData);

    DynamicMaterial = NewElement->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0);
    DynamicMaterial->SetTextureParameterValue("HeightMap", WaterHeightTexture);
    DynamicMaterial->SetTextureParameterValue("WhitewaterMap", WhiteWaterTexture);
    DynamicMaterial->SetTextureParameterValue("PrevMap", PrevMap);
    DynamicMaterial->SetVectorParameterValue("Color", FColor::Red);

	return true;
}

// #pragma optimize( "", off )
void ATensorFlowNetwork::UpdateScene()
{

    for (int x = 0; x < 64 * 64; x++) {
        InputGradient[x] = (0.5 + (-std::cos(Rotation) * (HGradient[x] - 32) * Velocity / (8 * 64) - std::sin(Rotation) * (VGradient[x] - 32) * Velocity / (8 * 64)));
        if (NetworkId != 0) {
            DistanceFieldSmall[x] = pow( pow(HGradient[x] - 32, 2) + pow(VGradient[x] - 32, 2),0.5)/(1.44*32);
        }

        float RotationDelta = std::atan2(VGradient[x] - 32.5, HGradient[x] - 32) - Rotation;
        InputRotationCos[x] = 0.5 + 0.5 * std::cos(RotationDelta);
        InputRotationSin[x] = 0.5 + 0.5 * std::sin(RotationDelta);
    }

    std::vector<std::vector<float>*> inputs {
        &InputGradient,
        &InputRotationCos,
        &InputRotationSin
    };

    if (2 < DisplayMode && DisplayMode < 6) {
        std::vector<float> UpscaledInput;
        UpscaledInput.reserve(256 * 256);

        for (int idx = 0; idx < 64; idx++) {
            std::vector<float> Row;
            Row.reserve(256);
            for (int PixelId = 64 * idx; PixelId < 64 + 64 * idx; PixelId++) {
                float pixel = (*inputs[DisplayMode - 3])[PixelId];
                Row.push_back(pixel);
                Row.push_back(pixel);
                Row.push_back(pixel);
                Row.push_back(pixel);
            }
            UpscaledInput.insert(UpscaledInput.end(), Row.begin(), Row.end());
            UpscaledInput.insert(UpscaledInput.end(), Row.begin(), Row.end());
            UpscaledInput.insert(UpscaledInput.end(), Row.begin(), Row.end());
            UpscaledInput.insert(UpscaledInput.end(), Row.begin(), Row.end());
        }
        PrevMap = WriteDataToTexture(PrevMap, UpscaledInput);
        return;
    }


    float* DistanceField;
    switch (ShapeId)
    {
    case 0:
        DistanceField = DistanceField0.data();
        break;
    case 1:
        DistanceField = DistanceField1.data();
        break;
    case 2:
        DistanceField = DistanceField2.data();
        break;
    case 3:
        DistanceField = DistanceField3.data();
        break;
    case 4:
        DistanceField = DistanceField4.data();
        break;
    default:
        DistanceField = DistanceField0.data();
        break;
    }

    auto begin = std::chrono::high_resolution_clock::now();
    std::vector<cppflow::tensor> output; 
    if (NetworkId == 0)
    {

        std::vector<float> XPosData(64 * 64 * 3);
        for (int x = 0; x < 64 * 64; x++) {
            XPosData[x * 3] = InputGradient[x];
            XPosData[x * 3 + 1] = InputRotationCos[x];
            XPosData[x * 3 + 2] = InputRotationSin[x];
        }
        cppflow::tensor XPos(XPosData, { 1, 64, 64, 3 });

        std::vector<float> XNData(256 * 256 * 3);
        for (int x = 0; x < 256 * 256; x++) {
            XNData[x * 3] = WaterHeightData[x];
            XNData[x * 3 + 1] = WhiteWaterData[x];
            XNData[x * 3 + 2] = DistanceField[x];
        }
        cppflow::tensor XN(XNData, { 1, 256, 256, 3 });

        output = (*Model)({ {"serving_default_x_n:0", XN}, {"serving_default_x_pos:0", XPos} }, { "StatefulPartitionedCall:0" });
    }
    else {

        std::vector<float> XPosData(64 * 64 * 4);
        for (int x = 0; x < 64 * 64; x++) {
            XPosData[x * 4] = DistanceFieldSmall[x];
            XPosData[x * 4 + 1] = - InputGradient[x] / 2.0;
            XPosData[x * 4 + 2] = InputRotationCos[x];
            XPosData[x * 4 + 3] = InputRotationSin[x];
        }
        cppflow::tensor XPos(XPosData, { 1, 64, 64, 4 });

        std::vector<float> XNData(256 * 256);
        for (int X = 0; X < 256 * 256; X++) {
            XNData[X] = WaterHeightData[X];
        }
        cppflow::tensor XN(XNData, { 1, 256, 256, 1 });

        output = (*ModelSimple)({ {"serving_default_x_n:0", XN}, {"serving_default_x_pos:0", XPos} }, { "StatefulPartitionedCall:0" });
    }

    auto end = std::chrono::high_resolution_clock::now();     
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    UE_LOG(LogTemp, Warning, TEXT("Network calculation time: %d ms (1/%d of a second)"), ms, 1000 / ms);

    if (NetworkId == 0) {
        Result = output[0].get_data<double>();

        for (int x = 0; x < 256 * 256; x++) {
            if (DisplayMode == 1) {
                PreviewData[x] = Result[x * 2];
            }
            WaterHeightData[x] += Result[x * 2];
            WaterHeightData[x] /= 2;

            WhiteWaterData[x] = Result[x * 2 + 1];
        }
    }
    else {
        Result = output[0].get_data<double>();
        for (int x = 0; x < 256 * 256; x++) {
            if (DisplayMode == 1) {
                PreviewData[x] = Result[x];
            }
            WaterHeightData[x] = Result[x];
            WhiteWaterData[x] = 0.0;
        }
    }

    WhiteWaterTexture = WriteDataToTexture(WhiteWaterTexture, WhiteWaterData);
    PrevMap = WriteDataToTexture(PrevMap, PreviewData);
    WaterHeightTexture = WriteDataToTexture(WaterHeightTexture, WaterHeightData);
    DynamicMaterial->SetTextureParameterValue("PrevMap", PrevMap);
}
// #pragma optimize( "", off )
void ATensorFlowNetwork::ChangeDisplayMode(const int NewMode)
{
    switch (NewMode)
    {
        case 0:
        {
            DynamicMaterial->SetScalarParameterValue("HeightDisplay", 1.0);
            DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 1.0);
            DynamicMaterial->SetScalarParameterValue("previewDisplay", 0.0);
            break;
        }
        case 2:
        {
            DynamicMaterial->SetScalarParameterValue("HeightDisplay", 0.0);
            DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 1.0);
            DynamicMaterial->SetScalarParameterValue("previewDisplay", 0.0);
            break;
        }
        default:
        {
            DynamicMaterial->SetScalarParameterValue("HeightDisplay", 0.0);
            DynamicMaterial->SetScalarParameterValue("WhiteWaterDisplay", 0.0);
            DynamicMaterial->SetScalarParameterValue("previewDisplay", 1.0);
            break;
        }
    }
    DisplayMode = NewMode;
}

void ATensorFlowNetwork::ChangeShape(const int NewShapeId)
{
    ShapeId = NewShapeId;
}

// #pragma optimize( "", on ) 

