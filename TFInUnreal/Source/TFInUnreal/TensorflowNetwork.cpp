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


std::vector<float> ReadData(const FString path) {
    std::string path_str(TCHAR_TO_UTF8 (*path));
    std::fstream in(path_str);
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

    WaterHeightTexture = WriteDataToTexture(WaterHeightTexture, WaterHeight);

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

        std::vector<float> x_pos_data(64 * 64 * 3);
        for (int x = 0; x < 64 * 64; x++) {
            x_pos_data[x * 3] = InputGradient[x];
            x_pos_data[x * 3 + 1] = InputRotationCos[x];
            x_pos_data[x * 3 + 2] = InputRotationSin[x];
        }
        cppflow::tensor x_pos(x_pos_data, { 1, 64, 64, 3 });

        std::vector<float> x_n_data(256 * 256 * 3);
        for (int x = 0; x < 256 * 256; x++) {
            x_n_data[x * 3] = WaterHeight[x];
            x_n_data[x * 3 + 1] = WhiteWaterData[x];
            x_n_data[x * 3 + 2] = DistanceField[x];
        }
        cppflow::tensor x_n(x_n_data, { 1, 256, 256, 3 });

        output = (*Model)({ {"serving_default_x_n:0", x_n}, {"serving_default_x_pos:0", x_pos} }, { "StatefulPartitionedCall:0" });
    }
    else {

        std::vector<float> x_pos_data(64 * 64 * 4);
        for (int x = 0; x < 64 * 64; x++) {
            x_pos_data[x * 4] = DistanceFieldSmall[x];
            x_pos_data[x * 4 + 1] = - InputGradient[x] / 2.0;
            x_pos_data[x * 4 + 2] = InputRotationCos[x];
            x_pos_data[x * 4 + 3] = InputRotationSin[x];
        }
        cppflow::tensor x_pos(x_pos_data, { 1, 64, 64, 4 });

        std::vector<float> x_n_data(256 * 256);
        for (int x = 0; x < 256 * 256; x++) {
            x_n_data[x] = WaterHeight[x];
        }
        cppflow::tensor x_n(x_n_data, { 1, 256, 256, 1 });

        output = (*ModelSimple)({ {"serving_default_x_n:0", x_n}, {"serving_default_x_pos:0", x_pos} }, { "StatefulPartitionedCall:0" });
    }
    auto end = std::chrono::high_resolution_clock::now();
     
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    UE_LOG(LogTemp, Warning, TEXT("Network calculation time: %d ms (1/%d of a second)"), ms, 1000 / ms);

    if (NetworkId == 0) {
        ResultDouble = output[0].get_data<double>();

        for (int x = 0; x < 256 * 256; x++) {
            WaterHeight[x] += ResultDouble[x * 2];
            WaterHeight[x] /= 2;
            WhiteWaterData[x] = ResultDouble[x * 2 + 1];
        }
    }
    else {
        ResultDouble = output[0].get_data<double>();
        for (int x = 0; x < 256 * 256; x++) {
            WaterHeight[x] = ResultDouble[x];
            WhiteWaterData[x] = 0.0;
        }
    }

    WhiteWaterTexture = WriteDataToTexture(WhiteWaterTexture, WhiteWaterData);
    PrevMap = WriteDataToTexture(PrevMap, WaterHeight);
    WaterHeightTexture = WriteDataToTexture(WaterHeightTexture, WaterHeight);
    DynamicMaterial->SetTextureParameterValue("PrevMap", PrevMap);
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

void ATensorFlowNetwork::ChangeShape(const int NewShapeId)
{
    ShapeId = NewShapeId;
}

// #pragma optimize( "", on ) 

