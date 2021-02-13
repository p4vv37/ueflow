// Fill out your copyright notice in the Description page of Project Settings.


#include "TensorFlowNetwork.h"

#define MULTIPLIER 100.0f
#define SLOWDOWN 1.0f

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
            m_lastFrame.Reserve((m_numberOfFrames + 1) * m_numberOfBlocks * 9);
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
            m_lastFrame.Add(element);
        }
    }

     
#ifdef UE_BUILD_DEBUG
    check(m_lastFrame.Num() == m_numberOfFrames * m_numberOfBlocks * 9 );
#endif // UE_BUILD_DEBUG


    // Preparation of scene
    for (int i = 0; i < m_numberOfBlocks; i++)
    {

        FTransform transform;
        FVector vec{ MULTIPLIER * m_lastFrame[9 * i],   MULTIPLIER * m_lastFrame[9 * i + 2], MULTIPLIER * m_lastFrame[9 * i + 1] };
        FVector vecRotation{ 
            utils::GetAngle(m_lastFrame[9 * i + 3], m_lastFrame[9 * i + 1 + 3]), 
            utils::GetAngle(m_lastFrame[9 * i + 3 + 4], m_lastFrame[9 * i + 5 + 3]),
            utils::GetAngle(m_lastFrame[9 * i + 2 + 3], m_lastFrame[9 * i + 3 + 3])
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
    mForceFramesLeft = 1;
}

// #pragma optimize( "", off )
// Called every frame
void ATensorFlowNetwork::UpdateScene()
{
    if (mForceFramesLeft && GetWorld()->GetTimeSeconds() - mSeconds > 1.0f / (30.0f / SLOWDOWN))
    {
        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        mSeconds = GetWorld()->GetTimeSeconds();
        std::vector<float> positions(m_elementsList.Num() * 3, 0);
        std::vector<float> orientations(m_elementsList.Num() * 3, 0);
        FVector loc{};
        FVector rot{};

        int i = 0;
        TArray<float> thisFrame;
        thisFrame.Reserve(m_numberOfBlocks * 9);
        for (AStaticMeshActor* elem : m_elementsList)
        {
            loc = elem->GetActorLocation();

            thisFrame.Add(loc.X / MULTIPLIER);
            thisFrame.Add(loc.Z / MULTIPLIER);
            thisFrame.Add(loc.Y / MULTIPLIER);
            rot = elem->GetActorRotation().Euler();

            float sin{ 0.0f }, cos{ 1.0f };
            utils::GetSinCos(rot.X, sin, cos);
            thisFrame.Add(sin);
            thisFrame.Add(cos);
            utils::GetSinCos(rot.Y, sin, cos);
            thisFrame.Add(sin);
            thisFrame.Add(cos);
            utils::GetSinCos(rot.Z, sin, cos);
            thisFrame.Add(sin);
            thisFrame.Add(cos);

            i++;
        }

        // Add force
        TArray<float> x_force;
        x_force.Reserve(3);
        float sin, cos;
        utils::GetSinCos(forceAngle, sin, cos);
        x_force.Add(sin);
        x_force.Add(cos);
        x_force.Add( force ); // force power.

        //auto input_abs_0 = cppflow::fill({ 10, 4, 9 }, 1.0f);
        //auto input_abs = cppflow::fill({ 10, 4, 9 }, 1.0f);
        //auto input_force = cppflow::fill({ 10, 3, 1 }, 1.0f);

        cppflow::tensor input_abs_0(std::vector<float>(m_lastFrame.GetData(), m_lastFrame.GetData() + m_lastFrame.Num()), { 1, m_numberOfBlocks, 9 });

        //for (int j = 0; j < m_numberOfBlocks * 9; j++) {
        //    m_lastFrame[j] = thisFrame[j] - m_lastFrame[j];
        //}

        cppflow::tensor input_delta(std::vector<float>(m_lastFrame.GetData(), m_lastFrame.GetData() + m_lastFrame.Num()), { 1, m_numberOfBlocks, 6 });
        cppflow::tensor input_abs(std::vector<float>(thisFrame.GetData(), thisFrame.GetData() + thisFrame.Num()), { 1, m_numberOfBlocks, 9 });
        cppflow::tensor input_force(std::vector<float>(x_force.GetData(), x_force.GetData() + x_force.Num()), { 1, 3, 1 });
    

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        auto output = m_model->operator()({ {"serving_default_input_abs_0:0", input_abs_0}, {"serving_default_input_abs:0", input_abs}, {"serving_default_input_force:0", input_force} }, { "StatefulPartitionedCall:0" });
        // auto output = m_model->operator()({ {"serving_default_input_delta:0", input_delta}, {"serving_default_input_abs:0", input_abs}, {"serving_default_input_force:0", input_force} }, { "StatefulPartitionedCall:0" });
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        auto values = output[0].get_data<float>();

        i = 0;
        // m_network.Predict(positions.data(), orientations.data());
        FQuat rotQuat;

        //const FString out = "E:/tmp/rel.csv";
        //TArray < FString > out_values;
        //FFileHelper::LoadANSITextFileToStrings(*out, nullptr, out_values);

        for (AStaticMeshActor* elem : m_elementsList)
        {
            // loc = elem->GetActorLocation();
            // loc.X += 1000.0 * FCString::Atof(*out_values[row + 0 + 6 * i]);
            // loc.Z += 1000.0 * FCString::Atof(*out_values[row + 1 + 6 * i]);
            // loc.Y += 1000.0 * FCString::Atof(*out_values[row + 2 + 6 * i]);
            // elem->SetActorLocation(loc);

            // rot = elem->GetActorRotation().Euler();
            // rot.X += FCString::Atof(*out_values[row + 3 + 6 * i]);
            // rot.Z += FCString::Atof(*out_values[row + 4 + 6 * i]);
            // rot.Y += FCString::Atof(*out_values[row + 5 + 6 * i]);
            // rotQuat.MakeFromEuler(rot);
            // elem->SetActorRotation(rotQuat);

            loc = elem->GetActorLocation();
            loc.X += MULTIPLIER * values[ 0 + 6 * i];
            loc.Z += MULTIPLIER * values[ 1 + 6 * i];
            loc.Y += MULTIPLIER * values[ 2 + 6 * i];
            elem->SetActorLocation(loc);

            rot = elem->GetActorRotation().Euler();
            rot.X += values[ 3 + 6 * i];
            rot.Z += values[ 4 + 6 * i];
            rot.Y += values[ 5 + 6 * i];
            rotQuat.MakeFromEuler(rot);
            elem->SetActorRotation(rotQuat);
            /// <summary>
            //loc = elem->GetActorLocation();
            //loc.X += values[i * 9] * 100;
            //loc.Z += values[i * 9 + 1] * 100;
            //loc.Y += values[i * 9 + 2] * 100;
            //elem->SetActorLocation(loc);

            //rot = elem->GetActorRotation().Euler();
            //rot.X += 180 * utils::GetAngle(values[i * 9 + 3], values[i * 9 + 1 + 3]);
            //rot.Z += 180 * utils::GetAngle(values[i * 9 + 2 + 3], values[i * 9 + 3 + 3]);
            //rot.Y += 180 * utils::GetAngle(values[i * 9 + 4 + 3], values[i * 9 + 5 + 3]);
            //rotQuat.MakeFromEuler(rot);
            //elem->SetActorRotation(rotQuat);
            /// </summary>
            i++;
        }

        check(m_lastFrame.Num() == m_numberOfBlocks * 9);

        for (int j = 0; j < m_numberOfBlocks * 9; j++) {
            m_lastFrame[j] = thisFrame[j];
        }

#ifdef UE_BUILD_DEBUG
        check(m_lastFrame.Num() == m_numberOfFrames * (m_numberOfBlocks * 9));
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

