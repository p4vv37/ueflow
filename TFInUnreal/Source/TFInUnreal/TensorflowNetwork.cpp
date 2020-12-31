// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

// Sets default values
ATensorFlowNetwork::ATensorFlowNetwork()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshFinder(TEXT("StaticMesh'/Game/Meshes/box.box'"));
    mMeshes.Emplace("box0", StaticMeshFinder.Object);
}


bool ATensorFlowNetwork::InitializeModel()
{
    if (!FPaths::DirectoryExists(ModelPath))
    {
        return false;
    }

    std::string stringPath = std::string(TCHAR_TO_UTF8(*ModelPath));
    cppflow::model model (stringPath);
    m_model = MakeUnique<cppflow::model>(std::string(TCHAR_TO_UTF8(*ModelPath)));

    const FString cgPath = ModelPath + "/setup.cfg";
    TArray < FString > config;
    FFileHelper::LoadANSITextFileToStrings(*cgPath, nullptr, config);

    FString separator = ";";
    FString left;
    FString right;

    TArray<float> initialData;

    for (FString line : config) {
        if (m_numberOfFrames < 0) {
            line.Split(separator, &left, &right, ESearchCase::CaseSensitive, ESearchDir::FromStart);
            m_numberOfFrames = FCString::Atof(*left);
            m_numberOfBlocks = FCString::Atof(*right);
            m_frames.Reserve((m_numberOfFrames + 1) * m_numberOfBlocks * 9);
            initialData.Reserve(m_numberOfBlocks * 9 + 3);
            continue;
        }
        line.Split(separator, &left, &right, ESearchCase::CaseSensitive, ESearchDir::FromStart);
        m_models.Add(&left);
        while (!right.IsEmpty()) {
            line = right;
            left = right = "";
            line.Split(separator, &left, &right, ESearchCase::CaseSensitive, ESearchDir::FromStart);
            initialData.Add(FCString::Atof(*left));
        }
    }

    // Fill n frames with initail data.
    for (int i = 0; i < m_numberOfFrames; ++i)
    {
        for (float element : initialData)
        {
            m_frames.Add(element);
        }

        // Add dummy force
        float sin, cos;
        utils::GetSinCos(forceAngle, sin, cos);
        m_frames.Add(sin);
        m_frames.Add(cos);
        m_frames.Add(0); // force power.
    }

     
#ifdef UE_BUILD_DEBUG
    check(m_frames.Num() == m_numberOfFrames * (m_numberOfBlocks * 9 + 3));
#endif // UE_BUILD_DEBUG


    // Preparation of scene
    float unitsScale{ 100.0f };
    for (int i = 0; i < m_numberOfBlocks; i++)
    {

        FTransform transform;
        FVector vec{ unitsScale * m_frames[9 * i] + 100,   unitsScale * m_frames[9 * i + 2], unitsScale * m_frames[9 * i + 1] };
        FVector vecRotation{ 
            utils::GetAngle(m_frames[9 * i + 3], m_frames[9 * i + 1 + 3]), 
            utils::GetAngle(m_frames[9 * i + 3 + 4], m_frames[9 * i + 5 + 3]),
            utils::GetAngle(m_frames[9 * i + 2 + 3], m_frames[9 * i + 3 + 3])
        };
        FRotator rotation;
        rotation.MakeFromEuler(vecRotation);
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Required_Fatal;
        SpawnParams.bHideFromSceneOutliner = 0;
        SpawnParams.Instigator = GetInstigator();
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AStaticMeshActor* NewElement = GetWorld()->SpawnActor<AStaticMeshActor>(vec, rotation, SpawnParams);
        NewElement->GetStaticMeshComponent()->SetStaticMesh(mMeshes["box0"]);
        NewElement->SetActorHiddenInGame(false);
        NewElement->SetMobility(EComponentMobility::Movable);

        if (!i)
        {
            NewElement->Tags.Add(FName("bottomElement"));
        }

        if (disablePhysics)
        {
            NewElement->DisableComponentsSimulatePhysics();
            NewElement->SetActorEnableCollision(false);
            NewElement->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }
        else
        {
            NewElement->GetStaticMeshComponent()->SetSimulatePhysics(true);
            NewElement->GetStaticMeshComponent()->RegisterComponent();
        }

        m_elementsList.Add(NewElement);
    }
	return true;
}

void ATensorFlowNetwork::ApplyForce()
{
    mForceFramesLeft = 3;
}

// #pragma optimize( "", off )
// Called every frame
void ATensorFlowNetwork::UpdateScene()
{
    if (GetWorld()->GetTimeSeconds() - mSeconds > 1.0f / 30.0f)
    {
        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        mSeconds = GetWorld()->GetTimeSeconds();
        std::vector<float> positions(m_elementsList.Num() * 3, 0);
        std::vector<float> orientations(m_elementsList.Num() * 3, 0);
        FVector loc{};
        FVector rot{};

        m_frames.PopNoCheck(m_elementsList.Num() * 9 + 3);

        int i = 0;
        for (AStaticMeshActor* elem : m_elementsList)
        {
            loc = elem->GetActorLocation();

            m_frames.Add(loc.X / 100.0f);
            m_frames.Add(loc.Y / 100.0f);
            m_frames.Add(loc.Z / 100.0f);
            rot = elem->GetActorRotation().Euler();

            float sin{ 0.0f }, cos{ 1.0f };
            utils::GetSinCos(rot.X, sin, cos);
            m_frames.Add(sin);
            m_frames.Add(cos);
            utils::GetSinCos(rot.Y, sin, cos);
            m_frames.Add(sin);
            m_frames.Add(cos);
            utils::GetSinCos(rot.Z, sin, cos);
            m_frames.Add(sin);
            m_frames.Add(cos);

            i++;
        }

        // Add force
        float sin, cos;
        utils::GetSinCos(forceAngle, sin, cos);
        m_frames.Add(sin);
        m_frames.Add(cos);
        m_frames.Add(100 * force * (mForceFramesLeft > 0)); // force power.
        //mForceFramesLeft -= 1;

        check(m_frames.Num() == m_numberOfFrames * (m_numberOfBlocks * 9 + 3));

        for (int frame = 0; frame < m_numberOfFrames; frame++) {
        
            for (int j = 0; j < m_numberOfBlocks; j++) {
                for (int k = 0; k < 6; k++) {
                    m_frames[frame * (m_numberOfBlocks * 9 + 3) + j * 9 + k] -= m_frames[j * 9 + k];
                }
            }
        }

        std::vector<float> buffer(m_frames.Num());
        TArrayView<float> view;
        view = m_frames.Compact();
        buffer.assign(view.GetData(), view.GetData() + view.Num());
        cppflow::tensor input(buffer, { 1, 3, 39 });
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        auto output = m_model->operator()(input);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto values = output.get_data<float>();

        i = 0;
        // m_network.Predict(positions.data(), orientations.data());
        FQuat rotQuat;
        for (AStaticMeshActor* elem : m_elementsList)
        {
            loc = elem->GetActorLocation();
            loc.X += values[i * 9] * 100;
            loc.Z += values[i * 9 + 1] * 100;
            loc.Y += values[i * 9 + 2] * 100;
            elem->SetActorLocation(loc);

            rot = elem->GetActorRotation().Euler();
            rot.X += 180 * utils::GetAngle(values[i * 9 + 3], values[i * 9 + 1 + 3]);
            rot.Z += 180 * utils::GetAngle(values[i * 9 + 2 + 3], values[i * 9 + 3 + 3]);
            rot.Y += 180 * utils::GetAngle(values[i * 9 + 4 + 3], values[i * 9 + 5 + 3]);
            rotQuat.MakeFromEuler(rot);
            elem->SetActorRotation(rotQuat);
            i++;
        }

#ifdef UE_BUILD_DEBUG
        check(m_frames.Num() == m_numberOfFrames * (m_numberOfBlocks * 9 + 3));
        FString log("!! all loop: ");
        std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
        auto timeAll = std::chrono::duration_cast<std::chrono::nanoseconds> (end1 - begin1).count();
        FString timeAllAsString = FString::FromInt(timeAll);
        log += timeAllAsString;
        log += "  ns, deep network:  ";
        auto timeNN = std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count();
        FString timeNNAsString = FString::FromInt(timeNN);
        log += timeNNAsString;
        log += " = ";
        int fps = 1000000000 / timeNN;
        FString fpsAsString = FString::FromInt(fps);
        log += fpsAsString;
        log += " max potential fps ";
        // std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Just to test how it affects FPS
        UE_LOG(LogTemp, Warning, TEXT("%s"), *log);
#endif // UE_BUILD_DEBUG

    }
}

#pragma optimize( "", on ) 

